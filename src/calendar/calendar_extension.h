// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CALENDAR_CALENDAR_EXTENSION_H_
#define CALENDAR_CALENDAR_EXTENSION_H_

#include "common/extension.h"

class CalendarExtension : public common::Extension {
 public:
  CalendarExtension();
  virtual ~CalendarExtension();

 private:
  virtual common::Instance* CreateInstance();
};

#endif // CALENDAR_CALENDAR_EXTENSION_H_
