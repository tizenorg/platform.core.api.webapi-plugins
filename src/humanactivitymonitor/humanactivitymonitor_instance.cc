// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "humanactivitymonitor/humanactivitymonitor_instance.h"

#include <functional>
#include <string>

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_result.h"

namespace extension {
namespace humanactivitymonitor {

using common::PlatformResult;
using common::ErrorCode;

HumanActivityMonitorInstance::HumanActivityMonitorInstance() {
  using std::placeholders::_1;
  using std::placeholders::_2;

#define REGISTER_SYNC(c, x) \
    RegisterSyncHandler(c, std::bind(&HumanActivityMonitorInstance::x, this, _1, _2));
  REGISTER_SYNC("HumanActivityMonitorManager_getHumanActivityData",
                HumanActivityMonitorManagerGetHumanActivityData);
  REGISTER_SYNC("HumanActivityMonitorManager_start",
                HumanActivityMonitorManagerStart);
  REGISTER_SYNC("HumanActivityMonitorManager_stop",
                HumanActivityMonitorManagerStop);
  REGISTER_SYNC("HumanActivityMonitorManager_setAccumulativePedometerListener",
                HumanActivityMonitorManagerSetAccumulativePedometerListener);
  REGISTER_SYNC("HumanActivityMonitorManager_unsetAccumulativePedometerListener",
                HumanActivityMonitorManagerUnsetAccumulativePedometerListener);
#undef REGISTER_SYNC
}

HumanActivityMonitorInstance::~HumanActivityMonitorInstance() {
}

#define CHECK_EXIST(args, name, out) \
    if (!args.contains(name)) { \
      ReportError(PlatformResult(ErrorCode::TYPE_MISMATCH_ERR, \
          name" is required argument"), &out); \
      return; \
    }


void HumanActivityMonitorInstance::HumanActivityMonitorManagerGetHumanActivityData(
    const picojson::value& args, picojson::object& out) {
  // TODO(r.galka) implement
}

void HumanActivityMonitorInstance::HumanActivityMonitorManagerStart(
    const picojson::value& args, picojson::object& out) {
  // TODO(r.galka) implement
}

void HumanActivityMonitorInstance::HumanActivityMonitorManagerStop(
    const picojson::value& args, picojson::object& out) {
  // TODO(r.galka) implement
}

void HumanActivityMonitorInstance::HumanActivityMonitorManagerSetAccumulativePedometerListener(
    const picojson::value& args, picojson::object& out) {
  // TODO(r.galka) implement
}

void HumanActivityMonitorInstance::HumanActivityMonitorManagerUnsetAccumulativePedometerListener(
    const picojson::value& args, picojson::object& out) {
  // TODO(r.galka) implement
}

#undef CHECK_EXIST

}  // namespace humanactivitymonitor
}  // namespace extension
