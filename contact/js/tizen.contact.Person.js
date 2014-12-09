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

    // helper functions ////////////////////////////////////////////////////////

    var _common = require('./tizen.Common');
    var Type = _common.Type;
    var Converter = _common.Converter;
    var AV = _common.ArgumentValidator;
    var Common = _common.Common;
    var _callSync = Common.getCallSync('contact');
    var _call = Common.getCall('contact');
    _common = undefined;

    // import flag /////////////////////////////////////////////////////////////
    var _struct = require('./tizen.contact.ContactDataStructures');
    var _editGuard = _struct.editGuard;
    _struct = undefined;

    // class Person ////////////////////////////////////////////////////////////

    var Person = function (data) {
        AV.validateConstructorCall(this, Person);

        var _id = '';
        var _displayName = '';
        var _contactCount = 0;
        var _hasPhoneNumber = false;
        var _hasEmail = false;
        var _isFavorite = false;
        var _displayContactId = '';

        if(data.hasOwnProperty('id') && Type.isString(data.id)) {
            _id = data.id;
        }
        if(data.hasOwnProperty('displayName') && Type.isString(data.displayName)) {
            _displayName = data.displayName;
        }
        if(data.hasOwnProperty('contactCount') && Type.isNumber(data.contactCount)) {
            _contactCount = data.contactCount;
        }
        if(data.hasOwnProperty('hasPhoneNumber') && Type.isBoolean(data.hasPhoneNumber)) {
            _hasPhoneNumber = data.hasPhoneNumber;
        }
        if(data.hasOwnProperty('hasEmail') && Type.isBoolean(data.hasEmail)) {
            _hasEmail = data.hasEmail;
        }
        if(data.hasOwnProperty('displayContactId') && Type.isString(data.displayContactId)) {
            _displayContactId = data.displayContactId;
        }
        if(data.hasOwnProperty('isFavorite') && Type.isBoolean(data.isFavorite)) {
            _isFavorite = data.isFavorite;
        }

        Object.defineProperties(this, {
            id: {
                get: function() {
                    return _id;
                },
                set: function(v) {
                    if(_editGuard.isEditEnabled()) {
                        _id = Converter.toString(v, false);
                    }
                },
                enumerable: true
            },
            displayName: {
                get: function() {
                    return _displayName;
                },
                set: function(v) {
                    if(_editGuard.isEditEnabled()) {
                        _displayName = Converter.toString(v, false);
                    }
                },
                enumerable: true
            },
            contactCount: {
                get: function() {
                    return _contactCount;
                },
                set: function(v) {
                    if(_editGuard.isEditEnabled()) {
                        _contactCount = Converter.toLong(v, false);
                    }
                },
                enumerable: true
            },
            hasPhoneNumber: {
                get: function() {
                    return _hasPhoneNumber;
                },
                set: function(v) {
                    if(_editGuard.isEditEnabled()) {
                        _hasPhoneNumber = Converter.toBoolean(v, false);
                    }
                },
                enumerable: true
            },
            hasEmail: {
                get: function() {
                    return _hasEmail;
                },
                set: function(v) {
                    if(_editGuard.isEditEnabled()) {
                        _hasEmail = Converter.toBoolean(v, false);
                    }
                },
                enumerable: true
            },

            isFavorite: {
                get: function() {
                    return _isFavorite;
                },
                set: function(v) {
                    _isFavorite = Converter.toBoolean(v, false);
                },
                enumerable: true
            },
            photoURI: {
                value: data.hasOwnProperty('photoURI') ? data.photoURI : null,
                writable: true,
                enumerable: true
            },
            ringtoneURI: {
                value: data.hasOwnProperty('ringtoneURI') ? data.ringtoneURI : null,
                writable: true,
                enumerable: true
            },
            displayContactId: {
                get: function() {
                    return _displayContactId;
                },
                set: function(v) {
                    _displayContactId = Converter.toString(v, false);
                },
                enumerable: true
            },
        });
    };

    // Aggregates another person to this person.
    Person.prototype.link = function () {
        var args = AV.validateMethod(arguments, [
            {
                name: 'personId',
                type: AV.Types.STRING,
                optional: false,
                nullable: false
            }
        ]);

        var result = _callSync('Person_link', {
            // TODO move to only sending the person id (in all functions)
            person: this,
            id: args.personId
        });
        if (Common.isFailure(result)) {
            throw Common.getErrorObject(result);
        }
        var _this = this;
        _editGuard.run(function() {
            _this.contactCount = _this.contactCount + 1;
        });
    };

    // Separates a contact from this person.
    Person.prototype.unlink = function (contactId) {
        var args = AV.validateMethod(arguments, [
            {
                name: 'contactId',
                type: AV.Types.STRING,
                optional: false,
                nullable: false
            }
        ]);

        var result = _callSync('Person_unlink', {
            // TODO move to only sending the person id (in all functions)
            person: this,
            id: args.contactId
        });
        if (Common.isFailure(result)) {
            throw Common.getErrorObject(result);
        }
        var _this = this;
        return _editGuard.run(function() {
            _this.contactCount = _this.contactCount - 1;
            return new Person(Common.getResultObject(result));
        });
    };

    // exports /////////////////////////////////////////////////////////////////
    module.exports = Person;

})();
