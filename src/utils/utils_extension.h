// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UTILS_UTILS_EXTENSION_H_
#define UTILS_UTILS_EXTENSION_H_

#include "common/extension.h"

class UtilsExtension : public common::Extension {
 public:
  UtilsExtension();
  virtual ~UtilsExtension();

 private:
  virtual common::Instance* CreateInstance();
};

#endif  // UTILS_UTILS_EXTENSION_H_
