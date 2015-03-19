// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "badge/badge_instance.h"

#include "common/converter.h"

namespace extension {
namespace badge {

namespace {
// The privileges that required in Badge API
const std::string kPrivilegeNotification = "http://tizen.org/privilege/notification";

}  // namespace

using namespace common;
using namespace extension::badge;

BadgeInstance& BadgeInstance::GetInstance() {
  static BadgeInstance instance;
  return instance;
}

BadgeInstance::BadgeInstance() {
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

BadgeInstance::~BadgeInstance() {}

void BadgeInstance::BadgeManagerSetBadgeCount(const JsonValue& args,
                                              JsonObject& out) {
  CHECK_PRIVILEGE_ACCESS(kPrivilegeNotification, &out);
  std::string app_id =
      common::FromJson<std::string>(args.get<JsonObject>(), "appId");
  const double count = args.get("count").get<double>();

  PlatformResult status = BadgeManager::GetInstance()->SetBadgeCount(
      app_id, static_cast<unsigned int>(count));
  if (status.IsSuccess())
    ReportSuccess(out);
  else
    ReportError(status, &out);
}

void BadgeInstance::BadgeManagerGetBadgeCount(const JsonValue& args,
                                              JsonObject& out) {
  CHECK_PRIVILEGE_ACCESS(kPrivilegeNotification, &out);
  std::string app_id =
      common::FromJson<std::string>(args.get<JsonObject>(), "appId");

  unsigned int count = 0;
  PlatformResult status =
      BadgeManager::GetInstance()->GetBadgeCount(app_id, &count);
  if (status.IsSuccess())
    ReportSuccess(JsonValue(std::to_string(count)), out);
  else
    ReportError(status, &out);
}

void BadgeInstance::BadgeManagerAddChangeListener(const JsonValue& args,
                                                  JsonObject& out) {
  CHECK_PRIVILEGE_ACCESS(kPrivilegeNotification, &out);
  PlatformResult status =
      BadgeManager::GetInstance()->AddChangeListener(args.get<JsonObject>());

  if (status.IsSuccess())
    ReportSuccess(out);
  else
    ReportError(status, &out);
}

void BadgeInstance::BadgeManagerRemoveChangeListener(const JsonValue& args,
                                                     JsonObject& out) {
  CHECK_PRIVILEGE_ACCESS(kPrivilegeNotification, &out);
  PlatformResult status =
      BadgeManager::GetInstance()->RemoveChangeListener(args.get<JsonObject>());

  if (status.IsSuccess())
    ReportSuccess(out);
  else
    ReportError(status, &out);
}

}  // namespace badge
}  // namespace extension
