// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BADGE_BADGE_MANAGER_H_
#define BADGE_BADGE_MANAGER_H_

#include <string>
#include <set>

#include "common/logger.h"
#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace badge {

typedef picojson::value JsonValue;
typedef picojson::object JsonObject;
typedef picojson::array JsonArray;
typedef std::string JsonString;

class BadgeManager {
 public:
  static BadgeManager* GetInstance();

  common::PlatformResult setBadgeCount(std::string appId, unsigned int count);
  common::PlatformResult getBadgeCount(std::string appId, unsigned int* count);
  common::PlatformResult addChangeListener(const JsonObject& obj);
  void removeChangeListener(const JsonObject& obj);
  static void badge_changed_cb(unsigned int, const char*, unsigned int, void*);

 private:
  BadgeManager();
  virtual ~BadgeManager();

  common::PlatformResult checkPermisionForCreatingBadge(const char* appId);
  char* getPkgnameByAppid(const char* appId);
  char* getPkgnameByPid();
  int isSameCertInfo(const char *caller, const char *pkgname);
  bool isAppInstalled(const std::string& appId);

  static bool is_cb_registered_;
  static std::set<std::string> watched_applications_;
};

}  // namespace badge
}  // namespace extension

#endif  // BADGE_BADGE_MANAGER_H_
