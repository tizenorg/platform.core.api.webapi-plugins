// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
