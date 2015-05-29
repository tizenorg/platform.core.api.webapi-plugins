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
 
#include <sys/types.h>
#include <algorithm>

#include "common/logger.h"
#include "common/platform_exception.h"

#include "tvinputdevice/tvinputdevice_manager.h"

namespace extension {
namespace tvinputdevice {

using common::UnknownException;
using common::InvalidValuesException;

TVInputDeviceManager::TVInputDeviceManager() {
    LOGD("Enter");
    setSupportedKeys();
}

TVInputDeviceManager::~TVInputDeviceManager() {
    LOGD("Enter");
    cleanSupportedKeys();
}

TVInputDeviceManager& TVInputDeviceManager::getInstance() {
    LOGD("Enter");
    static TVInputDeviceManager instance;
    return instance;
}

void TVInputDeviceManager::cleanSupportedKeys() {
    LOGD("Enter");
    m_availableKeys.clear();
}

void TVInputDeviceManager::setSupportedKeys() {
    LOGD("Entered");
    cleanSupportedKeys();
    InputDeviceKeyPtr key(new InputDeviceKey());
    m_availableKeys.push_back(key);
}

InputDeviceKeyPtr TVInputDeviceManager::getKey(
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
void TVInputDeviceManager::registerKey(std::string const& keyName) const {
    LOGD("Enter");
}
void TVInputDeviceManager::unregisterKey(std::string const& keyName) const {
    LOGD("Enter");
}

std::vector<InputDeviceKeyPtr> TVInputDeviceManager::getSupportedKeys() const {
    LOGD("Enter");
    return m_availableKeys;
}

}  // namespace tvinputdevice
}  // namespace extension
