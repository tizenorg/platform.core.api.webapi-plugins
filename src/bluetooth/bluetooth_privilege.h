/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
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

#ifndef BLUETOOTH_BLUETOOTH_PRIVILEGE_H_
#define BLUETOOTH_BLUETOOTH_PRIVILEGE_H_

#include <string>

namespace extension {
namespace bluetooth {

namespace Privilege {

extern const std::string kBluetoothAdmin;
extern const std::string kBluetoothManager;
extern const std::string kBluetoothGap;
extern const std::string kBluetoothSpp;
extern const std::string kBluetoothHealth;

} // namespace Privilege

} // namespace bluetooth
} // namespace extension

#endif // BLUETOOTH_BLUETOOTH_PRIVILEGE_H_
