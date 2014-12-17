// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var RecurrenceRuleFrequency = {
  DAILY: 'DAILY',
  WEEKLY: 'WEEKLY',
  MONTHLY: 'MONTHLY',
  YEARLY: 'YEARLY'
};

var ByDayValue = {
  MO: 'MO',
  TU: 'TU',
  WE: 'WE',
  TH: 'TH',
  FR: 'FR',
  SA: 'SA',
  SU: 'SU'
};

var CalendarRecurrenceRuleInit = function(data) {
  var _interval = 1;
  var _untilDate = null;
  var _daysOfTheWeek = [];
  var _occurrenceCount = -1;
  var _setPositions = [];
  var _exceptions = [];

  function _validateDaysOfTheWeek(v) {
    if (T.isArray(v)) {
      var allowedValues = Object.keys(ByDayValue);
      for (var i = 0; i < v.length; ++i) {
        if (allowedValues.indexOf(v[i]) < 0) {
          return false;
        }
      }
      return true;
    }

    return false;
  }

  function _validateSetPositions(v) {
    var valid = false;

    if (T.isArray(v)) {
      for (var i = 0; i < v.length; i++) {
        v[i] = parseInt(v[i]);
        if (isNaN(v[i]) || (v[i] < -366 || v[i] > 366 || v[i] === 0)) {
          return false;
        }
      }
      valid = true;
    }
    return valid;
  }

  function _validateExceptions(v) {
    var valid = false;

    if (T.isArray(v)) {
      for (var i = 0; i < v.length; i++) {
        if (!(v[i] instanceof tizen.TZDate)) {
          return false;
        }
      }
      valid = true;
    }
    return valid;
  }

  Object.defineProperties(this, {
    interval: {
      get: function() {
        return _interval;
      },
      set: function(v) {
        _interval = (T.isNumber(v) && v > 0) ? v : _interval;
      },
      enumerable: true
    },
    untilDate: {
      get: function() {
        return _untilDate;
      },
      set: function(v) {
        if (v instanceof tizen.TZDate) {
          _untilDate = v;
        }
      },
      enumerable: true
    },
    occurrenceCount: {
      get: function() {
        return _occurrenceCount;
      },
      set: function(v) {
        if (T.isNumber(v) && v >= -1) {
          _occurrenceCount = v;
        }
      },
      enumerable: true
    },
    daysOfTheWeek: {
      get: function() {
        return _daysOfTheWeek;
      },
      set: function(v) {
        _daysOfTheWeek = _validateDaysOfTheWeek(v) ? v : _daysOfTheWeek;
      },
      enumerable: true
    },
    setPositions: {
      get: function() {
        return _setPositions;
      },
      set: function(v) {
        _setPositions = _validateSetPositions(v) ? v : _setPositions;
      },
      enumerable: true
    },
    exceptions: {
      get: function() {
        return _exceptions;
      },
      set: function(v) {
        _exceptions = _validateExceptions(v) ? v : _exceptions;
      },
      enumerable: true
    }
  });

  if (data instanceof Object) {
    for (var prop in data) {
      if (this.hasOwnProperty(prop)) {
        this[prop] = data[prop];
      }
    }
  }
};

var CalendarRecurrenceRule = function(frequency, ruleInitDict) {
  AV.isConstructorCall(this, CalendarRecurrenceRule);

  CalendarRecurrenceRuleInit.call(this, ruleInitDict);

  var _frequency = null;

  Object.defineProperties(this, {
    frequency: {
      get: function() {
        return _frequency;
      },
      set: function(v) {
        if (v === null) {
          return;
        }
        _frequency = Converter.toEnum(v, Object.keys(RecurrenceRuleFrequency), false);
      },
      enumerable: true
    }
  });

  // @todo fix UTC, according to documentation frequency is not optional
  this.frequency = (!frequency) ? 'DAILY' : frequency;
};

CalendarRecurrenceRule.prototype = new CalendarRecurrenceRuleInit();
CalendarRecurrenceRule.prototype.constructor = CalendarRecurrenceRule;

tizen.CalendarRecurrenceRule = CalendarRecurrenceRule;
