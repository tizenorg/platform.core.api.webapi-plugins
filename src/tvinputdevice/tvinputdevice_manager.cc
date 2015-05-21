// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
