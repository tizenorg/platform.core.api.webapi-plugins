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
    var T = _common.Type;
    var Converter = _common.Converter;
    var AV = _common.ArgumentValidator;
    var C = _common.Common;
    _common = undefined;

    var AttendeeType = {
        INDIVIDUAL: 'INDIVIDUAL',
        GROUP: 'GROUP',
        RESOURCE: 'RESOURCE',
        ROOM: 'ROOM',
        UNKNOWN: 'UNKNOWN'
    };

    var AttendeeStatus = {
        PENDING: 'PENDING',
        ACCEPTED: 'ACCEPTED',
        DECLINED: 'DECLINED',
        TENTATIVE: 'TENTATIVE',
        DELEGATED: 'DELEGATED',
        COMPLETED: 'COMPLETED',
        IN_PROCESS: 'IN_PROCESS'
    };

    var AttendeeRole = {
        REQ_PARTICIPANT: 'REQ_PARTICIPANT',
        OPT_PARTICIPANT: 'OPT_PARTICIPANT',
        NON_PARTICIPANT: 'NON_PARTICIPANT',
        CHAIR: 'CHAIR'
    };

    var CalendarAttendeeInit = function (data) {
        var _name = null;
        var _role = 'REQ_PARTICIPANT';
        var _status = 'PENDING';
        var _RSVP = false;
        var _type = 'INDIVIDUAL';
        var _group = null;
        var _delegatorURI = null;
        var _delegateURI = null;
        var _contactRef = null;

        Object.defineProperties(this, {
            name: {
                get: function () {
                    return _name;
                },
                set: function (v) {
                    _name = Converter.toString(v, true);
                },
                enumerable: true
            },
            role: {
                get: function () {
                    return _role;
                },
                set: function (v) {
                    if (v === null) {
                        return;
                    }
                    _role = Converter.toEnum(v, Object.keys(AttendeeRole), false);
                },
                enumerable: true
            },
            status: {
                get: function () {
                    return _status;
                },
                set: function (v) {
                    if (v === null) {
                        return;
                    }
                    _status = Converter.toEnum(v, Object.keys(AttendeeStatus), false);
                },
                enumerable: true
            },
            RSVP: {
                get: function () {
                    return _RSVP;
                },
                set: function (v) {
                    _RSVP = Converter.toBoolean(v);
                },
                enumerable: true
            },
            type: {
                get: function () {
                    return _type;
                },
                set: function (v) {
                    if (v === null) {
                        return;
                    }
                    _type = Converter.toEnum(v, Object.keys(AttendeeType), false);
                },
                enumerable: true
            },
            group: {
                get: function () {
                    return _group;
                },
                set: function (v) {
                    _group = Converter.toString(v, true);
                },
                enumerable: true
            },
            delegatorURI: {
                get: function () {
                    return _delegatorURI;
                },
                set: function (v) {
                    _delegatorURI = Converter.toString(v, true);
                },
                enumerable: true
            },
            delegateURI: {
                get: function () {
                    return _delegateURI;
                },
                set: function (v) {
                    _delegateURI = Converter.toString(v, true);
                },
                enumerable: true
            },
            contactRef: {
                get: function () {
                    return _contactRef;
                },
                set: function (v) {
                    _contactRef = v instanceof tizen.ContactRef ? v : _contactRef;
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

    var CalendarAttendee = function (uri, attendeeInitDict) {
        AV.validateConstructorCall(this, CalendarAttendee);

        CalendarAttendeeInit.call(this, attendeeInitDict);

        var _uri = null;

        Object.defineProperties(this, {
            uri: {
                get: function () {
                    return _uri;
                },
                set: function (v) {
                    if (v === null) {
                        return;
                    }
                    _uri = Converter.toString(v, true);
                },
                enumerable: true
            }
        });

        this.uri = uri;
    };

    CalendarAttendee.prototype = new CalendarAttendeeInit();
    CalendarAttendee.prototype.constructor = CalendarAttendee;

    module.exports = CalendarAttendee;
})();
