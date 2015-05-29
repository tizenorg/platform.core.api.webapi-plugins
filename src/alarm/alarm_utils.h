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

#ifndef ALARM_ALARM_UTILS_H_
#define ALARM_ALARM_UTILS_H_

#include <app_control.h>

#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace alarm {
namespace util {

common::PlatformResult AppControlToService(const picojson::object& obj, app_control_h *app_control);
common::PlatformResult AppControlToServiceExtraData(const picojson::object& app_obj,
                                                    app_control_h *app_control);
} // util
} // alarm
} // extension

#endif // ALARM_ALARM_UTILS_H_
