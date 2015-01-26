// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "application/requested_application_control.h"

#include <app_manager.h>

#include "common/logger.h"
#include "tizen/tizen.h"

namespace extension {
namespace application {

RequestedApplicationControl::RequestedApplicationControl() {
}

RequestedApplicationControl::~RequestedApplicationControl() {
}

const picojson::value& RequestedApplicationControl::Value() {
  LoggerD("caller_app_id_: %s", caller_app_id_.c_str());
  data_["callerAppId"] = picojson::value(caller_app_id_);
  data_["appControl"] = picojson::value(app_control_.Value());

  value_ = picojson::value(data_);

  return value_;
}

bool RequestedApplicationControl::IsValid() const {
  return error_.empty();
}

std::string RequestedApplicationControl::get_caller_app_id() const {
  return caller_app_id_;
}

void RequestedApplicationControl::
  set_caller_app_id(const std::string& caller_app_id) {
  caller_app_id_ = caller_app_id;
  LoggerD("caller_app_id: %s", caller_app_id.c_str());
}

ApplicationControl& RequestedApplicationControl::get_app_control() {
  return app_control_;
}

void RequestedApplicationControl::
  set_app_control(const ApplicationControl& app_control) {
  app_control_.set_operation(app_control.get_operation());
  app_control_.set_uri(app_control.get_uri());
  app_control_.set_mime(app_control.get_mime());
  app_control_.set_category(app_control.get_category());
  app_control_.set_data_array(app_control.get_data_array());
}

}  // namespace application
}  // namespace extension
