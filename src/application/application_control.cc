// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "application/application_control.h"

#include <app_manager.h>

#include "common/logger.h"
#include "tizen/tizen.h"

namespace extension {
namespace application {

ApplicationControl::ApplicationControl() {
}

ApplicationControl::~ApplicationControl() {
}

const picojson::value& ApplicationControl::Value() {
  if (value_.is<picojson::null>() && IsValid()) {
    data_["operation"] = picojson::value(operation_);
    data_["uri"] = picojson::value(uri_);
    data_["mime"] = picojson::value(mime_);
    data_["category"] = picojson::value(category_);

    picojson::value datas = picojson::value(picojson::array());
    picojson::array& datas_array = datas.get<picojson::array>();
    for (auto it = data_array_.begin(); it != data_array_.end(); it++) {
      datas_array.push_back((*it)->Value());
    }
    data_["data"] = datas;

    value_ = picojson::value(data_);
  }
  return value_;
}

bool ApplicationControl::IsValid() const {
  return error_.empty();
}

std::string ApplicationControl::get_operation() const {
  return operation_;
}

void ApplicationControl::set_operation(const std::string& operation) {
  operation_ = operation;
}

std::string ApplicationControl::get_uri() const {
  return uri_;
}

void ApplicationControl::set_uri(const std::string& uri) {
  uri_ = uri;
}

std::string ApplicationControl::get_mime() const {
  return mime_;
}

void ApplicationControl::set_mime(const std::string& mime) {
  mime_ = mime;
}

std::string ApplicationControl::get_category() const {
  return category_;
}

void ApplicationControl::set_category(const std::string& category) {
  category_ = category;
}

ApplicationControlDataArray ApplicationControl::get_data_array() const {
  return data_array_;
}

void ApplicationControl::set_data_array
  (const ApplicationControlDataArray& data_array) {
  data_array_ = data_array;
}

void ApplicationControl::add_data_array
  (const ApplicationControlDataPtr& data_ptr) {
  data_array_.push_back(data_ptr);
}

}  // namespace application
}  // namespace extension
