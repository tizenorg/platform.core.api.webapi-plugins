// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
