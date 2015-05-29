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
 
#include <functional>
#include <string>
#include <vector>

#include "common/logger.h"

#include "tvinputdevice/tvinputdevice_instance.h"
#include "tvinputdevice/tvinputdevice_manager.h"

namespace extension {
namespace tvinputdevice {

TVInputDeviceInstance::TVInputDeviceInstance() {
    LOGD("Enter");
    using std::placeholders::_1;
    using std::placeholders::_2;
    #define REGISTER_SYNC(c, x) \
    RegisterSyncHandler(c, std::bind(&TVInputDeviceInstance::x, this, _1, _2));
    REGISTER_SYNC("TVInputDeviceManager_getSupportedKeys", getSupportedKeys);
    REGISTER_SYNC("TVInputDeviceManager_getKey", getKey);
    REGISTER_SYNC("TVInputDeviceManager_registerKey", registerKey);
    REGISTER_SYNC("TVInputDeviceManager_unregisterKey", unregisterKey);
    #undef REGISTER_SYNC
}

TVInputDeviceInstance::~TVInputDeviceInstance() {
    LOGD("Enter");
}

picojson::value TVInputDeviceInstance::inputDeviceKeyToJson(
        const InputDeviceKeyPtr keyPtr) {
    LOGD("Enter");
    picojson::value::object keyMap;
    keyMap.insert(
        std::make_pair("name",
        picojson::value(keyPtr->getName())));
    keyMap.insert(
        std::make_pair("code",
        picojson::value(static_cast<double>(keyPtr->getCode()))));
    return picojson::value(keyMap);
}

void TVInputDeviceInstance::getSupportedKeys(const picojson::value& args,
        picojson::object& out) {
    LOGD("Enter");
    std::vector<InputDeviceKeyPtr> inputDeviceKeys =
            TVInputDeviceManager::getInstance().getSupportedKeys();
    picojson::value::array picjsonValuesArray;
    for (auto it = inputDeviceKeys.begin(); it != inputDeviceKeys.end(); ++it) {
        picjsonValuesArray.push_back(inputDeviceKeyToJson(*it));
    }
    ReportSuccess(picojson::value(picjsonValuesArray), out);
}

void TVInputDeviceInstance::getKey(const picojson::value& args,
        picojson::object& out) {
    LOGD("Enter");
    std::string keyName = args.get("keyName").get<std::string>();
    InputDeviceKeyPtr keyPtr =
            TVInputDeviceManager::getInstance().getKey(keyName);
    ReportSuccess(inputDeviceKeyToJson(keyPtr), out);
}

void TVInputDeviceInstance::registerKey(const picojson::value& args,
        picojson::object& out) {
    LOGD("Enter");
    std::string keyName = args.get("keyName").get<std::string>();
    TVInputDeviceManager::getInstance().registerKey(keyName);
    ReportSuccess(out);
}

void TVInputDeviceInstance::unregisterKey(const picojson::value& args,
        picojson::object& out) {
    LOGD("Enter");
    std::string keyName = args.get("keyName").get<std::string>();
    TVInputDeviceManager::getInstance().unregisterKey(keyName);
    ReportSuccess(out);
}

}  // namespace tvinputdevice
}  // namespace extension
