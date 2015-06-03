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
 
#include "badge/badge_instance.h"

#include "common/converter.h"

namespace extension {
namespace badge {

using namespace common;
using namespace extension::badge;

BadgeInstance::BadgeInstance() : manager_(*this) {
  LoggerD("Enter");
  using std::placeholders::_1;
  using std::placeholders::_2;

#define REGISTER_SYNC(c, x) \
  RegisterSyncHandler(c, std::bind(&BadgeInstance::x, this, _1, _2));
  REGISTER_SYNC("BadgeManager_setBadgeCount", BadgeManagerSetBadgeCount);
  REGISTER_SYNC("BadgeManager_addChangeListener",
                BadgeManagerAddChangeListener);
  REGISTER_SYNC("BadgeManager_removeChangeListener",
                BadgeManagerRemoveChangeListener);
  REGISTER_SYNC("BadgeManager_getBadgeCount", BadgeManagerGetBadgeCount);
#undef REGISTER_SYNC
}

BadgeInstance::~BadgeInstance() {
  LoggerD("Enter");
}

void BadgeInstance::BadgeManagerSetBadgeCount(const JsonValue& args,
                                              JsonObject& out) {
  LoggerD("Enter");
  std::string app_id =
      common::FromJson<std::string>(args.get<JsonObject>(), "appId");
  const double count = args.get("count").get<double>();

  PlatformResult status = manager_.SetBadgeCount(
      app_id, static_cast<unsigned int>(count));
  if (status.IsSuccess())
    ReportSuccess(out);
  else
    ReportError(status, &out);
}

void BadgeInstance::BadgeManagerGetBadgeCount(const JsonValue& args,
                                              JsonObject& out) {
  LoggerD("Enter");
  std::string app_id =
      common::FromJson<std::string>(args.get<JsonObject>(), "appId");

  unsigned int count = 0;
  PlatformResult status =
      manager_.GetBadgeCount(app_id, &count);
  if (status.IsSuccess())
    ReportSuccess(JsonValue(std::to_string(count)), out);
  else
    ReportError(status, &out);
}

void BadgeInstance::BadgeManagerAddChangeListener(const JsonValue& args,
                                                  JsonObject& out) {
  LoggerD("Enter");
  PlatformResult status =
      manager_.AddChangeListener(args.get<JsonObject>());

  if (status.IsSuccess())
    ReportSuccess(out);
  else
    ReportError(status, &out);
}

void BadgeInstance::BadgeManagerRemoveChangeListener(const JsonValue& args,
                                                     JsonObject& out) {
  LoggerD("Enter");
  PlatformResult status =
      manager_.RemoveChangeListener(args.get<JsonObject>());

  if (status.IsSuccess())
    ReportSuccess(out);
  else
    ReportError(status, &out);
}

}  // namespace badge
}  // namespace extension
