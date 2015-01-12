// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BADGE_BADGE_MANAGER_H_
#define BADGE_BADGE_MANAGER_H_

#include <string>

#include "common/logger.h"
#include "common/picojson.h"

namespace extension {
namespace badge {

typedef picojson::value JsonValue;
typedef picojson::object JsonObject;
typedef picojson::array JsonArray;
typedef std::string JsonString;

class BadgeManager {
 public:
  static BadgeManager* GetInstance();

  void setBadgeCount(std::string appId, unsigned int count);
  unsigned int getBadgeCount(std::string appId);

 private:
  BadgeManager();
  virtual ~BadgeManager();

  void CheckErrorCode(int err);
  char* _badge_get_pkgname_by_appid(const char* appId);
  bool checkPermisionForCreatingBadge(const char* appId);
  char* _badge_get_pkgname_by_pid();
  int _badge_is_same_certinfo(const char *caller, const char *pkgname);

};

}  // namespace badge
}  // namespace extension

#endif  // BADGE_BADGE_MANAGER_H_
