// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BADGE_BADGE_MANAGER_H_
#define BADGE_BADGE_MANAGER_H_

#include <string>

namespace extension {
namespace badge {

class BadgeManager {
 public:
  BadgeManager();
  ~BadgeManager();
  BadgeManager* GetInstance();

  void setBadgeCount(std::string appId, long count);
  long getBadgeCount(std::string appId);
};

}  // namespace badge
}  // namespace extension

#endif  // BADGE_BADGE_MANAGER_H_
