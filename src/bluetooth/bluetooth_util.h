// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BLUETOOTH_BLUETOOTH_UTIL_H_
#define BLUETOOTH_BLUETOOTH_UTIL_H_

#include <memory>
#include <string>

#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace bluetooth {
namespace util {

double GetAsyncCallbackHandle(const picojson::value& data);

const picojson::object& GetArguments(const picojson::value& data);

} // util
} // bluetooth
} // extension

#endif // BLUETOOTH_BLUETOOTH_UTIL_H_
