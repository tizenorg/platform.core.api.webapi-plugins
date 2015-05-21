// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "alarm_instance.h"

#include "common/picojson.h"
#include "common/logger.h"

namespace extension {
namespace alarm {

using namespace common;

AlarmInstance::AlarmInstance() {
  LoggerD("Entered");
  using namespace std::placeholders;

  RegisterSyncHandler("AlarmManager_add",
                      std::bind(&AlarmManager::Add, &manager_, _1, _2));
  RegisterSyncHandler("AlarmManager_remove",
                      std::bind(&AlarmManager::Remove, &manager_, _1, _2));
  RegisterSyncHandler("AlarmManager_removeAll",
                      std::bind(&AlarmManager::RemoveAll, &manager_, _1, _2));
  RegisterSyncHandler("AlarmManager_get",
                      std::bind(&AlarmManager::Get, &manager_, _1, _2));
  RegisterSyncHandler("AlarmManager_getAll",
                      std::bind(&AlarmManager::GetAll, &manager_, _1, _2));
  //AlarmRelative
  RegisterSyncHandler("AlarmRelative_getRemainingSeconds",
                      std::bind(&AlarmManager::GetRemainingSeconds, &manager_, _1, _2));
  //AlarmAbsolute
  RegisterSyncHandler("AlarmAbsolute_getNextScheduledDate",
                      std::bind(&AlarmManager::GetNextScheduledDate, &manager_, _1, _2));
}

AlarmInstance::~AlarmInstance() {
  LoggerD("Entered");
}

} // namespace Alarm
} // namespace extension
