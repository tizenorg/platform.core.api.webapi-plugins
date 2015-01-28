// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <notification.h>

#include <string>
#include <functional>

#include "notification/notification_instance.h"
#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"
#include "common/typeutil.h"
#include "common/scope_exit.h"

namespace extension {
namespace notification {

namespace {
// The privileges that required in Notification API
const std::string kPrivilegeNotification = "";

}  // namespace

using common::UnknownException;
using common::TypeMismatchException;
using common::ScopeExit;
using common::operator+;

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

using common::WIDLTypeValidator::WIDLType;
using common::WIDLTypeValidator::IsType;

#define WIDL_TYPE_CHECK(args, name, wtype, out) \
    do { if (!IsType<wtype>(args, name)) {\
      ReportError(TypeMismatchException(name" is not valid type."), out);\
    }} while (0)

void NotificationInstance::NotificationManagerPost(
    const picojson::value& args, picojson::object& out) {

  WIDL_TYPE_CHECK(args, "type", WIDLType::StringType, out);
  WIDL_TYPE_CHECK(args, "title", WIDLType::StringType, out);

  int err;
  notification_type_e noti_type = NOTIFICATION_TYPE_NOTI;
  notification_h noti = notification_create(noti_type);
  SCOPE_EXIT {
    notification_free(noti);
  };

  /*
  if (IsType<WIDLType::StringType>(args, "iconPath")) {
    err = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON,
                           args.get("iconPath").get<std::string>().c_str());
    if (err != NOTIFICATION_ERROR_NONE) {
      LoggerE("Fail to set icon path. [%s]", get_error_message(err));
      return;
    }
    LoggerD("iconPath : %s", args.get("iconPath").get<std::string>().c_str());
  }
  */

  const std::string& title = args.get("title").get<std::string>();
  err = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE,
                              title.c_str(),
                              NULL,
                              NOTIFICATION_VARIABLE_TYPE_NONE);
  if (err != NOTIFICATION_ERROR_NONE) {
    LoggerE("Fail to set title. [%s]", get_error_message(err));
    return;
  }

  if (IsType<WIDLType::StringType>(args, "content")) {
    const std::string& content = args.get("content").get<std::string>();
    err = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT,
                                content.c_str(),
                                NULL,
                                NOTIFICATION_VARIABLE_TYPE_NONE);

    if (err != NOTIFICATION_ERROR_NONE) {
      LoggerE("Fail to set content. [%s]", get_error_message(err));
      return;
    }
  }

  err = notification_post(noti);
  if (err != NOTIFICATION_ERROR_NONE) {
    ReportError(UnknownException("failed to post notification"), out);
  }
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


#undef WIDL_TYPE_CHECK

}  // namespace notification
}  // namespace extension
