// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sys/types.h>
#include <utility>
#include <unistd.h>

#include "common/logger.h"
#include "common/scope_exit.h"
#include "common/tools.h"

#include "utils/utils_instance.h"

using common::PlatformResult;
using common::ErrorCode;

namespace extension {
namespace utils {

UtilsInstance::UtilsInstance() {
  using std::placeholders::_1;
  using std::placeholders::_2;

  LoggerD("Entered");
#define REGISTER_SYNC(c, x) \
  RegisterSyncHandler(c, std::bind(&UtilsInstance::x, this, _1, _2));
#define REGISTER_ASYNC(c, x) \
  RegisterSyncHandler(c, std::bind(&UtilsInstance::x, this, _1, _2));

  REGISTER_SYNC("Utils_getPkgApiVersion", GetPkgApiVersion);
  REGISTER_SYNC("Utils_checkPrivilegeAccess", CheckPrivilegeAccess);
  REGISTER_SYNC("Utils_checkBackwardCompabilityPrivilegeAccess", CheckBackwardCompabilityPrivilegeAccess);
  REGISTER_SYNC("Utils_toLongLong", ToLongLong);
  REGISTER_SYNC("Utils_toUnsignedLongLong", ToUnsignedLongLong);

#undef REGISTER_SYNC
#undef REGISTER_ASYNC
}

void UtilsInstance::GetPkgApiVersion(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  std::string api_version;
  PlatformResult ret = common::tools::GetPkgApiVersion(&api_version);
  if (ret.IsError()) {
    ReportError(ret, &out);
  }
  ReportSuccess(picojson::value(api_version), out);
}

void UtilsInstance::CheckPrivilegeAccess(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  const auto& privilege = args.get("privilege").to_str();
  CHECK_PRIVILEGE_ACCESS(privilege, &out);
  ReportSuccess(out);
}

void UtilsInstance::CheckBackwardCompabilityPrivilegeAccess(const picojson::value& args,
                                                            picojson::object& out) {
  LoggerD("Entered");
  const auto& current_priv = args.get("current_privilege").to_str();
  const auto& prev_priv = args.get("previous_privilege").to_str();

  CHECK_BACKWARD_COMPABILITY_PRIVILEGE_ACCESS(current_priv, prev_priv, &out);
  ReportSuccess(out);
}

namespace {

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

const double kTwoPow63 = 9223372036854775808.0;
const double kTwoPow64 = 18446744073709551616.0;

}  // namespace

void UtilsInstance::ToLongLong(const picojson::value& args,
                               picojson::object& out) {
  LoggerD("Entered");

  const auto& n = args.get("n");
  long long output = 0;

  if (n.is<double>()) {
    auto d = n.get<double>();
    d = sgn<double>(d) * std::floor(std::fabs(d));
    d = std::fmod(d, kTwoPow64);
    if (d > kTwoPow63) {
      d -= kTwoPow64;
    }
    output = static_cast<long long>(d);
  }

  ReportSuccess(picojson::value(static_cast<double>(output)), out);
}

void UtilsInstance::ToUnsignedLongLong(const picojson::value& args,
                                       picojson::object& out) {
  LoggerD("Entered");

  const auto& n = args.get("n");
  unsigned long long output = 0;

  if (n.is<double>()) {
    auto d = n.get<double>();
    d = sgn<double>(d) * std::floor(std::fabs(d));
    d = std::fmod(d, kTwoPow64);
    if (d < 0.0) {
      d += kTwoPow64;
    }
    output = static_cast<unsigned long long>(d);
  }

  ReportSuccess(picojson::value(static_cast<double>(output)), out);
}

}  // namespace utils
}  // namespace extension
