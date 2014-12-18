// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// class CalendarManager
var CalendarManager = function() {};

// IDs defined in C-API calendar_types2.h
var DefaultCalendarId = {
  EVENT: 1, // DEFAULT_EVENT_CALENDAR_BOOK_ID
  TASK: 2 // DEFAULT_TODO_CALENDAR_BOOK_ID
};

CalendarManager.prototype.getCalendars = function() {
  var args = AV.validateArgs(arguments, [{
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

  var args = AV.validateArgs(arguments, [{
    name: 'type',
    type: AV.Types.ENUM,
    values: Object.keys(CalendarType)
  }]);

  return new Calendar(new InternalCalendar({
    type: args.type,
    isUnified: true
  }));
};

CalendarManager.prototype.getDefaultCalendar = function() {

  var args = AV.validateArgs(arguments, [{
    name: 'type',
    type: AV.Types.ENUM,
    values: Object.keys(CalendarType)
  }
  ]);

  return this.getCalendar(args.type, DefaultCalendarId[args.type]);
};

CalendarManager.prototype.getCalendar = function() {

  var args = AV.validateArgs(arguments, [{
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

  var result = native_.callSync('CalendarManager_getCalendar', callArgs);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  return new Calendar(new InternalCalendar(native_.getResultObject(result)));
};

CalendarManager.prototype.addCalendar = function() {

  var args = AV.validateArgs(arguments, [{
    name: 'calendar',
    type: AV.Types.PLATFORM_OBJECT,
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

  var args = AV.validateArgs(arguments, [{
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

  var result = native_.callSync('CalendarManager_removeCalendar', callArgs);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

exports = new CalendarManager();
