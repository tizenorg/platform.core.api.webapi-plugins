// Copyright (c) 2014 Samsung Electronics Co., Ltd. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "websetting/websetting_instance.h"

#include <string>
#include <memory>

#include "common/logger.h"
#include "common/picojson.h"
#include "common/task-queue.h"
#include "common/platform_exception.h"

namespace {
const char kWrtServiceName[] = "wrt-service";
const char kSetUserAgentString[] = "Websetting_setUserAgentString";
const char kRemoveAllCookies[] = "Websetting_removeAllCookies";
}  // namespace

namespace extension {
namespace websetting {

typedef picojson::value JsonValue;
typedef picojson::object JsonObject;
typedef picojson::array JsonArray;
typedef std::string JsonString;

WebSettingInstance::WebSettingInstance(WebSettingExtension* extension)
    : extension_(extension) {
  using namespace std::placeholders;

#define REGISTER_ASYNC(c, x) \
  RegisterHandler(c, std::bind(&WebSettingInstance::x, this, _1, _2));
  REGISTER_ASYNC(kSetUserAgentString, setUserAgentString);
  REGISTER_ASYNC(kRemoveAllCookies, removeAllCookies);
#undef REGISTER_ASYNC
}

WebSettingInstance::~WebSettingInstance() {}

void WebSettingInstance::setUserAgentString(const picojson::value& args,
                                            picojson::object& out) {
  const double callback_id = args.get("callbackId").get<double>();
  auto get = [=](const std::shared_ptr<JsonValue>& response) -> void {
    try {
      const char *runtime_name =
          common::Extension::GetRuntimeVariable("runtime_name", 64).c_str();
      LoggerD("runtime_name: %s", runtime_name);
      if (strcmp(runtime_name, kWrtServiceName) == 0) {
        throw common::NotSupportedException("Not Implemented");
      }

      std::string userAgent = args.get("userAgentStr").to_str();
      extension_->current_app()->
          SetUserAgentString(userAgent).release();
      ReportSuccess(response->get<picojson::object>());
    } catch (const common::PlatformException& e) {
      ReportError(e, response->get<picojson::object>());
    }
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

void WebSettingInstance::removeAllCookies(const picojson::value& args,
                                          picojson::object& out) {
  const double callback_id = args.get("callbackId").get<double>();
  auto get = [=](const std::shared_ptr<JsonValue>& response) -> void {
    try {
      const char *runtime_name =
          common::Extension::GetRuntimeVariable("runtime_name", 64).c_str();
      LoggerD("runtime_name: %s", runtime_name);
      if (strcmp(runtime_name, kWrtServiceName) == 0) {
        throw common::NotSupportedException("Not Implemented");
      }

      extension_->current_app()->
          RemoveAllCookies().release();
      ReportSuccess(response->get<picojson::object>());
    } catch (const common::PlatformException& e) {
      ReportError(e, response->get<picojson::object>());
    }
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
