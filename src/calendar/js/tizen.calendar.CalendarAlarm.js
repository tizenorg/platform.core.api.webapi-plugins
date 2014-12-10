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

(function () {
    'use strict';

    var _common = require('./tizen.Common');
    var Converter = _common.Converter;
    var AV = _common.ArgumentValidator;
    _common = undefined;

    var AlarmMethod = {
        SOUND: 'SOUND',
        DISPLAY: 'DISPLAY'
    };

    var CalendarAlarm = function (time, method, description) {
        AV.validateConstructorCall(this, CalendarAlarm);

        var _absoluteDate = time instanceof tizen.TZDate && !this.before ? time : null;
        var _before = time instanceof tizen.TimeDuration && !this.absoluteDate ? time : null;
        var _method = Converter.toEnum(method, Object.keys(AlarmMethod), false);
        var _description = (description) ? Converter.toString(description, true) : '';

        Object.defineProperties(this, {
            absoluteDate: {
                get: function () {
                    return _absoluteDate;
                },
                set: function (v) {
                    _absoluteDate = v instanceof tizen.TZDate && !this.before ? v : null;
                },
                enumerable: true
            },
            before: {
                get: function () {
                    return _before;
                },
                set: function (v) {
                    _before = v instanceof tizen.TimeDuration && !this.absoluteDate ? v : null;
                },
                enumerable: true
            },
            method: {
                get: function () {
                    return _method;
                },
                set: function (v) {
                    if (v === null) {
                        return;
                    }
                    _method = Converter.toEnum(v, Object.keys(AlarmMethod), false);
                },
                enumerable: true
            },
            description: {
                get: function () {
                    return _description;
                },
                set: function (v) {
                    _description = Converter.toString(v, true);
                },
                enumerable: true
            }
        });
    };

    module.exports = CalendarAlarm;
})();
