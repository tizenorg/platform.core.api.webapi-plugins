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

var utils_ = xwalk.utils;
var validator_ = utils_.validator;
var types_ = validator_.Types;
var native_ = new utils_.NativeManager(extension);
var converter_ = utils_.converter;
var T = utils_.type;

var _GMT_ID = 'GMT';
var _LOCAL_ID = '__local__';

function _createShiftedDate(tzDate) {
  return new Date(tzDate._shiftedTimestamp);
}

function _createUTCDate(tzDate) {
  return new Date(tzDate._utcTimestamp);
}

function _fill(date, tzDate) {
  tzDate._shiftedTimestamp = date.getTime();
  tzDate._utcTimestamp = Number(tzDate._shiftedTimestamp) - Number(tzDate._timezoneOffset);
}

function _fillWithUTC(date, tzDate) {
  tzDate._utcTimestamp = date.getTime();
  tzDate._shiftedTimestamp = Number(tzDate._utcTimestamp) + Number(tzDate._timezoneOffset);
}

function PrivateTZDate(timestamp, timezone, offset) {
  Object.defineProperties(this, {
    ts : {value: timestamp, writable: false, enumerable: false},
    tzId : {value: timezone, writable: false, enumerable: false},
    o : {value: offset, writable: false, enumerable: false}
  });
}

function _getTimezoneOffset(timestamp, tzName) {
  var callArgs = {
      timezone  : converter_.toString(tzName),
      timestamp : converter_.toString(timestamp)
  };
  var result = native_.callSync('TZDate_getTimezoneOffset', callArgs);
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  var res = {
      offset : converter_.toLong(native_.getResultObject(result).offset),
      modifier : converter_.toLong(native_.getResultObject(result).modifier)
  };
  return res;
}

function _getLocalTimezoneOffset(utcTimestamp) {
  return -1 * (new Date(utcTimestamp).getTimezoneOffset()) * 60 * 1000; // cast to milliseconds
}

function _constructTZDate(obj, privateTZDate) {
  var utcTimestamp = privateTZDate.ts;
  var tzName = privateTZDate.tzId;
  var offset = privateTZDate.o;

  switch (tzName) {
  case _LOCAL_ID:
    console.log('Entered _constructTZDate for local timezone');

    tzName = tizen.time.getLocalTimezone();

    if (T.isNullOrUndefined(offset)) {
      offset = _getLocalTimezoneOffset(utcTimestamp);
    }
    break;

  case _GMT_ID:
    console.log('Entered _constructTZDate for GMT');

    if (T.isNullOrUndefined(offset)) {
      offset = 0;
    }
    break;

  default:
    console.log('Entered _constructTZDate for: ' + tzName);
    if (T.isNullOrUndefined(offset)) {
      // throws if tzName is invalid
      offset = _getTimezoneOffset(utcTimestamp, tzName).offset;
    }
    break;
  }

  Object.defineProperties(obj, {
    _utcTimestamp : {value: utcTimestamp, writable: true, enumerable: false},
    _shiftedTimestamp : {value: utcTimestamp + offset, writable: true, enumerable: false},
    _timezoneName : {value: tzName, writable: true, enumerable: false},
    _timezoneOffset : {value: offset, writable: true, enumerable: false}
  });
}

//class TZDate ////////////////////////////////////////////////////
tizen.TZDate = function(p1, p2, day, hours, minutes, seconds, milliseconds, timezone) {
  console.log("Entered tizen.TZDate");
  validator_.validateConstructorCall(this, tizen.TZDate);

  var priv;
  //copy constructor section (should be only for private usage)
  if (p1 instanceof PrivateTZDate) {
    priv = p1;
  } else {
    //Public constructor section
    console.log('Entered TZDate constructor with: ' + arguments.length + ' attributes');

    var date;

    if (arguments.length < 3) {
      if (T.isDate(p1)) {
        date = p1;
      } else {
        date = new Date();
      }
      timezone = p2;
    } else {
      p1 = p1 ? p1 : 0;
      p2 = p2 ? p2 : 0;
      day = day ? day : 0;
      hours = hours ? hours : 0;
      minutes = minutes ? minutes : 0;
      seconds = seconds ? seconds : 0;
      milliseconds = milliseconds ? milliseconds : 0;

      date = new Date(p1, p2, day, hours, minutes, seconds, milliseconds);
    }

    var utcTimestamp = date.getTime();
    var offset = _getLocalTimezoneOffset(utcTimestamp);
    var tzName = _LOCAL_ID;

    if (!T.isNullOrUndefined(timezone)) {
      timezone = converter_.toString(timezone);
      var timezoneTimestamp = new Date(Date.UTC(date.getFullYear(),
          date.getMonth(),
          date.getDate(),
          date.getHours(),
          date.getMinutes(),
          date.getSeconds(),
          date.getMilliseconds())).getTime();
      try {
        var offsetObject = _getTimezoneOffset(timezoneTimestamp, timezone);
        offset = offsetObject.offset;
        utcTimestamp = timezoneTimestamp - offset;
        //correction of missing/extra hour on DST change
        var modifier = offsetObject.modifier;
        if (modifier > 0) {
          //this is for case when 2AM becomes 3AM (but offset must be corrected -
              //missing one hour)
          offset += modifier;
        } else {
          //this is for case when extra hour appers - prevents error of
          //unnecessary shift of hour when timezone changes
          offset -= modifier;
          utcTimestamp += modifier;
        }
        tzName = timezone;
      } catch(e) {
        // in case of exception we fall back to local time zone
      }
    }

    priv = new PrivateTZDate(utcTimestamp, tzName, offset);
  }

  _constructTZDate(this, priv);
};

tizen.TZDate.prototype.getDate = function() {
  console.log('Entered TZDate.getDate');
  //getters realized with pattern
  //---> use _shiftedTimestamp (_utcTimestamp (UTC) with added _timezoneOffset)
  //---> create Date instance
  //---> return d.getUTCDate()  --- to avoid locale timezone impact of JS Date object
  return _createShiftedDate(this).getUTCDate();
};

function _updateTZDate(tzdate, args, param, func) {
  var a = validator_.validateMethod(args, [
                                           {
                                             name : param,
                                             type : validator_.Types.LONG
                                           }
                                           ]);

  //setters realized with pattern
  //---> use _shiftedTimestamp (_utcTimestamp (UTC) with added _timezoneOffset)
  //---> create Date instance
  //---> setUTCDate of JS Date object
  //---> getTime of object to set _shiftedTimestmp (avoiding timezone of JS Date)
  //---> fix _utcTimestamp with subtraction of _timezoneOffset
  var date = _createShiftedDate(tzdate);
  date[func](a[param]);
  _fill(date, tzdate);
}

function _updateTZDateUTC(tzdate, args, param, func) {
  var a = validator_.validateMethod(args, [
                                           {
                                             name : param,
                                             type : validator_.Types.LONG
                                           }
                                           ]);
  var date = _createUTCDate(tzdate);
  date[func](a[param]);
  _fillWithUTC(date, tzdate);
}

tizen.TZDate.prototype.setDate = function() {
  console.log('Entered TZDate.setDate');
  _updateTZDate(this, arguments, 'date', 'setUTCDate');
};

tizen.TZDate.prototype.getDay = function() {
  console.log('Entered TZDate.getDay');
  return _createShiftedDate(this).getUTCDay();
};

tizen.TZDate.prototype.getFullYear = function() {
  console.log('Entered TZDate.getFullYear');
  return _createShiftedDate(this).getUTCFullYear();
};

tizen.TZDate.prototype.setFullYear = function() {
  console.log('Entered TZDate.setFullYear');
  _updateTZDate(this, arguments, 'year', 'setUTCFullYear');
};

tizen.TZDate.prototype.getHours = function() {
  console.log('Entered TZDate.getHours');
  return _createShiftedDate(this).getUTCHours();
};

tizen.TZDate.prototype.setHours = function() {
  console.log('Entered TZDate.setHours');
  _updateTZDate(this, arguments, 'hours', 'setUTCHours');
};

tizen.TZDate.prototype.getMilliseconds = function() {
  console.log('Entered TZDate.getMilliseconds');
  return _createShiftedDate(this).getUTCMilliseconds();
};

tizen.TZDate.prototype.setMilliseconds = function() {
  console.log('Entered TZDate.setMilliseconds');
  _updateTZDate(this, arguments, 'ms', 'setUTCMilliseconds');
};

tizen.TZDate.prototype.getMinutes = function() {
  console.log('Entered TZDate.getMinutes');
  return _createShiftedDate(this).getUTCMinutes();
};

tizen.TZDate.prototype.setMinutes = function() {
  console.log('Entered TZDate.setMinutes');
  _updateTZDate(this, arguments, 'minutes', 'setUTCMinutes');
};

tizen.TZDate.prototype.getMonth = function() {
  console.log('Entered TZDate.getMonth');
  return _createShiftedDate(this).getUTCMonth();
};

tizen.TZDate.prototype.setMonth = function() {
  console.log('Entered TZDate.setMonth');
  _updateTZDate(this, arguments, 'month', 'setUTCMonth');
};

tizen.TZDate.prototype.getSeconds = function() {
  console.log('Entered TZDate.getSeconds');
  return _createShiftedDate(this).getUTCSeconds();
};

tizen.TZDate.prototype.setSeconds = function() {
  console.log('Entered TZDate.setSeconds');
  _updateTZDate(this, arguments, 'seconds', 'setUTCSeconds');
};

tizen.TZDate.prototype.getUTCDate = function() {
  console.log('Entered TZDate.getUTCDate');
  return _createUTCDate(this).getUTCDate();
};

tizen.TZDate.prototype.setUTCDate = function() {
  console.log('Entered TZDate.setUTCDate');
  _updateTZDateUTC(this, arguments, 'date', 'setUTCDate');
};

tizen.TZDate.prototype.getUTCDay = function() {
  console.log('Entered TZDate.getUTCDay');
  return _createUTCDate(this).getUTCDay();
};

tizen.TZDate.prototype.getUTCFullYear = function() {
  console.log('Entered TZDate.getUTCFullYear');
  return _createUTCDate(this).getUTCFullYear();
};

tizen.TZDate.prototype.setUTCFullYear = function() {
  console.log('Entered TZDate.setUTCFullYear');
  _updateTZDateUTC(this, arguments, 'year', 'setUTCFullYear');
};

tizen.TZDate.prototype.getUTCHours = function() {
  console.log('Entered TZDate.getUTCHours');
  return _createUTCDate(this).getUTCHours();
};

tizen.TZDate.prototype.setUTCHours = function() {
  console.log('Entered TZDate.setUTCHours');
  _updateTZDateUTC(this, arguments, 'hours', 'setUTCHours');
};

tizen.TZDate.prototype.getUTCMilliseconds = function() {
  console.log('Entered TZDate.getUTCMilliseconds');
  return _createUTCDate(this).getUTCMilliseconds();
};

tizen.TZDate.prototype.setUTCMilliseconds = function() {
  console.log('Entered TZDate.setUTCMilliseconds');
  _updateTZDateUTC(this, arguments, 'ms', 'setUTCMilliseconds');
};

tizen.TZDate.prototype.getUTCMinutes = function() {
  console.log('Entered TZDate.getUTCMinutes');
  return _createUTCDate(this).getUTCMinutes();
};

tizen.TZDate.prototype.setUTCMinutes = function() {
  console.log('Entered TZDate.setUTCMinutes');
  _updateTZDateUTC(this, arguments, 'minutes', 'setUTCMinutes');
};

tizen.TZDate.prototype.getUTCMonth = function() {
  console.log('Entered TZDate.getUTCMonth');
  return _createUTCDate(this).getUTCMonth();
};

tizen.TZDate.prototype.setUTCMonth = function() {
  console.log('Entered TZDate.setUTCMonth');
  _updateTZDateUTC(this, arguments, 'month', 'setUTCMonth');
};

tizen.TZDate.prototype.getUTCSeconds = function() {
  console.log('Entered TZDate.getUTCSeconds');
  return _createUTCDate(this).getUTCSeconds();
};

tizen.TZDate.prototype.setUTCSeconds = function() {
  console.log('Entered TZDate.setUTCSeconds');
  _updateTZDateUTC(this, arguments, 'seconds', 'setUTCSeconds');
};

tizen.TZDate.prototype.getTimezone = function() {
  console.log('Entered TZDate.getTimezone');
  return this._timezoneName;
};

tizen.TZDate.prototype.toTimezone = function() {
  console.log('Entered TZDate.toTimezone');
  var args = validator_.validateMethod(arguments, [
                                                   {
                                                     name : 'timezone',
                                                     type : validator_.Types.STRING
                                                   }
                                                   ]);
  return new tizen.TZDate(new PrivateTZDate(this._utcTimestamp, args.timezone));
};

tizen.TZDate.prototype.toLocalTimezone = function() {
  console.log('Entered TZDate.toLocalTimezone');
  return new tizen.TZDate(new PrivateTZDate(this._utcTimestamp, _LOCAL_ID));
};

tizen.TZDate.prototype.toUTC = function() {
  console.log('Entered TZDate.toUTC');
  return new tizen.TZDate(new PrivateTZDate(this._utcTimestamp, _GMT_ID));
};

tizen.TZDate.prototype.difference = function() {
  console.log('Entered TZDate.difference');
  var args = validator_.validateMethod(arguments, [
                                                   {
                                                     name : 'other',
                                                     type : validator_.Types.PLATFORM_OBJECT,
                                                     values : tizen.TZDate
                                                   }
                                                   ]);
  var length = this._utcTimestamp - args.other._utcTimestamp;
  var type = _timeDurationUnit.MSECS;
  if (length % _timeDurationUnitValue.DAYS === 0) {
    length /= _timeDurationUnitValue.DAYS;
    type = _timeDurationUnit.DAYS;
  }
  return new tizen.TimeDuration(length, type);
};

tizen.TZDate.prototype.equalsTo = function() {
  console.log('Entered TZDate.equalsTo');
  var args = validator_.validateMethod(arguments, [
                                                   {
                                                     name : 'other',
                                                     type : validator_.Types.PLATFORM_OBJECT,
                                                     values : tizen.TZDate
                                                   }
                                                   ]);
  return this._utcTimestamp === args.other._utcTimestamp;
};

tizen.TZDate.prototype.earlierThan = function() {
  console.log('Entered TZDate.earlierThan');
  var args = validator_.validateMethod(arguments, [
                                                   {
                                                     name : 'other',
                                                     type : validator_.Types.PLATFORM_OBJECT,
                                                     values : tizen.TZDate
                                                   }
                                                   ]);
  return this._utcTimestamp < args.other._utcTimestamp;
};

tizen.TZDate.prototype.laterThan = function() {
  console.log('Entered TZDate.laterThan');
  var args = validator_.validateMethod(arguments, [
                                                   {
                                                     name : 'other',
                                                     type : validator_.Types.PLATFORM_OBJECT,
                                                     values : tizen.TZDate
                                                   }
                                                   ]);
  return this._utcTimestamp > args.other._utcTimestamp;
};

tizen.TZDate.prototype.addDuration = function() {
  console.log('Entered TZDate.addDuration');
  var args = validator_.validateMethod(arguments, [
                                                   {
                                                     name : 'duration',
                                                     type : validator_.Types.PLATFORM_OBJECT,
                                                     values : tizen.TimeDuration
                                                   }
                                                   ]);
  return new tizen.TZDate(new PrivateTZDate(this._utcTimestamp +
      _getLengthInMsecsUnit(args.duration.length, args.duration.unit),
      this._timezoneName));
};

tizen.TZDate.prototype.toLocaleDateString = function() {
  console.log('Entered TZDate.toLocaleDateString');
  var result = native_.callSync('TZDate_toLocaleDateString',
      {timezone: String(this._timezoneName),
    timestamp: String(this._utcTimestamp)});
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  return native_.getResultObject(result).string;
};

tizen.TZDate.prototype.toLocaleTimeString = function() {
  console.log('Entered TZDate.toLocaleTimeString');
  var result = native_.callSync('TZDate_toLocaleTimeString',
      {timezone: String(this._timezoneName),
    timestamp: String(this._utcTimestamp)});
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  return native_.getResultObject(result).string;
};

tizen.TZDate.prototype.toLocaleString = function() {
  console.log('Entered TZDate.toLocaleString');
  var result = native_.callSync('TZDate_toLocaleString',
      {timezone: String(this._timezoneName),
    timestamp: String(this._utcTimestamp)});
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  return native_.getResultObject(result).string;
};

tizen.TZDate.prototype.toDateString = function() {
  console.log('Entered TZDate.toDateString');
  var result = native_.callSync('TZDate_toDateString',
      {timezone: String(this._timezoneName),
    timestamp: String(this._utcTimestamp)});
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  return native_.getResultObject(result).string;
};

tizen.TZDate.prototype.toTimeString = function() {
  console.log('Entered TZDate.toTimeString');
  var result = native_.callSync('TZDate_toTimeString',
      {timezone: String(this._timezoneName),
    timestamp: String(this._utcTimestamp)});
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  return native_.getResultObject(result).string;
};

tizen.TZDate.prototype.toString = function() {
  console.log('Entered TZDate.toString');
  var result = native_.callSync('TZDate_toString',
      {timezone: String(this._timezoneName),
    timestamp: String(this._utcTimestamp)});
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  return  native_.getResultObject(result).string;
};

tizen.TZDate.prototype.getTimezoneAbbreviation = function() {
  console.log('Entered TZDate.getTimezoneAbbreviation');
  var result = native_.callSync('TZDate_getTimezoneAbbreviation',
      {timezone: String(this._timezoneName),
    timestamp: String(this._utcTimestamp)});
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  return native_.getResultObject(result).abbreviation;
};

tizen.TZDate.prototype.secondsFromUTC = function() {
  console.log('Entered TZDate.secondsFromUTC');
  return -this._timezoneOffset/1000;
};

tizen.TZDate.prototype.isDST = function() {
  console.log('Entered TZDate.isDST');
  var result = native_.callSync('TZDate_isDST',
      {timezone: String(this._timezoneName),
    timestamp: String(this._utcTimestamp)});
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  return native_.getResultObject(result).isDST;
};

tizen.TZDate.prototype.getPreviousDSTTransition = function() {
  console.log('Entered TZDate.getPreviousDSTTransition');
  var result = native_.callSync('TZDate_getPreviousDSTTransition',
      {timezone: String(this._timezoneName),
    timestamp: String(this._utcTimestamp)});
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  return new tizen.TZDate(new PrivateTZDate(native_.getResultObject(result).prevDSTDate,
      this._timezoneName));
};

tizen.TZDate.prototype.getNextDSTTransition = function() {
  console.log('Entered TZDate.getNextDSTTransition');
  var result = native_.callSync('TZDate_getNextDSTTransition',
      {timezone: String(this._timezoneName),
    timestamp: String(this._utcTimestamp)});
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  return new tizen.TZDate(new PrivateTZDate(native_.getResultObject(result).nextDSTDate,
      this._timezoneName));
};

//TimeUtil helpers ///////////////////////////////////////////////////
var _timeDurationUnit = {
    MSECS: 'MSECS',
    SECS : 'SECS',
    MINS : 'MINS',
    HOURS: 'HOURS',
    DAYS : 'DAYS'
};

var _timeDurationUnitValue = {
    MSECS: Number(1),
    SECS : Number(1000),
    MINS : Number(60*1000),
    HOURS: Number(60*60*1000),
    DAYS : Number(24*60*60*1000)
};

function _getLengthInMsecsUnit(length, unit) {
  if (unit === _timeDurationUnit.MSECS) {
    return length;
  } else if (unit === _timeDurationUnit.SECS) {
    return length * _timeDurationUnitValue.SECS;
  } else if (unit === _timeDurationUnit.MINS) {
    return length * _timeDurationUnitValue.MINS;
  } else if (unit === _timeDurationUnit.HOURS) {
    return length * _timeDurationUnitValue.HOURS;
  } else if (unit === _timeDurationUnit.DAYS) {
    return length * _timeDurationUnitValue.DAYS;
  } else {
    native_.throwTypeMismatch();
  }
}

function _convertMsecsToBiggestPossibleUnit(len) {
  var length;
  var unit;
  if (len % _timeDurationUnitValue.DAYS === 0) {
    length = len / _timeDurationUnitValue.DAYS;
    unit = _timeDurationUnit.DAYS;
  } else if (len % _timeDurationUnitValue.HOURS === 0) {
    length = len / _timeDurationUnitValue.HOURS;
    unit = _timeDurationUnit.HOURS;
  } else if (len % _timeDurationUnitValue.MINS === 0) {
    length = len / _timeDurationUnitValue.MINS;
    unit = _timeDurationUnit.MINS;
  } else if (len % _timeDurationUnitValue.SECS === 0) {
    length = len / _timeDurationUnitValue.SECS;
    unit = _timeDurationUnit.SECS;
  } else {
    length = len;
    unit = _timeDurationUnit.MSECS;
  }
  return new tizen.TimeDuration(length, unit);
}

//class tizen.TimeDuration ////////////////////////////////////////////////////
tizen.TimeDuration = function(length, unit) {
  console.log('Entered TimeDuration constructor');
  validator_.validateConstructorCall(this, tizen.TimeDuration);
  var l, u;
  if (arguments.length >= 2) {
    l = converter_.toLongLong(length);
    unit = converter_.toString(unit);
    if (T.hasProperty(_timeDurationUnit, unit)) {
      u = unit;
    } else {
      u = _timeDurationUnit.MSECS;
    }
  } else if (arguments.length === 1) {
    l = converter_.toLongLong(length);
    u = _timeDurationUnit.MSECS;
  } else {
    l = undefined;
    u = undefined;
  }
  function lengthSetter(val) {
    if (!T.isNullOrUndefined(val)) {
      l = val;
    }
  }
  function unitSetter(val) {
    if (!T.isNullOrUndefined(val)) {
      u = val;
    }
  }
  Object.defineProperties(this, {
    length : {
      enumerable : true,
      set : lengthSetter,
      get : function() {
        return l;
      }
    },
    unit : {
      enumerable : true,
      set : unitSetter,
      get : function() {
        return u;
      }
    }
  });
}

tizen.TimeDuration.prototype.difference = function() {
  console.log('Entered TimeDuration.difference');

  var args = validator_.validateMethod(arguments, [
                                                   {
                                                     name : 'other',
                                                     type : validator_.Types.PLATFORM_OBJECT,
                                                     values : tizen.TimeDuration
                                                   }
                                                   ]);

  if (this.unit === args.other.unit) {
    return new tizen.TimeDuration(this.length - args.other.length, this.unit);
  } else {
    var l1 = _getLengthInMsecsUnit(this.length, this.unit);
    var l2 = _getLengthInMsecsUnit(args.other.length, args.other.unit);
    return _convertMsecsToBiggestPossibleUnit(l1 - l2);
  }
};

tizen.TimeDuration.prototype.equalsTo = function() {
  console.log('Entered TimeDuration.equalsTo');

  var args = validator_.validateMethod(arguments, [
                                                   {
                                                     name : 'other',
                                                     type : validator_.Types.PLATFORM_OBJECT,
                                                     values : tizen.TimeDuration
                                                   }
                                                   ]);

  if (this.unit === args.other.unit) {
    return (this.length === args.other.length) ? true : false;
  } else {
    var l1 = _getLengthInMsecsUnit(this.length, this.unit);
    var l2 = _getLengthInMsecsUnit(args.other.length, args.other.unit);
    return (l1 === l2) ? true : false;
  }
};

tizen.TimeDuration.prototype.lessThan = function() {
  console.log('Entered TimeDuration.lessThan');

  var args = validator_.validateMethod(arguments, [
                                                   {
                                                     name : 'other',
                                                     type : validator_.Types.PLATFORM_OBJECT,
                                                     values : tizen.TimeDuration
                                                   }
                                                   ]);

  if (this.unit === args.other.unit) {
    return (this.length < args.other.length) ? true : false;
  } else {
    var l1 = _getLengthInMsecsUnit(this.length, this.unit);
    var l2 = _getLengthInMsecsUnit(args.other.length, args.other.unit);
    return (l1 < l2) ? true : false;
  }
};

tizen.TimeDuration.prototype.greaterThan = function() {
  console.log('Entered TimeDuration.greaterThan');

  var args = validator_.validateMethod(arguments, [
                                                   {
                                                     name : 'other',
                                                     type : validator_.Types.PLATFORM_OBJECT,
                                                     values : tizen.TimeDuration
                                                   }
                                                   ]);

  if (this.unit === args.other.unit) {
    return (this.length > args.other.length) ? true : false;
  } else {
    var l1 = _getLengthInMsecsUnit(this.length, this.unit);
    var l2 = _getLengthInMsecsUnit(args.other.length, args.other.unit);
    return (l1 > l2) ? true : false;
  }
};


//class TimeUtil ////////////////////////////////////////////////////
exports.getCurrentDateTime = function() {
  console.log('Entered TimeUtil.getCurrentDateTime');
  return new tizen.TZDate();
};

exports.getLocalTimezone = function() {
  console.log('Entered TimeUtil.getLocalTimezone');
  var result = native_.callSync('TZDate_getLocalTimezone', {});
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  return native_.getResultObject(result).timezoneId;
};

var _availableTimezones = [];       //an array for holding available timezones

exports.getAvailableTimezones = function() {
  console.log('Entered TimeUtil.getAvailableTimezones');
  if (_availableTimezones.length === 0) {
    var result = native_.callSync('TimeUtil_getAvailableTimezones', {});
    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }
    _availableTimezones = native_.getResultObject(result).availableTimezones;
  }

  return _availableTimezones.slice(0);
};

exports.getDateFormat = function() {
  console.log('Entered TimeUtil.getDateFormat');

  var args = validator_.validateMethod(arguments, [
                                                   {
                                                     name : 'shortformat',
                                                     type : validator_.Types.BOOLEAN,
                                                     optional : true,
                                                     nullable : true
                                                   }
                                                   ]);

  if (!args.has.shortformat || T.isNull(args.shortformat)) {
    args.shortformat = false;
  }

  var result = native_.callSync('TimeUtil_getDateFormat', {shortformat: args.shortformat});
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  return native_.getResultObject(result).format;
};

exports.getTimeFormat = function() {
  console.log('Entered TimeUtil.getTimeFormat');
  var result = native_.callSync('TimeUtil_getTimeFormat', {});
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  return native_.getResultObject(result).format;
};

exports.isLeapYear = function() {
  console.log('Entered TimeUtil.isLeapYear');

  var args = validator_.validateMethod(arguments, [
                                                   {
                                                     name : 'year',
                                                     type : validator_.Types.LONG
                                                   }
                                                   ]);

  // new Date(year, 1, 29).getMonth() === 1 <-- does not work for years 0-99
  return ((args.year % 4 === 0) && (args.year % 100 !== 0)) || (args.year % 400 === 0);
};


var _timeUtilDateTimeChangeListener;
function _timeChangedListenerCallback(eventObj) {
  console.log("_timeChangedListenerCallback");
  native_.callIfPossible(_timeUtilDateTimeChangeListener);
}

exports.setDateTimeChangeListener = function() {
  console.log('Entered TimeUtil.setDateTimeChangeListener');
  var args = validator_.validateMethod(arguments, [
                                                   {
                                                     name : 'changeCallback',
                                                     type : validator_.Types.FUNCTION
                                                   }
                                                   ]);
  var result = native_.callSync('TimeUtil_setDateTimeChangeListener', {});
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  _timeUtilDateTimeChangeListener = args.changeCallback;
  native_.addListener("DateTimeChangeListener", _timeChangedListenerCallback);
};

exports.unsetDateTimeChangeListener = function() {
  console.log('Entered TimeUtil.unsetDateTimeChangeListener');
  var result = native_.callSync('TimeUtil_unsetDateTimeChangeListener', {});
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  native_.removeListener('DateTimeChangeListener');
  _timeUtilDateTimeChangeListener = undefined;
};

var _timeUtilTimezoneChangeListener;
function _timezoneListenerCallback(eventObj) {
  console.log("_timezoneListenerCallback");
  native_.callIfPossible(_timeUtilTimezoneChangeListener);
}

exports.setTimezoneChangeListener = function() {
  console.log('Entered TimeUtil.setTimezoneChangeListener');
  var args = validator_.validateMethod(arguments, [
                                                   {
                                                     name : 'changeCallback',
                                                     type : validator_.Types.FUNCTION
                                                   }
                                                   ]);
  var result = native_.callSync('TimeUtil_setTimezoneChangeListener', {});
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  _timeUtilTimezoneChangeListener = args.changeCallback;
  native_.addListener('TimezoneChangeListener',
      _timezoneListenerCallback);

};

exports.unsetTimezoneChangeListener = function() {
  console.log('Entered TimeUtil.unsetTimezoneChangeListener');
  native_.removeListener('TimezoneChangeListener');
  var result = native_.callSync('TimeUtil_unsetTimezoneChangeListener', {});
  _timeUtilTimezoneChangeListener = undefined;
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};