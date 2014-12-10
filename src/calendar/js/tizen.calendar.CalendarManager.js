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

(function () {
    'use strict';

    var _common = require('./tizen.Common');
    var Cal = require('./tizen.calendar.Calendar');
    var Calendar = Cal.Calendar;
    var InternalCalendar = Cal.InternalCalendar;
    var AV = _common.ArgumentValidator;
    var C = _common.Common;
    var _callSync = C.getCallSync('calendar');
    var _call = C.getCall('calendar');
    _common = undefined;

    // class CalendarManager
    var CalendarManager = function () {};

    var CalendarType = {
        EVENT: 'EVENT',
        TASK: 'TASK'
    };

    // IDs defined in C-API calendar_types2.h
    var DefaultCalendarId = {
        EVENT: 1, // DEFAULT_EVENT_CALENDAR_BOOK_ID
        TASK: 2 // DEFAULT_TODO_CALENDAR_BOOK_ID
    };

    CalendarManager.prototype.getCalendars = function () {
        var args = AV.validateMethod(arguments, [{
            name: 'type',
            type: AV.Types.ENUM,
            values: Object.keys(CalendarType)
        },
        {
            name: 'successCallback',
            type: AV.Types.FUNCTION
        },
        {
            name: 'errorCallback',
            type: AV.Types.FUNCTION,
            optional: true,
            nullable: true
        }]);

        var callArgs = {
            type: args.type
        };

        var callback = function (result) {
            if (C.isFailure(result)) {
                C.callIfPossible(args.errorCallback, C.getErrorObject(result));
            } else {
                var calendars = C.getResultObject(result);
                var c = [];
                calendars.forEach(function (i) {
                    c.push(new Calendar(new InternalCalendar(i)));
                });
                args.successCallback(c);
            }
        };

        var result = _call('CalendarManager_getCalendars', callArgs, callback);

        if (C.isFailure(result)) {
            throw C.getErrorObject(result);
        }
    };

    CalendarManager.prototype.getUnifiedCalendar = function () {

        var args = AV.validateMethod(arguments, [{
            name: 'type',
            type: AV.Types.ENUM,
            values: Object.keys(CalendarType)
        }]);

        return new Calendar(new InternalCalendar({
            type: args.type,
            isUnified: true
        }));
    };

    CalendarManager.prototype.getDefaultCalendar = function () {

        var args = AV.validateMethod(arguments, [{
                name: 'type',
                type: AV.Types.ENUM,
                values: Object.keys(CalendarType)
            }
        ]);

        return this.getCalendar(args.type, DefaultCalendarId[args.type]);
    };

    CalendarManager.prototype.getCalendar = function () {

        var args = AV.validateMethod(arguments, [{
                name: 'type',
                type: AV.Types.ENUM,
                values: Object.keys(CalendarType)
            },
            {
                name: 'id',
                type: AV.Types.STRING
            }
        ]);

        var callArgs = {
            type: args.type,
            id: args.id
        };

        var result = _callSync('CalendarManager_getCalendar', callArgs);

        if (C.isFailure(result)) {
            throw C.getErrorObject(result);
        }

        return new Calendar(new InternalCalendar(C.getResultObject(result)));
    };

    CalendarManager.prototype.addCalendar = function () {

        var args = AV.validateMethod(arguments, [{
            name: 'calendar',
            type: AV.Types.PLATFORM_OBJECT,
            values: Calendar
        }]);

        var callArgs = {
            calendar: args.calendar
        };

        var result = _callSync('CalendarManager_addCalendar', callArgs);

        if (C.isFailure(result)) {
            throw C.getErrorObject(result);
        }

        args.calendar.id = new InternalCalendar({
            id: C.getResultObject(result)
        });
    };

    CalendarManager.prototype.removeCalendar = function () {

        var args = AV.validateMethod(arguments, [{
                name: 'type',
                type: AV.Types.ENUM,
                values: Object.keys(CalendarType)
            },
            {
                name: 'id',
                type: AV.Types.STRING
            }
        ]);

        var callArgs = {
            type: args.type,
            id: args.id
        };

        var result = _callSync('CalendarManager_removeCalendar', callArgs);

        if (C.isFailure(result)) {
            throw C.getErrorObject(result);
        }
    };

    module.exports = CalendarManager;

})();
