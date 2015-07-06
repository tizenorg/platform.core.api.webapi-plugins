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

#ifndef COMMON_ASSERT_H_
#define COMMON_ASSERT_H_

#include <cassert>

#include "common/logger.h"

#define AssertMsg(condition, message)                                          \
  do {                                                                         \
    std::string msg(message);                                                  \
    if (!(condition))                                                          \
      LoggerE("assert(%s)%s", #condition,                                      \
            !msg.empty() ? std::string(", message: " + msg).c_str() : "");     \
    assert(!!(condition) && message);                                          \
  } while(0)

#define Assert(condition) AssertMsg(condition, "")

#endif  // COMMON_ASSERT_H_
