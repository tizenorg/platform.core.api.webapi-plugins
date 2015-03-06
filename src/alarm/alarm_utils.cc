// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "alarm_utils.h"

#include "common/logger.h"

namespace extension {
namespace alarm {
namespace util {

using namespace common;

void CheckAccess(const std::string& privilege) {
  // TODO: check access to privilege, throw exception on failure
}

PlatformResult AppControlToService(const picojson::object& obj, app_control_h *app_control) {
  LoggerD("Entered");
}

PlatformResult AppControlToServiceExtraData(const picojson::object& app_obj,
                                            app_control_h *app_control) {
  LoggerD("Entered");
}

} // util
} // alarm
} // extension
