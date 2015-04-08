// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "bluetooth_util.h"

using namespace common;

namespace extension {
namespace bluetooth {
namespace util {

namespace {
const char* JSON_CALLBACK_ID = "callbackId";
} // namespace

double GetAsyncCallbackHandle(const picojson::value& data) {
  return data.get(JSON_CALLBACK_ID).get<double>();
}

const picojson::object& GetArguments(const picojson::value& data) {
  return data.get<picojson::object>();
}

} // util
} // bluetooth
} // extension
