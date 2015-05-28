// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/logger.h"
#include "utils/utils_instance.h"

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

  REGISTER_SYNC("Utils_checkPrivilegeAccess", CheckPrivilegeAccess);
  REGISTER_SYNC("Utils_checkBackwardCompabilityPrivilegeAccess", CheckBackwardCompabilityPrivilegeAccess);

#undef REGISTER_SYNC
#undef REGISTER_ASYNC
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

}  // namespace utils
}  // namespace extension
