// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef HUMANACTIVITYMONITOR_HUMANACTIVITYMONITOR_INSTANCE_H_
#define HUMANACTIVITYMONITOR_HUMANACTIVITYMONITOR_INSTANCE_H_

#include <memory>

#include "common/extension.h"
#include "common/platform_result.h"
#include "humanactivitymonitor/humanactivitymonitor_manager.h"

namespace extension {
namespace humanactivitymonitor {

class HumanActivityMonitorInstance : public common::ParsedInstance {
 public:
  HumanActivityMonitorInstance();
  virtual ~HumanActivityMonitorInstance();

 private:
  void HumanActivityMonitorManagerStop(
      const picojson::value& args, picojson::object& out);
  void HumanActivityMonitorManagerUnsetAccumulativePedometerListener(
      const picojson::value& args, picojson::object& out);
  void HumanActivityMonitorManagerGetHumanActivityData(
      const picojson::value& args, picojson::object& out);
  void HumanActivityMonitorManagerStart(
      const picojson::value& args, picojson::object& out);
  void HumanActivityMonitorManagerSetAccumulativePedometerListener(
      const picojson::value& args, picojson::object& out);

  std::shared_ptr<HumanActivityMonitorManager> manager_;
  common::PlatformResult Init();
};

} // namespace humanactivitymonitor
} // namespace extension

#endif  // HUMANACTIVITYMONITOR_HUMANACTIVITYMONITOR_INSTANCE_H_
