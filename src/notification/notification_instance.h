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

  void NotificationManagerPlayLEDCustomEffect(const picojson::value& args,
                                              picojson::object& out);
  void NotificationManagerStopLEDCustomEffect(const picojson::value& args,
                                              picojson::object& out);
};

}  // namespace notification
}  // namespace extension

#endif  // NOTIFICATION_NOTIFICATION_INSTANCE_H_
