// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "alarm_instance.h"

#include "alarm_manager.h"
#include "common/picojson.h"
#include "common/logger.h"

namespace extension {
namespace alarm {

using namespace common;

namespace {
AlarmManager* alarm_manager = &AlarmManager::GetInstance();
}

AlarmInstance& AlarmInstance::GetInstance() {
  static AlarmInstance instance;
  return instance;
}

AlarmInstance::AlarmInstance() {
  LoggerD("Entered");
  using namespace std::placeholders;

  RegisterSyncHandler("AlarmManager_add",
                      std::bind(&AlarmManager::Add, alarm_manager, _1, _2));
  RegisterSyncHandler("AlarmManager_remove",
                      std::bind(&AlarmManager::Remove, alarm_manager, _1, _2));
  RegisterSyncHandler("AlarmManager_removeAll",
                      std::bind(&AlarmManager::RemoveAll, alarm_manager, _1, _2));
  RegisterSyncHandler("AlarmManager_get",
                      std::bind(&AlarmManager::Get, alarm_manager, _1, _2));
  RegisterSyncHandler("AlarmManager_getAll",
                      std::bind(&AlarmManager::GetAll, alarm_manager, _1, _2));
  //AlarmRelative
  RegisterSyncHandler("AlarmRelative_getRemainingSeconds",
                      std::bind(&AlarmManager::GetRemainingSeconds, alarm_manager, _1, _2));
  //AlarmAbsolute
  RegisterSyncHandler("AlarmAbsolute_getNextScheduledDate",
                      std::bind(&AlarmManager::GetNextScheduledDate, alarm_manager, _1, _2));
}

AlarmInstance::~AlarmInstance() {
  LoggerD("Entered");
}

} // namespace Alarm
} // namespace extension
