// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FILESYSTEM_FILESYSTEM_EXTENSION_H_
#define FILESYSTEM_FILESYSTEM_EXTENSION_H_

#include "common/extension.h"

class FilesystemExtension : public common::Extension {
 public:
  FilesystemExtension();
  virtual ~FilesystemExtension();

 private:
  virtual common::Instance* CreateInstance();
};

#endif // FILESYSTEM_FILESYSTEM_EXTENSION_H_
