/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
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

#include "time/time_manager.h"

#include <unicode/timezone.h>
#include <unicode/calendar.h>
#include <unistd.h>

#include "common/logger.h"
#include "time/time_instance.h"

using common::PlatformResult;
using common::ErrorCode;
using common::Instance;

namespace extension {
namespace time {

TimeManager::TimeManager(TimeInstance* instance)
            : instance_(instance),
              current_timezone_(GetDefaultTimezone()),
              is_time_listener_registered_(false),
              is_timezone_listener_registered_(false) {
  LoggerD("Entered");
}

TimeManager::~TimeManager() {
  LoggerD("Entered");
  if (is_time_listener_registered_) {
    UnregisterVconfCallback(kTimeChange);
  }
  if (is_timezone_listener_registered_) {
    UnregisterVconfCallback(kTimezoneChange);
  }
}

PlatformResult TimeManager::GetTimezoneOffset(const std::string& timezone_id,
                                              const std::string& timestamp_str,
                                              std::string* offset,
                                              std::string* modifier) {
  LoggerD("Entered");
  std::unique_ptr<UnicodeString> unicode_id (new UnicodeString(timezone_id.c_str()));
  std::unique_ptr<TimeZone> tz (TimeZone::createTimeZone(*unicode_id));

  if (TimeZone::getUnknown() == *tz) {
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid parameter passed.");
  }

  const int32_t oneHour = 3600000;
  UDate date = std::stod(timestamp_str);
  int32_t stdOffset = 0;
  int32_t dstOffset = 0;
  UErrorCode ec = U_ZERO_ERROR;
  //offset is get for target LOCAL date timestamp, but it should be UTC timestamp,
  //so it has to be checked below against DST edge condition
  tz->getOffset(date, false, stdOffset, dstOffset, ec);
  LOGD("stdOffset: %d, dstOffset: %d", stdOffset, dstOffset);

  //this section checks if date is not in DST transition point
  //check if date shifted to UTC timestamp is still with the same offset
  int32_t dstOffsetBefore = 0;
  tz->getOffset(date - stdOffset - dstOffset, false, stdOffset, dstOffsetBefore, ec);
  LOGD("stdOffset: %d, dstOffsetBefore: %d", stdOffset, dstOffsetBefore);

  //it has to be checked if it is 'missing' hour case
  int32_t dstOffsetAfterBefore = 0;
  tz->getOffset(date - stdOffset - dstOffset + oneHour,
                false, stdOffset, dstOffsetAfterBefore, ec);
  LOGD("stdOffset: %d, dstOffsetAfterBefore: %d", stdOffset, dstOffsetAfterBefore);

  //offset would be minimum of local and utc timestamp offsets
  //(to work correctly even if DST transtion is 'now')
  dstOffset = std::min(dstOffset, dstOffsetBefore);

  *offset = std::to_string(stdOffset + dstOffset);
  *modifier = std::to_string(dstOffsetAfterBefore - dstOffsetBefore);
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult TimeManager::RegisterVconfCallback(ListenerType type) {
  LoggerD("Entered");
  if (!is_time_listener_registered_ && !is_timezone_listener_registered_){
    LOGD("registering listener on platform");
    if (0 != vconf_notify_key_changed(
        VCONFKEY_SYSTEM_TIME_CHANGED, OnTimeChangedCallback, this)) {
      LOGE("Failed to register vconf callback");
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to register vconf callback");
    }
  } else {
    LOGD("not registering listener on platform - already registered");
  }
  switch (type) {
    case kTimeChange :
      is_time_listener_registered_ = true;
      LOGD("time change listener registered");
      break;
    case kTimezoneChange :
      is_timezone_listener_registered_ = true;
      LOGD("time zone change listener registered");
      break;
    default :
      LOGE("Unknown type of listener");
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown type of listener");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult TimeManager::UnregisterVconfCallback(ListenerType type) {
  LoggerD("Entered");
  switch (type) {
    case kTimeChange :
      is_time_listener_registered_ = false;
      LOGD("time change listener unregistered");
      break;
    case kTimezoneChange :
      is_timezone_listener_registered_ = false;
      LOGD("time zone change listener unregistered");
      break;
    default :
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown type of listener");
  }
  if (!is_time_listener_registered_ && !is_timezone_listener_registered_) {
    LOGD("unregistering listener on platform");
    if (0 != vconf_ignore_key_changed(VCONFKEY_SYSTEM_TIME_CHANGED, OnTimeChangedCallback)) {
      LOGE("Failed to unregister vconf callback");
      // silent fail
      //return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to unregister vconf callback");
    }
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

void TimeManager::OnTimeChangedCallback(keynode_t* /*node*/, void* event_ptr) {
  LoggerD("Entered");
  TimeManager* manager = static_cast<TimeManager*>(event_ptr);
  TimeInstance* instance = manager->GetTimeInstance();
  std::string defaultTimezone = GetDefaultTimezone();

  if (manager->GetCurrentTimezone() != defaultTimezone) {
    manager->SetCurrentTimezone(defaultTimezone);
    //call timezone callback

    const std::shared_ptr<picojson::value>& response =
        std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    response->get<picojson::object>()["listenerId"] = picojson::value("TimezoneChangeListener");
    //    ReportSuccess(result,response->get<picojson::object>());
    Instance::PostMessage(instance, response->serialize().c_str());
  }
  //call date time callback
  const std::shared_ptr<picojson::value>& response =
      std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
  response->get<picojson::object>()["listenerId"] = picojson::value("DateTimeChangeListener");
  //  ReportSuccess(result,response->get<picojson::object>());
  Instance::PostMessage(instance, response->serialize().c_str());
}

std::string TimeManager::GetDefaultTimezone() {
  LoggerD("Entered");
  char buf[1024];
  std::string result;
  ssize_t len = readlink("/etc/localtime", buf, sizeof(buf)-1);
  if (len != -1) {
    buf[len] = '\0';
  } else {
    /* handle error condition */
    return result;
  }
  result = std::string(buf+strlen("/usr/share/zoneinfo/"));

  LoggerD("tzpath = %s", result.c_str());
  return result;
}

std::string TimeManager::GetCurrentTimezone(){
  LoggerD("Entered");
  return current_timezone_;
}

void TimeManager::SetCurrentTimezone(const std::string& new_timezone){
  LoggerD("Entered");
  current_timezone_ = new_timezone;
}

TimeInstance* TimeManager::GetTimeInstance() {
  LoggerD("Entered");
  return instance_;
}

} // time
} // extension
