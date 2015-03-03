// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "websetting/websetting_instance.h"

#include <string>
#include <memory>

#include "common/logger.h"
#include "common/picojson.h"
#include "common/task-queue.h"
#include "common/scope_exit.h"

namespace {
// The privileges that required in Websetting API
const std::string kPrivilegeWebsetting =
    "http://tizen.org/privilege/websetting";

const char kWrtServiceName[] = "wrt-service";
}  // namespace

namespace extension {
namespace websetting {

using namespace common;

typedef picojson::value JsonValue;
typedef picojson::object JsonObject;
typedef picojson::array JsonArray;
typedef std::string JsonString;

WebSettingInstance::WebSettingInstance(WebSettingExtension* extension)
    : extension_(extension) {
  using std::placeholders::_1;
  using std::placeholders::_2;

  #define REGISTER_ASYNC(c, x) \
      RegisterSyncHandler(c, std::bind(&WebSettingInstance::x, this, _1, _2));

  REGISTER_ASYNC("WebSettingManager_setUserAgentString",
                 WebSettingManagerSetUserAgentString);
  REGISTER_ASYNC("WebSettingManager_removeAllCookies",
                 WebSettingManagerRemoveAllCookies);

  #undef REGISTER_ASYNC
}

WebSettingInstance::~WebSettingInstance() {}

void WebSettingInstance::WebSettingManagerSetUserAgentString(
    const picojson::value& args, picojson::object& out) {
  const double callback_id = args.get("callbackId").get<double>();
  auto get = [=](const std::shared_ptr<JsonValue>& response) -> void {
    const char* runtime_name =
        common::Extension::GetRuntimeVariable("runtime_name", 64).c_str();
    LoggerD("runtime_name: %s", runtime_name);
    if (strcmp(runtime_name, kWrtServiceName) == 0) {
      ReportError(
          PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, "Not Implemented"),
          &response->get<picojson::object>());
      return;
    }

    std::string userAgent = args.get("userAgentStr").to_str();
    extension_->current_app()->SetUserAgentString(userAgent).release();

    ReportSuccess(response->get<picojson::object>());
  };

  auto get_response =
      [callback_id, this](const std::shared_ptr<JsonValue>& response) -> void {
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair("callbackId", picojson::value(callback_id)));
    LoggerD("callback is %s", response->serialize().c_str());
    PostMessage(response->serialize().c_str());
  };

  common::TaskQueue::GetInstance().Queue<JsonValue>(
      get, get_response,
      std::shared_ptr<JsonValue>(new JsonValue(JsonObject())));
}

void WebSettingInstance::WebSettingManagerRemoveAllCookies(
    const picojson::value& args, picojson::object& out) {
  const double callback_id = args.get("callbackId").get<double>();
  auto get = [=](const std::shared_ptr<JsonValue>& response) -> void {
    const char* runtime_name =
        common::Extension::GetRuntimeVariable("runtime_name", 64).c_str();
    LoggerD("runtime_name: %s", runtime_name);
    if (strcmp(runtime_name, kWrtServiceName) == 0) {
      ReportError(
          PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, "Not Implemented"),
          &response->get<picojson::object>());
      return;
    }

    extension_->current_app()->RemoveAllCookies().release();
    ReportSuccess(response->get<picojson::object>());
  };

  auto get_response =
      [callback_id, this](const std::shared_ptr<JsonValue>& response) -> void {
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair("callbackId", picojson::value(callback_id)));
    LoggerD("callback is %s", response->serialize().c_str());
    PostMessage(response->serialize().c_str());
  };

  common::TaskQueue::GetInstance().Queue<JsonValue>(
      get, get_response,
      std::shared_ptr<JsonValue>(new JsonValue(JsonObject())));
}

}  // namespace websetting
}  // namespace extension
