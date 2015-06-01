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
#include "native-plugin.h"
#include "calendar_manager.h"
#include "calendar.h"

namespace webapi {
namespace calendar {

using namespace webapi::common;

class CalendarPlugin : public NativePlugin {
 public:
  CalendarPlugin();
  ~CalendarPlugin();
  virtual void OnLoad();

 private:
  CalendarManager* manager_;
  Calendar* calendar_;
};

EXPORT_NATIVE_PLUGIN(webapi::calendar::CalendarPlugin);

CalendarPlugin::CalendarPlugin() {
  manager_ = &CalendarManager::GetInstance();
  calendar_ = &Calendar::GetInstance();
}

CalendarPlugin::~CalendarPlugin() { manager_ = nullptr; }

void CalendarPlugin::OnLoad() {
  using std::placeholders::_1;
  using std::placeholders::_2;

  dispatcher_.AddFunction(
      "CalendarManager_getCalendars",
      std::bind(&CalendarManager::GetCalendars, manager_, _1, _2));

  dispatcher_.AddFunction(
      "CalendarManager_getCalendar",
      std::bind(&CalendarManager::GetCalendar, manager_, _1, _2));

  dispatcher_.AddFunction(
      "CalendarManager_addCalendar",
      std::bind(&CalendarManager::AddCalendar, manager_, _1, _2));

  dispatcher_.AddFunction(
      "CalendarManager_removeCalendar",
      std::bind(&CalendarManager::RemoveCalendar, manager_, _1, _2));

  dispatcher_.AddFunction("Calendar_get",
                          std::bind(&Calendar::Get, calendar_, _1, _2));

  dispatcher_.AddFunction("Calendar_add",
                          std::bind(&Calendar::Add, calendar_, _1, _2));

  dispatcher_.AddFunction("Calendar_addBatch",
                          std::bind(&Calendar::AddBatch, calendar_, _1, _2));

  dispatcher_.AddFunction("Calendar_update",
                          std::bind(&Calendar::Update, calendar_, _1, _2));

  dispatcher_.AddFunction("Calendar_updateBatch",
                          std::bind(&Calendar::UpdateBatch, calendar_, _1, _2));

  dispatcher_.AddFunction("Calendar_remove",
                          std::bind(&Calendar::Remove, calendar_, _1, _2));

  dispatcher_.AddFunction("Calendar_removeBatch",
                          std::bind(&Calendar::RemoveBatch, calendar_, _1, _2));

  dispatcher_.AddFunction("Calendar_find",
                          std::bind(&Calendar::Find, calendar_, _1, _2));

  dispatcher_.AddFunction(
      "Calendar_addChangeListener",
      std::bind(&Calendar::AddChangeListener, calendar_, _1, _2));

  dispatcher_.AddFunction(
      "Calendar_removeChangeListener",
      std::bind(&Calendar::RemoveChangeListener, calendar_, _1, _2));
}

}  // namespace calendar
}  // namespace webapi
