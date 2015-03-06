// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "alarm_manager.h"

#include <app.h>
#include <app_alarm.h>

#include "common/logger.h"
#include "common/converter.h"
#include "common/scope_exit.h"
#include "common/platform_result.h"

#include "alarm_instance.h"
#include "alarm_utils.h"

using namespace common;
using namespace common::tools;

namespace extension {
namespace alarm {

namespace {
const int kDateSize = 22; //"yyy mm dd hh mm ss dd" e.g 115 11 28 11 25 50 -1
const std::string kPrivilegeAlarm = "http://tizen.org/privilege/alarm";

const std::string kAlarmRelative = "AlarmRelative";
const std::string kAlarmAbsolute = "AlarmAbsolute";

const char* kAlarmAbsoluteRecurrenceTypeKey = "RECURRENCE";
const char* kAlarmAbsoluteReccurrenceTypeInterval = "INTERVAL";
const char* kAlarmAbsoluteReccurrenceTypeByDayValue = "BYDAYVALUE";
const char* kAlarmAbsoluteRecurrenceTypeNone = "NONE";
const char* kAlarmAbsoluteDateKey = "DATE";

const char* kAlarmKeyType = "TYPE";

const char* kAlarmTypeValueAbsolute = "ABSOLUTE";
const char* kAlarmTypeValueRelative = "RELATIVE";

const char* kAlarmRelativeDelayKey = "RELATIVE_DELAY";

const char* kSundayShort = "SU";
const char* kMondayShort = "MO";
const char* kTuesdayShort = "TU";
const char* kWednesdayShort = "WE";
const char* kThuesdayShort = "TH";
const char* kFridayShort = "FR";
const char* kSaturdayShort = "SA";
}

AlarmManager::AlarmManager() {
}

AlarmManager::~AlarmManager() {
}

AlarmManager& AlarmManager::GetInstance() {
  static AlarmManager instance;
  return instance;
}

void AlarmManager::Add(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  util::CheckAccess(kPrivilegeAlarm);

  if (!args.contains("alarm")) {
    LoggerE("Invalid parameter passed.");
    ReportError(PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid parameter passed."), &out);
    return;
  }
  const picojson::object& alarm = args.get("alarm").get<picojson::object>();

  std::string alarm_type;
  if (args.contains("type")) {
    alarm_type = args.get("type").get<std::string>();
  }

  std::string app_id;
  if (args.contains("applicationId")) {
    app_id = args.get("applicationId").get<std::string>();
  }

  app_control_h app_control = nullptr;
  SCOPE_EXIT {
    app_control_destroy(app_control);
  };

  if (args.contains("appControl")) {
    PlatformResult result = util::AppControlToService(
        args.get("appControl").get<picojson::object>(), &app_control);
    if (!result) {
      ReportError(result, &out);
      return;
    }
  } else {
    app_control_create(&app_control);
    app_control_set_operation(app_control, APP_CONTROL_OPERATION_DEFAULT);
  }

  app_control_set_app_id(app_control, app_id.c_str());

  int alarm_id = 0;

  if (kAlarmRelative == alarm_type) {
    app_control_add_extra_data(app_control, kAlarmKeyType, kAlarmTypeValueRelative);

    const auto it_delay = alarm.find("delay");
    const auto it_period = alarm.find("period");

    if (alarm.end() == it_delay) {
      LoggerE("Invalid parameter passed.");
      ReportError(PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid parameter passed."), &out);
      return;
    }
    int delay = static_cast<int>(it_delay->second.get<double>());

    int period = 0;
    if (alarm.end() != it_period) {
      period = static_cast<int>(it_period->second.get<double>());
    }

    std::string delay_str = std::to_string(delay);
    int ret = app_control_add_extra_data(app_control, kAlarmRelativeDelayKey, delay_str.c_str());
    if (APP_CONTROL_ERROR_NONE != ret) {
      LoggerE("Fail to add data from app_control.");
      ReportError(PlatformResult(
          ErrorCode::UNKNOWN_ERR, "Fail to add data from app_control."), &out);
      return;
    }

    ret = alarm_schedule_after_delay(app_control, delay, period, &alarm_id);
    if (ALARM_ERROR_NONE != ret) {
      LoggerE("Error while add alarm to server.");
      ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Error while add alarm to server."), &out);
      return;
    }
  } else {
    app_control_add_extra_data(app_control, kAlarmKeyType, kAlarmTypeValueAbsolute);
    app_control_add_extra_data(
        app_control, kAlarmAbsoluteRecurrenceTypeKey, kAlarmAbsoluteRecurrenceTypeNone);

    const auto it_period = alarm.find("period");
    const auto it_daysOfWeek = alarm.find("daysOfWeek");

    long long int seconds = 0;
    if (args.contains("seconds")) {
      seconds = atoll(args.get("seconds").get<std::string>().c_str());
    }

    int period = 0;
    time_t second = seconds / 1000;
    struct tm *start_date;

    start_date = localtime(&second);
    mktime(start_date);

    char str_date[kDateSize];

    snprintf(str_date, sizeof(str_date), "%d %d %d %d %d %d %d", start_date->tm_year,
             start_date->tm_mon, start_date->tm_mday, start_date->tm_hour, start_date->tm_min,
             start_date->tm_sec, start_date->tm_isdst);

    app_control_add_extra_data(app_control, kAlarmAbsoluteDateKey, str_date);

    int ret = 0;
    if (it_period->second.is<double>()) {
      period = static_cast<int>(it_period->second.get<double>());
      ret = alarm_schedule_at_date(app_control, start_date, period, &alarm_id);
    } else if (it_daysOfWeek->second.is<picojson::array>()) {
      picojson::array days_of_week = it_daysOfWeek->second.get<picojson::array>();
      int repeat_value = 0;
      for (auto iter = days_of_week.begin(); iter != days_of_week.end(); ++iter) {
        auto day = (*iter).get<std::string>();
        if (kSundayShort == day) {
          repeat_value |= ALARM_WEEK_FLAG_SUNDAY;
        } else if (kMondayShort == day) {
          repeat_value |= ALARM_WEEK_FLAG_MONDAY;
        } else if (kTuesdayShort == day) {
          repeat_value |= ALARM_WEEK_FLAG_TUESDAY;
        } else if (kWednesdayShort == day) {
          repeat_value |= ALARM_WEEK_FLAG_WEDNESDAY;
        } else if (kThuesdayShort == day) {
          repeat_value |= ALARM_WEEK_FLAG_THURSDAY;
        } else if (kFridayShort == day) {
          repeat_value |= ALARM_WEEK_FLAG_FRIDAY;
        } else if (kSaturdayShort == day) {
          repeat_value |= ALARM_WEEK_FLAG_SATURDAY;
        } else {
          LoggerE("Invalid days of the week value.");
          ReportError(PlatformResult(
              ErrorCode::TYPE_MISMATCH_ERR, "Invalid days of the week value."), &out);
          return;
        }
        ret = alarm_schedule_with_recurrence_week_flag(
            app_control, start_date, repeat_value, &alarm_id);
      }
    } else {
      ret = alarm_schedule_at_date(app_control, start_date, 0, &alarm_id);
    }

    if (ALARM_ERROR_NONE != ret) {
      LoggerE("Adding alarm to server failed.");
      ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Adding alarm to server failed."), &out);
      return;
    }
  }

  // result object
  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();

  result_obj.insert(std::make_pair("alarm_id", picojson::value(std::to_string(alarm_id))));
  ReportSuccess(result, out);
}

void AlarmManager::Remove(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  util::CheckAccess(kPrivilegeAlarm);

  int id = 0;
  if (args.contains("id")) {
    id = static_cast<int>(args.get("id").get<double>());
  }

  if (id <= 0) {
      LoggerE("id is wrong: %d", id);
      ReportError(PlatformResult(ErrorCode::NOT_FOUND_ERR, "Invalid id."), &out);
      return;
  }

  int ret = alarm_cancel(id);
  if (ALARM_ERROR_NONE == ret) {
    ReportSuccess(out);
  } else if (ALARM_ERROR_INVALID_PARAMETER == ret) {
    LoggerE("Alarm not found.");
    ReportError(PlatformResult(ErrorCode::NOT_FOUND_ERR, "Alarm not found."), &out);
  } else {
    LoggerE("Platform thrown unknown error.");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Platform unknown error."), &out);
  }
}

void AlarmManager::RemoveAll(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  util::CheckAccess(kPrivilegeAlarm);

  if (ALARM_ERROR_NONE != alarm_cancel_all()) {
    LoggerE("Platform unknown error.");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Platform unknown error."), &out);
    return;
  }

  ReportSuccess(out);
}

void AlarmManager::Get(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
}

void AlarmManager::GetAll(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
}

void AlarmManager::GetRemainingSeconds(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
}

void AlarmManager::GetNextScheduledDate(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
}

} // namespace alarm
} // namespace extension
