// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NETWORKBEARERSELECTION_NETWORKBEARERSELECTION_EXTENSION_H_
#define NETWORKBEARERSELECTION_NETWORKBEARERSELECTION_EXTENSION_H_

#include "common/extension.h"

class NetworkBearerSelectionExtension : public common::Extension {
 public:
  NetworkBearerSelectionExtension();
  virtual ~NetworkBearerSelectionExtension();

 private:
  virtual common::Instance* CreateInstance();
};

#endif // NETWORKBEARERSELECTION_NETWORKBEARERSELECTION_EXTENSION_H_
