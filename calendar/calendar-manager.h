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

#ifndef WEBAPI_PLUGINS_CALENDAR_MANAGER_H_
#define WEBAPI_PLUGINS_CALENDAR_MANAGER_H_

#include "json-parser.h"

namespace webapi {
namespace calendar {

class CalendarManager {
public:
    /**
     * Signature: @code void getCalendars(type, successCallback, errorCallback); @endcode
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
    void GetCalendars(const common::json::Object& args, common::json::Object& out);
    void GetCalendar(const common::json::Object& args, common::json::Object& out);
    void AddCalendar(const common::json::Object& args, common::json::Object& out);
    void RemoveCalendar(const common::json::Object& args, common::json::Object& out);

    static CalendarManager& GetInstance();
    virtual ~CalendarManager();
    bool IsConnected();

private:
    CalendarManager();
    bool is_connected_;
};

} // namespace calendar
} // namespace webapi

#endif	/* WEBAPI_PLUGINS_CALENDAR_MANAGER_H_ */
