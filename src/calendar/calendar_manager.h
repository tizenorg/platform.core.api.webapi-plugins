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

#ifndef CALENDAR_CALENDAR_MANAGER_H_
#define CALENDAR_CALENDAR_MANAGER_H_

#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace calendar {

typedef picojson::value JsonValue;
typedef picojson::object JsonObject;
typedef picojson::array JsonArray;
typedef std::string JsonString;

class CalendarManager {
 public:
  /**
   * Signature: @code void getCalendars(type, successCallback, errorCallback);
   * @endcode
   * JSON: @code data: {method: 'CalendarManager_getCalendars',
   *                    args: {type: type}} @endcode
   * Invocation: @code native.call(request, result_callback); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   * Result callback:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success', result: {calendarsArray}}
   * @endcode
   */
  common::PlatformResult GetCalendars(const JsonObject& args, JsonArray& array);
  common::PlatformResult GetCalendar(const JsonObject& args, JsonObject& out);
  common::PlatformResult AddCalendar(const JsonObject& args, JsonObject& out);
  common::PlatformResult RemoveCalendar(const JsonObject& args,
                                        JsonObject& out);

  static CalendarManager& GetInstance();
  virtual ~CalendarManager();
  bool IsConnected();

 private:
  CalendarManager();
  bool is_connected_;
};

}  // namespace calendar
}  // namespace webapi

#endif /* CALENDAR_CALENDAR_MANAGER_H_ */
