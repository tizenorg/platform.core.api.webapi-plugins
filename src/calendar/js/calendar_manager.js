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

// class CalendarManager
var CalendarManager = function() {};

var _PRIVILEGE_CALENDAR_READ = "http://tizen.org/privilege/calendar.read";
// IDs defined in C-API calendar_types2.h
var DefaultCalendarId = {
  EVENT: 1, // DEFAULT_EVENT_CALENDAR_BOOK_ID
  TASK: 2 // DEFAULT_TODO_CALENDAR_BOOK_ID
};

CalendarManager.prototype.getCalendars = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.CALENDAR_READ);

  var args = validator_.validateArgs(arguments, [{
    name: 'type',
    type: types_.ENUM,
    values: Object.keys(CalendarType)
  },
  {
    name: 'successCallback',
    type: types_.FUNCTION
  },
  {
    name: 'errorCallback',
    type: types_.FUNCTION,
    optional: true,
    nullable: true
  }]);

  var callArgs = {
    type: args.type
  };

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
    } else {
      var calendars = native_.getResultObject(result);
      var c = [];
      calendars.forEach(function(i) {
        c.push(new Calendar(new InternalCalendar(i)));
      });
      args.successCallback(c);
    }
  };

  native_.call('CalendarManager_getCalendars', callArgs, callback);
};

CalendarManager.prototype.getUnifiedCalendar = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.CALENDAR_READ);

  var args = validator_.validateArgs(arguments, [{
    name: 'type',
    type: types_.ENUM,
    values: Object.keys(CalendarType)
  }]);

  return new Calendar(new InternalCalendar({
    type: args.type,
    isUnified: true
  }));
};

CalendarManager.prototype.getDefaultCalendar = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.CALENDAR_READ);

  var args = validator_.validateArgs(arguments, [{
    name: 'type',
    type: types_.ENUM,
    values: Object.keys(CalendarType)
  }
  ]);

  return this.getCalendar(args.type, DefaultCalendarId[args.type]);
};

CalendarManager.prototype.getCalendar = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.CALENDAR_READ);

  var args = validator_.validateArgs(arguments, [{
    name: 'type',
    type: types_.ENUM,
    values: Object.keys(CalendarType)
  },
  {
    name: 'id',
    type: types_.STRING
  }
  ]);

  if (isNaN(Number(args.id))) {
      throw new WebAPIException(WebAPIException.NOT_FOUND_ERR,
          'Calendar not found');
  }

  var callArgs = {
    type: args.type,
    id: args.id
  };

  var result = native_.callSync('CalendarManager_getCalendar', callArgs);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  return new Calendar(new InternalCalendar(native_.getResultObject(result)));
};

CalendarManager.prototype.addCalendar = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.CALENDAR_WRITE);

  var args = validator_.validateArgs(arguments, [{
    name: 'calendar',
    type: types_.PLATFORM_OBJECT,
    values: Calendar
  }]);

  var callArgs = {
    calendar: args.calendar
  };

  var result = native_.callSync('CalendarManager_addCalendar', callArgs);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  args.calendar.id = new InternalCalendar({
    id: native_.getResultObject(result)
  });
};

CalendarManager.prototype.removeCalendar = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.CALENDAR_WRITE);

  var args = validator_.validateArgs(arguments, [{
    name: 'type',
    type: types_.ENUM,
    values: Object.keys(CalendarType)
  },
  {
    name: 'id',
    type: types_.STRING
  }
  ]);

  var callArgs = {
    type: args.type,
    id: args.id
  };

  var result = native_.callSync('CalendarManager_removeCalendar', callArgs);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

exports = new CalendarManager();
