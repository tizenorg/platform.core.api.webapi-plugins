// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "calendar/calendar_extension.h"

#include "calendar/calendar_instance.h"

// This will be generated from calendar_api.js
extern const char kSource_calendar_api[];

common::Extension* CreateExtension() {
  return new CalendarExtension;
}

CalendarExtension::CalendarExtension() {
  SetExtensionName("tizen.calendar");
  SetJavaScriptAPI(kSource_calendar_api);

  const char* entry_points[] = {
    "tizen.Calendar",
    "tizen.CalendarEventId",
    "tizen.CalendarEvent",
    "tizen.CalendarTask",
    "tizen.CalendarAlarm",
    "tizen.CalendarAttendee",
    "tizen.CalendarRecurrenceRule",
    NULL
  };
  SetExtraJSEntryPoints(entry_points);
}

CalendarExtension::~CalendarExtension() {}

common::Instance* CalendarExtension::CreateInstance() {
//  return new CalendarInstance;
}
