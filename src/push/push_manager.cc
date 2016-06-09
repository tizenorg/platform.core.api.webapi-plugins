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

#include "push/push_manager.h"
#include <unistd.h>
#include <pcrecpp.h>
#include <app_control.h>
#include <app_control_internal.h>
#include <app_manager.h>
#include <bundle.h>

#include "common/extension.h"
#include "common/logger.h"

namespace extension {
namespace push {

using common::PlatformResult;
using common::ErrorCode;

namespace {

ErrorCode ConvertPushError(int e) {
  ErrorCode error;

  switch(e) {
    case PUSH_SERVICE_ERROR_NONE:
      error = ErrorCode::NO_ERROR;
      break;

    case PUSH_SERVICE_ERROR_INVALID_PARAMETER:
      error = ErrorCode::INVALID_VALUES_ERR;
      break;

    case PUSH_SERVICE_ERROR_OUT_OF_MEMORY:
    case PUSH_SERVICE_ERROR_NOT_CONNECTED:
    case PUSH_SERVICE_ERROR_OPERATION_FAILED:
    default:
      error = ErrorCode::ABORT_ERR;
      break;
  }

  return error;
}

}  // namespace

PushManager::PushManager() :
    m_handle(NULL),
    m_listener(NULL),
    m_state(PUSH_SERVICE_STATE_UNREGISTERED),
    app_control_(nullptr),
    operation_(nullptr) {
    LoggerD("Enter");

    initAppId();
    InitAppControl();

    int ret = push_service_connect(m_pkgId.c_str(), onPushState, onPushNotify, NULL,
        &m_handle);
    if (ret != PUSH_SERVICE_ERROR_NONE) {
        LoggerE("Failed to connect to push (%d)", ret);
    }
}

PushManager::~PushManager() {
    LoggerD("Enter");

    if (m_handle) {
      push_service_disconnect(m_handle);
    }

    if (app_control_ && (APP_CONTROL_ERROR_NONE != app_control_destroy(app_control_))) {
      LoggerE("Failed to destroy app control");
    }

    if (operation_) {
      free(operation_);
    }
}

void PushManager::setListener(EventListener* listener) {
    LoggerD("Enter");
    m_listener = listener;
}

void PushManager::initAppId() {
    LoggerD("Enter");
    int pid = getpid();
    char *temp = NULL;
    int ret = app_manager_get_app_id(pid, &temp);
    if (ret != APP_MANAGER_ERROR_NONE || temp == NULL) {
        LoggerE("Failed to get appid (%d)", ret);
        return;
    }

    m_appId = temp;
    free(temp);
    temp = NULL;

    app_info_h info;
    ret = app_manager_get_app_info(m_appId.c_str(), &info);
    if (ret != APP_MANAGER_ERROR_NONE) {
        LoggerE("Failed to get app info (%d)", ret);
        return;
    }

    ret = app_info_get_package(info, &temp);
    if (ret == APP_MANAGER_ERROR_NONE && temp != NULL) {
        m_pkgId = temp;
        free(temp);
    } else {
        LoggerE("Failed to get pkg id (%d)", ret);
    }

    app_info_destroy(info);
}

void PushManager::InitAppControl() {
  ScopeLogger();

  const auto encoded_bundle = GetEncodedBundle();

  auto bundle = bundle_decode((bundle_raw*) (encoded_bundle.c_str()),
                              encoded_bundle.length());
  if (nullptr == bundle) {
    LoggerE("Failed to decode bundle");
    return;
  }

  app_control_h app_control = nullptr;
  int ret = app_control_create_event(bundle, &app_control);
  bundle_free(bundle);

  if (APP_CONTROL_ERROR_NONE != ret) {
    LoggerE("app_control_create_event() failed: %d (%s)", ret, get_error_message(ret));
  } else {
    app_control_ = app_control;
    ret = app_control_get_operation(app_control, &operation_);
    if (APP_CONTROL_ERROR_NONE != ret) {
      LoggerE("app_control_get_operation() failed: %d (%s)", ret, get_error_message(ret));
    }
  }
}

PushManager& PushManager::getInstance() {
  static PushManager instance;
  return instance;
}

PlatformResult PushManager::registerApplication(double callbackId) {
  LoggerD("Enter");

  double* pcallback = new double(callbackId);

  int ret = push_service_register(m_handle, onApplicationRegister, static_cast<void*>(pcallback));
  if (ret != PUSH_SERVICE_ERROR_NONE) {
    delete pcallback;

    return LogAndCreateResult(
        ConvertPushError(ret), "Failed to register", ("push_service_register failed (%d)", ret));
  }
  return common::PlatformResult(ErrorCode::NO_ERROR);
}

common::PlatformResult PushManager::unregisterApplication(double callbackId) {
    LoggerD("Enter");
    double* pcallbackId = new double(callbackId);
    if (m_state == PUSH_SERVICE_STATE_UNREGISTERED) {
        LoggerD("Already unregister, call unregister callback");
        if (!g_idle_add(onFakeDeregister, pcallbackId)) {
            delete pcallbackId;
            return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                "Unknown error", ("g_idle_add failed"));
        }
    } else {
        int ret = push_service_deregister(m_handle, onDeregister, pcallbackId);
        if (ret != PUSH_SERVICE_ERROR_NONE) {
            delete pcallbackId;
            if (ret == PUSH_SERVICE_ERROR_INVALID_PARAMETER) {
              LoggerE("[push_service_deregister] PUSH_SERVICE_ERROR_INVALID_PARAMETER");
            } else if (ret == PUSH_SERVICE_ERROR_OUT_OF_MEMORY) {
              LoggerE("[push_service_deregister] PUSH_SERVICE_ERROR_OUT_OF_MEMORY");
            } else if (ret == PUSH_SERVICE_ERROR_NOT_CONNECTED) {
              LoggerE("[push_service_deregister] PUSH_SERVICE_ERROR_NOT_CONNECTED");
            } else if (ret == PUSH_SERVICE_ERROR_OPERATION_FAILED) {
              LoggerE("[push_service_deregister] PUSH_SERVICE_ERROR_OPERATION_FAILED");
            }
            return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                "Unknown error",
                ("Failed to deregister: push_service_deregister failed (%d)", ret));
        }
    }
    return common::PlatformResult(ErrorCode::NO_ERROR);
}

common::PlatformResult PushManager::getRegistrationId(std::string& id) {
    LoggerD("Enter");
    char* temp = NULL;
    int ret = push_service_get_registration_id(m_handle, &temp);
    if (ret != PUSH_SERVICE_ERROR_NONE) {
        if (ret == PUSH_SERVICE_ERROR_INVALID_PARAMETER) {
          LoggerE("[push_service_get_registration_id]   PUSH_SERVICE_ERROR_INVALID_PARAMETER");
        } else if (ret == PUSH_SERVICE_ERROR_OUT_OF_MEMORY) {
          LoggerE("[push_service_get_registration_id]   PUSH_SERVICE_ERROR_OUT_OF_MEMORY");
        } else if (ret == PUSH_SERVICE_ERROR_NO_DATA) {
          LoggerE("[push_service_get_registration_id]   PUSH_SERVICE_ERROR_NO_DATA");
        }
        return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
            "Unknown error",
            ("Failed to get id: push_service_get_registration_id failed (%d)", ret));
    }
    id = temp;
    free(temp);
    return common::PlatformResult(ErrorCode::NO_ERROR);
}

common::PlatformResult PushManager::getUnreadNotifications() {
    LoggerD("Enter");
    int ret = push_service_request_unread_notification(m_handle);
    if (ret != PUSH_SERVICE_ERROR_NONE) {
        return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
            "Unknown error",
            ("Failed to send request: push_service_request_unread_notification failed (%d)", ret));
    }
    return common::PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult PushManager::getPushMessage(picojson::value* out) {
  LoggerD("Enter");

  push_service_notification_h handle = nullptr;
  int ret = push_service_app_control_to_notification(app_control_, operation_,
                                                     &handle);

  if (ret != PUSH_SERVICE_ERROR_NONE) {
    if (PUSH_SERVICE_ERROR_NO_DATA == ret || PUSH_SERVICE_ERROR_NOT_SUPPORTED == ret) {
      // application was not started by push service, return null
      *out = picojson::value{};
      return common::PlatformResult(ErrorCode::NO_ERROR);
    } else {
      return LogAndCreateResult(
              ConvertPushError(ret), "Failed to get message",
              ("push_service_app_control_to_notification failed: (%d)", ret));
    }
  }

  picojson::object notification;
  notificationToJson(handle, &notification);
  push_service_free_notification(handle);

  *out = picojson::value{notification};

  return common::PlatformResult(ErrorCode::NO_ERROR);
}

void PushManager::notificationToJson(push_service_notification_h noti, picojson::object* obj) {
  LoggerD("Enter");

  char* temp = nullptr;
  int ret = push_service_get_notification_data(noti, &temp);
  if (ret != PUSH_SERVICE_ERROR_NONE) {
    LoggerE("Failed to get appData");
    return;
  }
  (*obj)["appData"] = picojson::value(temp);
  free(temp);

  char* fullMessage = nullptr;
  ret = push_service_get_notification_message(noti, &fullMessage);
  if (ret != PUSH_SERVICE_ERROR_NONE) {
    LoggerE("Failed to get message");
    return;
  }
  (*obj)["message"] = picojson::value(fullMessage);

  // parse query string and find value for alertMessage
  pcrecpp::StringPiece input(fullMessage);
  pcrecpp::RE re("([^=]+)=([^&]*)&?");
  string key;
  string value;
  while (re.Consume(&input, &key, &value)) {
    if (key == "alertMessage") {
      (*obj)["alertMessage"] = picojson::value(key);
      break;
    }
  }
  free(fullMessage);

  long long int date = -1;
  ret = push_service_get_notification_time(noti, &date);
  if (ret != PUSH_SERVICE_ERROR_NONE) {
    LoggerE("Failed to get date");
    return;
  }
  (*obj)["date"] = picojson::value(static_cast<double>(date));

  ret = push_service_get_notification_sender(noti, &temp);
  if (ret != PUSH_SERVICE_ERROR_NONE) {
    LoggerE("Failed to get sender");
    return;
  }
  (*obj)["sender"] = picojson::value(temp);
  free(temp);

  ret = push_service_get_notification_session_info(noti, &temp);
  if (ret != PUSH_SERVICE_ERROR_NONE) {
    LoggerE("Failed to get session info");
    return;
  }
  std::string session_info = temp;
  (*obj)["sesionInfo"] = picojson::value(temp);
  free(temp);

  ret = push_service_get_notification_request_id(noti, &temp);
  if (ret != PUSH_SERVICE_ERROR_NONE) {
    LoggerE("Failed to get request id");
    return;
  }
  (*obj)["requestId"] = picojson::value(temp);
  free(temp);

  int type;
  ret = push_service_get_notification_type(noti, &type);
  if (ret != PUSH_SERVICE_ERROR_NONE) {
    LoggerE("Failed to get type");
    return;
  }
  (*obj)["type"] = picojson::value(static_cast<double>(type));
}

void PushManager::onPushState(push_service_state_e state, const char* err,
        void* user_data) {
    LoggerD("Enter %d, err: %s", state, err);
    getInstance().m_state = state;
}

void PushManager::onPushNotify(push_service_notification_h noti, void* user_data) {
    LoggerD("Enter");
    if (!getInstance().m_listener) {
        LoggerW("Listener not set, ignoring");
        return;
    }

    getInstance().m_listener->onPushNotify(noti);
}

void PushManager::onApplicationRegister(push_service_result_e result, const char* msg, void* user_data) {
  LoggerD("Enter");

  if (!getInstance().m_listener) {
    LoggerW("Listener not set, ignoring");
    return;
  }

  double* callbackId = static_cast<double*>(user_data);
  std::string id;
  PlatformResult res(ErrorCode::NO_ERROR);

  if (PUSH_SERVICE_RESULT_SUCCESS == result) {
    LoggerD("Success");
    char *temp = nullptr;
    int ret = push_service_get_registration_id(getInstance().m_handle, &temp);
    if (PUSH_SERVICE_ERROR_NONE == ret) {
      LoggerD("Registration id retrieved");
      id = temp;
      free(temp);
    } else {
      res = LogAndCreateResult(
          ErrorCode::UNKNOWN_ERR, "Failed to retrieve registration id",
          ("Failed to retrieve registration id: push_service_get_registration_id(%d)", ret));
    }
  } else {
    if (PUSH_SERVICE_RESULT_TIMEOUT == result) {
      LoggerE("PUSH_SERVICE_RESULT_TIMEOUT");
    } else if (PUSH_SERVICE_RESULT_SERVER_ERROR == result) {
      LoggerE("PUSH_SERVICE_RESULT_SERVER_ERROR");
    } else if (PUSH_SERVICE_RESULT_SYSTEM_ERROR == result) {
      LoggerE("PUSH_SERVICE_RESULT_SYSTEM_ERROR");
    }
    res = LogAndCreateResult(ErrorCode::UNKNOWN_ERR, msg == nullptr ? "Unknown error" : msg);
  }

  // onPushState is not always called when onPushRegister is successful
  getInstance().m_state = PUSH_SERVICE_STATE_REGISTERED;
  getInstance().m_listener->onPushRegister(*callbackId, res, id);
  delete callbackId;
}

gboolean PushManager::onFakeDeregister(gpointer user_data) {
    LoggerD("Enter");
    if (!getInstance().m_listener) {
        LoggerW("Listener not set, ignoring");
        return G_SOURCE_REMOVE;
    }
    double* callbackId = static_cast<double*>(user_data);
    getInstance().m_listener->onDeregister(*callbackId,
        PlatformResult(ErrorCode::NO_ERROR));
    delete callbackId;
    return G_SOURCE_REMOVE;
}

void PushManager::onDeregister(push_service_result_e result, const char* msg,
        void* user_data) {
    LoggerD("Enter");
    if (!getInstance().m_listener) {
        LoggerW("Listener not set, ignoring");
        return;
    }
    double* callbackId = static_cast<double*>(user_data);
    if (result == PUSH_SERVICE_RESULT_SUCCESS) {
        getInstance().m_listener->onDeregister(*callbackId,
            PlatformResult(ErrorCode::NO_ERROR));
    } else {
        getInstance().m_listener->onDeregister(*callbackId,
            LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                msg == NULL ? "Unknown error" : msg));
    }
    delete callbackId;
}


std::string PushManager::GetEncodedBundle() {
  LoggerD("Entered");

  std::string result;
  std::size_t size = 512;

  // make sure we read the whole variable, if the length of read variable is equal
  // to the size we were trying to obtain, the variable is likely to be longer
  do {
    size <<= 1;
    result = common::GetCurrentExtension()->GetRuntimeVariable("encoded_bundle", size);
  } while (strlen(result.c_str()) == size);

  return result;
}

}  // namespace push
}  // namespace extension

