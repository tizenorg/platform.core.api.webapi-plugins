// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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