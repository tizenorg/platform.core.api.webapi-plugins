// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_APPLICATION_APPLICATION_EXTENSION_H_
#define SRC_APPLICATION_APPLICATION_EXTENSION_H_

#include <string>

#include "common/extension.h"

class ApplicationExtension : public common::Extension {
 public:
  explicit ApplicationExtension(const std::string& app_id);
  virtual ~ApplicationExtension();

 private:
  std::string app_id_;
  // common::Extension implementation.
  virtual common::Instance* CreateInstance();
};

#endif  // SRC_APPLICATION_APPLICATION_EXTENSION_H_
