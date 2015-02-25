// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "application/application.h"

#include "common/extension.h"
#include "common/logger.h"
#include "common/picojson.h"

#include "common/platform_result.h"

using common::ErrorCode;

namespace extension {
namespace application {

Application::Application() {
}

Application::~Application() {
}

void Application::Hide() {
}

void Application::Exit() {
}

std::string Application::get_context_id() {
  return context_id_;
}

void Application::set_context_id(const std::string& context_id) {
  context_id_ = context_id;
}

ApplicationInformationPtr Application::get_app_info() const {
  return app_info_;
}

void Application::set_app_info(const ApplicationInformationPtr& app_info) {
  app_info_ = app_info;
}

const picojson::value& Application::Value() {
  if (!app_info_->IsValid()) {
    LoggerD("ErrorCode::UNKNOWN_ERR");
    picojson::object obj;
    obj["error"] = picojson::value(static_cast<double>(ErrorCode::UNKNOWN_ERR));
    value_ = picojson::value(obj);
  } else {
    picojson::object obj;
    LoggerD("Value returns appInfo, contextId");
    obj["appInfo"] = app_info_->Value();
    obj["contextId"] = picojson::value(context_id_);
    value_ = picojson::value(obj);
  }
  return value_;
}

}  // namespace application
}  // namespace extension
