// Copyright (c) 2014 Samsung Electronics Co., Ltd. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXIF_EXIF_EXTENSION_H_
#define EXIF_EXIF_EXTENSION_H_

#include "common/extension.h"

class ExifExtension : public common::Extension {
 public:
  ExifExtension();
  virtual ~ExifExtension();

 private:
  // common::Extension implementation.
  virtual common::Instance* CreateInstance();
};

#endif  // EXIF_EXIF_EXTENSION_H_
