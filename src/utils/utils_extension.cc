// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/logger.h"
#include "utils/utils_extension.h"
#include "utils/utils_instance.h"

// This will be generated from tizen_api.js.
extern const char kSource_utils_api[];

common::Extension* CreateExtension() {
  LoggerD("Entered");
  return new UtilsExtension;
}

UtilsExtension::UtilsExtension() {
  LoggerD("Entered");
  SetExtensionName("xwalk.utils");
  SetJavaScriptAPI(kSource_utils_api);
}

UtilsExtension::~UtilsExtension() {
  LoggerD("Entered");
}

common::Instance* UtilsExtension::CreateInstance() {
  LoggerD("Entered");
  return new extension::utils::UtilsInstance();
}
