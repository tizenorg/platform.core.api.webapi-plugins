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

#include "power/power_instance.h"

#include <functional>

#include <device/display.h>
#include <device/power.h>
#include <device/callback.h>

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"

#include "power_manager.h"

namespace extension {
namespace power {

namespace {
const std::map<std::string, PowerResource> kPowerResourceMap = {
    {"SCREEN", POWER_RESOURCE_SCREEN},
    {"CPU", POWER_RESOURCE_CPU}
};

const std::map<std::string, PowerState> kPowerStateMap = {
    {"SCREEN_OFF", POWER_STATE_SCREEN_OFF},
    {"SCREEN_DIM", POWER_STATE_SCREEN_DIM},
    {"SCREEN_NORMAL", POWER_STATE_SCREEN_NORMAL},
    {"SCREEN_BRIGHT", POWER_STATE_SCREEN_BRIGHT},
    {"CPU_AWAKE", POWER_STATE_CPU_AWAKE}
};
} // namespace

using namespace common;
using namespace extension::power;

PowerInstance::PowerInstance() {
  LoggerD("Enter");
  using std::placeholders::_1;
  using std::placeholders::_2;

  #define REGISTER_SYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&PowerInstance::x, this, _1, _2));
  REGISTER_SYNC("PowerManager_turnScreenOff", PowerManagerTurnscreenoff);
  REGISTER_SYNC("PowerManager_restoreScreenBrightness", PowerManagerRestorescreenbrightness);
  REGISTER_SYNC("PowerManager_request", PowerManagerRequest);
  REGISTER_SYNC("PowerManager_getScreenBrightness", PowerManagerGetscreenbrightness);
  REGISTER_SYNC("PowerManager_release", PowerManagerRelease);
  REGISTER_SYNC("PowerManager_isScreenOn", PowerManagerIsscreenon);
  REGISTER_SYNC("PowerManager_turnScreenOn", PowerManagerTurnscreenon);
  REGISTER_SYNC("PowerManager_setScreenBrightness", PowerManagerSetscreenbrightness);
  #undef REGISTER_SYNC
  PowerManager::GetInstance()->AddListener(this);
}

PowerInstance::~PowerInstance() {
  LoggerD("Enter");
  PowerManager::GetInstance()->RemoveListener(this);
}

enum PowerCallbacks {
  PowerManagerTurnscreenoffCallback,
  PowerManagerRestorescreenbrightnessCallback,
  PowerManagerRequestCallback,
  PowerManagerGetscreenbrightnessCallback,
  PowerManagerReleaseCallback,
  PowerManagerUnsetscreenstatechangelistenerCallback,
  PowerManagerIsscreenonCallback,
  PowerManagerTurnscreenonCallback,
  PowerManagerSetscreenbrightnessCallback,
  PowerManagerSetscreenstatechangelistenerCallback
};

#define CHECK_EXIST(args, name, out) \
    if (!args.contains(name)) {\
      ReportError(TypeMismatchException(name" is required argument"), out);\
      return;\
    }

void PowerInstance::PowerManagerRequest(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  const std::string& resource = args.get("resource").get<std::string>();
  const std::string& state = args.get("state").get<std::string>();

  PlatformResult result =
      PowerManager::GetInstance()->Request(kPowerResourceMap.at(resource),
                                           kPowerStateMap.at(state));
  if (result.IsError())
    ReportError(result, &out);
  else
    ReportSuccess(out);
}

void PowerInstance::PowerManagerRelease(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  const std::string& resource = args.get("resource").get<std::string>();
  PlatformResult result =
      PowerManager::GetInstance()->Release(kPowerResourceMap.at(resource));
  if (result.IsError())
    ReportError(result, &out);
  else
    ReportSuccess(out);
}

void PowerInstance::PowerManagerGetscreenbrightness(const picojson::value& args,
                                                    picojson::object& out) {
  LoggerD("Enter");
  double brightness;
  PlatformResult result =
      PowerManager::GetInstance()->GetScreenBrightness(&brightness);
  if (result.IsError())
    ReportError(result, &out);
  else
    ReportSuccess(picojson::value(brightness), out);
}

void PowerInstance::PowerManagerSetscreenbrightness(const picojson::value& args,
                                                    picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "brightness", out)

  double brightness = args.get("brightness").get<double>();
  PlatformResult result =
      PowerManager::GetInstance()->SetScreenBrightness(brightness);
  if (result.IsError())
    ReportError(result, &out);
  else
    ReportSuccess(out);
}

void PowerInstance::PowerManagerIsscreenon(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  bool state = false;
  PlatformResult result = PowerManager::GetInstance()->IsScreenOn(&state);
  if (result.IsError())
    ReportError(result, &out);
  else
    ReportSuccess(picojson::value(state), out);
}

void PowerInstance::PowerManagerRestorescreenbrightness(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  PlatformResult result =
      PowerManager::GetInstance()->RestoreScreenBrightness();
  if (result.IsError())
    ReportError(result, &out);
  else
    ReportSuccess(out);
}

void PowerInstance::PowerManagerTurnscreenon(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  PlatformResult result = PowerManager::GetInstance()->SetScreenState(true);
  if (result.IsError())
    ReportError(result, &out);
  else
    ReportSuccess(out);
}

void PowerInstance::PowerManagerTurnscreenoff(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  PlatformResult result = PowerManager::GetInstance()->SetScreenState(false);
  if (result.IsError())
    ReportError(result, &out);
  else
    ReportSuccess(out);
}

void PowerInstance::OnScreenStateChanged(PowerState prev_state, PowerState new_state) {
  LoggerD("Enter");
  picojson::value event = picojson::value(picojson::object());
  picojson::object& obj = event.get<picojson::object>();
  obj["cmd"] = picojson::value("ScreenStateChanged");
  obj["listenerId"] = picojson::value("SCREEN_STATE_LISTENER");
  for (auto it = kPowerStateMap.begin(); it != kPowerStateMap.end(); ++it) {
    if (it->second == prev_state) {
      obj["prev_state"] = picojson::value(it->first);
    }
    if (it->second == new_state) {
      obj["new_state"] = picojson::value(it->first);
    }
  }

  Instance::PostMessage(this, event.serialize().c_str());
}

#undef CHECK_EXIST
} // namespace power
} // namespace extension
