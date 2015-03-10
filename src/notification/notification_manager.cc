// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "notification/notification_instance.h"
#include "common/logger.h"
#include "common/converter.h"

namespace extension {
namespace notification {

using namespace common;

NotificationManager::NotificationManager() {}

NotificationManager::~NotificationManager() {}

NotificationManager* NotificationManager::GetInstance() {
  static NotificationManager instance;
  return &instance;
}

common::PlatformResult NotificationManager::Post(const picojson::object& args,
                                                 int* id) {
  return PlatformResult(ErrorCode::NO_ERROR);
}

common::PlatformResult NotificationManager::Update(
    const picojson::object& args) {
  return PlatformResult(ErrorCode::NO_ERROR);
}

common::PlatformResult NotificationManager::Remove(
    const picojson::object& args) {
  return PlatformResult(ErrorCode::NO_ERROR);
}

common::PlatformResult NotificationManager::RemoveAll() {
  return PlatformResult(ErrorCode::NO_ERROR);
}

common::PlatformResult NotificationManager::Get(const picojson::object& args,
                                                picojson::object& out) {
  return PlatformResult(ErrorCode::NO_ERROR);
}

common::PlatformResult NotificationManager::GetAll(picojson::array& out) {
  return PlatformResult(ErrorCode::NO_ERROR);
}

}  // namespace notification
}  // namespace extension
