// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/platform_result.h"
#include "common/logger.h"

namespace common {

PlatformResult::PlatformResult(const ErrorCode& error_code, const std::string& message) :
    error_code_(error_code),
    message_(message)
{
  LoggerD("Enter");
}

picojson::value PlatformResult::ToJSON() const {
  LoggerD("Enter");
  picojson::value::object obj;
  obj["code"] = picojson::value(static_cast<double>(error_code_));
  if (!message_.empty())
    obj["message"] = picojson::value(message_);
  picojson::value ret(obj);
  return ret;
}

}  // namespace common


