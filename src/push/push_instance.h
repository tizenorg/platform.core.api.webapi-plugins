// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_PUSH_PUSH_INSTANCE_H_
#define SRC_PUSH_PUSH_INSTANCE_H_

#include <string>
#include "common/extension.h"
#include "push/push_manager.h"

namespace extension {
namespace push {

class PushInstance: public common::ParsedInstance, public EventListener {
 public:
    PushInstance();
    virtual ~PushInstance();
    virtual void onPushRegister(double callbackId,
            common::PlatformResult result, const std::string& id);
 private:
     void registerService(const picojson::value& args, picojson::object& out);
     void unregisterService(const picojson::value& args, picojson::object& out);
     void connectService(const picojson::value& args, picojson::object& out);
     void disconnectService(const picojson::value& args, picojson::object& out);
     void getRegistrationId(const picojson::value& args, picojson::object& out);
     void getUnreadNotifications(const picojson::value& args,
            picojson::object& out);
};

}  // namespace push
}  // namespace extension

#endif  // SRC_PUSH_PUSH_INSTANCE_H_
