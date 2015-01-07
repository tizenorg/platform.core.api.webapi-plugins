// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "badge/badge_instance.h"

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"

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

#undef REGISTER_SYNC
}

BadgeInstance::~BadgeInstance() {}

}  // namespace badge
}  // namespace extension
