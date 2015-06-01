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

#include "common/converter.h"
#include "common/logger.h"
#include <stdexcept>
#include <string>

namespace common {

const picojson::value& FindValue(const picojson::object& in,
                                     const char* name) {
  LoggerD("Enter");
  auto it = in.find(name);
  if (it == in.end()) {
    throw common::UnknownException(
        std::string("Failed to find required JSON property: ") + name + ".");
  }
  return it->second;
}

long stol(const std::string& str, std::size_t* pos, int base) {
  try {
    return std::stol(str, pos, base);
  }
  catch (const std::invalid_argument& e) {
    LoggerE("invalid_argument");
    throw common::InvalidValuesException(e.what());
  }
  catch (const std::out_of_range& e) {
    LoggerE("InvalidValuesException");
    throw common::InvalidValuesException(e.what());
  }
}

}  // webapi
