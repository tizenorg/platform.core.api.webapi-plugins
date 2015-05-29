/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

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
