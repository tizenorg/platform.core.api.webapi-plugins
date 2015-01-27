// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "application/application_context.h"

#include <app_manager.h>

#include "common/logger.h"
#include "tizen/tizen.h"

namespace extension {
namespace application {

ApplicationContext::ApplicationContext() {
}

ApplicationContext::ApplicationContext(const std::string& context_id)
  : context_id_(context_id) {
}

ApplicationContext::~ApplicationContext() {
}

const picojson::value& ApplicationContext::Value() {
  if (value_.is<picojson::null>() && IsValid()) {
    picojson::object obj;
    obj["id"] = picojson::value(context_id_);
    obj["appId"] = picojson::value(app_id_);
    value_ = picojson::value(obj);
  }
  return value_;
}

bool ApplicationContext::IsValid() const {
  return error_.empty();
}

std::string ApplicationContext::get_context_id() {
  return context_id_;
}

void ApplicationContext::set_context_id(const std::string& context_id) {
  context_id_ = context_id;
}

std::string ApplicationContext::get_app_id() {
  return app_id_;
}

void ApplicationContext::set_app_id(const std::string& app_id) {
  app_id_ = app_id;
}

}  // namespace application
}  // namespace extension
