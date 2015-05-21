// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
