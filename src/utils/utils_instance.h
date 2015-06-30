// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UTILS_UTILS_INSTANCE_H_
#define UTILS_UTILS_INSTANCE_H_

#include "common/extension.h"

namespace extension {
namespace utils {

class UtilsInstance : public common::ParsedInstance {
 public:
  UtilsInstance();
  virtual ~UtilsInstance() {}

 private:
  void GetPkgApiVersion(const picojson::value& args, picojson::object& out);
  void CheckPrivilegeAccess(const picojson::value& args, picojson::object& out);
  void CheckBackwardCompabilityPrivilegeAccess(const picojson::value& args, picojson::object& out);
};
}  // namespace utils
}  // namespace extension

#endif  // UTILS_UTILS_INSTANCE_H_
