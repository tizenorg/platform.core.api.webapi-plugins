// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NOTIFICATION_NOTIFICATION_INSTANCE_H_
#define NOTIFICATION_NOTIFICATION_INSTANCE_H_

#include "common/extension.h"
#include "notification/notification_manager.h"

namespace extension {
namespace notification {

class NotificationInstance : public common::ParsedInstance {
 public:
  NotificationInstance();
  virtual ~NotificationInstance();

 private:
  NotificationManager* manager_;

  void NotificationManagerPost(const picojson::value& args,
                               picojson::object& out);
  void NotificationManagerUpdate(const picojson::value& args,
                                 picojson::object& out);
  void NotificationManagerRemove(const picojson::value& args,
                                 picojson::object& out);
  void NotificationManagerRemoveAll(const picojson::value& args,
                                    picojson::object& out);
  void NotificationManagerGet(const picojson::value& args,
                              picojson::object& out);
  void NotificationManagerGetAll(const picojson::value& args,
                                 picojson::object& out);
};

}  // namespace notification
}  // namespace extension

#endif  // NOTIFICATION_NOTIFICATION_INSTANCE_H_
