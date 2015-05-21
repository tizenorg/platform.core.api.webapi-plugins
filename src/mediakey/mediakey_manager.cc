// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mediakey/mediakey_manager.h"
#include "common/logger.h"
#include "common/platform_exception.h"

namespace extension {
namespace mediakey {

using common::UnknownException;
using common::ErrorCode;
MediaKeyListener::~MediaKeyListener() {
  LoggerD("Enter");
}

MediaKeyManager& MediaKeyManager::GetInstance() {
  static MediaKeyManager manager;
  return manager;
}

MediaKeyManager::MediaKeyManager()
    : m_media_key_listener_registered(false) {
  LoggerD("Enter");
}

common::PlatformResult MediaKeyManager::RegisterMediaKeyEventListener(
    MediaKeyListener* listener) {
  LoggerD("Enter");
  if (!m_media_key_listener_registered) {
    int ret = media_key_reserve(MediaKeyEventCallback, NULL);
    if (MEDIA_KEY_ERROR_NONE != ret) {
      LoggerD("Failed to register "
              "a media keys change event callback: %d",
              ret);
      return common::PlatformResult(
          ErrorCode::UNKNOWN_ERR,
          "Failed to register a media keys change event callback");
    }
    m_media_key_listener = listener;
    m_media_key_listener_registered = true;
    LOGD("Added media keys change event callback");
  }
  return common::PlatformResult(ErrorCode::NO_ERROR);
}
common::PlatformResult MediaKeyManager::UnregisterMediaKeyEventListener() {
  LoggerD("Enter");
  if (m_media_key_listener_registered) {
    int ret = media_key_release();
    if (MEDIA_KEY_ERROR_NONE != ret) {
      LoggerD("Failed to unregister "
              "the change event callback function: %d",
              ret);
      return common::PlatformResult(
          ErrorCode::UNKNOWN_ERR,
          "Failed to unregister the change event callback function");
    }
  }
  m_media_key_listener = NULL;
  m_media_key_listener_registered = false;
  return common::PlatformResult(ErrorCode::NO_ERROR);
}

void MediaKeyManager::MediaKeyEventCallback(media_key_e key,
                                            media_key_event_e status,
                                            void* user_data) {
  LoggerD("Enter");
  if (!GetInstance().m_media_key_listener) {
    LOGD("Listener is null. Ignoring");
    return;
  }
  if (MEDIA_KEY_STATUS_PRESSED == status) {
    GetInstance().m_media_key_listener->OnPressedMediaKeyEventCallback(key);
  } else if (MEDIA_KEY_STATUS_RELEASED == status) {
    GetInstance().m_media_key_listener->OnReleasedMediaKeyEventCallback(key);
  }
}

}  // namespace mediakey
}  // namespace extension
