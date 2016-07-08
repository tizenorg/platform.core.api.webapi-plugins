/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
 
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