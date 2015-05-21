// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CALLHISTORY_CALLHISTORY_EXTENSION_H_
#define CALLHISTORY_CALLHISTORY_EXTENSION_H_

#include "common/extension.h"

class CallHistoryExtension : public common::Extension {
 public:
  CallHistoryExtension();
  virtual ~CallHistoryExtension();

 private:
  virtual common::Instance* CreateInstance();
};

#endif // CALLHISTORY_CALLHISTORY_EXTENSION_H_
