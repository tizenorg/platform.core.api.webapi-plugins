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
 
#include <media_key.h>
#include "common/platform_result.h"

#ifndef SRC_MEDIAKEY_MEDIAKEY_MANAGER_H_
#define SRC_MEDIAKEY_MEDIAKEY_MANAGER_H_

namespace extension {
namespace mediakey {

class MediaKeyListener {
 public:
  virtual void OnPressedMediaKeyEventCallback(media_key_e type)= 0;
  virtual void OnReleasedMediaKeyEventCallback(media_key_e type)= 0;
  virtual ~MediaKeyListener();
};

class MediaKeyManager {
 public:
  common::PlatformResult RegisterMediaKeyEventListener(
      MediaKeyListener* listener);
  common::PlatformResult UnregisterMediaKeyEventListener();
  static MediaKeyManager& GetInstance();
  static void MediaKeyEventCallback(media_key_e key, media_key_event_e status,
                                    void* user_data);

 private:
  // Not copyable, assignable, movable
  MediaKeyManager(MediaKeyManager const&) = delete;
  void operator=(MediaKeyManager const&) = delete;
  MediaKeyManager(MediaKeyManager &&) = delete;

  MediaKeyManager();
  MediaKeyListener* m_media_key_listener;
  bool m_media_key_listener_registered;
};

}  // namespace mediakey
}  // namespace extension

#endif  // SRC_MEDIAKEY_MEDIAKEY_MANAGER_H_
