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
 
#include "mediakey/mediakey_manager.h"
#include "common/logger.h"
#include "common/platform_exception.h"
#include <glib.h>
#include <Ecore.h>

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
    : m_media_key_listener(nullptr),
      m_media_key_listener_registered(false) {
  LoggerD("Enter");
}

static int timeout_id_=0;

static gboolean itr_ecore(gpointer user_data){
    //LoggerD("Enter");
    ecore_main_loop_iterate();

  return true;
}

common::PlatformResult MediaKeyManager::RegisterMediaKeyEventListener(
    MediaKeyListener* listener) {
  LoggerD("Enter");




  if (!m_media_key_listener_registered) {
    LoggerD("before calling media_key_reserve");

    ecore_init(); 
    timeout_id_ = g_timeout_add(100, itr_ecore, this);

    int ret = media_key_reserve(MediaKeyEventCallback, NULL);

    LoggerD("after calling media_key_reserve - result = %d", ret);
    if (MEDIA_KEY_ERROR_NONE != ret) {
      if (timeout_id_ > 0) {
        // cancel terminate callback
        g_source_remove(timeout_id_);
        timeout_id_ = 0;
      }
      return LogAndCreateResult(
          ErrorCode::UNKNOWN_ERR,
          "Failed to register a media keys change event callback",
          ("media_key_reserve() error: %d, message: %s", ret, get_error_message(ret)));
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

    if (timeout_id_ > 0) {
      // cancel terminate callback
      g_source_remove(timeout_id_);
      timeout_id_ = 0;
    }

    if (MEDIA_KEY_ERROR_NONE != ret) {
      return LogAndCreateResult(
          ErrorCode::UNKNOWN_ERR,
          "Failed to unregister the change event callback function",
          ("media_key_release() error: %d, message: %s", ret, get_error_message(ret)));
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
