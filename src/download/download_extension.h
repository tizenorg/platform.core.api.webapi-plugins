// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DOWNLOAD_DOWNLOAD_EXTENSION_H_
#define DOWNLOAD_DOWNLOAD_EXTENSION_H_

#include "common/extension.h"

class DownloadExtension : public common::Extension {
 public:
  DownloadExtension();
  virtual ~DownloadExtension();

 private:
  virtual common::Instance* CreateInstance();
};

#endif  // DOWNLOAD_DOWNLOAD_EXTENSION_H_
