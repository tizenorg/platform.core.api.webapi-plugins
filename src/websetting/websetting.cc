// Copyright (c) 2014 Samsung Electronics Co., Ltd. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "websetting/websetting.h"

#include <sys/types.h>
#include <unistd.h>
#include <utility>

#include "common/logger.h"
#include "common/platform_result.h"

using common::ErrorCode;
using common::PlatformResult;

namespace {

const char kRuntimeServiceName[] = "org.crosswalkproject.Runtime1";
const char kRuntimeRunningManagerPath[] = "/running1";
const char kRuntimeRunningAppInterface[] =
    "org.crosswalkproject.Running.Application1";

// The runtime process exports object for each running app on the session bus.
GDBusProxy* CreateRunningAppProxy(const std::string& app_id) {
  LoggerD("Entered");

  GError* error = nullptr;
  GDBusConnection* connection =
      g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &error);
  if (!connection) {
    LoggerE("Couldn't get the session bus connection: %s", error->message);
    g_error_free(error);
    return nullptr;
  }

  std::string path = std::string(kRuntimeRunningManagerPath) + "/" + app_id;
  // Every application id contains '.' character and since object path
  // is created from application id it also contains '.' character.
  // The d-bus proxy doesn't accept '.' character in object path
  // And that is why the substantiation is needed here.
  std::replace(path.begin(), path.end(), '.', '_');
  GDBusProxy* proxy = g_dbus_proxy_new_sync(
      connection, G_DBUS_PROXY_FLAGS_NONE, nullptr, kRuntimeServiceName,
      path.c_str(), kRuntimeRunningAppInterface, nullptr, &error);
  if (!proxy) {
    LoggerE("Couldn't create proxy for %s: %s", kRuntimeRunningAppInterface,
            error->message);
    g_error_free(error);
    return nullptr;
  }

  return proxy;
}

}  // namespace

WebSetting::WebSetting(const std::string& app_id)
    : app_id_(app_id), running_app_proxy_(nullptr) {}

WebSetting::~WebSetting() {
  if (running_app_proxy_) g_object_unref(running_app_proxy_);
}

common::PlatformResult WebSetting::RemoveAllCookies() {
  LoggerD("Entered");
  if (!running_app_proxy_) {
    if (!(running_app_proxy_ = CreateRunningAppProxy(app_id_))) {
      LoggerE("Failed to create proxy");
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Unable to remove cookies.");
    }
  }
  GError* error = nullptr;
  GVariant* result =
      g_dbus_proxy_call_sync(running_app_proxy_, "RemoveAllCookies", nullptr,
                             G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
  if (!result) {
    LoggerE("Failed to call 'RemoveuserAgentAllCookies': %s", error->message);
    g_error_free(error);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Unable to remove cookies.");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

common::PlatformResult WebSetting::SetUserAgentString(
    const std::string& user_agent) {
  LoggerD("Entered");
  if (!running_app_proxy_) {
    if (!(running_app_proxy_ = CreateRunningAppProxy(app_id_))) {
      LoggerE("Failed to create proxy");
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Unable to set user agent.");
    }
  }
  GError* error = nullptr;
  GVariant* result =
      g_dbus_proxy_call_sync(running_app_proxy_, "SetUserAgentString",
                             g_variant_new("(s)", user_agent.c_str()),
                             G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
  if (!result) {
    LoggerE("Fail to call 'SetUserAgentString': %s", error->message);
    g_error_free(error);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Unable to set user agent.");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}
