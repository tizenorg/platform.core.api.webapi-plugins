// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "notification/notification_instance.h"

#include <notification.h>

#include <string>
#include <functional>

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"
#include "common/typeutil.h"

namespace extension {
namespace notification {

namespace {
// The privileges that required in Notification API
const std::string kPrivilegeNotification = "";

}  // namespace

using common::TypeMismatchException;

NotificationInstance::NotificationInstance() {
  using std::placeholders::_1;
  using std::placeholders::_2;
  #define REGISTER_SYNC(c, x) \
    RegisterSyncHandler(c, std::bind(&NotificationInstance::x, this, _1, _2));
  REGISTER_SYNC("NotificationManager_get", NotificationManagerGet);
  REGISTER_SYNC("NotificationManager_update", NotificationManagerUpdate);
  REGISTER_SYNC("NotificationManager_remove", NotificationManagerRemove);
  REGISTER_SYNC("NotificationManager_getAll", NotificationManagerGetall);
  REGISTER_SYNC("NotificationManager_post", NotificationManagerPost);
  REGISTER_SYNC("NotificationManager_removeAll", NotificationManagerRemoveall);
  #undef REGISTER_SYNC
}

NotificationInstance::~NotificationInstance() {
}



#define CHECK_EXIST(args, name, out) \
    if (!args.contains(name)) {\
      ReportError(TypeMismatchException(name" is required argument"), out);\
      return;\
    }

using common::WIDLTypeValidator::WIDLType;
using common::WIDLTypeValidator::IsType;

void NotificationInstance::NotificationManagerPost(
    const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "id", out)
  CHECK_EXIST(args, "type", out)
  CHECK_EXIST(args, "title", out)

  bool check;
  check = IsType<WIDLType::StringType>(args, "id");
  check = IsType<WIDLType::StringType>(args, "type");
  check = IsType<WIDLType::StringType>(args, "title");

  notification_type_e noti_type = NOTIFICATION_TYPE_NOTI;

  notification_h noti = notification_create(noti_type);
}
void NotificationInstance::NotificationManagerUpdate(
    const picojson::value& args, picojson::object& out) {
}
void NotificationInstance::NotificationManagerRemove(
    const picojson::value& args, picojson::object& out) {
}
void NotificationInstance::NotificationManagerRemoveall(
    const picojson::value& args, picojson::object& out) {
}
void NotificationInstance::NotificationManagerGet(
    const picojson::value& args, picojson::object& out) {
}
void NotificationInstance::NotificationManagerGetall(
    const picojson::value& args, picojson::object& out) {
}


#undef CHECK_EXIST

}  // namespace notification
}  // namespace extension
