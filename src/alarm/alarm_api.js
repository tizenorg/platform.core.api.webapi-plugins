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

var T = xwalk.utils.type;
var Converter = xwalk.utils.converter;
var AV = xwalk.utils.validator;
var Privilege = xwalk.utils.privilege;

var native = new xwalk.utils.NativeManager(extension);

var AlarmManager = function () {
    Object.defineProperties(this, {
        PERIOD_MINUTE:  { value: 60, writable: false, enumerable: true},
        PERIOD_HOUR:    { value: 3600, writable: false, enumerable: true},
        PERIOD_DAY:     { value: 86400, writable: false, enumerable: true},
        PERIOD_WEEK:    { value: 604800, writable: false, enumerable: true},
    });
};

function InternalData_(data) {
    if (!(this instanceof InternalData_)) {
        return new InternalData_(data);
    }

    for(var key in data) {
        if (data.hasOwnProperty(key)) {
            this[key] = data[key];
        }
    }
}

function UpdateInternalData_(internal, data) {
    var values = InternalData_(data);

    for(var key in data) {
        if (values.hasOwnProperty(key) && internal.hasOwnProperty(key)) {
            internal[key] = values;
        }
    }
}

//class AlarmManager ////////////////////////////////////////////////////
AlarmManager.prototype.add = function () {
    xwalk.utils.checkPrivilegeAccess(Privilege.ALARM);

    var args = AV.validateMethod(arguments, [
        {
            name : 'alarm',
            type : AV.Types.PLATFORM_OBJECT,
            values : [tizen.AlarmRelative, tizen.AlarmAbsolute]
        },
        {
            name : 'applicationId',
            type : AV.Types.STRING,
        },
        {
            name : 'appControl',
            type : AV.Types.PLATFORM_OBJECT,
            values : tizen.ApplicationControl,
            optional : true,
            nullable : true
        },
    ]);

    var type = null, seconds = 0;
    if (args.alarm instanceof tizen.AlarmRelative) {
        type = 'AlarmRelative';
    } else if (args.alarm instanceof tizen.AlarmAbsolute) {
        type = 'AlarmAbsolute';
        seconds = args.alarm.date.getTime();
    }

    var callArgs = {};
    callArgs.alarm = args.alarm;
    callArgs.applicationId = args.applicationId;
    if (args.has.appControl) {
        callArgs.appControl = args.appControl;
    }

    callArgs.type = type;
    callArgs.seconds = Converter.toString(seconds);

    var result = native.callSync('AlarmManager_add', callArgs);
    if (native.isFailure(result)) {
        throw native.getErrorObject(result);
    } else {
        UpdateInternalData_(args.alarm, native.getResultObject(result));
    }
};

AlarmManager.prototype.remove = function () {
    xwalk.utils.checkPrivilegeAccess(Privilege.ALARM);

    var args = AV.validateMethod(arguments, [
        {
            name : 'id',
            type : AV.Types.STRING,
        }
    ]);

    var result = native.callSync('AlarmManager_remove', {id: Number(args.id)});

    if (native.isFailure(result)) {
        throw native.getErrorObject(result);
    }
};

AlarmManager.prototype.removeAll = function () {
    xwalk.utils.checkPrivilegeAccess(Privilege.ALARM);

    var result = native.callSync('AlarmManager_removeAll', {});

    if (native.isFailure(result)) {
        throw native.getErrorObject(result);
    }
};

AlarmManager.prototype.get = function () {
    var args = AV.validateMethod(arguments, [
        {
            name : 'id',
            type : AV.Types.STRING,
        }
    ]);

    var result = native.callSync('AlarmManager_get', {id: Number(args.id)});

    if (native.isFailure(result)) {
        throw native.getErrorObject(result);
    } else {
        result = native.getResultObject(result);
        if ('AlarmRelative' === result.type) {
            return new tizen.AlarmRelative(result.delay, result.period, InternalData_(result));
        } else {
            var date = new Date(result.year, result.month, result.day,
                    result.hour, result.min, result.sec);

            return new tizen.AlarmAbsolute(date, result.second, InternalData_(result));
        }
    }
};

AlarmManager.prototype.getAll = function () {
    var result = native.callSync('AlarmManager_getAll', {});

    if (native.isFailure(result)) {
        throw native.getErrorObject(result);
    } else {
        var data = native.getResultObject(result);
        var md = [];
        data.forEach(function (i) {
            if ('AlarmRelative'=== i.type) {
                md.push(new tizen.AlarmRelative(i.delay, i.period, InternalData_(i)));
            } else {
                var date = new Date(i.year, i.month, i.day,
                        i.hour, i.min, i.sec);
                md.push(new tizen.AlarmAbsolute(date, i.second, InternalData_(i)));
            }
        });
        return md;
    }
};

//class Alarm //////////////////////////////////////////////////////////
function Alarm(id) {
    var m_id = null;

    if (!T.isNullOrUndefined(id) && id instanceof InternalData_) {
        m_id = id.id;
    }

    var _internal = {
        'id' : m_id
    };

    Object.defineProperties(this, {
        id: {
            get: function () {return _internal.id;},
            set: function (value) {
                if (value instanceof InternalData_) {
                    _internal.id = value.id;
                }
            },
            enumerable: true
        }
    });
}
//class AlarmRelative //////////////////////////////////////////////////

tizen.AlarmRelative = function(delay, period, internal) {
    AV.validateConstructorCall(this, tizen.AlarmRelative);

    var m_period = null;

    var m_delay = Converter.toLong(delay);

    if (arguments.length >= 2) {
        m_period = Converter.toLong(period, true);
    }

    Alarm.call(this, internal);

    Object.defineProperties(this, {
        delay:     { value: m_delay, writable: false, enumerable: true},
        period:    { value: m_period, writable: false, enumerable: true}
    });
}

tizen.AlarmRelative.prototype = new Alarm();

tizen.AlarmRelative.prototype.constructor = tizen.AlarmRelative;

tizen.AlarmRelative.prototype.getRemainingSeconds = function () {
    var result = native.callSync('AlarmRelative_getRemainingSeconds', {id: Number(this.id)});

    if (native.isFailure(result)) {
        throw native.getErrorObject(result);
    } else {
        return Converter.toLong(native.getResultObject(result).seconds, true);
    }
};

function makeDateConst(obj) {
    console.log('Enter MakeConst');
    obj.setDate = function() {};
    obj.setFullYear = function() {};
    obj.setHours = function() {};
    obj.setMilliseconds = function() {};
    obj.setMinutes = function() {};
    obj.setMonth = function() {};
    obj.setSeconds = function() {};
    obj.setTime = function() {};
    obj.setUTCDate = function() {};
    obj.setUTCFullYear = function() {};
    obj.setUTCHours = function() {};
    obj.setUTCMilliseconds = function() {};
    obj.setUTCMinutes = function() {};
    obj.setUTCMonth = function() {};
    obj.setUTCSeconds = function() {};
    obj.setYear = function() {};
    console.log('Leave MakeConst');
}

//class AlarmAbsolute //////////////////////////////////////////////////

tizen.AlarmAbsolute = function(date, second, internal) {
    AV.validateConstructorCall(this, tizen.AlarmAbsolute);

    var m_period = null, m_daysOfWeek = [], m_date;

    if (T.isDate(date)) {
        m_date = date;
        if (arguments.length >= 2) {
            if(T.isArray(second)){
                m_daysOfWeek = second;
            } else {
                m_period = Converter.toLong(second);
            }
        }

        Alarm.call(this, internal);
    } else {
        m_period = undefined;
    }
    makeDateConst(m_date);
    Object.defineProperties(this, {
        date:       { value: m_date, writable: false, enumerable: true},
        period:     { value: m_period, writable: false, enumerable: true},
        daysOfTheWeek: { value: m_daysOfWeek, writable: false, enumerable: true}
    });
}

tizen.AlarmAbsolute.prototype = new Alarm();

tizen.AlarmAbsolute.prototype.constructor = tizen.AlarmAbsolute;

tizen.AlarmAbsolute.prototype.getNextScheduledDate = function () {
    var result = native.callSync('AlarmAbsolute_getNextScheduledDate', {id: Number(this.id)});

    if (native.isFailure(result)) {
        throw native.getErrorObject(result);
    } else {
        var d = native.getResultObject(result);
        if (T.isNull(d.year)) {
            return null;
        } else {
            var date = new Date(d.year, d.month, d.day, d.hour, d.min, d.sec);
            return date;
        }
    }
};

//exports //////////////////////////////////////////////////////////////
exports = new AlarmManager();
