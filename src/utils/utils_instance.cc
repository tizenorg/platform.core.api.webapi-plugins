// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "utils/utils_instance.h"

namespace extension {
namespace utils {

UtilsInstance::UtilsInstance() {
  using std::placeholders::_1;
  using std::placeholders::_2;

#define REGISTER_SYNC(c, x) \
  RegisterSyncHandler(c, std::bind(&UtilsInstance::x, this, _1, _2));
#define REGISTER_ASYNC(c, x) \
  RegisterSyncHandler(c, std::bind(&UtilsInstance::x, this, _1, _2));

  REGISTER_SYNC("Utils_checkPrivilegeAccess", CheckPrivilegeAccess);

#undef REGISTER_SYNC
#undef REGISTER_ASYNC
}

void UtilsInstance::CheckPrivilegeAccess(const picojson::value& args, picojson::object& out) {
  const auto& privilege = args.get("privilege").to_str();
  CHECK_PRIVILEGE_ACCESS(privilege, &out);
  ReportSuccess(out);
}

}  // namespace utils
}  // namespace extension
