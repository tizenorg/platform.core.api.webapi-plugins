// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_TVINPUTDEVICE_TVINPUTDEVICE_MANAGER_H_
#define SRC_TVINPUTDEVICE_TVINPUTDEVICE_MANAGER_H_

#include <vector>
#include <string>

#include "tvinputdevice/tvinputdevice_key.h"

namespace extension {
namespace tvinputdevice {

class TVInputDeviceManager {
 public:
    InputDeviceKeyPtr getKey(std::string const& keyName) const;

    void registerKey(std::string const& keyName) const;

    void unregisterKey(std::string const& keyName) const;

    std::vector<InputDeviceKeyPtr> getSupportedKeys() const;

    static TVInputDeviceManager& getInstance();

 private:
    TVInputDeviceManager();
    virtual ~TVInputDeviceManager();

    void setSupportedKeys();
    std::vector<InputDeviceKeyPtr> m_availableKeys;

    void cleanSupportedKeys();
};

}  // namespace tvinputdevice
}  // namespace extension

#endif  // SRC_TVINPUTDEVICE_TVINPUTDEVICE_MANAGER_H_
