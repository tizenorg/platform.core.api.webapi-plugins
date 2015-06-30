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

var _global = window || global || {};

var utils_ = xwalk.utils;
var dateConverter_ = utils_.dateConverter;
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

var ItemConverter = function() {};

ItemConverter.prototype.toTizenObject = function(item) {
  var tmp = {};
  for (var prop in item) {
    if (prop === 'startDate' ||
            prop === 'endDate' ||
            prop === 'dueDate' ||
            prop === 'completedDate' ||
            prop === 'lastModificationDate') {
      tmp[prop] = dateConverter_.toTZDate(item[prop], item.isAllDay);
    } else {
      tmp[prop] = item[prop];
    }
  }

  var alarms = [];
  var alarm, time;
  for (var i = 0; i < tmp.alarms.length; i++) {
    alarm = tmp.alarms[i];
    if (alarm.absoluteDate) {
      time = dateConverter_.toTZDate(alarm.absoluteDate, tmp.isAllDay);
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
    untilDate = dateConverter_.toTZDate(tmp.recurrenceRule.untilDate, tmp.isAllDay);
    tmp.recurrenceRule.untilDate = untilDate;

    for (var i = 0; i < tmp.recurrenceRule.exceptions.length; i++) {
      exceptions.push(dateConverter_.toTZDate(tmp.recurrenceRule.exceptions[i], tmp.isAllDay));
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
      tmp[prop] = dateConverter_.fromTZDate(item[prop]);
    } else if (item[prop] instanceof Array) {
      tmp[prop] = [];
      for (var i = 0, length = item[prop].length; i < length; i++) {
        if (item[prop][i] instanceof _global.Object) {
          tmp[prop][i] = {};
          for (var p in item[prop][i]) {
            if (item[prop][i][p] instanceof tizen.TZDate) {
              tmp[prop][i][p] = dateConverter_.fromTZDate(item[prop][i][p]);
            } else {
              tmp[prop][i][p] = item[prop][i][p];
            }
          }
        } else {
          tmp[prop] = item[prop];
        }
      }
    } else if (item[prop] instanceof _global.Object) {
      tmp[prop] = {};
      for (var p in item[prop]) {
        if (item[prop][p] instanceof tizen.TZDate) {
          tmp[prop][p] = dateConverter_.fromTZDate(item[prop][p]);
        } else if (item[prop][p] instanceof Array) {
          tmp[prop][p] = [];
          for (var j = 0, l = item[prop][p].length; j < l; j++) {
            tmp[prop][p].push(dateConverter_.fromTZDate(item[prop][p][j]));
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
