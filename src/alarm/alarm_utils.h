// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ALARM_ALARM_UTILS_H_
#define ALARM_ALARM_UTILS_H_

#include <app_control.h>

#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace alarm {
namespace util {

void CheckAccess(const std::string& privilege);

common::PlatformResult AppControlToService(const picojson::object& obj, app_control_h *app_control);
common::PlatformResult AppControlToServiceExtraData(const picojson::object& app_obj,
                                                           app_control_h *app_control);
} // util
} // alarm
} // extension

#endif // ALARM_ALARM_UTILS_H_
