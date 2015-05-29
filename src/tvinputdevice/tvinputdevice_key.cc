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
 
#include "common/logger.h"
#include "common/platform_exception.h"

#include "tvinputdevice/tvinputdevice_key.h"

namespace extension {
namespace tvinputdevice {

InputDeviceKey::InputDeviceKey():
    m_code(0), m_name("") {
    LOGD("Enter");
}

InputDeviceKey::InputDeviceKey(std::string name, int32_t code):
        m_name(name), m_code(code) {
    LOGD("Key Name %s", m_name.c_str() );
}

InputDeviceKey::~InputDeviceKey() {
    LOGD("Enter");
}

std::string InputDeviceKey::getName() const {
    LOGD("Enter");
    LOGD("Key Name %s", m_name.c_str() );
    return m_name;
}

void InputDeviceKey::setName(std::string name) {
    LOGD("Key Name %s", name.c_str() );
    m_name = name;
}

int32_t InputDeviceKey::getCode() const {
    return m_code;
}

void InputDeviceKey::setCode(int32_t code) {
    m_code = code;
}


}  // namespace tvinputdevice
}  // namespace extension
