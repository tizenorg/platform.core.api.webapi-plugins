// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var _minuteInMilliseconds = 60 * 1000;
var _hourInMilliseconds = _minuteInMilliseconds * 60;

var utils_ = xwalk.utils;
var validator_ = utils_.validator;
var types_ = validator_.Types;
var native_ = new utils_.NativeManager(extension);

exports.getCurrentDateTime = function() {
  return new tizen.TZDate();
};

exports.getLocalTimezone = function() {
  var result = native_.callSync('Time_getLocalTimeZone');
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  return native_.getResultObject(result);
};

var _availableTimezonesCache = [];

exports.getAvailableTimezones = function() {
  if (_availableTimezonesCache.length)
    return _availableTimezonesCache;

  var result = native_.callSync('Time_getAvailableTimeZones');
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  _availableTimezonesCache = native_.getResultObject(result);
  return _availableTimezonesCache;
};

exports.getDateFormat = function() {
  var args = validator_.validateArgs(arguments, [{
    name: 'shortformat',
    type: types_.BOOLEAN,
    optional: true,
    nullable: true
  }]);

  if (!args.has.shortformat) {
    args.shortformat = false;
  }

  var result = native_.callSync('Time_getDateFormat', {shortformat: args.shortformat});
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  return native_.getResultObject(result);
};

exports.getTimeFormat = function() {
  var result = native_.callSync('Time_getTimeFormat');
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  return native_.getResultObject(result);
};

exports.isLeapYear = function(year) {
  if (year === undefined)
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR);

  return ((year % 4 === 0) && (year % 100 !== 0)) || (year % 400 === 0);
};

var _timeUtilDateTimeChangeListener;

function _timeUtilDateTimeChangeListenerCallback() {
  native_.callIfPossible(_timeUtilDateTimeChangeListener);
}

exports.setDateTimeChangeListener = function() {
  var args = validator_.validateArgs(arguments, [
    {
      name: 'changeCallback',
      type: types_.FUNCTION
    }
  ]);
  _timeUtilDateTimeChangeListener = args.changeCallback;
  native_.addListener('DateTimeChangeListener',
      _timeUtilDateTimeChangeListenerCallback);
  var result = native_.callSync('Time_setDateTimeChangeListener', {});
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

exports.unsetDateTimeChangeListener = function() {
  native_.removeListener('DateTimeChangeListener',
      _timeUtilDateTimeChangeListenerCallback);
  var result = native_.callSync('Time_unsetDateTimeChangeListener', {});
  _timeUtilDateTimeChangeListener = undefined;
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

var _timeUtilTimezoneChangeListener;

function _timeUtilTimezoneChangeListenerCallback() {
  native_.callIfPossible(_timeUtilTimezoneChangeListener);
}

exports.setTimezoneChangeListener = function() {
  var args = validator_.validateArgs(arguments, [
    {
      name: 'changeCallback',
      type: types_.FUNCTION
    }
  ]);

  _timeUtilTimezoneChangeListener = args.changeCallback;
  native_.addListener('TimezoneChangeListener',
      _timeUtilTimezoneChangeListenerCallback);
  var result = native_.callSync('Time_setTimezoneChangeListener', {});
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

exports.unsetTimezoneChangeListener = function() {
  native_.removeListener('TimezoneChangeListener',
      _timeUtilTimezoneChangeListenerCallback);
  var result = native_.callSync('Time_unsetTimezoneChangeListener', {});
  _timeUtilTimezoneChangeListener = undefined;
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

function _throwProperTizenException(e) {
  if (e instanceof TypeError)
    throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR);
  else if (e instanceof RangeError)
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR);
  else
    throw new WebAPIException(WebAPIException.UNKNOWN_ERR);
}

var TimeDurationUnit = {
  MSECS: 'MSECS',
  SECS: 'SECS',
  MINS: 'MINS',
  HOURS: 'HOURS',
  DAYS: 'DAYS'
};

tizen.TimeDuration = function(length, unit) {
  validator_.isConstructorCall(this, tizen.TimeDuration);

  var length_ = length !== null ? Math.floor(length) : 0;
  var unit_ = Object.keys(TimeDurationUnit).indexOf(unit) >= 0 ? unit : TimeDurationUnit.MSECS;

  Object.defineProperties(this, {
    length: {
      get: function() {
        return length_;
      },
      set: function(v) {
        if (v !== null) {
          length_ = Math.floor(v);
        }
      },
      enumerable: true
    },
    unit: {
      get: function() {
        return unit_;
      },
      set: function(v) {
        if (Object.keys(TimeDurationUnit).indexOf(v) >= 0) {
          unit_ = v;
        }
      },
      enumerable: true
    }
  });
};

function makeMillisecondsDurationObject(length) {
  var dayInMsecs = _hourInMilliseconds * 24;
  length = Math.floor(length);

  if ((length % dayInMsecs) === 0)
    return new tizen.TimeDuration(length / dayInMsecs, TimeDurationUnit.DAYS);

  return new tizen.TimeDuration(length, TimeDurationUnit.MSECS);
}

tizen.TimeDuration.prototype.getMilliseconds = function() {
  var m;
  switch (this.unit) {
    case TimeDurationUnit.MSECS:
      m = 1;
      break;
    case TimeDurationUnit.SECS:
      m = 1000;
      break;
    case TimeDurationUnit.MINS:
      m = 60 * 1000;
      break;
    case TimeDurationUnit.HOURS:
      m = 3600 * 1000;
      break;
    case TimeDurationUnit.DAYS:
      m = 86400 * 1000;
      break;
  }
  return m * this.length;
};

tizen.TimeDuration.prototype.difference = function() {
  var args = validator_.validateArgs(arguments, [{
    name: 'other',
    type: types_.PLATFORM_OBJECT,
    values: tizen.TimeDuration
  }]);

  try {
    return makeMillisecondsDurationObject(this.getMilliseconds() -
                                          args.other.getMilliseconds());
  } catch (e) {
    _throwProperTizenException(e);
  }
};

tizen.TimeDuration.prototype.equalsTo = function() {
  var args = validator_.validateArgs(arguments, [{
    name: 'other',
    type: types_.PLATFORM_OBJECT,
    values: tizen.TimeDuration
  }]);

  try {
    return this.getMilliseconds() === args.other.getMilliseconds();
  } catch (e) {
    _throwProperTizenException(e);
  }
};

tizen.TimeDuration.prototype.lessThan = function() {
  var args = validator_.validateArgs(arguments, [{
    name: 'other',
    type: types_.PLATFORM_OBJECT,
    values: tizen.TimeDuration
  }]);

  try {
    return this.getMilliseconds() < args.other.getMilliseconds();
  } catch (e) {
    _throwProperTizenException(e);
  }
};

tizen.TimeDuration.prototype.greaterThan = function() {
  var args = validator_.validateArgs(arguments, [{
    name: 'other',
    type: types_.PLATFORM_OBJECT,
    values: tizen.TimeDuration
  }]);

  try {
    return this.getMilliseconds() > args.other.getMilliseconds();
  } catch (e) {
    _throwProperTizenException(e);
  }
};

tizen.TimeDuration.prototype.toString = function() {
  return this.length + ' ' + this.unit;
};

function getTimezoneOffset(_timezone, _timeInMs) {
  var result = native_.callSync('Time_getTimeZoneOffset', {
    timezone: _timezone,
    value: _timeInMs
  });
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  return native_.getResultObject(result);
}

function getMsUTC(date, timezone) {
  var ms_utc = Date.UTC(date.getUTCFullYear(),
                        date.getUTCMonth(),
                        date.getUTCDate(),
                        date.getUTCHours(),
                        date.getUTCMinutes(),
                        date.getUTCSeconds(),
                        date.getUTCMilliseconds());
  if (arguments.length === 2) {
    var result = native_.callSync('Time_getMsUTC', {
      timezone: timezone,
      value: ms_utc
    });
    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }

    ms_utc = native_.getResultObject(result);
  }

  return ms_utc;
}

tizen.TZDate = function(year, month, day, hours, minutes, seconds, milliseconds, timezone) {
  validator_.isConstructorCall(this, tizen.TZDate);

  if (timezone) {
    if (tizen.time.getAvailableTimezones().indexOf(timezone) < 0) {
      throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR);
    }

    this.timezone_ = timezone;
  } else {
    this.timezone_ = tizen.time.getLocalTimezone();
  }

  var hours = hours || 0;
  var minutes = minutes || 0;
  var seconds = seconds || 0;
  var milliseconds = milliseconds || 0;

  if (!arguments.length) {
    this.date_ = new Date();
  } else if (arguments.length === 1 || arguments.length === 2) {
    if (arguments[0] instanceof Date) {
      this.date_ = arguments[0];
    } else {
      this.date_ = new Date();
    }
    if (arguments[1]) {
      if (tizen.time.getAvailableTimezones().indexOf(arguments[1]) < 0) {
        throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR);
      }
      this.timezone_ = arguments[1];
    }
  } else {
    this.date_ = {};
    if (timezone) {
      var d = new Date();
      d.setUTCFullYear(year);
      d.setUTCMonth(month);
      d.setUTCDate(day);
      d.setUTCHours(hours);
      d.setUTCMinutes(minutes);
      d.setUTCSeconds(seconds);
      d.setUTCMilliseconds(milliseconds);
      this.date_ = new Date(getMsUTC(d, timezone));
    } else {
      this.date_ = new Date(year, month, day, hours, minutes, seconds, milliseconds);
    }
  }
};

tizen.TZDate.prototype.getDate = function() {
  return this.date_.getDate();
};

tizen.TZDate.prototype.setDate = function() {
  var args = validator_.validateArgs(arguments, [{
    name: 'date',
    type: types_.LONG
  }]);

  this.date_.setDate(args.date);
};

tizen.TZDate.prototype.getDay = function() {
  return this.date_.getDay();
};

tizen.TZDate.prototype.getFullYear = function() {
  return this.date_.getFullYear();
};

tizen.TZDate.prototype.setFullYear = function() {
  var args = validator_.validateArgs(arguments, [{
    name: 'year',
    type: types_.LONG
  }]);

  this.date_.setFullYear(args.year);
};

tizen.TZDate.prototype.getHours = function() {
  var d = this.addDuration(new tizen.TimeDuration(getTimezoneOffset(this.timezone_,
                                           getMsUTC(this.date_))));
  return d.date_.getUTCHours();
};

tizen.TZDate.prototype.setHours = function() {
  var args = validator_.validateArgs(arguments, [{
    name: 'hours',
    type: types_.LONG
  }]);

  this.date_.setHours(args.hours);
};

tizen.TZDate.prototype.getMilliseconds = function() {
  return this.date_.getMilliseconds();
};

tizen.TZDate.prototype.setMilliseconds = function() {
  var args = validator_.validateArgs(arguments, [{
    name: 'ms',
    type: types_.LONG
  }]);

  this.date_.setMilliseconds(args.ms);
};

tizen.TZDate.prototype.getMonth = function() {
  return this.date_.getMonth();
};

tizen.TZDate.prototype.setMonth = function() {
  var args = validator_.validateArgs(arguments, [{
    name: 'month',
    type: types_.LONG
  }]);

  this.date_.setMonth(args.month);
};

tizen.TZDate.prototype.getMinutes = function() {
  return this.date_.getMinutes();
};

tizen.TZDate.prototype.setMinutes = function() {
  var args = validator_.validateArgs(arguments, [{
    name: 'minutes',
    type: types_.LONG
  }]);

  this.date_.setMinutes(args.minutes);
};

tizen.TZDate.prototype.getSeconds = function() {
  return this.date_.getSeconds();
};

tizen.TZDate.prototype.setSeconds = function() {
  var args = validator_.validateArgs(arguments, [{
    name: 'seconds',
    type: types_.LONG
  }]);

  this.date_.setSeconds(args.seconds);
};

tizen.TZDate.prototype.getUTCDate = function() {
  var d = this.addDuration(new tizen.TimeDuration(getTimezoneOffset(this.timezone_,
                                          getMsUTC(this.date_)) * -1));
  return d.getDate();
};

tizen.TZDate.prototype.setUTCDate = function() {
  var args = validator_.validateArgs(arguments, [{
    name: 'date',
    type: types_.LONG
  }]);

  this.date_.setUTCDate(args.date);
};

tizen.TZDate.prototype.getUTCDay = function() {
  var d = this.addDuration(new tizen.TimeDuration(getTimezoneOffset(this.timezone_,
                                         getMsUTC(this.date_)) * -1));
  return d.getDay();
};

tizen.TZDate.prototype.getUTCFullYear = function() {
  var d = this.addDuration(new tizen.TimeDuration(getTimezoneOffset(this.timezone_,
                                          getMsUTC(this.date_)) * -1));
  return d.getFullYear();
};

tizen.TZDate.prototype.setUTCFullYear = function() {
  var args = validator_.validateArgs(arguments, [{
    name: 'year',
    type: types_.LONG
  }]);

  this.date_.setUTCFullYear(args.year);
};

tizen.TZDate.prototype.getUTCHours = function() {
  return this.date_.getUTCHours();
};

tizen.TZDate.prototype.setUTCHours = function() {
  var args = validator_.validateArgs(arguments, [{
    name: 'hours',
    type: types_.LONG
  }]);

  var offset_hours = getTimezoneOffset(this.timezone_, getMsUTC(this.date_)) /
                     _hourInMilliseconds;
  this.date_.setHours(args.hours + offset_hours);
};

tizen.TZDate.prototype.getUTCMilliseconds = function() {
  var d = this.addDuration(new tizen.TimeDuration(getTimezoneOffset(this.timezone_,
                                              getMsUTC(this.date_)) * -1));
  return d.getMilliseconds();
};

tizen.TZDate.prototype.setUTCMilliseconds = function() {
  var args = validator_.validateArgs(arguments, [{
    name: 'ms',
    type: types_.LONG
  }]);

  this.date_.setUTCMilliseconds(args.ms);
};

tizen.TZDate.prototype.getUTCMinutes = function() {
  var d = this.addDuration(new tizen.TimeDuration(getTimezoneOffset(this.timezone_,
                                             getMsUTC(this.date_)) * -1));
  return d.getMinutes();
};

tizen.TZDate.prototype.setUTCMinutes = function() {
  var args = validator_.validateArgs(arguments, [{
    name: 'minutes',
    type: types_.LONG
  }]);

  this.date_.setUTCMinutes(args.minutes);
};

tizen.TZDate.prototype.getUTCMonth = function() {
  var d = this.addDuration(new tizen.TimeDuration(getTimezoneOffset(this.timezone_,
                                           getMsUTC(this.date_)) * -1));
  return d.getMonth();
};

tizen.TZDate.prototype.setUTCMonth = function() {
  var args = validator_.validateArgs(arguments, [{
    name: 'month',
    type: types_.LONG
  }]);

  this.date_.setUTCMonth(args.month);
};

tizen.TZDate.prototype.getUTCSeconds = function() {
  var d = this.addDuration(new tizen.TimeDuration(getTimezoneOffset(this.timezone_,
                                         getMsUTC(this.date_)) * -1));
  return d.getSeconds();
};

tizen.TZDate.prototype.setUTCSeconds = function() {
  var args = validator_.validateArgs(arguments, [{
    name: 'secs',
    type: types_.LONG
  }]);

  this.date_.setUTCSeconds(args.secs);
};

tizen.TZDate.prototype.getTime = function() {
  return this.date_.getTime();
};

tizen.TZDate.prototype.getTimezone = function() {
  return this.timezone_;
};

tizen.TZDate.prototype.toTimezone = function() {
  var args = validator_.validateArgs(arguments, [{
    name: 'timezone',
    type: types_.STRING
  }]);

  if (!args.timezone)
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR);

  return new tizen.TZDate(new Date(this.date_.getTime()), args.timezone);
};

tizen.TZDate.prototype.toLocalTimezone = function() {
  return this.toTimezone(tizen.time.getLocalTimezone());
};

tizen.TZDate.prototype.toUTC = function() {
  return this.toTimezone('GMT');
};

tizen.TZDate.prototype.difference = function() {
  var args = validator_.validateArgs(arguments, [{
    name: 'other',
    type: types_.PLATFORM_OBJECT,
    values: tizen.TZDate
  }]);

  try {
    return makeMillisecondsDurationObject(this.getTime() - args.other.getTime());
  } catch (e) {
    _throwProperTizenException(e);
  }
};

tizen.TZDate.prototype.equalsTo = function() {
  var args = validator_.validateArgs(arguments, [{
    name: 'other',
    type: types_.PLATFORM_OBJECT,
    values: tizen.TZDate
  }]);

  try {
    return this.getTime() === args.other.getTime();
  } catch (e) {
    _throwProperTizenException(e);
  }
};

tizen.TZDate.prototype.earlierThan = function() {
  var args = validator_.validateArgs(arguments, [{
    name: 'other',
    type: types_.PLATFORM_OBJECT,
    values: tizen.TZDate
  }]);

  try {
    return this.getTime() < args.other.getTime();
  } catch (e) {
    _throwProperTizenException(e);
  }
};

tizen.TZDate.prototype.laterThan = function(other) {
  var args = validator_.validateArgs(arguments, [{
    name: 'other',
    type: types_.PLATFORM_OBJECT,
    values: tizen.TZDate
  }]);

  try {
    return this.getTime() > args.other.getTime();
  } catch (e) {
    _throwProperTizenException(e);
  }
};

tizen.TZDate.prototype.addDuration = function() {
  var args = validator_.validateArgs(arguments, [{
    name: 'duration',
    type: types_.PLATFORM_OBJECT,
    values: tizen.TimeDuration
  }]);

  try {
    var date = new tizen.TZDate(new Date(this.date_.getTime()), this.timezone_);
    date.setMilliseconds(args.duration.getMilliseconds() + date.getMilliseconds());
    return date;
  } catch (e) {
    _throwProperTizenException(e);
  }
};

tizen.TZDate.prototype.toLocaleDateString = function() {
  var result = native_.callSync('Time_toDateString', {
    timezone: this.timezone_,
    value: this.date_.getTime(),
    trans: '',
    locale: true
  });
  if (native_.isFailure(result)) {
    return '';
  }

  return native_.getResultObject(result);
};

tizen.TZDate.prototype.toLocaleTimeString = function() {
  var result = native_.callSync('Time_toTimeString', {
    timezone: this.timezone_,
    value: this.date_.getTime(),
    trans: '',
    locale: true
  });
  if (native_.isFailure(result)) {
    return '';
  }

  return native_.getResultObject(result);
};

tizen.TZDate.prototype.toLocaleString = function() {
  var result = native_.callSync('Time_toString', {
    timezone: this.timezone_,
    value: this.date_.getTime(),
    trans: '',
    locale: true
  });
  if (native_.isFailure(result)) {
    return '';
  }

  return native_.getResultObject(result);
};

tizen.TZDate.prototype.toDateString = function() {
  var result = native_.callSync('Time_toDateString', {
    timezone: this.timezone_,
    value: this.date_.getTime(),
    trans: '',
    locale: false
  });
  if (native_.isFailure(result)) {
    return '';
  }

  return native_.getResultObject(result);
};

tizen.TZDate.prototype.toTimeString = function() {
  var result = native_.callSync('Time_toTimeString', {
    timezone: this.timezone_,
    value: this.date_.getTime(),
    trans: '',
    locale: false
  });
  if (native_.isFailure(result)) {
    return '';
  }

  return native_.getResultObject(result);
};

tizen.TZDate.prototype.toString = function() {
  var result = native_.callSync('Time_toString', {
    timezone: this.timezone_,
    value: this.date_.getTime(),
    trans: '',
    locale: false
  });
  if (native_.isFailure(result)) {
    return '';
  }

  return native_.getResultObject(result);
};

tizen.TZDate.prototype.getTimezoneAbbreviation = function() {
  var result = native_.callSync('Time_getTimeZoneAbbreviation', {
    timezone: this.timezone_,
    value: this.date_.getTime()
  });
  if (native_.isFailure(result)) {
    return '';
  }

  return native_.getResultObject(result);
};

tizen.TZDate.prototype.secondsFromUTC = function() {
  return this.date_.getTimezoneOffset() * 60;
};

tizen.TZDate.prototype.isDST = function() {
  var result = native_.callSync('Time_isDST', {
    timezone: this.timezone_,
    value: getMsUTC(this.date_)
  });
  if (native_.isFailure(result)) {
    return false;
  }

  return native_.getResultObject(result);
};

tizen.TZDate.prototype.getPreviousDSTTransition = function() {
  var result = native_.callSync('Time_getDSTTransition', {
    'timezone': this.timezone_,
    'value': getMsUTC(this.date_),
    'trans': 'NEXT_TRANSITION'
  });
  if (native_.isFailure(result)) {
    return null;
  }
  var _result = native_.getResultObject(result);
  if (result.error || _result === 0)
    return null;
  var OffsetInMilliseconds = this.date_.getTimezoneOffset() * _minuteInMilliseconds * -1;
  return new tizen.TZDate(new Date(_result - OffsetInMilliseconds), this.timezone_);
};

tizen.TZDate.prototype.getNextDSTTransition = function() {
  var result = native_.callSync('Time_getDSTTransition', {
    timezone: this.timezone_,
    value: getMsUTC(this.date_),
    trans: 'PREV_TRANSITION'
  });
  if (native_.isFailure(result)) {
    return null;
  }
  var _result = native_.getResultObject(result);
  if (result.error || _result === 0)
    return null;
  var OffsetInMilliseconds = this.date_.getTimezoneOffset() * _minuteInMilliseconds * -1;
  return new tizen.TZDate(new Date(_result - OffsetInMilliseconds), this.timezone_);
};
