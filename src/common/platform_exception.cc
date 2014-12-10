// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/platform_exception.h"

namespace common {

PlatformException::PlatformException(
    const std::string& name,
    const std::string& message)
    : name_(name),
    message_(message) {
}

PlatformException::~PlatformException() {
}

std::string PlatformException::name() const {
  return name_;
}

std::string PlatformException::message() const {
  return message_;
}

picojson::value PlatformException::ToJSON() const {
  picojson::value::object obj;
  obj["name"] = picojson::value(name_);
  obj["message"] = picojson::value(message_);
  picojson::value ret(obj);
  return ret;
}

} // namespace common
