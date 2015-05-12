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

#ifndef SRC_INPUTDEVICE_INPUTDEVICE_KEY_H_
#define SRC_INPUTDEVICE_INPUTDEVICE_KEY_H_

#include <sys/types.h>
#include <string>
#include <memory>

namespace extension {
namespace inputdevice {

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

}  // namespace inputdevice
}  // namespace extension

#endif  // SRC_INPUTDEVICE_INPUTDEVICE_KEY_H_
