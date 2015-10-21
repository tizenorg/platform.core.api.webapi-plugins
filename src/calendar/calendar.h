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

#ifndef CALENDAR_CALENDAR_H_
#define CALENDAR_CALENDAR_H_

#include <memory>
#include <calendar-service2/calendar_types.h>

#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace calendar {

typedef picojson::value JsonValue;
typedef picojson::object JsonObject;
typedef picojson::array JsonArray;
typedef std::string JsonString;

typedef std::shared_ptr<JsonValue> JsonValuePtr;

class CalendarInstance;

class Calendar {
 public:
  explicit Calendar(CalendarInstance& instance);
  ~Calendar();

  /**
   * Signature: @code CalendarItem get(id); @endcode
   * JSON: @code data: {method: 'Calendar_get', args: {id, calendarId}} @endcode
   * Invocation: @code native.callSync(request); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success', item: {}}
   * @endcode
   */
  common::PlatformResult Get(const JsonObject& args, JsonObject& out);

  /**
   * Signature: @code void add(item); @endcode
   * JSON: @code data: {method: 'Calendar_add', args: {item, type}} @endcode
   * Invocation: @code native.callSync(request); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   */
  common::PlatformResult Add(const JsonObject& args, JsonObject& out);

  /**
   * Signature: @code void addBatch(items, successCallback, errorCallback);
   * @endcode
   * JSON: @code data: {method: 'Calendar_addBatch', args: {items, type}}
   * @endcode
   * Invocation: @code native.call(request); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success', result: items}
   * @endcode
   */
  common::PlatformResult AddBatch(const JsonObject& args, JsonArray& array);

  /**
   * Signature: @code void update(item, updateAllInstances); @endcode
   * JSON: @code data: {method: 'Calendar_update', args: {item, type,
   * updateAllInstances}} @endcode
   * Invocation: @code native.callSync(request); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   */
  common::PlatformResult Update(const JsonObject& args, JsonObject& out);

  /**
   * Signature: @code void updateBatch(items, successCallback, errorCallback,
   * updateAllInstances);
   * @endcode
   * JSON: @code data: {method: 'Calendar_updateBatch', args: {
   * items, type, updateAllInstances}} @endcode
   * Invocation: @code native.call(request); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   */
  common::PlatformResult UpdateBatch(const JsonObject& args, JsonArray& array);

  /**
   * Signature: @code void remove(item); @endcode
   * JSON: @code data: {method: 'Calendar_remove', args: {type, id}} @endcode
   * Invocation: @code native.callSync(request); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   */
  common::PlatformResult Remove(const JsonObject& args, JsonObject& out);

  /**
   * Signature: @code void removeBatch(items, successCallback, errorCallback);
   * @endcode
   * JSON: @code data: {method: 'Calendar_removeBatch', args: {items, type}}
   * @endcode
   * Invocation: @code native.call(request); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   */
  common::PlatformResult RemoveBatch(const JsonObject& args, JsonArray& array);

  /**
   * Signature: @code void find(successCallback, errorCallback, filter,
   * sortMode); @endcode
   * JSON: @code data: {method: 'Calendar_find', args: {calendarId, filter,
   * sortMode}} @endcode
   * Invocation: @code native.call(request, result_callback); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   * Result callback:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success', result: {calendarItemsArray}}
   * @endcode
   */
  common::PlatformResult Find(const JsonObject& args, JsonArray& array);

  /**
   * Signature: @code void addChangeListener(successCallback); @endcode
   * JSON: @code data: {method: 'Calendar_addChangeListener',
   * args: {type, listenerId}} @endcode
   * Invocation: @code native.callSync(request); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   */
  common::PlatformResult AddChangeListener(const JsonObject& args,
                                           JsonObject& out);

  /**
   * Signature: @code void removeChangeListener(); @endcode
   * JSON: @code data: {method: 'Calendar_removeChangeListener', args: {type}}
   * @endcode
   * Invocation: @code native.callSync(request); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   */
  common::PlatformResult RemoveChangeListener(const JsonObject& args,
                                              JsonObject& out);

 private:
  std::map<std::string, std::string> listeners_registered_;
  int current_db_version_;
  CalendarInstance& instance_;

  static void ChangeCallback(const char* view_uri, void* user_data);
  common::PlatformResult ErrorChecker(int errorCode);
  common::PlatformResult SetDefaultFilter(calendar_query_h* calendar_query, int type, int id);
};

}  // namespace calendar
}  // namespace webapi

#endif  // CALENDAR_CALENDAR_H_
