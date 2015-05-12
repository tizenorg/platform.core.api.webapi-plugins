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
 

#include "../inputdevice/inputdevice_manager.h"
#include <sys/types.h>
#include <algorithm>

#include "common/logger.h"
#include "common/platform_exception.h"


namespace extension {
namespace inputdevice {

using common::UnknownException;
using common::InvalidValuesException;

InputDeviceManager::InputDeviceManager() {
    LOGD("Enter");
    setSupportedKeys();
}

InputDeviceManager::~InputDeviceManager() {
    LOGD("Enter");
    cleanSupportedKeys();
}

InputDeviceManager& InputDeviceManager::getInstance() {
    LOGD("Enter");
    static InputDeviceManager instance;
    return instance;
}

void InputDeviceManager::cleanSupportedKeys() {
    LOGD("Enter");
    m_availableKeys.clear();
}

void InputDeviceManager::setSupportedKeys() {
    LOGD("Entered");
    cleanSupportedKeys();
    InputDeviceKeyPtr key(new InputDeviceKey());
    m_availableKeys.push_back(key);
}

InputDeviceKeyPtr InputDeviceManager::getKey(
        std::string const& keyName) const {
    LOGD("Enter");
    auto it = std::find_if(m_availableKeys.begin(), m_availableKeys.end(),
        [ keyName ](InputDeviceKeyPtr _pKey)->bool{
            if (_pKey->getName() == keyName) {
                return true;
            } else {
                return false;
            }
        });

    if (it != m_availableKeys.end()) {
      return *it;
    } else {
        return NULL;
    }
}
void InputDeviceManager::registerKey(std::string const& keyName) const {
    LOGD("Enter");
}
void InputDeviceManager::unregisterKey(std::string const& keyName) const {
    LOGD("Enter");
}

std::vector<InputDeviceKeyPtr> InputDeviceManager::getSupportedKeys() const {
    LOGD("Enter");
    return m_availableKeys;
}

}  // namespace inputdevice
}  // namespace extension
