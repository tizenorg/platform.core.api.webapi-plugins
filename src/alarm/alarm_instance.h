// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ALARM_ALARM_INSTANCE_H_
#define ALARM_ALARM_INSTANCE_H_

#include "common/extension.h"

#include "alarm/alarm_manager.h"

namespace extension {
namespace alarm {

class AlarmInstance: public common::ParsedInstance {
 public:
  AlarmInstance();
  virtual ~AlarmInstance();

 private:
  AlarmManager manager_;
};

} // namespace alarm
} // namespace extension

#endif // ALARM_ALARM_INSTANCE_H_
