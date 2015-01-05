// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
// The privileges that required in Power API
const std::string kPrivilegePower = "http://tizen.org/privilege/power";

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
  using namespace std::placeholders;
  #define REGISTER_SYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&PowerInstance::x, this, _1, _2));
  REGISTER_SYNC("PowerManager_request", Request);
  REGISTER_SYNC("PowerManager_release", Release);
  REGISTER_SYNC("PowerManager_getScreenBrightness", GetScreenBrightness);
  REGISTER_SYNC("PowerManager_setScreenBrightness", SetScreenBrightness);
  REGISTER_SYNC("PowerManager_isScreenOn", IsScreenOn);
  REGISTER_SYNC("PowerManager_restoreScreenBrightness", RestoreScreenBrightness);
  REGISTER_SYNC("PowerManager_setScreenState", SetScreenState);
  #undef REGISTER_SYNC

  PowerManager::GetInstance()->AddListener(this);
}

PowerInstance::~PowerInstance() {
}

void PowerInstance::Request(const picojson::value& args, picojson::object& out) {
  const std::string& resource = args.get("resource").get<std::string>();
  const std::string& state = args.get("state").get<std::string>();

  PowerManager::GetInstance()->Request(kPowerResourceMap.at(resource),
                                       kPowerStateMap.at(state));
  ReportSuccess(out);
}

void PowerInstance::Release(const picojson::value& args, picojson::object& out) {
  const std::string& resource = args.get("resource").get<std::string>();
  PowerManager::GetInstance()->Release(kPowerResourceMap.at(resource));
  ReportSuccess(out);
}

void PowerInstance::GetScreenBrightness(const picojson::value& args, picojson::object& out) {
  double brightness = PowerManager::GetInstance()->GetScreenBrightness();
  ReportSuccess(picojson::value(brightness), out);
}

void PowerInstance::SetScreenBrightness(const picojson::value& args, picojson::object& out) {
  double brightness = args.get("brightness").get<double>();
  PowerManager::GetInstance()->SetScreenBrightness(brightness);
  ReportSuccess(out);
}

void PowerInstance::IsScreenOn(const picojson::value& args, picojson::object& out) {
  bool ret = PowerManager::GetInstance()->IsScreenOn();
  ReportSuccess(picojson::value(ret), out);
}

void PowerInstance::RestoreScreenBrightness(const picojson::value& args, picojson::object& out) {
  PowerManager::GetInstance()->RestoreScreenBrightness();
  ReportSuccess(out);
}

void PowerInstance::SetScreenState(const picojson::value& args, picojson::object& out) {
  bool onoff = args.get("on").get<bool>();
  PowerManager::GetInstance()->SetScreenState(onoff);
  ReportSuccess(out);
}

void PowerInstance::OnScreenStateChanged(PowerState prev_state, PowerState new_state) {
  picojson::value event = picojson::value(picojson::object());
  picojson::object& obj = event.get<picojson::object>();
  obj["cmd"] = picojson::value("ScreenStateChanged");
  obj["listenerId"] = picojson::value("ScreenStateChanged");

  for (auto it = kPowerStateMap.begin(); it != kPowerStateMap.end(); ++it) {
    if (it->second == prev_state) {
      obj["prev_state"] = picojson::value(it->first);
    }
    if (it->second == new_state) {
      obj["new_state"] = picojson::value(it->first);
    }
  }

  PostMessage(event.serialize().c_str());
}

} // namespace power
} // namespace extension
