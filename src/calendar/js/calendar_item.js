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

var CalendarTextFormat = {
  ICALENDAR_20: 'ICALENDAR_20',
  VCALENDAR_10: 'VCALENDAR_10'
};

var CalendarItemVisibility = {
  PUBLIC: 'PUBLIC', //default
  PRIVATE: 'PRIVATE',
  CONFIDENTIAL: 'CONFIDENTIAL'
};

var CalendarItemPriority = {
  HIGH: 'HIGH',
  MEDIUM: 'MEDIUM',
  LOW: 'LOW',
  NONE: 'NONE' //default
};

var CalendarItemStatus = {
  TENTATIVE: 'TENTATIVE',
  CONFIRMED: 'CONFIRMED',
  CANCELLED: 'CANCELLED',
  NONE: 'NONE', //default for both CalendarEvent and CalendarTask
  NEEDS_ACTION: 'NEEDS_ACTION',
  IN_PROCESS: 'IN_PROCESS',
  COMPLETED: 'COMPLETED'
};

var EventAvailability = {
  BUSY: 'BUSY', //default for CalendarEvent
  FREE: 'FREE'
};

var CalendarEventId = function(uid, rid) {
  validator_.isConstructorCall(this, CalendarEventId);

  var _uid = null;

  Object.defineProperties(this, {
    uid: {
      get: function() {
        return _uid;
      },
      set: function(v) {
        if (v === null) {
          return;
        }
        _uid = converter_.toString(v, true);
      },
      enumerable: true
    },
    rid: {
      value: (rid) ? converter_.toString(rid, true) : null,
      writable: true,
      enumerable: true
    }
  });

  this.uid = uid;
};

// class CalendarItem
var CalendarItem = function(data) {
  var _id = null;
  var _calendarId = null;
  var _lastModificationDate = null;
  var _description = '';
  var _summary = '';
  var _isAllDay = false;
  var _startDate = null;
  var _duration = null;
  var _location = '';
  var _geolocation = null;
  var _organizer = '';
  var _visibility = CalendarItemVisibility.PUBLIC;
  var _status = CalendarItemStatus.NONE;
  var _priority = CalendarItemPriority.NONE;
  var _alarms = [];
  var _categories = [];
  var _attendees = [];

  function _validateAlarms(v) {
    var valid = false;

    if (type_.isArray(v)) {
      for (var i = 0; i < v.length; i++) {
        if (!(v[i] instanceof tizen.CalendarAlarm)) {
          return false;
        }
      }
      valid = true;
    }
    return valid;
  }

  function _validateAttendees(v) {
    var valid = false;

    if (type_.isArray(v)) {
      for (var i = 0; i < v.length; i++) {
        if (!(v[i] instanceof tizen.CalendarAttendee)) {
          return false;
        }
      }
      valid = true;
    }
    return valid;
  }

  function _validateCategories(v) {
    var valid = false;

    if (type_.isArray(v)) {
      for (var i = 0; i < v.length; i++) {
        if (!(type_.isString(v[i]))) {
          return false;
        }
      }
      valid = true;
    }
    return valid;
  }

  Object.defineProperties(this, {
    id: {
      get: function() {
        return _id;
      },
      set: function(v) {
        if (_edit.canEdit) {
          if (v instanceof _global.Object) {
            _id = new CalendarEventId(v.uid, v.rid);
          } else {
            _id = converter_.toString(v, true);
          }
        }
      },
      enumerable: true
    },
    calendarId: {
      get: function() {
        return _calendarId;
      },
      set: function(v) {
        if (_edit.canEdit) {
          _calendarId = v;
        }
      },
      enumerable: true
    },
    lastModificationDate: {
      get: function() {
        return _lastModificationDate;
      },
      set: function(v) {
        if (_edit.canEdit) {
          _lastModificationDate = v instanceof tizen.TZDate ? v :
              tizen.time.getCurrentDateTime();
        }
      },
      enumerable: true
    },
    description: {
      get: function() {
        return _description;
      },
      set: function(v) {
        _description = v ? converter_.toString(v, true) : _description;
      },
      enumerable: true
    },
    summary: {
      get: function() {
        return _summary;
      },
      set: function(v) {
        _summary = v ? converter_.toString(v, true) : _summary;
      },
      enumerable: true
    },
    isAllDay: {
      get: function() {
        return _isAllDay;
      },
      set: function(v) {
        _isAllDay = v ? converter_.toBoolean(v) : _isAllDay;
      },
      enumerable: true
    },
    startDate: {
      get: function() {
        return _startDate;
      },
      set: function(v) {
        _startDate = v instanceof tizen.TZDate ? v : _startDate;
        this.duration = _duration;
      },
      enumerable: true
    },
    duration: {
      get: function() {
        return _duration;
      },
      set: function(v) {
        // set duration as dueDate or endDate
        var _startDate = this.startDate ?
            this.startDate : tizen.time.getCurrentDateTime();
        if (this instanceof tizen.CalendarEvent) {
          this.endDate = v instanceof tizen.TimeDuration ?
              _startDate.addDuration(v) : this.endDate;
        } else {
          this.dueDate = v instanceof tizen.TimeDuration ?
              _startDate.addDuration(v) : this.dueDate;
        }
        _duration = v instanceof tizen.TimeDuration ? v : null;
        //@todo Fix UTC, UTC expect duration value but according to documentation:
        // ... the implementation may not save the duration itself,
        // rather convert it to the corresponding endDate/dueDate attribute and save it.
        // For example, if you set the startDate and the duration attributes and save the item,
        // you may see that the duration is null while endDate/dueDate is non-null
        // after retrieving it because the implementation has calculated the endDate/dueDate
        // based on the duration and the startDate then saved it, not the duration.
      },
      enumerable: true
    },
    location: {
      get: function() {
        return _location;
      },
      set: function(v) {
        _location = v ? converter_.toString(v) : _location;
      },
      enumerable: true
    },
    geolocation: {
      get: function() {
        return _geolocation;
      },
      set: function(v) {
        _geolocation = v instanceof tizen.SimpleCoordinates ? v : _geolocation;
      },
      enumerable: true
    },
    organizer: {
      get: function() {
        return _organizer;
      },
      set: function(v) {
        _organizer = v ? converter_.toString(v) : _organizer;
      },
      enumerable: true
    },
    visibility: {
      get: function() {
        return _visibility;
      },
      set: function(v) {
        _visibility = v ? converter_.toEnum(v, Object.keys(CalendarItemVisibility), false) :
                _visibility;
      },
      enumerable: true
    },
    status: {
      get: function() {
        return _status;
      },
      set: function(v) {
        if (v === null) {
          return;
        }
        if (this instanceof tizen.CalendarEvent) {
          _status = v ? converter_.toEnum(v, Object.keys(CalendarItemStatus).slice(0, 4), false) :
                        CalendarItemStatus.NONE;
        } else {
          _status = v ? converter_.toEnum(v, Object.keys(CalendarItemStatus).slice(2), false) :
                        CalendarItemStatus.NONE;
        }
      },
      enumerable: true
    },
    priority: {
      get: function() {
        return _priority;
      },
      set: function(v) {
        if (v === null) {
          return;
        }
        _priority = v ? converter_.toEnum(v, Object.keys(CalendarItemPriority), false) :
                    _status;
      },
      enumerable: true
    },
    alarms: {
      get: function() {
        return _alarms;
      },
      set: function(v) {
        _alarms = _validateAlarms(v) ? v : _alarms;
      },
      enumerable: true
    },
    categories: {
      get: function() {
        return _categories;
      },
      set: function(v) {
        _categories = _validateCategories(v) ? v : _categories;
      },
      enumerable: true
    },
    attendees: {
      get: function() {
        return _attendees;
      },
      set: function(v) {
        _attendees = _validateAttendees(v) ? v : _attendees;
      },
      enumerable: true
    }
  });

  if (data instanceof _global.Object) {
    for (var prop in data) {
      if (this.hasOwnProperty(prop)) {
        this[prop] = data[prop];
      }
    }
  }

};

CalendarItem.prototype.convertToString = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.CALENDAR_READ);

  var args = validator_.validateArgs(arguments, [
    {
      name: 'format',
      type: types_.ENUM,
      values: Object.keys(CalendarTextFormat)
    }
  ]);

  var _checkNumber = function(n) {
    return n < 10 ? '0' + n : n;
  };

  var _timeFormat = function(d) {
    return ';TZID=' + d.getTimezone() +
        ':' + d.getFullYear() + _checkNumber(d.getMonth()) + _checkNumber(d.getDate()) +
            'T' + _checkNumber(d.getHours()) + _checkNumber(d.getMinutes()) +
            _checkNumber(d.getSeconds() + 'Z');
  };

  var _this = this;
  var _dtStart = '';

  if (_this.startDate) {
    _dtStart = _timeFormat(_this.startDate);
  } else {
    _dtStart = _timeFormat(tizen.time.getCurrentDateTime());
  }

  var _dtEnd = _dtStart;

  if (_this.endDate) {
    _dtEnd = _timeFormat(_this.endDate);
  } else if (_this.dueDate) {
    _dtEnd = _timeFormat(_this.dueDate);
  }

  var _description = _this.description.length ? ':' + _this.description : '';
  var _location = _this.location.length ? ':' + _this.location : '';
  var _organizer = _this.organizer.length ? ';CN=' + _this.organizer : '';
  var _priority = _this.priority.length ? ':' + _this.priority : '';
  var _summary = _this.summary.length ? ':' + _this.summary : '';
  var _categories = _this.categories.length ? ':' + _this.categories.join(', ') : '';
  var _visibility = _this.visibility;
  var _status = _this.status;
  var _version = args.format === CalendarTextFormat.ICALENDAR_20 ? ':2.0' : ':1.0';

  var vEven = [
    'BEGIN:VCALENDAR',
    'VERSION' + _version,
    'BEGIN:VEVENT',
    'CLASS:' + _visibility,
    'TRANSP:OPAQUE',
    'DTSTART' + _dtStart,
    'DESCRIPTION' + _description,
    'LOCATION' + _location,
    'ORGANIZER' + _organizer,
    'PRIORITY' + _priority,
    'SUMMARY' + _summary,
    'DTEND' + _dtEnd,
    'CATEGORIES' + _categories,
    'END:VEVENT',
    'END:VCALENDAR'
  ].join('\n');

  var vTodo = [
    'BEGIN:VCALENDAR',
    'VERSION' + _version,
    'BEGIN:VTODO',
    'SUMMARY' + _summary,
    'DUE' + _dtEnd,
    'STATUS' + _status,
    'END:VTODO',
    'END:VCALENDAR'
  ].join('\n');

  if (this instanceof tizen.CalendarTask) {
    return vTodo;
  } else {
    return vEven;
  }

};

CalendarItem.prototype.clone = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.CALENDAR_READ);

  var tmp = _itemConverter.toTizenObject(_itemConverter.fromTizenObject(this));

  tmp.id = null;

  return this instanceof tizen.CalendarEvent ? new tizen.CalendarEvent(tmp) :
      new tizen.CalendarTask(tmp);
};

function _convertFromStringToItem(str) {
  if (str.indexOf('VCALENDAR') === -1) {
    return;
  }

  var _startDate = null;
  var _description = '';
  var _location = null;
  var _organizer = null;
  var _priority = null;
  var _summary = null;
  var _categories = [];
  var _visibility = null;
  var _status = null;
  var _endDate = null;
  var _dueDate = null;
  var sep;

  if (str.indexOf('\r\n') > -1) {
    sep = '\r\n';
  } else if (str.indexOf('\n') > -1) {
    sep = '\n';
  } else {
    return;
  }

  function _convertTime(v) {
    var y = parseInt(v.substring(0, 4) , 10);
    var m = parseInt(v.substring(4, 6) , 10);
    var d = parseInt(v.substring(6, 8) , 10);
    var h = parseInt(v.substring(9, 11) , 10);
    var n = parseInt(v.substring(11, 13) , 10);
    var s = parseInt(v.substring(13, 15) , 10);

    return new tizen.TZDate(y, m, d, h, n, s);
  }

  var arr = str.split(sep);

  for (var i = 0; i < arr.length; i++) {
    if (arr[i].indexOf('SUMMARY') > -1) {
      _summary = arr[i].split(':')[1];
    } else if (arr[i].indexOf('CATEGORIES') > -1) {
      var c = arr[i].split(':')[1];
      _categories = c.split(',');
    } else if (arr[i].indexOf('ORGANIZER') > -1) {
      _organizer = arr[i].split('=')[1];
    } else if (arr[i].indexOf('DESCRIPTION') > -1) {
      _description = arr[i].split(':')[1];
    } else if (arr[i].indexOf('CLASS') > -1) {
      _visibility = arr[i].split(':')[1];
    } else if (arr[i].indexOf('LOCATION') > -1) {
      _location = arr[i].split(':')[1];
    } else if (arr[i].indexOf('PRIORITY') > -1) {
      _priority = arr[i].split(':')[1];
    } else if (arr[i].indexOf('STATUS') > -1) {
      _status = arr[i].split(':')[1];
    } else if (arr[i].indexOf('DTSTART') > -1) {
      _startDate = _convertTime(arr[i].split(':')[1]);
    } else if (arr[i].indexOf('DTEND') > -1) {
      _endDate = _convertTime(arr[i].split(':')[1]);
    } else if (arr[i].indexOf('DUE') > -1) {
      _dueDate = _convertTime(arr[i].split(':')[1]);
    }
  }

  return {
    visibility: _visibility,
    startDate: _startDate,
    description: _description,
    location: _location,
    organizer: _organizer,
    priority: _priority,
    summary: _summary,
    status: _status,
    categories: _categories,
    endDate: _endDate,
    dueDate: _dueDate
  };

}

var CalendarTaskInit = function(data) {
  CalendarItem.call(this, {
    status: CalendarItemStatus.NONE
  });

  var _dueDate = null;
  var _completedDate = null;
  var _progress = 0;

  Object.defineProperties(this, {
    dueDate: {
      get: function() {
        return _dueDate;
      },
      set: function(v) {
        if (!v instanceof tizen.TZDate && this.startDate) {
          v = this.startDate;
        }

        _dueDate = v instanceof tizen.TZDate ? v : _dueDate;
      },
      enumerable: true
    },
    completedDate: {
      get: function() {
        return _completedDate;
      },
      set: function(v) {
        _completedDate = v instanceof tizen.TZDate ? v : _completedDate;
      },
      enumerable: true
    },
    progress: {
      get: function() {
        return _progress;
      },
      set: function(v) {
        if (v === null) {
          return;
        }
        _progress = (type_.isNumber(v) && (v >= 0 || v <= 100)) ? v : _progress;
      },
      enumerable: true
    }
  });

  if (data instanceof _global.Object) {
    for (var prop in data) {
      if (this.hasOwnProperty(prop)) {
        this[prop] = data[prop];
      }
    }
  }
};

var CalendarTask = function(taskInitDict, format) {
  validator_.isConstructorCall(this, CalendarTask);

  if (type_.isString(taskInitDict) && Object.keys(CalendarTextFormat).indexOf(format) > -1) {
    CalendarTaskInit.call(this, _convertFromStringToItem(taskInitDict));
  } else {
    CalendarTaskInit.call(this, taskInitDict);
  }
};

CalendarTask.prototype = new CalendarItem();
CalendarTask.prototype.constructor = CalendarTask;


var CalendarEventInit = function(data) {
  CalendarItem.call(this, {
    status: CalendarItemStatus.NONE
  });

  var _isDetached = false;
  var _endDate = null;
  var _availability = EventAvailability.BUSY;
  var _recurrenceRule = null;

  var _validateReccurence = function(v) {
    if (_isDetached && v !== null) {
      throw new WebAPIException(WebAPIException.NOT_SUPPORTED_ERR,
        'Recurrence can\'t be set because event is detached');
    }

    if (v === null || v instanceof tizen.CalendarRecurrenceRule) {
      return v;
    } else {
      return _recurrenceRule;
    }
  };

  Object.defineProperties(this, {
    isDetached: {
      get: function() {
        return _isDetached;
      },
      set: function(v) {
        if (_edit.canEdit) {
          _isDetached = v;
        }
      },
      enumerable: true
    },
    endDate: {
      get: function() {
        return _endDate;
      },
      set: function(v) {
        if (!v instanceof tizen.TZDate && this.startDate) {
          v = this.startDate;
        }

        _endDate = v instanceof tizen.TZDate ? v : _endDate;
      },
      enumerable: true
    },
    availability: {
      get: function() {
        return _availability;
      },
      set: function(v) {
        _availability = Object.keys(EventAvailability).indexOf(v) > -1 ? v :
                _availability;
      },
      enumerable: true
    },
    recurrenceRule: {
      get: function() {
        return _recurrenceRule;
      },
      set: function(v) {
        _recurrenceRule = _validateReccurence(v);
      },
      enumerable: true
    }
  });

  if (data instanceof _global.Object) {
    for (var prop in data) {
      if (this.hasOwnProperty(prop)) {
        this[prop] = data[prop];
      }
    }
  }
};

var CalendarEvent = function(eventInitDict, format) {
  validator_.isConstructorCall(this, CalendarEvent);

  if (type_.isString(eventInitDict) && Object.keys(CalendarTextFormat).indexOf(format) > -1) {
    CalendarEventInit.call(this, _convertFromStringToItem(eventInitDict));
  } else {
    CalendarEventInit.call(this, eventInitDict);
  }
};

CalendarEvent.prototype = new CalendarItem();
CalendarEvent.prototype.constructor = CalendarEvent;

CalendarEvent.prototype.expandRecurrence = function(startDate, endDate, successCallback, errorCallback) {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.CALENDAR_READ);

  if (arguments.length < 3) {
    throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR);
  }
  if (!(startDate instanceof tizen.TZDate)) {
    throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR);
  }
  if (!(endDate instanceof tizen.TZDate)) {
    throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR);
  }
  if (typeof successCallback !== 'function') {
    throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR);
  }
  if (errorCallback) {
    if (typeof errorCallback !== 'function') {
      throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR);
    }
  }
  if (!(this.recurrenceRule instanceof tizen.CalendarRecurrenceRule)) {
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
      'The event is not recurring.');
  }

  var args = validator_.validateArgs(arguments, [
    {
      name: 'startDate',
      type: types_.PLATFORM_OBJECT,
      values: tizen.TZDate
    },
    {
      name: 'endDate',
      type: types_.PLATFORM_OBJECT,
      values: tizen.TZDate
    },
    {
      name: 'successCallback',
      type: types_.FUNCTION,
      nullable: true
    },
    {
      name: 'errorCallback',
      type: types_.FUNCTION,
      optional: true,
      nullable: true
    }
  ]);

  // invoke callbacks in "next tick"
  setTimeout(function() {
    var result = _recurrenceManager.get(this, startDate, endDate);

    if (result instanceof Array) {
      args.successCallback(result);
    } else if (args.errorCallback) {
      args.errorCallback(result);
    }
  }.bind(this), 1);
};

tizen.CalendarEventId = CalendarEventId;
tizen.CalendarEvent = CalendarEvent;
tizen.CalendarTask = CalendarTask;
