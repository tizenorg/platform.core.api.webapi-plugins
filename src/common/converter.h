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
 
#ifndef COMMON_CONVERTER_H_
#define COMMON_CONVERTER_H_

#include "common/picojson.h"
#include "common/platform_exception.h"

namespace common {

// This is a wrapper around std::stol which throws exceptions from common rather
// than std
long stol(const std::string &str, std::size_t *pos = 0, int base = 10);

const picojson::value &FindValue(const picojson::object &in, const char *name);

inline bool IsNull(const picojson::value &in) {
  return in.is<picojson::null>();
}

inline bool IsNull(const picojson::object &in, const char *name) {
  return IsNull(FindValue(in, name));
}

template <typename T>
const T &JsonCast(const picojson::value &in) {
  if (!in.is<T>()) {
    throw common::UnknownException(std::string("Invalid JSON type"));
  }
  return in.get<T>();
}

template <typename T>
const T &FromJson(const picojson::object &in, const char *name) {
  const picojson::value &v = FindValue(in, name);
  return JsonCast<T>(v);
}

template <typename T, typename... Names>
const T &FromJson(const picojson::object &in, const char *name,
                  Names... names) {
  const picojson::value &v = FindValue(in, name);
  if (!v.is<picojson::object>()) {
    throw common::UnknownException(
        std::string("Invalid JSON type for property: ") + name + ".");
  }
  return FromJson<T>(v.get<picojson::object>(), names...);
}

}  // common

#endif  // COMMON_CONVERTER_H_
