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
 
#include <functional>
#include <map>
#include "common/logger.h"
#include "mediakey/mediakey_instance.h"
#include "mediakey/mediakey_manager.h"

namespace extension {
namespace mediakey {

const std::map<media_key_e, std::string> kMediaKeyTypeMap = {
    { MEDIA_KEY_PLAY, "MEDIA_PLAY" },
    { MEDIA_KEY_STOP, "MEDIA_STOP" },
    { MEDIA_KEY_PAUSE, "MEDIA_PAUSE" },
    { MEDIA_KEY_PREVIOUS, "MEDIA_PREVIOUS" },
    { MEDIA_KEY_NEXT, "MEDIA_NEXT" },
    { MEDIA_KEY_FASTFORWARD, "MEDIA_FAST_FORWARD" },
    { MEDIA_KEY_REWIND, "MEDIA_REWIND" },
    { MEDIA_KEY_PLAYPAUSE, "MEDIA_PLAY_PAUSE" } };

MediaKeyInstance::MediaKeyInstance() {
  LoggerD("Entered");
  using std::placeholders::_1;
  using std::placeholders::_2;
#define REGISTER_SYNC(c, x) \
    RegisterSyncHandler(c, std::bind(&MediaKeyInstance::x, this, _1, _2));
  REGISTER_SYNC("MediaKeyManager_setMediaKeyEventListener",
                SetMediaKeyEventListener);
  REGISTER_SYNC("MediaKeyManager_unsetMediaKeyEventListener",
                UnsetMediaKeyEventListener);
#undef REGISTER_SYNC
}

MediaKeyInstance::~MediaKeyInstance() {
  LoggerD("Entered");
  MediaKeyManager::GetInstance().UnregisterMediaKeyEventListener();
}

void MediaKeyInstance::SetMediaKeyEventListener(const picojson::value& args,
                                                picojson::object& out) {

  LoggerD("Enter");

  common::PlatformResult result = MediaKeyManager::GetInstance()
      .RegisterMediaKeyEventListener(this);
  if (result.IsError()) {
    LOGD("Error occured");
    ReportError(result, &out);
  } else {
    ReportSuccess(out);
  }
}

void MediaKeyInstance::UnsetMediaKeyEventListener(const picojson::value& args,
                                                  picojson::object& out) {
  LoggerD("Enter");
  common::PlatformResult result = MediaKeyManager::GetInstance()
      .UnregisterMediaKeyEventListener();
  if (result.IsError()) {
    LOGD("Error occured");
    ReportError(result, &out);
  } else {
    ReportSuccess(out);
  }
}

void MediaKeyInstance::OnPressedMediaKeyEventCallback(media_key_e type) {
  LoggerD("Enter");
  PostEvent("onPressedMediaKeyEventCallback", type);
}

void MediaKeyInstance::OnReleasedMediaKeyEventCallback(media_key_e type) {
  LoggerD("Enter");
  PostEvent("onReleasedMediaKeyEventCallback", type);
}

void MediaKeyInstance::PostEvent(const std::string& eventCallback,
                                 media_key_e type) {
  LoggerD("Enter");
  auto k = kMediaKeyTypeMap.find(type);
  if (k != kMediaKeyTypeMap.end()) {
    picojson::value event = picojson::value(picojson::object());
    picojson::object& obj = event.get<picojson::object>();
    obj["listenerId"] = picojson::value(eventCallback);
    obj["type"] = picojson::value((k->second).c_str());
    Instance::PostMessage(this, event.serialize().c_str());
  }
  else {
    LoggerD("Unsupported key");
  }
}

}  // namespace mediakey
}  // namespace extension
