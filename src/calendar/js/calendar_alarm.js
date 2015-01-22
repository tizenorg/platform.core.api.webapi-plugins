// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var AlarmMethod = {
  SOUND: 'SOUND',
  DISPLAY: 'DISPLAY'
};

var CalendarAlarm = function(time, method, description) {
  validator_.isConstructorCall(this, CalendarAlarm);

  var _absoluteDate = time instanceof tizen.TZDate && !this.before ? time : null;
  var _before = time instanceof tizen.TimeDuration && !this.absoluteDate ? time : null;
  var _method = converter_.toEnum(method, Object.keys(AlarmMethod), false);
  var _description = (description) ? converter_.toString(description, true) : '';

  Object.defineProperties(this, {
    absoluteDate: {
      get: function() {
        return _absoluteDate;
      },
      set: function(v) {
        _absoluteDate = v instanceof tizen.TZDate && !_before ? v : null;
      },
      enumerable: true
    },
    before: {
      get: function() {
        return _before;
      },
      set: function(v) {
        _before = v instanceof tizen.TimeDuration && !_absoluteDate ? v : null;
      },
      enumerable: true
    },
    method: {
      get: function() {
        return _method;
      },
      set: function(v) {
        if (v === null) {
          return;
        }
        _method = converter_.toEnum(v, Object.keys(AlarmMethod), false);
      },
      enumerable: true
    },
    description: {
      get: function() {
        return _description;
      },
      set: function(v) {
        _description = converter_.toString(v, true);
      },
      enumerable: true
    }
  });
};

tizen.CalendarAlarm = CalendarAlarm;
