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

class BadgeInstance;

class BadgeManager {
 public:
  explicit BadgeManager(BadgeInstance& instance);
  ~BadgeManager();

  common::PlatformResult SetBadgeCount(const std::string& app_id,
                                       unsigned int count);
  common::PlatformResult GetBadgeCount(const std::string& app_id,
                                       unsigned int* count);
  common::PlatformResult AddChangeListener(const JsonObject& obj);
  common::PlatformResult RemoveChangeListener(const JsonObject& obj);
  static void badge_changed_cb(unsigned int, const char*, unsigned int, void*);

 private:
  bool IsAppInstalled(const std::string& app_id);

  BadgeInstance& instance_;

  bool is_cb_registered_;
  std::set<std::string> watched_applications_;
};

}  // namespace badge
}  // namespace extension

#endif  // BADGE_BADGE_MANAGER_H_
