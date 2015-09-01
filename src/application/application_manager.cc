/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "application_manager.h"

#include <type_traits>
#include <unistd.h>

#include <app_info.h>
#include <app_manager.h>
#include <app_manager_extension.h>
#include <aul.h>
#include <package_manager.h>
#include <pkgmgr-info.h>
#include <bundle.h>
#include <bundle_internal.h>

#include "application/application_instance.h"
#include "application/application_utils.h"
#include "common/current_application.h"
#include "common/logger.h"
#include "common/platform_result.h"
#include "common/task-queue.h"
#include "common/scope_exit.h"

using namespace common;
using namespace tools;

namespace extension {
namespace application {

namespace {
const std::string kTizenApisFileScheme = "file://";
const std::string kTizenApisAppSlash = "/";
const std::string kTizenApisAppShared = "shared";

const char* kStartKey = "start";
const char* kEndKey = "end";
const char* kAppidKey = "appid";
const char* kOkValue = "ok";
const char* kInstallEvent = "install";
const char* kUpdateEvent = "update";
const char* kUninstallEvent = "uninstall";

const std::string kAction = "action";
const std::string kCallbackId = "callbackId";
const std::string kOnInstalled = "oninstalled";
const std::string kOnUpdated = "onupdated";
const std::string kOnUninstalled = "onuninstalled";
const std::string kData = "data";

const std::map<std::string, std::string> event_map_ = {
  {SYSTEM_EVENT_BATTERY_CHARGER_STATUS, EVENT_KEY_BATTERY_CHARGER_STATUS},
  {SYSTEM_EVENT_BATTERY_LEVEL_STATUS, EVENT_KEY_BATTERY_LEVEL_STATUS},
  {SYSTEM_EVENT_USB_STATUS, EVENT_KEY_USB_STATUS},
  {SYSTEM_EVENT_EARJACK_STATUS, EVENT_KEY_EARJACK_STATUS},
  {SYSTEM_EVENT_DISPLAY_STATE, EVENT_KEY_DISPLAY_STATE},
  {SYSTEM_EVENT_LOW_MEMORY, EVENT_KEY_LOW_MEMORY},
  {SYSTEM_EVENT_WIFI_STATE, EVENT_KEY_WIFI_STATE},
  {SYSTEM_EVENT_BT_STATE, EVENT_KEY_BT_STATE},
  {SYSTEM_EVENT_LOCATION_ENABLE_STATE, EVENT_KEY_LOCATION_ENABLE_STATE},
  {SYSTEM_EVENT_GPS_ENABLE_STATE, EVENT_KEY_GPS_ENABLE_STATE},
  {SYSTEM_EVENT_NPS_ENABLE_STATE, EVENT_KEY_NPS_ENABLE_STATE},
  {SYSTEM_EVENT_INCOMMING_MSG, EVENT_KEY_MSG_TYPE},
  {SYSTEM_EVENT_TIME_ZONE, EVENT_KEY_TIME_ZONE},
  {SYSTEM_EVENT_HOUR_FORMAT, EVENT_KEY_HOUR_FORMAT},
  {SYSTEM_EVENT_LANGUAGE_SET, EVENT_KEY_LANGUAGE_SET},
  {SYSTEM_EVENT_REGION_FORMAT, EVENT_KEY_REGION_FORMAT},
  {SYSTEM_EVENT_SILENT_MODE, EVENT_KEY_SILENT_MODE},
  {SYSTEM_EVENT_VIBRATION_STATE, EVENT_KEY_VIBRATION_STATE},
  {SYSTEM_EVENT_SCREEN_AUTOROTATE_STATE, EVENT_KEY_SCREEN_AUTOROTATE_STATE},
  {SYSTEM_EVENT_MOBILE_DATA_STATE, EVENT_KEY_MOBILE_DATA_STATE},
  {SYSTEM_EVENT_DATA_ROAMING_STATE, EVENT_KEY_DATA_ROAMING_STATE},
  {SYSTEM_EVENT_FONT_SET, EVENT_KEY_FONT_SET}
};
}

ApplicationManager::ApplicationManager(ApplicationInstance& instance) :
  pkgmgr_client_handle_(nullptr),
  pkgmgrinfo_client_handle_(nullptr),
  instance_(instance) {
    LoggerD("Enter");
}

ApplicationManager::~ApplicationManager() {
  LoggerD("Enter");
  StopAppInfoEventListener();
}

void ApplicationManager::GetCurrentApplication(const std::string& app_id,
                                                         picojson::object* out) {
  LoggerD("Entered");

  // obtain handle to application info
  pkgmgrinfo_appinfo_h handle;
  int ret = pkgmgrinfo_appinfo_get_appinfo(app_id.c_str(), &handle);
  if (PMINFO_R_OK != ret) {
    LoggerE("Failed to get app info.");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to get app info."), out);
    return;
  }

  picojson::value app_info = picojson::value(picojson::object());
  picojson::object& app_info_obj = app_info.get<picojson::object>();

  ApplicationUtils::CreateApplicationInformation(handle, &app_info_obj);
  pkgmgrinfo_appinfo_destroy_appinfo(handle);

  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();

  result_obj.insert(std::make_pair(
      "contextId", picojson::value(std::to_string(CurrentApplication::GetInstance().GetProcessId()))));
  result_obj.insert(std::make_pair("appInfo", app_info));

  ReportSuccess(result, *out);
}

class TerminateHandler {
 public:
  TerminateHandler(int callback_id, ApplicationInstance* app_instance) :
    callback_handle_(callback_id),
    pid_(-1),
    timeout_id_(0),
    app_instance_(app_instance) {
  }

  void set_pid(pid_t pid) {
    pid_ = pid;
  }

  pid_t pid() const {
    return pid_;
  }

  void Invoke(const std::shared_ptr<picojson::value>& response) {
    LoggerD("Entered");

    if (timeout_id_ > 0) {
      // cancel terminate callback
      g_source_remove(timeout_id_);
      timeout_id_ = 0;
    }

    ApplicationInstance* app_instance = this->app_instance_;
    int callback_id = this->callback_handle_;
    TaskQueue::GetInstance().Async<picojson::value>([callback_id, app_instance](
        const std::shared_ptr<picojson::value>& response) {
      picojson::object& obj = response->get<picojson::object>();
      obj.insert(std::make_pair(kCallbackId, picojson::value(static_cast<double>(callback_id))));
      Instance::PostMessage(app_instance, response->serialize().c_str());
    }, response);
  }

  void LaunchCheckTerminate() {
    LoggerD("Entered");
    timeout_id_ = g_timeout_add(3000, CheckTerminate, this);
    LoggerD("END");
  }

 private:
  static gboolean CheckTerminate(gpointer user_data) {
    LoggerD("Entered");
    TerminateHandler* that = static_cast<TerminateHandler*>(user_data);
    LoggerD("PID: %d", that->pid_);

    // we're canceling the callback by returning false, no need for Invoke() to do that again
    that->timeout_id_ = 0;

    char* app_id = nullptr;
    std::shared_ptr<picojson::value> response{new picojson::value(picojson::object())};

    LoggerD("checking if application is still alive");
    if (app_manager_get_app_id(that->pid_, &app_id) == APP_MANAGER_ERROR_NONE) {
        LoggerD("application is alive - failure");
        free(app_id);
        // context is still alive, report error
        ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to kill application."),
                    &response->get<picojson::object>());
    } else {
        LoggerD("application is dead - success");
        ReportSuccess(response->get<picojson::object>());
    }

    that->Invoke(response);
    delete that;

    return false;
  }

  int callback_handle_;
  pid_t pid_;
  guint timeout_id_;
  ApplicationInstance* app_instance_;
};

#define CHECK_RESULT(result, response, handler) \
  if (result.IsError()) { \
    ReportError(result, &response->get<picojson::object>()); \
    handler->Invoke(response); \
    delete handler; \
    return; \
  }

void ApplicationManager::AsyncResponse(PlatformResult& result,
                                       std::shared_ptr<picojson::value>* response) {

  LoggerD("Enter");
  ReportError(result, &(*response)->get<picojson::object>());

  TaskQueue::GetInstance().Async<picojson::value>([this](
      const std::shared_ptr<picojson::value>& response) {
        Instance::PostMessage(&this->instance_, response->serialize().c_str());
      }, *response);
}

void ApplicationManager::Kill(const picojson::value& args) {
  LoggerD("Entered");

  PlatformResult result = PlatformResult(ErrorCode::NO_ERROR);

  int callback_id = -1;
  const auto& callback = args.get(kCallbackId);
  if (callback.is<double>()) {
    callback_id = static_cast<int>(callback.get<double>());
  }

  const auto& context = args.get("contextId");
  if (!context.is<std::string>()) {
    LoggerE("Invalid parameter passed.");
    result = PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid parameter passed.");
  }

  const std::string& context_id = context.get<std::string>();

  if (context_id.empty() && result.IsSuccess()) {
    LoggerE("Context ID is empty.");
    result = PlatformResult(ErrorCode::NOT_FOUND_ERR, "Context ID is empty.");
  }

  std::shared_ptr<picojson::value> response(new picojson::value(picojson::object()));
  picojson::object& obj = response->get<picojson::object>();
  obj.insert(std::make_pair(kCallbackId, picojson::value(static_cast<double>(callback_id))));

  if (result.IsError()) {
    LoggerE("Failed args.get");
    AsyncResponse(result, &response);
    return;
  }

  auto kill = [this, callback_id, context_id]() -> void {
    LoggerD("Entered Kill async");

    std::shared_ptr<picojson::value> response =
        std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));

    TerminateHandler* handler = new TerminateHandler(callback_id, &this->instance_);
    PlatformResult result = PlatformResult(ErrorCode::NO_ERROR);

    pid_t pid = -1;
    try {
      pid = std::stoi(context_id);
    } catch (...) {
      LoggerE("Failed to convert string to int");
      result = PlatformResult(ErrorCode::NOT_FOUND_ERR, "Failed to convert string to int.");
      CHECK_RESULT(result, response, handler)
    }

    if (pid <= 0) {
      LoggerE("Context ID cannot be negative value");
      result = PlatformResult(ErrorCode::NOT_FOUND_ERR, "Context ID cannot be negative value.");
      CHECK_RESULT(result, response, handler)
    }

    // if kill request comes for current context, throw InvalidValuesException
    if (CurrentApplication::GetInstance().GetProcessId() == pid) {
      LoggerE("Cannot kill current application.");
      result = PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Cannot kill current application.");
      CHECK_RESULT(result, response, handler)
    }

    LoggerD("Kill async, pid: %d", pid);

    char* app_id = nullptr;
    int ret = app_manager_get_app_id(pid, &app_id);
    // automatically release the memory
    std::unique_ptr<char, void(*)(void*)> app_id_ptr(app_id, &std::free);

    if (APP_MANAGER_ERROR_NONE != ret) {
      LoggerE("Failed to get application ID, error: %d", ret);
      result = PlatformResult(ErrorCode::NOT_FOUND_ERR, "Failed to get application ID.");
      CHECK_RESULT(result, response, handler)
    }

    LoggerD("Kill async, app ID: %s", app_id);

    // acquire application context
    app_context_h app_context = nullptr;

    ret = app_manager_get_app_context(app_id, &app_context);
    std::unique_ptr<std::remove_pointer<app_context_h>::type, int(*)(app_context_h)>
    app_context_ptr(app_context, &app_context_destroy); // automatically release the memory

    if (APP_MANAGER_ERROR_NONE != ret) {
      LoggerE("Failed to get application context handle");
      result = PlatformResult(ErrorCode::NOT_FOUND_ERR, "Failed to get application ID.");
      CHECK_RESULT(result, response, handler)
    }

    auto terminate_callback = [](app_context_h app_context,
        app_context_event_e event,
        void* user_data) {
      LoggerD("terminate_callback: %d", event);

      if (APP_CONTEXT_EVENT_TERMINATED != event) {
        LoggerD("ignoring event");
        return;
      }

      int pid = 0;
      int ret = app_context_get_pid(app_context, &pid);

      if (APP_MANAGER_ERROR_NONE != ret) {
        LoggerE("Failed to get pid of terminated app (%d)", ret);
        return;
      }

      TerminateHandler* handler = static_cast<TerminateHandler*>(user_data);

      LoggerD("Expected PID: %d, got: %d", handler->pid(), pid);

      if (handler->pid() == pid) {
        std::shared_ptr<picojson::value> response =
            std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
        ReportSuccess(response->get<picojson::object>());
        handler->Invoke(response);
        delete handler;
      }
    };

    LoggerD("Kill async, setting callback");
    handler->set_pid(pid);
    ret = app_manager_set_app_context_event_cb(terminate_callback, handler);

    if (APP_MANAGER_ERROR_NONE != ret) {
      LoggerE("Error while registering app context event (%d)", ret);
      result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to register termination callback.");
      CHECK_RESULT(result, response, handler)
    }

    // due to platform issue, sometimes termination event is not reported to callback
    // registered with app_manager_set_app_context_event_cb()
    // this is a workaround, it should be removed when issue is solved
    handler->LaunchCheckTerminate();

    LoggerD("Kill async, KILL!!!!!!!!!");

    // terminate application
    ret = app_manager_terminate_app(app_context);

    if (APP_MANAGER_ERROR_NONE != ret) {
      LoggerE("Failed to terminate application.");
      result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to terminate application.");
      CHECK_RESULT(result, response, handler)
    }

    LoggerD("Kill async, end, waiting for notification");
  };

  TaskQueue::GetInstance().Queue(kill);
}

void ApplicationManager::Launch(const picojson::value& args) {
  LoggerD("Entered");

  int callback_id = -1;
  const auto& callback = args.get(kCallbackId);
  if (callback.is<double>()) {
    callback_id = static_cast<int>(callback.get<double>());
  }

  std::shared_ptr<picojson::value> response(new picojson::value(picojson::object()));
  picojson::object& obj = response->get<picojson::object>();
  obj.insert(std::make_pair(kCallbackId, picojson::value(static_cast<double>(callback_id))));

  const auto& app_id = args.get("id");
  if (!app_id.is<std::string>()) {
    LoggerE("Invalid parameter passed.");
    PlatformResult ret = PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid parameter passed.");
    AsyncResponse(ret, &response);
    return;
  }
  const std::string& id = app_id.get<std::string>();

  auto launch = [id](const std::shared_ptr<picojson::value>& response) -> void {
    PlatformResult result = PlatformResult(ErrorCode::NO_ERROR);
    const char* app_id = id.c_str();
    const int retry_count = 3;

    int retry = 0;
    int ret = 0;

    while (retry < retry_count) {
      ret = aul_open_app(app_id);

      if (ret >= 0) {
        break;
      }

      // delay 300ms for each retry
      struct timespec sleep_time = { 0, 300L * 1000L * 1000L };
      nanosleep(&sleep_time, nullptr);
      ++retry;

      LoggerD("Retry launch request: %d", retry);
    }

    if (ret < 0) {
      result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown error has occurred.");

      switch (ret) {
        case AUL_R_EINVAL:
        case AUL_R_ERROR:
        case AUL_R_ENOAPP:
          LoggerE("aul_open_app returns Not Found error");
          result = PlatformResult(ErrorCode::NOT_FOUND_ERR, "Launchpad returns not found error.");
          break;

        case AUL_R_ECOMM:
          LoggerE("aul_open_app returns internal IPC error");
          result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Internal IPC error has occurred.");
          break;
      }

      ReportError(result, &response->get<picojson::object>());
    } else {
      LoggerD("Launch request success");
      ReportSuccess(response->get<picojson::object>());
    }
  };

  auto launch_response = [this](const std::shared_ptr<picojson::value>& response) -> void {
    Instance::PostMessage(&this->instance_, response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<picojson::value>(launch, launch_response, response);
}

void ApplicationManager::LaunchAppControl(const picojson::value& args) {
  LoggerD("Entered");

  int callback_id = -1;
  const auto& callback = args.get(kCallbackId);
  if (callback.is<double>()) {
    callback_id = static_cast<int>(callback.get<double>());
  }

  std::shared_ptr<picojson::value> response(new picojson::value(picojson::object()));
  picojson::object& response_obj = response->get<picojson::object>();
  response_obj.insert(
      std::make_pair(kCallbackId, picojson::value(static_cast<double>(callback_id))));

  PlatformResult result = PlatformResult(ErrorCode::NO_ERROR);
  const auto& control = args.get("appControl");
  if (!control.is<picojson::object>()) {
    LoggerE("Invalid parameter passed.");
    result = PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid parameter passed.");
    AsyncResponse(result, &response);
    return;
  }
  const picojson::object& app_control_obj = control.get<picojson::object>();

  std::string launch_mode_str;
  const auto& launch_mode = control.get("launchMode");
  if (launch_mode.is<std::string>()) {
    launch_mode_str = launch_mode.get<std::string>();
  }

  app_control_h app_control = nullptr;
  result = ApplicationUtils::ApplicationControlToService(app_control_obj, &app_control);
  std::shared_ptr<std::remove_pointer<app_control_h>::type>
  app_control_ptr(app_control, &app_control_destroy); // automatically release the memory

  if (result.IsError()) {
    LoggerE("Application control to service failed.");
    AsyncResponse(result, &response);
    return;
  }

  std::string app_id;
  const auto& id = args.get("id");
  if (id.is<std::string>()) {
    app_id = id.get<std::string>();
  }

  std::string reply_callback;
  const auto& reply = args.get("replyCallback");
  if (reply.is<std::string>()) {
    reply_callback = reply.get<std::string>();
  }

  auto launch = [this, app_control_ptr, app_id, launch_mode_str, reply_callback](
      const std::shared_ptr<picojson::value>& response) -> void {
    LoggerD("Entered");

    if (!app_id.empty()) {
      LoggerD("app_id: %s", app_id.c_str());

      int ret = app_control_set_app_id(app_control_ptr.get(), app_id.c_str());

      if (APP_CONTROL_ERROR_NONE != ret) {
        LoggerE("Invalid parameter passed.");
        ReportError(PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid parameter passed."),
                    &response->get<picojson::object>());
        return;
      }
    }

    if (!launch_mode_str.empty()) {
      app_control_launch_mode_e launch_mode;

      if ("SINGLE" == launch_mode_str) {
        launch_mode = APP_CONTROL_LAUNCH_MODE_SINGLE;
      } else if ("GROUP" == launch_mode_str) {
        launch_mode = APP_CONTROL_LAUNCH_MODE_GROUP;
      } else {
        LoggerE("Invalid parameter passed.");
        ReportError(PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid parameter passed."),
                    &response->get<picojson::object>());
        return;
      }

      int ret = app_control_set_launch_mode(app_control_ptr.get(), launch_mode);
      if (APP_CONTROL_ERROR_NONE != ret) {
        LoggerE("Setting launch mode failed.");
        ReportError(PlatformResult(ErrorCode::NOT_FOUND_ERR, "Setting launch mode failed."),
                    &response->get<picojson::object>());
        return;
      }
    }

    app_control_reply_cb callback = nullptr;
    struct ReplayCallbackData {
      ApplicationInstance* app_instance;
      std::string reply_callback;
    };

    ReplayCallbackData* user_data = nullptr;

    if (!reply_callback.empty()) {
      user_data = new ReplayCallbackData();
      user_data->app_instance = &this->instance_;
      user_data->reply_callback = reply_callback;

      callback = [](app_control_h request,
          app_control_h reply, app_control_result_e result, void* user_data) {
        LoggerD("send_launch_request callback");

        picojson::value return_value = picojson::value(picojson::object());
        picojson::object& return_value_obj = return_value.get<picojson::object>();
        ReplayCallbackData* reply_callback = static_cast<ReplayCallbackData*>(user_data);

        if (APP_CONTROL_RESULT_SUCCEEDED == result) {
          const std::string data = "data";
          return_value_obj.insert(std::make_pair(data, picojson::value(picojson::array())));
          if (!ApplicationUtils::ServiceToApplicationControlDataArray(
              reply, &return_value_obj.find(data)->second.get<picojson::array>())) {
            return_value_obj.erase(data);
          }
          ReportSuccess(return_value_obj);
        } else {
          ReportError(return_value_obj);
        }

        return_value_obj.insert(
            std::make_pair("listenerId", picojson::value(reply_callback->reply_callback)));
        Instance::PostMessage(reply_callback->app_instance, return_value.serialize().c_str());
        delete reply_callback;
      };
    }

    const int retry_count = 3;

    int retry = 0;
    int ret = 0;

    while (retry < retry_count) {
      ret = app_control_send_launch_request(app_control_ptr.get(), callback, user_data);

      if (APP_CONTROL_ERROR_NONE == ret) {
        break;
      }

      // delay 300ms for each retry
      struct timespec sleep_time = { 0, 300L * 1000L * 1000L };
      nanosleep(&sleep_time, nullptr);
      ++retry;

      LoggerD("Retry launch request: %d", retry);
    }

    if (APP_CONTROL_ERROR_NONE != ret) {
      delete user_data;

      switch (ret) {
        case APP_CONTROL_ERROR_INVALID_PARAMETER:
          LoggerE("app_control_send_launch_request returns APP_CONTROL_ERROR_INVALID_PARAMETER");
          ReportError(PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid parameter returned."),
                      &response->get<picojson::object>());
          return;
        case APP_CONTROL_ERROR_OUT_OF_MEMORY:
          LoggerE("app_control_send_launch_request returns APP_CONTROL_ERROR_OUT_OF_MEMORY");
          ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Out of memory."),
                      &response->get<picojson::object>());
          return;
        case APP_CONTROL_ERROR_LAUNCH_REJECTED:
        case APP_CONTROL_ERROR_APP_NOT_FOUND:
          LoggerE("app_control_send_launch_request returns APP_CONTROL_ERROR_APP_NOT_FOUND");
          ReportError(PlatformResult(ErrorCode::NOT_FOUND_ERR, "No matched application found."),
                      &response->get<picojson::object>());
          return;
        default:
          LoggerE("app_control_send_launch_request returns UNKNOWN ERROR!!!");
          ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown error."),
                      &response->get<picojson::object>());
          return;
      }
    }

    ReportSuccess(response->get<picojson::object>());
  };

  auto launch_response = [this](const std::shared_ptr<picojson::value>& response) -> void {
    Instance::PostMessage(&this->instance_, response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<picojson::value>(launch, launch_response, response);
}

void ApplicationManager::FindAppControl(const picojson::value& args) {
  LoggerD("Entered");

  int callback_id = -1;
  const auto& callback = args.get(kCallbackId);
  if (callback.is<double>()) {
    callback_id = static_cast<int>(callback.get<double>());
  }

  std::shared_ptr<picojson::value> response(new picojson::value(picojson::object()));
  picojson::object& response_obj = response->get<picojson::object>();
  response_obj.insert(
      std::make_pair(kCallbackId, picojson::value(static_cast<double>(callback_id))));

  PlatformResult result = PlatformResult(ErrorCode::NO_ERROR);
  const auto& control = args.get("appControl");
  if (!control.is<picojson::object>()) {
    LoggerE("Invalid parameter passed.");
    result = PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid parameter passed.");
    AsyncResponse(result, &response);
    return;
  }

  const picojson::object& app_control_obj = control.get<picojson::object>();

  app_control_h app_control = nullptr;
  result = ApplicationUtils::ApplicationControlToService(app_control_obj, &app_control);
  std::shared_ptr<std::remove_pointer<app_control_h>::type>
  app_control_ptr(app_control, &app_control_destroy); // automatically release the memory

  if (result.IsError()) {
    LoggerE("Application control to service failed.");
    AsyncResponse(result, &response);
    return;
  }

  auto find = [app_control_ptr](const std::shared_ptr<picojson::value>& response) -> void {
    auto app_control_matched = [](app_control_h app_control, const char* appid, void* user_data) -> bool {
      if (nullptr == appid) {
        LoggerD("appid is NULL");
        return false;
      }

      pkgmgrinfo_appinfo_h handle;
      int ret = pkgmgrinfo_appinfo_get_appinfo(appid, &handle);
      if (PMINFO_R_OK != ret) {
        LoggerE("Failed to get appInfo");
      } else {
        picojson::array* array = static_cast<picojson::array*>(user_data);
        array->push_back(picojson::value(picojson::object()));

        ApplicationUtils::CreateApplicationInformation(handle, &array->back().get<picojson::object>());
        pkgmgrinfo_appinfo_destroy_appinfo(handle);
      }

      return true;
    };

    picojson::object& response_obj = response->get<picojson::object>();
    auto it_result = response_obj.find("result");
    picojson::object& result_obj = it_result->second.get<picojson::object>();
    auto array = result_obj.insert(
        std::make_pair("informationArray", picojson::value(picojson::array())));

    int ret = app_control_foreach_app_matched(
        app_control_ptr.get(), app_control_matched, &array.first->second.get<picojson::array>());

    if (APP_CONTROL_ERROR_NONE != ret) {
      LoggerE("app_control_foreach_app_matched error: %d", ret);

      ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR,"Unknown error"), &response_obj);
      // remove copied ApplicationControl from result
      response_obj.erase(it_result);
    } else {
      ReportSuccess(response_obj);
    }
  };

  auto find_response = [this](const std::shared_ptr<picojson::value>& response) -> void {
    Instance::PostMessage(&this->instance_, response->serialize().c_str());
  };

  // prepare result object, we need to do that here, as input parameter is passed to result callback
  auto ret = response_obj.insert(std::make_pair("result", picojson::value(picojson::object())));
  // reinsert application control
  ret.first->second.get<picojson::object>().insert(std::make_pair("appControl", args.get("appControl")));

  TaskQueue::GetInstance().Queue<picojson::value>(find, find_response, response);
}

void ApplicationManager::GetAppsContext(const picojson::value& args) {
  LoggerD("Entered");

  int callback_id = -1;
  const auto& callback = args.get(kCallbackId);
  if (callback.is<double>()) {
    callback_id = static_cast<int>(callback.get<double>());
  }

  auto get_apps_context = [](const std::shared_ptr<picojson::value>& response) -> void {
    picojson::object& response_obj = response->get<picojson::object>();
    picojson::value result = picojson::value(picojson::object());
    picojson::object& result_obj = result.get<picojson::object>();
    picojson::array& array =
        result_obj.insert(std::make_pair("contexts", picojson::value(
            picojson::array()))).first->second.get<picojson::array>();

    auto app_context_cb = [](app_context_h app_context, void* user_data) -> bool {
      if (nullptr == user_data) {
        return false;
      }

      picojson::array* array = static_cast<picojson::array*>(user_data);
      array->push_back(picojson::value(picojson::object()));

      if (!ApplicationUtils::CreateApplicationContext(
          app_context, &array->back().get<picojson::object>())) {
        array->pop_back();
        return false;
      }

      return true;
    };

    int ret = app_manager_foreach_app_context(app_context_cb, &array);

    if (APP_MANAGER_ERROR_NONE != ret) {
      LoggerE("app_manager_foreach_app_context error");
      ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown error."), &response_obj);
    } else {
      ReportSuccess(result, response_obj);
    }
  };

  auto get_apps_context_response = [this, callback_id](
      const std::shared_ptr<picojson::value>& response) -> void {
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair(kCallbackId, picojson::value(static_cast<double>(callback_id))));
    Instance::PostMessage(&this->instance_, response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<picojson::value>(
      get_apps_context,
      get_apps_context_response,
      std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));
}

void ApplicationManager::GetAppContext(const picojson::value& args, picojson::object* out) {
  LoggerD("Entered");
  pid_t pid = 0;

  const auto& context_id = args.get("contextId");
  if (context_id.is<std::string>()) {
    try {
      pid = std::stoi(context_id.get<std::string>());
    } catch(...) {
      LoggerE("Failed to convert context id.");
      ReportError(PlatformResult(ErrorCode::NOT_FOUND_ERR, "Failed to convert context id."), out);
      return;
    }
  } else {
    pid = CurrentApplication::GetInstance().GetProcessId();
  }

  char* app_id = nullptr;

  int ret = app_manager_get_app_id(pid, &app_id);
  // automatically release the memory
  std::unique_ptr<char, void(*)(void*)> app_id_ptr(app_id, &std::free);

  if (APP_MANAGER_ERROR_NONE != ret || nullptr == app_id) {
    switch(ret) {
      case APP_MANAGER_ERROR_NO_SUCH_APP:
        LoggerE("app_manager_get_app_id returned: APP_MANAGER_ERROR_NO_SUCH_APP");
        ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "No such application exist."), out);
        return;

      case APP_MANAGER_ERROR_INVALID_PARAMETER:
        LoggerE("app_manager_get_app_id returned: APP_MANAGER_ERROR_INVALID_PARAMETER");
        ReportError(PlatformResult(ErrorCode::NOT_FOUND_ERR, "Application not found."), out);
        return;

      default:
        LoggerE("app_manager_get_app_id returned: %d", ret);
        ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown error."), out);
        return;
    }
  }

  picojson::value result = picojson::value(picojson::object());
  ApplicationUtils::CreateApplicationContext(pid, app_id, &result.get<picojson::object>());

  ReportSuccess(result, *out);
}

void ApplicationManager::GetAppsInfo(const picojson::value& args) {
  LoggerD("Entered");

  int callback_id = -1;
  const auto& callback = args.get(kCallbackId);
  if (callback.is<double>()) {
    callback_id = static_cast<int>(callback.get<double>());
  }

  auto get_apps_info = [](const std::shared_ptr<picojson::value>& response) -> void {
    picojson::object& response_obj = response->get<picojson::object>();
    picojson::value result = picojson::value(picojson::object());
    picojson::object& result_obj = result.get<picojson::object>();
    picojson::array& array =
        result_obj.insert(std::make_pair("informationArray", picojson::value(
            picojson::array()))).first->second.get<picojson::array>();

    auto app_info_cb = [](pkgmgrinfo_appinfo_h handle, void* user_data) -> int {
      if (nullptr == user_data) {
        return -1;
      }

      picojson::array* array = static_cast<picojson::array*>(user_data);
      array->push_back(picojson::value(picojson::object()));

      ApplicationUtils::CreateApplicationInformation(handle, &array->back().get<picojson::object>());

      return 0;
    };

    int ret = pkgmgrinfo_appinfo_get_installed_list(app_info_cb, &array);

    if (APP_MANAGER_ERROR_NONE != ret) {
      LoggerE("pkgmgrinfo_appinfo_get_installed_list error");
      ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown error."), &response_obj);
    } else {
      ReportSuccess(result, response_obj);
    }
  };

  auto get_apps_info_response = [this, callback_id](
      const std::shared_ptr<picojson::value>& response) -> void {
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair(kCallbackId, picojson::value(static_cast<double>(callback_id))));
    Instance::PostMessage(&this->instance_, response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<picojson::value>(
      get_apps_info,
      get_apps_info_response,
      std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));
}

void ApplicationManager::GetAppInfo(const std::string& app_id, picojson::object* out) {
  LoggerD("Entered");

  pkgmgrinfo_appinfo_h handle = nullptr;

  if (PMINFO_R_OK != pkgmgrinfo_appinfo_get_appinfo(app_id.c_str(), &handle)) {
    LoggerE("Failed to get app info");
    ReportError(PlatformResult(ErrorCode::NOT_FOUND_ERR, "Failed to get app info."), out);
    return;
  }

  picojson::value result = picojson::value(picojson::object());
  ApplicationUtils::CreateApplicationInformation(handle, &result.get<picojson::object>());
  pkgmgrinfo_appinfo_destroy_appinfo(handle);

  ReportSuccess(result, *out);
}

char* ApplicationManager::GetPackageId(const std::string& app_id) {
  LoggerD("Entered");
  app_info_h handle;
  char* pkg_id = nullptr;

  int ret = app_manager_get_app_info(app_id.c_str(), &handle);
  if (APP_MANAGER_ERROR_NONE != ret) {
    LoggerE("Failed to get app info.");
    return nullptr;
  }

  ret = app_info_get_package(handle, &pkg_id);
  if (APP_MANAGER_ERROR_NONE != ret) {
    LoggerE("Failed to get package id.");
    pkg_id = nullptr;
  }

  ret = app_info_destroy(handle);
  if (APP_MANAGER_ERROR_NONE != ret) {
    LoggerE("Failed to destroy app info.");
  }

  return pkg_id;
}

void ApplicationManager::GetAppCerts(const std::string& app_id, picojson::object* out) {
  LoggerD("Entered");

  char* package_id = nullptr;

  package_id = GetPackageId(app_id);
  // automatically release the memory
  std::unique_ptr<char, void(*)(void*)> package_id_ptr(package_id, &std::free);

  if (!package_id) {
    LoggerE("Failed to get package.");
    ReportError(PlatformResult(ErrorCode::NOT_FOUND_ERR, "Failed to get package."), out);
    return;
  }

  package_info_h pkg_info = nullptr;
  int ret = package_info_create(package_id, &pkg_info);

  std::unique_ptr<std::remove_pointer<package_info_h>::type, int(*)(package_info_h)>
  pkg_info_ptr(pkg_info, &package_info_destroy); // automatically release the memory

  if (PACKAGE_MANAGER_ERROR_NONE != ret) {
    LoggerE("Failed to get package info.");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to get package info."), out);
    return;
  }

  auto cert_info_cb = [](package_info_h handle, package_cert_type_e cert_type,
      const char* cert_value, void* user_data) -> bool {
    const char* cert_name = nullptr;

    switch(cert_type) {
      case PACKAGE_INFO_AUTHOR_ROOT_CERT:
        cert_name = "AUTHOR_ROOT";
        break;
      case PACKAGE_INFO_AUTHOR_INTERMEDIATE_CERT:
        cert_name = "AUTHOR_INTERMEDIATE";
        break;
      case PACKAGE_INFO_AUTHOR_SIGNER_CERT:
        cert_name = "AUTHOR_SIGNER";
        break;
      case PACKAGE_INFO_DISTRIBUTOR_ROOT_CERT:
        cert_name = "DISTRIBUTOR_ROOT";
        break;
      case PACKAGE_INFO_DISTRIBUTOR_INTERMEDIATE_CERT:
        cert_name = "DISTRIBUTOR_INTERMEDIATE";
        break;
      case PACKAGE_INFO_DISTRIBUTOR_SIGNER_CERT:
        cert_name = "DISTRIBUTOR_SIGNER";
        break;
      case PACKAGE_INFO_DISTRIBUTOR2_ROOT_CERT:
        cert_name = "DISTRIBUTOR2_ROOT";
        break;
      case PACKAGE_INFO_DISTRIBUTOR2_INTERMEDIATE_CERT:
        cert_name = "DISTRIBUTOR2_INTERMEDIATE";
        break;
      case PACKAGE_INFO_DISTRIBUTOR2_SIGNER_CERT:
        cert_name = "DISTRIBUTOR2_SIGNER";
        break;
      default:
        LoggerD("Unknown certificate type: %d", cert_type);
        break;
    }

    picojson::array* array = static_cast<picojson::array*>(user_data);
    array->push_back(picojson::value(picojson::object()));

    ApplicationUtils::CreateApplicationCertificate(
        cert_name, cert_value, &array->back().get<picojson::object>());

    return true;
  };

  picojson::value result = picojson::value(picojson::array());

  ret = package_info_foreach_cert_info(pkg_info, cert_info_cb, &result.get<picojson::array>());

  if ((PACKAGE_MANAGER_ERROR_NONE != ret) && (PACKAGE_MANAGER_ERROR_IO_ERROR != ret)) {
    LoggerE("Failed to get certificates info.");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to get certificates info."), out);
    return;
  }

  ReportSuccess(result, *out);
}

void ApplicationManager::GetAppSharedUri(const std::string& app_id, picojson::object* out) {
  LoggerD("Entered");

  char* package_id = nullptr;

  package_id = GetPackageId(app_id);
  // automatically release the memory
  std::unique_ptr<char, void(*)(void*)> package_id_ptr(package_id, &std::free);

  if (!package_id) {
    LoggerE("Failed to get package.");
    ReportError(PlatformResult(ErrorCode::NOT_FOUND_ERR, "Failed to get package."), out);
    return;
  }

  pkgmgrinfo_pkginfo_h pkg_info = nullptr;

  int ret = pkgmgrinfo_pkginfo_get_pkginfo(package_id, &pkg_info);
  std::unique_ptr<std::remove_pointer<pkgmgrinfo_pkginfo_h>::type, int(*)(pkgmgrinfo_pkginfo_h)>
  pkg_info_ptr(pkg_info, &pkgmgrinfo_pkginfo_destroy_pkginfo); // automatically release the memory

  if (PMINFO_R_OK != ret) {
    LoggerE("Failed to get package info.");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to get package info."), out);
    return;
  }

  char* root_path = nullptr;
  ret = pkgmgrinfo_pkginfo_get_root_path(pkg_info, &root_path);

  if (PMINFO_R_OK != ret || nullptr == root_path) {
    LoggerE("Failed to get root path.");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to get root path."), out);
    return;
  }

  picojson::value result = picojson::value(kTizenApisFileScheme +
                                           root_path +
                                           kTizenApisAppSlash +
                                           kTizenApisAppShared +
                                           kTizenApisAppSlash);
  ReportSuccess(result, *out);
}

void ApplicationManager::GetAppMetaData(const std::string& app_id, picojson::object* out) {
  LoggerD("Entered");

  pkgmgrinfo_appinfo_h handle = nullptr;

  int ret = pkgmgrinfo_appinfo_get_appinfo(app_id.c_str(), &handle);
  std::unique_ptr<std::remove_pointer<pkgmgrinfo_appinfo_h>::type, int(*)(pkgmgrinfo_appinfo_h)>
  pkg_info_ptr(handle, &pkgmgrinfo_appinfo_destroy_appinfo); // automatically release the memory

  if (PMINFO_R_OK != ret) {
    LoggerE("Failed to get app info.");
    ReportError(PlatformResult(ErrorCode::NOT_FOUND_ERR, "Failed to get app info."), out);
    return;
  }

  auto meta_data_cb = [](const char* meta_key, const char* meta_value, void* user_data) -> int {
    if (nullptr == meta_key || nullptr == meta_value) {
      LoggerE("meta_key or meta_value is null");
      return 0;
    }

    picojson::array* array = static_cast<picojson::array*>(user_data);
    array->push_back(picojson::value(picojson::object()));

    ApplicationUtils::CreateApplicationMetaData(meta_key, meta_value,
                                                &array->back().get<picojson::object>());
    return 0;
  };

  picojson::value result = picojson::value(picojson::array());
  ret = pkgmgrinfo_appinfo_foreach_metadata(handle, meta_data_cb, &result.get<picojson::array>());

  if (PMINFO_R_OK != ret) {
    LoggerE("Failed to get metadata.");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to get metadata."), out);
    return;
  }

  ReportSuccess(result, *out);
}

class ApplicationListChangedBroker {
 public:
    ApplicationListChangedBroker(){}
  enum class Event {
    kInstalled,
    kUpdated,
    kUninstalled,
  };

  static int ClientStatusListener(int id, const char* type, const char* package, const char* key,
                                  const char* val, const void* msg, void* data) {
    LoggerD("Entered");
    ApplicationListChangedBroker* that = static_cast<ApplicationListChangedBroker*>(data);

    if (0 == strcasecmp(key, kStartKey)) {
      that->HandleStart(val, package);
    } else if (0 == strcasecmp(key, kEndKey) && 0 == strcasecmp(val, kOkValue)) {
      that->HandleEnd(package);
    } else {
      LoggerD("Ignored key: %s", key);
    }

    return 0;
  }

  static int AppUninstallListener(int id, const char* type, const char* package, const char* key,
                                  const char* val, const void* msg, void* data) {
    LoggerD("Entered");

    ApplicationListChangedBroker* that = static_cast<ApplicationListChangedBroker*>(data);

    if (0 == strcasecmp(key, kStartKey)) {
      that->HandleUninstallStart();
    } else if (0 == strcasecmp(key, kAppidKey)) {
      that->AddUninstalledAppId(val);
    } else if (0 == strcasecmp(key, kEndKey)) {
      that->HandleUninstallEnd();
    } else {
      LoggerD("Ignored key: %s", key);
    }

    return 0;
  }

  void AddApplicationInstance(ApplicationInstance* app_instance) {
    LoggerD("Entered");
      app_instance_list_.push_back(app_instance);
  }

  void RemoveApplicationInstance(ApplicationInstance* app_instance) {
    LoggerD("Entered");
    for (auto it = app_instance_list_.begin(); it != app_instance_list_.end(); it++) {
      if (*it == app_instance) {
       app_instance_list_.erase(it);
       return;
      }
    }
  }
 private:
  void HandleStart(const char* event_type, const char* package) {
    LoggerD("Entered");
    app_list_.clear();
    set_event_type(event_type);
  }

  void HandleEnd(const char* package) {
    LoggerD("Entered");

    if (Event::kUninstalled == event_type_) {
      return;
    }

    GetApplicationIdsFromPackage(package);

    for (auto& app_id : app_list_) {
      picojson::value value = picojson::value(picojson::object());
      picojson::object& data_obj = value.get<picojson::object>();

      switch (event_type_) {
        case Event::kInstalled:
          data_obj.insert(std::make_pair(kAction, picojson::value(kOnInstalled)));
          break;

        case Event::kUpdated:
          data_obj.insert(std::make_pair(kAction, picojson::value(kOnUpdated)));
          break;
      }

      switch (event_type_) {
        case Event::kInstalled:
        case Event::kUpdated:
        {
          pkgmgrinfo_appinfo_h handle = nullptr;
          if (PMINFO_R_OK != pkgmgrinfo_appinfo_get_appinfo(app_id.c_str(), &handle)) {
            LoggerE("Failed to get application information handle.");
            continue;
          }
          auto info = data_obj.insert(std::make_pair(kData, picojson::value(picojson::object())));
          ApplicationUtils::CreateApplicationInformation(
              handle, &info.first->second.get<picojson::object>());
          pkgmgrinfo_appinfo_destroy_appinfo(handle);
        }
        break;
      }

      data_obj["listenerId"] = picojson::value("ApplicationEventListener");

      for (auto instance : app_instance_list_) {
        Instance::PostMessage(instance, value.serialize().c_str());
      }
    }
  }

  void GetApplicationIdsFromPackage(const char* package) {
    LoggerD("Entered");
    package_info_h package_info = nullptr;

    int ret = package_info_create(package, &package_info);
    if (PACKAGE_MANAGER_ERROR_NONE != ret) {
      LoggerE("Failed to create package info");
      return;
    }

    ret = package_info_foreach_app_from_package(package_info,
                                                PACKAGE_INFO_ALLAPP,
                                                ApplicationIdCallback,
                                                this);
    if (PACKAGE_MANAGER_ERROR_NONE != ret) {
      LoggerE("Failed to get application IDs");
    }

    ret = package_info_destroy(package_info);
    if (PACKAGE_MANAGER_ERROR_NONE != ret) {
      LoggerE("Failed to destroy package info");
    }
  }

  void set_event_type(const char* type) {
    LoggerD("Entered");
    if (0 == strcasecmp(type, kInstallEvent)) {
      event_type_ = Event::kInstalled;
    } else if (0 == strcasecmp(type, kUpdateEvent)) {
      event_type_ = Event::kUpdated;
    } else if (0 == strcasecmp(type, kUninstallEvent)) {
      event_type_ = Event::kUninstalled;
    }
  }

  static bool ApplicationIdCallback(package_info_app_component_type_e comp_type,
                                    const char* app_id, void* user_data) {
    LoggerD("Entered");
    if (nullptr != app_id) {
      static_cast<ApplicationListChangedBroker*>(user_data)->app_list_.push_back(app_id);
    }
    return true;
  }

  void HandleUninstallStart() {
    LoggerD("Entered");
    app_list_.clear();
    set_event_type(kUninstallEvent);
  }

  void AddUninstalledAppId(const char* app_id) {
    LoggerD("Entered");
    if (nullptr != app_id) {
      app_list_.push_back(app_id);
    }
  }

  void HandleUninstallEnd() {
    LoggerD("Entered");
    for (auto& app_id : app_list_) {
      picojson::value value = picojson::value(picojson::object());
      picojson::object& data_obj = value.get<picojson::object>();

      data_obj.insert(std::make_pair(kAction, picojson::value(kOnUninstalled)));
      data_obj.insert(std::make_pair(kData, picojson::value(app_id)));

      data_obj["listenerId"] = picojson::value("ApplicationEventListener");

      for (auto instance : app_instance_list_) {
        Instance::PostMessage(instance, value.serialize().c_str());
      }
    }
  }

  Event event_type_;
  std::vector<std::string> app_list_;
  std::vector<ApplicationInstance*> app_instance_list_;
};

static ApplicationListChangedBroker g_application_list_changed_broker;

void ApplicationManager::StartAppInfoEventListener(picojson::object* out) {
  LoggerD("Entered");

  if (nullptr == pkgmgr_client_handle_ || nullptr == pkgmgrinfo_client_handle_) {
    if (nullptr == pkgmgr_client_handle_) {
      pkgmgr_client_handle_ = pkgmgr_client_new(PC_LISTENING);
    }

    if (nullptr == pkgmgrinfo_client_handle_) {
      pkgmgrinfo_client_handle_ = pkgmgrinfo_client_new(PMINFO_LISTENING);
    }

    if (nullptr == pkgmgr_client_handle_ || nullptr == pkgmgrinfo_client_handle_) {
      LoggerE("Failed to register listener.");
      if (nullptr != pkgmgr_client_handle_) {
        pkgmgr_client_free(pkgmgr_client_handle_);
        pkgmgr_client_handle_ = nullptr;
      }
      else if (nullptr != pkgmgrinfo_client_handle_) {
        pkgmgrinfo_client_free(pkgmgrinfo_client_handle_);
        pkgmgrinfo_client_handle_ = nullptr;
      }
      ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to register listener."), out);
      return;
    }

    g_application_list_changed_broker.AddApplicationInstance(&instance_);
    pkgmgr_client_listen_status(pkgmgr_client_handle_,
                                ApplicationListChangedBroker::ClientStatusListener,
                                &g_application_list_changed_broker);

    pkgmgrinfo_client_set_status_type(pkgmgrinfo_client_handle_,
                                      PACKAGE_MANAGER_STATUS_TYPE_UNINSTALL);
    pkgmgrinfo_client_listen_status(pkgmgrinfo_client_handle_,
                                ApplicationListChangedBroker::AppUninstallListener,
                                &g_application_list_changed_broker);
  } else {
    LoggerD("Broker callback is already registered.");
  }

  ReportSuccess(*out);
}

void ApplicationManager::StopAppInfoEventListener() {
  LoggerD("Entered");

  if (nullptr != pkgmgr_client_handle_ || nullptr != pkgmgrinfo_client_handle_) {
    if (nullptr != pkgmgr_client_handle_) {
      pkgmgr_client_free(pkgmgr_client_handle_);
      pkgmgr_client_handle_ = nullptr;
    }
    if (nullptr != pkgmgrinfo_client_handle_) {
      pkgmgrinfo_client_free(pkgmgrinfo_client_handle_);
      pkgmgrinfo_client_handle_ = nullptr;
    }
    g_application_list_changed_broker.RemoveApplicationInstance(&instance_);
  } else {
    LoggerD("Broker callback is already unregistered.");
  }
}

void ApplicationManager::GetApplicationInformationSize(const picojson::value& args,
                                                       picojson::object* out) {
  LoggerD("Entered");

  const auto& package_id = args.get("packageId");
  if (!package_id.is<std::string>()) {
    LoggerE("Invalid parameter passed.");
    ReportError(PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid parameter passed."), out);
    return;
  }

  const std::string& package_id_str = package_id.get<std::string>();

  // get installed size from package server (to solve smack issue)
  pkgmgr_client* pc = pkgmgr_client_new(PC_REQUEST);
  int size = -1;

  if (nullptr == pc) {
    LoggerE("Failed to create pkgmgr client");
  } else {
    size = pkgmgr_client_request_service(PM_REQUEST_GET_SIZE, PM_GET_TOTAL_SIZE, pc, NULL,
                                         package_id_str.c_str(), NULL, NULL, NULL);

    if (size < 0) {
      LoggerE("Failed to get installed size");
    }

    pkgmgr_client_free(pc);
  }

  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();
  result_obj.insert(std::make_pair("size", picojson::value(static_cast<double>(size))));

  ReportSuccess(result, *out);
}

void ApplicationManager::BroadcastEventHelper(
    const picojson::value& args,
    picojson::object& out, bool trusted) {

  LoggerD("Entered");

  int ret;
  std::string event_str = args.get("name").get<std::string>();
  const char* event_name = event_str.c_str();

  bundle* data = bundle_create();
  SCOPE_EXIT {
    bundle_free(data);
  };

  ret = bundle_add(data, "data", args.get("data").serialize().c_str());

  if (ret != EVENT_ERROR_NONE) {
    LoggerE("bundle_add failed, error");
    ReportError(out);
    return;
  }

  if (trusted) {
    ret = event_publish_trusted_app_event(event_name, data);
  } else {
    ret = event_publish_app_event(event_name, data);
  }

  if (ret == EVENT_ERROR_NONE) {
    ReportSuccess(out);
  } else {
    LoggerE("event_publish_app_event failed, error");
    ReportError(out);
  }
}

void ApplicationManager::OnEvent(const char* event_name,
                                 bundle* event_data,
                                 void* user_data) {
  LoggerD("Entered");
  LOGGER(DEBUG) << event_name;

  ApplicationManager* manager = static_cast<ApplicationManager*>(user_data);

  if (!manager->event_callback_) {
    LOGGER(DEBUG) << "No event listener registered, skipping.";
    return;
  }

  picojson::value event = picojson::value(picojson::object());
  picojson::object& event_o = event.get<picojson::object>();

  int ret;
  char* val = nullptr;

  if (event_map_.count(event_name)) { // system event
    const std::string& key = event_map_.at(event_name);
    std::string state = "true";
    if (key != "") {
      ret = bundle_get_str(event_data, key.c_str(), &val);
      if (EVENT_ERROR_NONE != ret) {
        LOGGER(ERROR) << "failed to read bundle data, error: " << ret;
        return;
      }

      state = std::string(val);
    }

    LOGGER(DEBUG) << "state is: " << state;
    event_o["value"] = picojson::value(state);

  } else { // user event
    ret = bundle_get_str(event_data, "data", &val);
    if (EVENT_ERROR_NONE != ret) {
      LOGGER(ERROR) << "failed to read bundle data, error: " << ret;
      return;
    }

    picojson::value data;
    std::string err;
    picojson::parse(data, val, val + strlen(val), &err);
    if (!err.empty()) {
      LOGGER(ERROR) << "Failed to parse bundle data: " << err;
      return;
    }

    event_o["data"] = data;
  }

  LOGGER(DEBUG) << "event_name is: " << event_name;
  event_o["name"] = picojson::value(event_name);

  manager->event_callback_(&event);
}

PlatformResult ApplicationManager::StartEventListener(const std::string& event_name,
                                                      const JsonCallback& callback) {
  LoggerD("Entered");

  int ret;
  event_handler_h event_handler;

  ret = event_add_event_handler(event_name.c_str(), OnEvent, this, &event_handler);
  LOGGER(DEBUG) << "event_add_event_handler() result: " << ret;
  if (EVENT_ERROR_PERMISSION_DENIED == ret) {
    LOGGER(ERROR) << "event_add_event_handler failed, error: " << ret;
    return PlatformResult(ErrorCode::SECURITY_ERR, "The privilege is required");
  } else if (EVENT_ERROR_NONE != ret) {
    LOGGER(ERROR) << "event_add_event_handler failed, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Error setting event listener");
  }

  event_handler_map_[event_name] = event_handler;

  event_callback_ = callback;
  LOGGER(DEBUG) << "event_add_event_handler success";
  return PlatformResult(ErrorCode::NO_ERROR);
}

void ApplicationManager::StopEventListener(const std::string& event_name) {
  LoggerD("Entered");

  int ret;
  event_handler_h event_handler;

  if (event_handler_map_.find(event_name) != event_handler_map_.end()) {
    event_handler = event_handler_map_[event_name];

    ret = event_remove_event_handler(event_handler);
    if (EVENT_ERROR_NONE != ret) {
      LOGGER(ERROR) << "event_remove_event_handler failed, error: " << ret;
      return;
    }

    event_handler_map_.erase(event_name);
  }
}

} // namespace application
} // namespace extension
