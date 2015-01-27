// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "application/application_controldata.h"

#include <app_manager.h>

#include "common/logger.h"
#include "tizen/tizen.h"

namespace extension {
namespace application {

ApplicationControlData::ApplicationControlData() {
}

ApplicationControlData::~ApplicationControlData() {
}

const picojson::value& ApplicationControlData::Value() {
  if (value_.is<picojson::null>() && IsValid()) {
    data_["key"] = picojson::value(ctr_key_);
    picojson::value values = picojson::value(picojson::array());
    picojson::array& values_array = values.get<picojson::array>();
    for (auto it = ctr_value_.begin(); it != ctr_value_.end(); it++) {
      values_array.push_back(picojson::value(*it));
    }
    data_["value"] = values;

    value_ = picojson::value(data_);
  }
  return value_;
}

bool ApplicationControlData::IsValid() const {
  return error_.empty();
}

std::string ApplicationControlData::get_ctr_key() const {
  return ctr_key_;
}

void ApplicationControlData::set_ctr_key(const std::string& ctr_key) {
  ctr_key_ = ctr_key;
}

std::vector<std::string> ApplicationControlData::get_ctr_value() const {
  return ctr_value_;
}

void ApplicationControlData::set_ctr_value
  (const std::vector<std::string> &ctr_values) {
  ctr_value_ = ctr_values;
}

void ApplicationControlData::add_ctr_value(const std::string& ctr_value) {
  ctr_value_.push_back(ctr_value);
}



}  // namespace application
}  // namespace extension
