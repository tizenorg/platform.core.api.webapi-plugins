// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "application/application_metadata.h"

#include <app_manager.h>

#include "common/logger.h"
#include "tizen/tizen.h"

namespace extension {
namespace application {

ApplicationMetaData::ApplicationMetaData() {

}

ApplicationMetaData::ApplicationMetaData(const ApplicationMetaDataPtr app) {

}

ApplicationMetaData::~ApplicationMetaData() {
}

const picojson::value& ApplicationMetaData::Value() {
  if (value_.is<picojson::null>() && IsValid()) {
    picojson::object obj;
    obj["key"] = picojson::value(meta_key_);
    obj["value"] = picojson::value(meta_value_);
    value_ = picojson::value(obj);
  }
  return value_;
}

bool ApplicationMetaData::IsValid() const {
  return error_.empty();
}

std::string ApplicationMetaData::get_meta_key() const {
  return meta_key_;
}

void ApplicationMetaData::set_meta_key(const std::string& meta_key) {
  meta_key_ = meta_key;
}

std::string ApplicationMetaData::get_meta_value() const {
  return meta_value_;
}

void ApplicationMetaData::set_meta_value(const std::string& meta_value) {
  meta_value_ = meta_value;
}
 
} // namespace application
} // namespace extension
