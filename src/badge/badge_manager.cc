// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "badge_manager.h"

#include <cstring>

#include "common/logger.h"
#include "common/platform_exception.h"

using namespace common;

namespace extension {
namespace badge {

BadgeManager::BadgeManager() {}

BadgeManager::~BadgeManager() {}

BadgeManager* BadgeManager::GetInstance() {
  static BadgeManager instance;
  return &instance;
}

void setBadgeCount(std::string appId, long count) {
  throw UnknownException("Not implemented.");
}

long getBadgeCount(std::string appId) {
  throw UnknownException("Not implemented.");
}

}  // namespace badge
}  // namespace extension
