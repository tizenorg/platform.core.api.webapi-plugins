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
 
#include "requested_application_control.h"

#include <app_manager.h>
#include <app_control_internal.h>
#include <bundle.h>
#include <bundle_internal.h>

#include "common/logger.h"
#include "application/application_utils.h"

using namespace common;
using namespace tools;

namespace extension {
namespace application {

PlatformResult RequestedApplicationControl::set_bundle(const std::string& encoded_bundle) {
  LoggerD("Entered");

  if (encoded_bundle != bundle_) {
    bundle_ = encoded_bundle;

    bundle* bundle = bundle_decode((bundle_raw*)(encoded_bundle.c_str()),
                                   encoded_bundle.length());
    app_control_h app_control = nullptr;
    int ret = app_control_create_event(bundle, &app_control);
    bundle_free(bundle);

    if (APP_CONTROL_ERROR_NONE != ret) {
      LoggerE("Failed to create app_control");
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to create app_control.");
    }

    set_app_control(app_control);
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

void RequestedApplicationControl::set_app_control(app_control_h app_control) {
  LoggerD("Entered");

  app_control_.reset(app_control, app_control_destroy);

  char* tmp_str = nullptr;
  int ret = app_control_get_caller(app_control, &tmp_str);

  if ((APP_CONTROL_ERROR_NONE == ret) && (nullptr != tmp_str)) {
    caller_app_id_ = tmp_str;
  } else {
    LoggerW("Failed to get callerAppId because of platform error");
    LoggerW("Please ignore if the application is launched in debug mode");
  }

  free(tmp_str);
}

void RequestedApplicationControl::ToJson(picojson::object* out) {
  LoggerD("Entered");

  if (app_control_) {
    out->insert(std::make_pair("callerAppId", picojson::value(caller_app_id_)));
    auto appControl = out->insert(std::make_pair(
        "appControl", picojson::value(picojson::object())));
    ApplicationUtils::ServiceToApplicationControl(
        app_control_.get(), &appControl.first->second.get<picojson::object>());
  }
}

void RequestedApplicationControl::ReplyResult(const picojson::value& args, picojson::object* out) {
  LoggerD("Entered");

  const auto& data_arr = args.get("data");
  if (!data_arr.is<picojson::array>()) {
    LoggerE("Invalid parameter passed.");
    ReportError(PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid parameter passed."), out);
    return;
  }

  // read input data
  const picojson::array& data = data_arr.get<picojson::array>();

  const std::string& encoded_bundle = GetEncodedBundle();

  PlatformResult result = set_bundle(encoded_bundle);
  if (result.IsError()) {
    LoggerE("Failed set_bundle()");
    ReportError(result, out);
    return;
  }

  // code to check caller liveness
  result = VerifyCallerPresence();
  if (result.IsError()) {
    LoggerE("Failed VerifyCallerPresence()");
    ReportError(result, out);
    return;
  }

  // create reply
  app_control_h reply;
  app_control_create(&reply);
  std::unique_ptr<std::remove_pointer<app_control_h>::type, int(*)(app_control_h)>
  reply_ptr(reply, &app_control_destroy); // automatically release the memory

  if (!data.empty()) {
    for (auto iter = data.begin(); iter != data.end(); ++iter) {
      result = ApplicationUtils::ApplicationControlDataToServiceExtraData(
          iter->get<picojson::object>(), reply);
      if (result.IsError()) {
        LoggerE("Failed ApplicationControlDataToServiceExtraData()");
        ReportError(result, out);
        return;
      }
    }
  } else {
    LoggerD("appControlDataArray is empty");
  }

  // send reply
  if (APP_CONTROL_ERROR_NONE !=
      app_control_reply_to_launch_request(reply, app_control_.get(), APP_CONTROL_RESULT_SUCCEEDED)) {
    LoggerE("Cannot find caller.");
    ReportError(PlatformResult(ErrorCode::NOT_FOUND_ERR, "Cannot find caller."), out);
    return;
  }

  ReportSuccess(*out);
}

void RequestedApplicationControl::ReplyFailure(picojson::object* out) {
  LoggerD("Entered");

  // read input data
  const std::string& encoded_bundle = GetEncodedBundle();

  PlatformResult result = set_bundle(encoded_bundle);
  if (result.IsError()) {
    LoggerE("Failed set_bundle()");
    ReportError(result, out);
    return;
  }

  // code to check caller liveness
  result = VerifyCallerPresence();
  if (result.IsError()) {
    LoggerE("Failed VerifyCallerPresence()");
    ReportError(result, out);
    return;
  }

  // create reply
  app_control_h reply;
  app_control_create(&reply);
  std::unique_ptr<std::remove_pointer<app_control_h>::type, int(*)(app_control_h)>
  reply_ptr(reply, &app_control_destroy); // automatically release the memory

  // send reply
  int ret = app_control_reply_to_launch_request(reply, app_control_.get(), APP_CONTROL_RESULT_FAILED);
  if (APP_CONTROL_ERROR_NONE != ret) {
    LoggerE("Cannot find caller.");
    ReportError(PlatformResult(ErrorCode::NOT_FOUND_ERR, "Cannot find caller."), out);
    return;
  }

  ReportSuccess(*out);
}

std::string RequestedApplicationControl::GetEncodedBundle() {
  LoggerD("Entered");

  std::string result;
  std::size_t size = 512;

  // make sure we read whole variable, if the length of read variable is equal
  // to the size we were trying to obtain, the variable is likely to be longer
  do {
    size <<= 1;
    result = GetCurrentExtension()->GetRuntimeVariable("encoded_bundle", size);
  } while (strlen(result.c_str()) == size);

  return result;
}

PlatformResult RequestedApplicationControl::VerifyCallerPresence() {
  LoggerD("Entered");

  if (caller_app_id_.empty()) {
    LoggerE("caller_app_id_ is empty. This means caller is dead.");
    return PlatformResult(ErrorCode::NOT_FOUND_ERR, "Cannot find caller.");
  } else {
    bool running = false;

    int ret = app_manager_is_running(caller_app_id_.c_str(), &running);

    if ((APP_MANAGER_ERROR_NONE != ret) || !running) {
      LoggerE("Caller is not running");
      return PlatformResult(ErrorCode::NOT_FOUND_ERR, "Cannot find caller.");
    }

    return PlatformResult(ErrorCode::NO_ERROR);
  }
}

}  // namespace application
}  // namespace extension
