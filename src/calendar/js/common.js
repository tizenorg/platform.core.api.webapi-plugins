// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var utils_ = xwalk.utils;
var type_ = utils_.type;
var converter_ = utils_.converter;
var validator_ = utils_.validator;
var types_ = validator_.Types;
var native_ = new xwalk.utils.NativeManager(extension);


var EditManager = function() {
  this.canEdit = false;
};

EditManager.prototype.allow = function() {
  this.canEdit = true;
};

EditManager.prototype.disallow = function() {
  this.canEdit = false;
};

var _edit = new EditManager();

var DateConverter = function() {};

DateConverter.prototype.toTZDate = function(v, isAllDay) {
  if (typeof v === 'number') {
    v = {
      UTCTimestamp: v
        };
    isAllDay = false;
  }

  if (!(v instanceof Object)) {
    return v;
  }

  if (isAllDay) {
    return new tizen.TZDate(v.year, v.month, v.day,
        null, null, null, null, v.timezone || null);
  } else {
    return new tizen.TZDate(new Date(v.UTCTimestamp * 1000), 'UTC').toLocalTimezone();
  }
};

DateConverter.prototype.fromTZDate = function(v) {
  if (!(v instanceof tizen.TZDate)) {
    return v;
  }

  var utc = v.toUTC();
  var timestamp = new Date(utc.getFullYear(), utc.getMonth(), utc.getDate(), utc.getHours(),
      utc.getMinutes(), utc.getSeconds()) / 1000;

  return {
    year: v.getFullYear(),
    month: v.getMonth(),
    day: v.getDate(),
    timezone: v.getTimezone(),
    UTCTimestamp: timestamp
  };

};

var _dateConverter = new DateConverter();

var ItemConverter = function() {};

ItemConverter.prototype.toTizenObject = function(item) {
  var tmp = {};
  for (var prop in item) {
    if (prop === 'startDate' ||
            prop === 'endDate' ||
            prop === 'dueDate' ||
            prop === 'completedDate' ||
            prop === 'lastModificationDate') {
      tmp[prop] = _dateConverter.toTZDate(item[prop], item.isAllDay);
    } else {
      tmp[prop] = item[prop];
    }
  }

  var alarms = [];
  var alarm, time;
  for (var i = 0; i < tmp.alarms.length; i++) {
    alarm = tmp.alarms[i];
    if (alarm.absoluteDate) {
      time = _dateConverter.toTZDate(alarm.absoluteDate, tmp.isAllDay);
    } else if (alarm.before) {
      time = new tizen.TimeDuration(alarm.before.length, alarm.before.unit);
    }
    alarms.push(new tizen.CalendarAlarm(time, alarm.method, alarm.description));
  }
  tmp.alarms = alarms;

  var attendees = [];
  for (var i = 0; i < tmp.attendees.length; i++) {
    if (tmp.attendees[i].contactRef) {
      var contactRef = new tizen.ContactRef(tmp.attendees[i].contactRef.addressBookId,
                                                  tmp.attendees[i].contactRef.contactId);
      tmp.attendees[i].contactRef = contactRef;
    }
    if (tmp.attendees[i].uri) {
      attendees.push(new tizen.CalendarAttendee(tmp.attendees[i].uri, tmp.attendees[i]));
    }
  }
  tmp.attendees = attendees;

  var untilDate;
  var exceptions = [];
  if (tmp.recurrenceRule) {
    untilDate = _dateConverter.toTZDate(tmp.recurrenceRule.untilDate, tmp.isAllDay);
    tmp.recurrenceRule.untilDate = untilDate;

    for (var i = 0; i < tmp.recurrenceRule.exceptions.length; i++) {
      exceptions.push(_dateConverter.toTZDate(tmp.recurrenceRule.exceptions[i], tmp.isAllDay));
    }
    tmp.recurrenceRule.exceptions = exceptions;

    var recurrenceRule = new tizen.CalendarRecurrenceRule(tmp.recurrenceRule.frequency, tmp.recurrenceRule);
    tmp.recurrenceRule = recurrenceRule;
  }

  if (tmp.duration) {
    var duration = new tizen.TimeDuration(tmp.duration.length, tmp.duration.unit);
    tmp.duration = duration;
  }

  if (tmp.geolocation) {
    var geolocation = new tizen.SimpleCoordinates(tmp.geolocation.latitude, tmp.geolocation.longitude);
    tmp.geolocation = geolocation;
  }

  return tmp;
};

ItemConverter.prototype.fromTizenObject = function(item) {
  var tmp = {};
  for (var prop in item) {
    if (item[prop] instanceof tizen.TZDate) {
      tmp[prop] = _dateConverter.fromTZDate(item[prop]);
    } else if (item[prop] instanceof Array) {
      tmp[prop] = [];
      for (var i = 0, length = item[prop].length; i < length; i++) {
        if (item[prop][i] instanceof Object) {
          tmp[prop][i] = {};
          for (var p in item[prop][i]) {
            if (item[prop][i][p] instanceof tizen.TZDate) {
              tmp[prop][i][p] = _dateConverter.fromTZDate(item[prop][i][p]);
            } else {
              tmp[prop][i][p] = item[prop][i][p];
            }
          }
        } else {
          tmp[prop] = item[prop];
        }
      }
    } else if (item[prop] instanceof Object) {
      tmp[prop] = {};
      for (var p in item[prop]) {
        if (item[prop][p] instanceof tizen.TZDate) {
          tmp[prop][p] = _dateConverter.fromTZDate(item[prop][p]);
        } else if (item[prop][p] instanceof Array) {
          tmp[prop][p] = [];
          for (var j = 0, l = item[prop][p].length; j < l; j++) {
            tmp[prop][p].push(_dateConverter.fromTZDate(item[prop][p][j]));
          }
        } else {
          tmp[prop][p] = item[prop][p];
        }
      }
    } else {
      tmp[prop] = item[prop];
    }
  }

  return tmp;
};

var _itemConverter = new ItemConverter();

function _daysInYear(y) {
  if ((y % 4 === 0 && y % 100) || y % 400 === 0) {
    return 366;
  }
  return 365;
}

function _daysInMonth(m, y) {
  switch (m) {
    case 1 :
      return _daysInYear(y) === 366 ? 29 : 28;
    case 3 :
    case 5 :
    case 8 :
    case 10 :
      return 30;
    default :
      return 31;
  }
}

var RecurrenceManager = function() {};

RecurrenceManager.prototype.get = function(event, startDate, endDate) {
  var events = [];
  var frequency = event.recurrenceRule.frequency;
  var interval = event.recurrenceRule.interval;
  var untilDate = event.recurrenceRule.untilDate;
  var occurrenceCount = event.recurrenceRule.occurrenceCount;
  var exceptions = event.recurrenceRule.exceptions;
  var isDetached = event.isDetached;
  var startEvent = event.startDate;
  var startDate = startDate;
  var endDate = endDate;

  if (isDetached) {
    return 'The event is detached.';
  }

  if (startEvent.laterThan(startDate)) {
    startDate = startEvent;
  }

  if (untilDate) {
    endDate = untilDate.laterThan(endDate) ? endDate : untilDate;
  }

  var timeDifference = endDate.difference(startDate);
  var daysDifference = timeDifference.length;

  function checkDays(date) {
    switch (frequency) {
      case 'DAILY' :
        return 1;
      case 'WEEKLY' :
        return 7;
      case 'MONTHLY' :
        return _daysInMonth(date.getMonth(), date.getFullYear());
      case 'YEARLY' :
        return _daysInYear(date.getFullYear());
    }
  }

  function checkException(date) {
    for (var j = 0; j < exceptions.length; j++) {
      if (exceptions[j].equalsTo(date)) {
        return true;
      }
    }
    return false;
  }

  var dates = [];
  var date = startDate;
  var push = true;
  var _interval = occurrenceCount >= 0 ? occurrenceCount :
      (daysDifference + 1) / checkDays(startDate);

  for (var i = 0; i < _interval; ++i) {
    if (exceptions) {
      checkException(date) ? push = false : null;
    }

    if (push) {
      if (endDate.laterThan(date) || endDate.equalsTo(date)) {
        dates.push(date);
      }
    }
    date = date.addDuration(new tizen.TimeDuration((checkDays(date) * interval), 'DAYS'));
  }

  var tmp;
  for (var i = 0; i < dates.length; i++) {
    tmp = event.clone();
    _edit.allow();
    tmp.startDate = dates[i];
    if (event.id instanceof tizen.CalendarEventId) {
      tmp.id = new tizen.CalendarEventId(event.id.uid, +new Date());
      tmp.isDetached = true;
    }
    _edit.disallow();

    events.push(tmp);
  }

  return events;
};

var _recurrenceManager = new RecurrenceManager();

//TODO: Can be moved to utils
var Common = function() {};

Common.prototype.repackFilter = function(filter) {
  if (filter instanceof tizen.AttributeFilter) {
    return {
      filterType: 'AttributeFilter',
      attributeName: filter.attributeName,
      matchFlag: filter.matchFlag,
      matchValue: _dateConverter.fromTZDate(filter.matchValue)
    };
  }
  if (filter instanceof tizen.AttributeRangeFilter) {
    return {
      filterType: 'AttributeRangeFilter',
      attributeName: filter.attributeName,
      initialValue: _dateConverter.fromTZDate(filter.initialValue),
      endValue: _dateConverter.fromTZDate(filter.endValue)
    };
  }
  if (filter instanceof tizen.CompositeFilter) {
    var _f = [];
    var filters = filter.filters;

    for (var i = 0; i < filters.length; ++i) {
      _f.push(this.repackFilter(filters[i]));
    }

    return {
      filterType: 'CompositeFilter',
      type: filter.type,
      filters: _f
    };
  }

  return null;
};

var C = new Common();
