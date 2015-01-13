// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef POWER_POWER_INSTANCE_H_
#define POWER_POWER_INSTANCE_H_

#include "common/extension.h"

#include "power_manager.h"

namespace extension {
namespace power {

class PowerInstance
    : public common::ParsedInstance,
      public PowerManagerListener {
 public:
  PowerInstance();
  virtual ~PowerInstance();

 private:
  void PowerManagerRequest(const picojson::value& args, picojson::object& out);
  void PowerManagerRelease(const picojson::value& args, picojson::object& out);
  void PowerManagerGetscreenbrightness(const picojson::value& args, picojson::object& out);
  void PowerManagerSetscreenbrightness(const picojson::value& args, picojson::object& out);
  void PowerManagerIsscreenon(const picojson::value& args, picojson::object& out);
  void PowerManagerRestorescreenbrightness(const picojson::value& args, picojson::object& out);
  void PowerManagerTurnscreenon(const picojson::value& args, picojson::object& out);
  void PowerManagerTurnscreenoff(const picojson::value& args, picojson::object& out);
  void SetScreenState(const picojson::value& args, picojson::object& out);

  // Override
  void OnScreenStateChanged(PowerState prev_state, PowerState new_state);
};

} // namespace power
} // namespace extension

#endif // POWER_POWER_INSTANCE_H_
