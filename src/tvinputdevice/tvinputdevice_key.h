// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_TVINPUTDEVICE_TVINPUTDEVICE_KEY_H_
#define SRC_TVINPUTDEVICE_TVINPUTDEVICE_KEY_H_

#include <sys/types.h>
#include <string>
#include <memory>

namespace extension {
namespace tvinputdevice {

class InputDeviceKey {
 public:
    InputDeviceKey();

    InputDeviceKey(std::string name, int32_t code);

    virtual ~InputDeviceKey();

    std::string getName() const;
    void setName(std::string name);

    int32_t getCode() const;
    void setCode(int32_t code);

 private:
    std::string m_name;
    int32_t m_code;
};

typedef std::shared_ptr<InputDeviceKey> InputDeviceKeyPtr;

}  // namespace tvinputdevice
}  // namespace extension

#endif  // SRC_TVINPUTDEVICE_TVINPUTDEVICE_KEY_H_
