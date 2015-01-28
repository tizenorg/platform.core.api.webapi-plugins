// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// File copied from Crosswalk

#ifndef DATASYNC_DATASYNC_EXTENSION_H_
#define DATASYNC_DATASYNC_EXTENSION_H_

#include "common/extension.h"
#include "datasync/datasync_manager.h"

class DatasyncExtension : public common::Extension {
 public:
  DatasyncExtension();
  virtual ~DatasyncExtension();

  extension::datasync::DataSyncManager& manager();

 private:
  // common::Extension implementation.
  virtual common::Instance* CreateInstance();
};

#endif  // DATASYNC_DATASYNC_EXTENSION_H_
