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

#include "alarm_manager.h"

#include <alarm.h>
#include <app.h>
#include <app_alarm.h>
#include <app_control_internal.h>

#include "common/logger.h"
#include "common/converter.h"
#include "common/scope_exit.h"

#include "alarm_instance.h"
#include "alarm_utils.h"

using namespace common;
using namespace common::tools;

namespace extension {
namespace alarm {

namespace {
const int kDateSize = 22; //"yyy mm dd hh mm ss dd" e.g 115 11 28 11 25 50 -1

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

void AlarmManager::Add(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

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

  if (args.contains("appControl") && args.get("appControl").is<picojson::object>()) {
    PlatformResult result = util::AppControlToService(
        args.get("appControl").get<picojson::object>(), &app_control);
    if (!result) {
      LoggerE("Failed: util::AppControlToService");
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

    if (alarm.end() == it_delay || alarm.end() == it_period || !it_delay->second.is<double>()) {
      LoggerE("Invalid parameter passed.");
      ReportError(PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid parameter passed."), &out);
      return;
    }
    int delay = static_cast<int>(it_delay->second.get<double>());

    int period = 0;
    if (it_period->second.is<double>()) {
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
    const auto it_daysOfTheWeek = alarm.find("daysOfTheWeek");

    long long int seconds = 0;
    if (args.contains("seconds")) {
      seconds = atoll(args.get("seconds").get<std::string>().c_str());
    }

    time_t second = seconds / 1000;
    struct tm start_date = {0};

    tzset();
    if (nullptr == localtime_r(&second, &start_date)) {
      LoggerE("Invalid date.");
      ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Invalid date."), &out);
      return;
    }

    mktime(&start_date);

    char str_date[kDateSize];

    snprintf(str_date, sizeof(str_date), "%d %d %d %d %d %d %d", start_date.tm_year,
             start_date.tm_mon, start_date.tm_mday, start_date.tm_hour, start_date.tm_min,
             start_date.tm_sec, start_date.tm_isdst);

    app_control_add_extra_data(app_control, kAlarmAbsoluteDateKey, str_date);

    int ret = 0;
    if (it_period->second.is<double>()) {
      int period = static_cast<int>(it_period->second.get<double>());
      ret = alarm_schedule_at_date(app_control, &start_date, period, &alarm_id);
    } else if (it_daysOfTheWeek->second.is<picojson::array>() &&
        !(it_daysOfTheWeek->second.get<picojson::array>()).empty()) {
      picojson::array days_of_the_week = it_daysOfTheWeek->second.get<picojson::array>();
      int repeat_value = 0;
      for (auto iter = days_of_the_week.begin(); iter != days_of_the_week.end(); ++iter) {
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
            app_control, &start_date, repeat_value, &alarm_id);
      }
    } else {
      ret = alarm_schedule_at_date(app_control, &start_date, 0, &alarm_id);
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

  result_obj.insert(std::make_pair("id", picojson::value(std::to_string(alarm_id))));
  ReportSuccess(result, out);
}

void AlarmManager::Remove(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  int id = 0;

  if (args.contains("id") && args.get("id").is<double>()) {
    id = static_cast<int>(args.get("id").get<double>());
  }

  if (id <= 0) {
      LoggerE("id is wrong: %d", id);
      ReportError(PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid id."), &out);
      return;
  }

  int ret = alarm_cancel(id);
  if (ALARM_ERROR_NONE == ret) {
    ReportSuccess(out);
  } else if (ALARM_ERROR_INVALID_PARAMETER == ret) {
    LoggerE("Alarm not found.");
    ReportError(PlatformResult(ErrorCode::NOT_FOUND_ERR, "Alarm not found."), &out);
  } else {
    LoggerE("Platform unknown error.");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Platform unknown error."), &out);
  }
}

void AlarmManager::RemoveAll(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  if (ALARM_ERROR_NONE != alarm_cancel_all()) {
    LoggerE("Platform unknown error.");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Platform unknown error."), &out);
    return;
  }

  ReportSuccess(out);
}

PlatformResult AlarmManager::GetAlarm(int id, picojson::object& obj) {
  LoggerD("Entered");

  if (id <= 0) {
    LoggerE("id is wrong: %d", id);
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid id.");
  }

  int ret = ALARM_ERROR_NONE;
  app_control_h app_control = nullptr;
  char* alarm_type = nullptr;
  char* date_string = nullptr;
  char* delay_string = nullptr;

  SCOPE_EXIT {
    app_control_destroy(app_control);
    free(alarm_type);
    free(date_string);
    free(delay_string);
  };

  ret = alarm_get_app_control(id, &app_control);
  if (ALARM_ERROR_NONE != ret) {
    LoggerE("Alarm not found");
    return PlatformResult(ErrorCode::NOT_FOUND_ERR, "Alarm not found.");
  }

  ret = app_control_get_extra_data(app_control, kAlarmKeyType, &alarm_type);
  if (APP_CONTROL_ERROR_NONE != ret) {
    LoggerE("Getting data failed");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown error occurred.");
  }

  obj.insert(std::make_pair("id", picojson::value(std::to_string(id))));

  if (!strcmp(alarm_type, kAlarmTypeValueAbsolute)) {
    struct tm date;
    memset(&date, 0, sizeof(tm));

    ret = app_control_get_extra_data(app_control, kAlarmAbsoluteDateKey, &date_string);

    if (APP_CONTROL_ERROR_NONE != ret) {
      LoggerE("Failed to get data.");
      return PlatformResult(ErrorCode::NOT_FOUND_ERR, "Failed to get data.");
    }

    sscanf(date_string, "%5d %5d %5d %5d %5d %5d %5d", &date.tm_year, &date.tm_mon,
           &date.tm_mday, &date.tm_hour, &date.tm_min, &date.tm_sec, &date.tm_isdst);
    mktime(&date);

    obj.insert(std::make_pair("year", picojson::value(std::to_string(date.tm_year + 1900))));
    obj.insert(std::make_pair("month", picojson::value(std::to_string(date.tm_mon))));
    obj.insert(std::make_pair("day", picojson::value(std::to_string(date.tm_mday))));
    obj.insert(std::make_pair("hour", picojson::value(std::to_string(date.tm_hour))));
    obj.insert(std::make_pair("min", picojson::value(std::to_string(date.tm_min))));
    obj.insert(std::make_pair("sec", picojson::value(std::to_string(date.tm_sec))));

    int interval = 0;

    app_control_get_extra_data(app_control, kAlarmAbsoluteRecurrenceTypeKey, &alarm_type);

    if (!strcmp(alarm_type, kAlarmAbsoluteReccurrenceTypeInterval)) {
      ret = alarm_get_scheduled_period(id, &interval);
      if (ALARM_ERROR_NONE != ret) {
        LoggerE("Unknown error occurred.");
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown error occurred.");
      }

      obj.insert(std::make_pair("second", picojson::value(std::to_string(interval))));
    } else if (!strcmp(alarm_type, kAlarmAbsoluteReccurrenceTypeByDayValue)) {
      int byDayValue = 0;

      ret = alarm_get_scheduled_recurrence_week_flag(id, &byDayValue);
      if (ALARM_ERROR_NONE != ret) {
        LoggerE("Failed to get data.");
        return PlatformResult(ErrorCode::NOT_FOUND_ERR, "Failed to get data.");
      }

      picojson::array& array =
          obj.insert(std::make_pair("second", picojson::value(picojson::array())))
          .first->second.get<picojson::array>();

      if (byDayValue & ALARM_WEEK_FLAG_SUNDAY)
        array.push_back(picojson::value(kSundayShort));
      if (byDayValue & ALARM_WEEK_FLAG_MONDAY)
        array.push_back(picojson::value(kMondayShort));
      if (byDayValue & ALARM_WEEK_FLAG_TUESDAY)
        array.push_back(picojson::value(kTuesdayShort));
      if (byDayValue & ALARM_WEEK_FLAG_WEDNESDAY)
        array.push_back(picojson::value(kWednesdayShort));
      if (byDayValue & ALARM_WEEK_FLAG_THURSDAY)
        array.push_back(picojson::value(kThuesdayShort));
      if (byDayValue & ALARM_WEEK_FLAG_FRIDAY)
        array.push_back(picojson::value(kFridayShort));
      if (byDayValue & ALARM_WEEK_FLAG_SATURDAY)
        array.push_back(picojson::value(kSaturdayShort));
    }

    obj.insert(std::make_pair("type", picojson::value(kAlarmAbsolute)));

  } else if (!strcmp(alarm_type, kAlarmTypeValueRelative)) {
    int interval = 0;

    ret = alarm_get_scheduled_period(id, &interval);
    if (ALARM_ERROR_NONE != ret) {
      LoggerE("Unknown error occurred.");
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown error occurred.");
    }

    ret = app_control_get_extra_data(app_control, kAlarmRelativeDelayKey, &delay_string);
    if (APP_CONTROL_ERROR_NONE != ret) {
      LoggerE("Failed to get data.");
      return PlatformResult(ErrorCode::NOT_FOUND_ERR, "Failed to get data.");
    }

    obj.insert(std::make_pair("type", picojson::value(kAlarmRelative)));
    obj.insert(std::make_pair("delay", picojson::value(delay_string)));
    obj.insert(std::make_pair("period", picojson::value(std::to_string(interval))));
  } else {
    LoggerE("Unknown error occurred.");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown error occurred.");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

void AlarmManager::Get(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  int id = 0;

  if (args.contains("id") && args.get("id").is<double>()) {
    id = static_cast<int>(args.get("id").get<double>());
  }

  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();

  PlatformResult platform_result = GetAlarm(id, result_obj);

  if (!platform_result) {
    ReportError(platform_result, &out);
  } else {
    ReportSuccess(result, out);
  }
}

static bool AlarmIterateCB(int alarm_id, void *user_data) {
  LoggerD("Enter");

  std::vector<int> *alarm_ids = reinterpret_cast<std::vector<int>*>(user_data);

  alarm_ids->push_back(alarm_id);
  return true;
}

void AlarmManager::GetAll(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  std::vector<int> alarm_ids;
  int ret = alarm_foreach_registered_alarm(AlarmIterateCB, &alarm_ids);

  if (ALARM_ERROR_NONE != ret) {
    LoggerE("Platform unknown error.");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Platform unknown error."), &out);
    return;
  }

  picojson::value result_array = picojson::value(picojson::array());
  picojson::array& array_obj = result_array.get<picojson::array>();

  for (size_t i = 0 ; i < alarm_ids.size(); i++) {
    picojson::value result = picojson::value(picojson::object());
    picojson::object& obj = result.get<picojson::object>();

    PlatformResult platform_result = GetAlarm(alarm_ids.at(i), obj);
    if (!platform_result) {
      LoggerE("Failed GetAlarm()");
      ReportError(platform_result, &out);
      return;
    }
    array_obj.push_back(result);
  }

  ReportSuccess(result_array, out);
}

void AlarmManager::GetRemainingSeconds(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  struct tm date;
  struct tm current;
  time_t current_time;
  time_t next_time;

  int id = 0;

  if (args.contains("id") && args.get("id").is<double>()) {
    id = static_cast<int>(args.get("id").get<double>());
  }

  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();

  int ret = alarm_get_scheduled_date(id, &date);
  if(ALARM_ERROR_NONE != ret) {
    LoggerI("alarm_get_scheduled_date error %d", ret);
    if (ALARM_ERROR_INVALID_PARAMETER == ret || ALARM_ERROR_CONNECTION_FAIL == ret) {
      result_obj.insert(std::make_pair("seconds", picojson::value()));
      ReportSuccess(result, out);
      return;
    } else {
      LoggerE("Platform unknown error.");
      ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Platform unknown error."), &out);
      return;
    }
  }

  alarm_get_current_time(&current);
  next_time = mktime(&date);
  current_time = mktime(&current);

  long seconds = next_time - current_time;

  result_obj.insert(std::make_pair("seconds", picojson::value(std::to_string(seconds))));
  ReportSuccess(result, out);
}

void AlarmManager::GetNextScheduledDate(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  int id = 0;

  if (args.contains("id") && args.get("id").is<double>()) {
    id = static_cast<int>(args.get("id").get<double>());
  }

  struct tm date;
  int ret = alarm_get_scheduled_date(id, &date);

  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();

  if (ALARM_ERROR_NONE != ret) {
    result_obj.insert(std::make_pair("year", picojson::value()));
    ReportSuccess(result, out);
    return;
  }

  struct tm curr_date;
  ret = alarm_get_current_time(&curr_date);
  if (ALARM_ERROR_NONE != ret || mktime(&date) < mktime(&curr_date)) {
    result_obj.insert(std::make_pair("year", picojson::value()));
    ReportSuccess(result, out);
    return;
  }

  // tm struct contains years since 1900
  // there is added 1900 to tm_year to return proper date
  result_obj.insert(std::make_pair("year", picojson::value(std::to_string(date.tm_year + 1900))));
  result_obj.insert(std::make_pair("month", picojson::value(std::to_string(date.tm_mon))));
  result_obj.insert(std::make_pair("day", picojson::value(std::to_string(date.tm_mday))));
  result_obj.insert(std::make_pair("hour", picojson::value(std::to_string(date.tm_hour))));
  result_obj.insert(std::make_pair("min", picojson::value(std::to_string(date.tm_min))));
  result_obj.insert(std::make_pair("sec", picojson::value(std::to_string(date.tm_sec))));

  ReportSuccess(result, out);
}

} // namespace alarm
} // namespace extension
