// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/logger.h"
#include "push/push_manager.h"

namespace extension {
namespace push {

PushManager::PushManager() {
    LoggerD("Enter");
}

PushManager::~PushManager() {
    LoggerD("Enter");
}

PushManager& PushManager::getInstance() {
  static PushManager instance;
  return instance;
}

}  // namespace push
}  // namespace extension

