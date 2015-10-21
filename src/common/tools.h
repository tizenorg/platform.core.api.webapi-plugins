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

#ifndef COMMON_TOOLS_H_
#define COMMON_TOOLS_H_

#include <string>
#include <vector>

#include "common/picojson.h"
#include "common/platform_exception.h"
#include "common/platform_result.h"

namespace common {
namespace tools {

void ReportSuccess(picojson::object& out);
void ReportSuccess(const picojson::value& result, picojson::object& out);
void ReportError(picojson::object& out);
void ReportError(const PlatformException& ex, picojson::object& out);
void ReportError(const PlatformResult& error, picojson::object* out);

common::PlatformResult CheckAccess(const std::string& privilege);
common::PlatformResult CheckAccess(const std::vector<std::string>& privileges);

#define CHECK_PRIVILEGE_ACCESS(privilege, out) \
do { \
  auto r = common::tools::CheckAccess(privilege); \
  if (!r) { \
    common::tools::ReportError(r, out); \
    return; \
  } \
} while (0)

#define CHECK_BACKWARD_COMPABILITY_PRIVILEGE_ACCESS(current_priv, prev_priv, out) \
do { \
  auto ret = common::tools::CheckAccess(current_priv); \
  auto ret2 = common::tools::CheckAccess(prev_priv); \
\
  if (!ret && ret2) { \
    ret = ret2; \
  } \
\
  if (!ret) { \
    common::tools::ReportError(ret, out); \
    return; \
  } \
} while (0)

/**
 * @brief Safe wrapper of strerror() function.
 *
 * @param[in] error_code - error code to be passed to strerror()
 *
 * @return string representation of error_code
 */
std::string GetErrorString(int error_code);

int HexToInt(char c);
unsigned char* HexToBin(const char* hex, int size, unsigned char* bin, int bin_size);
char* BinToHex(const unsigned char* bin, int size, char* hex, int hex_size);

}  // namespace tools
}  // namespace common

#endif  // COMMON_TOOLS_H_
