// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "badge/badge_instance.h"

#include "common/converter.h"

namespace extension {
namespace badge {

namespace {
// The privileges that required in Badge API
const std::string kPrivilegeBadge = "http://tizen.org/privilege/badge";

}  // namespace

using namespace common;
using namespace extension::badge;

BadgeInstance::BadgeInstance() {
  using namespace std::placeholders;
#define REGISTER_SYNC(c, x) \
  RegisterSyncHandler(c, std::bind(&BadgeInstance::x, this, _1, _2));

  REGISTER_SYNC("Badge_setBadgeCount", setBadgeCount);
  REGISTER_SYNC("Badge_getBadgeCount", getBadgeCount);

#undef REGISTER_SYNC
}

BadgeInstance::~BadgeInstance() {}

void BadgeInstance::setBadgeCount(const JsonValue& args, JsonObject& out) {
  std::string appId = common::FromJson<std::string>(args.get<JsonObject>(), "appId");
  const double count = args.get("count").get<double>();
  BadgeManager::GetInstance()->setBadgeCount(appId, (unsigned int)count);
  ReportSuccess(out);
}

void BadgeInstance::getBadgeCount(const JsonValue& args, JsonObject& out) {
  std::string appId = common::FromJson<std::string>(args.get<JsonObject>(), "appId");
  unsigned int count = BadgeManager::GetInstance()->getBadgeCount(appId);
  ReportSuccess(JsonValue(std::to_string(count)), out);
}

}  // namespace badge
}  // namespace extension
