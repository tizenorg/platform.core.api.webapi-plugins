// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_TVINPUTDEVICE_TVINPUTDEVICE_INSTANCE_H_
#define SRC_TVINPUTDEVICE_TVINPUTDEVICE_INSTANCE_H_

#include "common/picojson.h"
#include "common/extension.h"

#include "tvinputdevice/tvinputdevice_manager.h"

namespace extension {
namespace tvinputdevice {

class TVInputDeviceInstance : public common::ParsedInstance {
 public:
    TVInputDeviceInstance();
    virtual ~TVInputDeviceInstance();

 private:
    picojson::value inputDeviceKeyToJson(const InputDeviceKeyPtr keyPtr);
    void getSupportedKeys(const picojson::value& args, picojson::object& out);
    void getKey(const picojson::value& args, picojson::object& out);
    void registerKey(const picojson::value& args, picojson::object& out);
    void unregisterKey(const picojson::value& args, picojson::object& out);
};

}  // namespace tvinputdevice
}  // namespace extension

#endif  // SRC_TVINPUTDEVICE_TVINPUTDEVICE_INSTANCE_H_
