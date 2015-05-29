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

#ifndef NOTIFICATION_NOTIFICATION_MANAGER_H_
#define NOTIFICATION_NOTIFICATION_MANAGER_H_

#include <notification.h>

#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace notification {

class NotificationManager {
 public:
  static NotificationManager* GetInstance();

  common::PlatformResult Post(const picojson::object& args,
                              picojson::object& out);
  common::PlatformResult Update(const picojson::object& args);
  common::PlatformResult Remove(const picojson::object& args);
  common::PlatformResult RemoveAll();
  common::PlatformResult Get(const picojson::object& args,
                             picojson::object& out);
  common::PlatformResult GetAll(picojson::array& out);

  common::PlatformResult PlayLEDCustomEffect(const picojson::object& args);
  common::PlatformResult StopLEDCustomEffect();

 private:
  NotificationManager();
  virtual ~NotificationManager();
};

}  // namespace notification
}  // namespace extension

#endif /* NOTIFICATION_NOTIFICATION_MANAGER_H_ */
