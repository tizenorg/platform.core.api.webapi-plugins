// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BADGE_BADGE_EXTENSION_H_
#define BADGE_BADGE_EXTENSION_H_

#include "common/extension.h"

class BadgeExtension : public common::Extension {
 public:
  BadgeExtension();
  virtual ~BadgeExtension();

 private:
  virtual common::Instance* CreateInstance();
};

#endif  // BADGE_BADGE_EXTENSION_H_
