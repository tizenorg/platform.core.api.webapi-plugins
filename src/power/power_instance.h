/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

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
