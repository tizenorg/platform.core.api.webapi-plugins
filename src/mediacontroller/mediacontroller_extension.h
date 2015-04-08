// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIACONTROLLER_MEDIACONTROLLER_EXTENSION_H_
#define MEDIACONTROLLER_MEDIACONTROLLER_EXTENSION_H_

#include "common/extension.h"

class MediaControllerExtension : public common::Extension {
 public:
  MediaControllerExtension();
  virtual ~MediaControllerExtension();

 private:
  virtual common::Instance* CreateInstance();
};

#endif  // MEDIACONTROLLER_MEDIACONTROLLER_EXTENSION_H_
