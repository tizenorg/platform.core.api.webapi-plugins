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

    // This flag is workaround. It is caused by specification
    // which tell to edit readonly fields.
    var _canEdit = 0;

    var EditGuard = function() {
    };

    EditGuard.prototype.run = function(callback) {
        try {
            this.enable();
            var result = callback();
            this.disable();
            return result;
        } catch (ex) {
            this.disable();
            throw ex;
        }
    };

    EditGuard.prototype.enable = function() {
        _canEdit++;
        console.log('RAISING LEVEL TO ' + _canEdit);
    };

    EditGuard.prototype.disable = function() {
        _canEdit--;
        console.log('LOWERING LEVEL TO ' + _canEdit);
    };

    EditGuard.prototype.isEditEnabled = function() {
        return _canEdit > 0;
    };

    var _editGuard = new EditGuard();

    // Adjusts properties to have the correct format expected by the native side.
    // Currently only translates JS Date
    var _toJsonObject = function(obj) {
        var ret;
        if (Type.isDate(obj)) {
            var year = ('0000' + obj.getFullYear()).slice(-4);
            var month = ('00' + (obj.getMonth() + 1)).slice(-2);
            var day = ('00' + obj.getDate()).slice(-2);
            return Number(year + month + day);
        }
        if (Type.isArray(obj)) {
            ret = [];
            for (var i = 0; i < obj.length; ++i) {
                ret[i] = _toJsonObject(obj[i]);
            }
            return ret;
        }
        if (obj instanceof Object) {
            ret = {};
            for (var prop in obj) {
                if (obj.hasOwnProperty(prop)) {
                    ret[prop] = _toJsonObject(obj[prop]);
                }
            }
            return ret;
        }
        return obj;
    };

    var _currentWatchId = 1;

    var _getNextWatchId = function () {
        return _currentWatchId++;
    };

    var _fromJsonDate = function(date) {
        date = date + '';
        var year = date.substr(0, 4);
        var month = date.substr(4, 2);
        var day = date.substr(6, 2);
        return new Date(year, month - 1, day);
    };

    var _promote = function(val, type) {
        return _editGuard.run(function() {
            if (Type.isArray(val)) {
                var ret = [];
                for (var i = 0; i < val.length; ++i) {
                    ret.push(new type(val[i]));
                }
                return ret;
            }
            return new type(val);
        });
    }

    // An enumerator that indicates the types for the relationships.
    var ContactRelationshipType = {
        ASSISTANT: 'ASSISTANT',
        BROTHER: 'BROTHER',
        CHILD: 'CHILD',
        DOMESTIC_PARTNER: 'DOMESTIC_PARTNER',
        FATHER: 'FATHER',
        FRIEND: 'FRIEND',
        MANAGER: 'MANAGER',
        MOTHER: 'MOTHER',
        PARENT: 'PARENT',
        PARTNER: 'PARTNER',
        REFERRED_BY: 'REFERRED_BY',
        RELATIVE: 'RELATIVE',
        SISTER: 'SISTER',
        SPOUSE: 'SPOUSE',
        OTHER: 'OTHER',
        CUSTOM: 'CUSTOM'
    };

    // An enumerator that indicates the types for instant messenger.
    var ContactInstantMessengerType = {
        GOOGLE: 'GOOGLE',
        WLM: 'WLM',
        YAHOO: 'YAHOO',
        FACEBOOK: 'FACEBOOK',
        ICQ: 'ICQ',
        AIM: 'AIM',
        QQ: 'QQ',
        JABBER: 'JABBER',
        SKYPE: 'SKYPE',
        IRC: 'IRC',
        OTHER: 'OTHER',
        CUSTOM: 'CUSTOM'
    };

    // class ContactRef ////////////////////////////////////////////////////////

    var ContactRef = function (data) {
        AV.validateConstructorCall(this, ContactRef);
        var _contactId = '';
        var _addressBookId = '';
        Object.defineProperties(this, {
            addressBookId: {
                get: function () {
                    return _addressBookId;
                },
                set: function (v) {
                    _addressBookId = Converter.toString(v, false);
                },
                enumerable: true
            },
            contactId: {
                get: function () {
                    return _contactId;
                },
                set: function (v) {
                    _contactId = Converter.toString(v, false);
                },
                enumerable: true
            }
        });

        if (Type.isObject(data)) {
            this.addressBookId = data.addressBookId;
            this.contactId = data.contactId;
        } else {
            try {
                var args = AV.validateMethod(arguments, [
                    {
                        name: 'addressBookId',
                        type: AV.Types.STRING,
                        optional: false,
                        nullable: false
                    },
                    {
                        name: 'contactId',
                        type: AV.Types.STRING,
                        optional: false,
                        nullable: false
                    }
                ]);
                _addressBookId = args.addressBookId;
                _contactId = args.contactId;
            } catch(x) {
                // Constructors shouldn't throw
            }
        }
    };

    // class ContactGroup //////////////////////////////////////////////////

    var ContactGroup = function (name, ringtone, photo) {
        AV.validateConstructorCall(this, ContactGroup);

        var _id = null;
        var _address = null;
        var _readOnly = false;
        var _name = '';
        var _ringtoneURI = null;
        var _photoURI = null;

        if (name && Type.isString(name)) {
            _name = name.length ? name : '';
        }

        if (ringtone && Type.isString(ringtone)) {
            _ringtoneURI = ringtone.length ? ringtone : null;
        }

        if (photo && Type.isString(photo)) {
            _photoURI = photo.length ? photo : null;
        }

        Object.defineProperties(this, {
            id: {
                get: function () {
                    return _id;
                },
                set: function (v) {
                    if (_editGuard.isEditEnabled()) {
                        _id = Converter.toString(v, true);
                    }
                },
                enumerable: true
            },
            addressBookId: {
                get: function () {
                    return _address;
                },
                set: function (v) {
                    if (_editGuard.isEditEnabled()) {
                        _address = Converter.toString(v, true);
                    }
                },
                enumerable: true
            },
            name: {
                get: function() {
                    return _name;
                },
                set: function(v) {
                    _name = Converter.toString(v, false);
                },
                enumerable: true
            },
            ringtoneURI: {
                get: function() {
                    return _ringtoneURI;
                },
                set: function(v) {
                    _ringtoneURI = Converter.toString(v, true);
                },
                enumerable: true
            },
            photoURI: {
                get: function() {
                    return _photoURI;
                },
                set: function(v) {
                    _photoURI = Converter.toString(v, true);
                },
                enumerable: true
            },
            readOnly: {
                get: function () {
                    return _readOnly;
                },
                set: function (v) {
                    if (_editGuard.isEditEnabled()) {
                        _readOnly = Converter.toBoolean(v, false);
                    }
                },
                enumerable: true
            },
        });

        if (_editGuard.isEditEnabled()) {
            var data = arguments[0];
            if (Type.isObject(data)) {
                for (var prop in data) {
                    if (this.hasOwnProperty(prop)) {
                        this[prop] = data[prop];
                    }
                }
            }
        }
    };

    // class ContactEmailAddress ///////////////////////////////////////////

    var ContactEmailAddress = function (address, types, isDefault) {
        AV.validateConstructorCall(this, ContactEmailAddress);

        var _email = '';
        var _label = '';
        var _isDefault = false;
        var _types = ['WORK'];

        if (Type.isString(address) && address.indexOf('@') > 0 &&
            address.indexOf('@') !== (address.length - 1)) {
            _email = address;
        }

        if (Type.isBoolean(isDefault)) {
            _isDefault = isDefault;
        }

        if (Type.isArray(types)) {
            _types = [];
            for (var i = 0; i < types.length; ++i) {
                if (Type.isString(types[i])) {
                    _types.push(types[i]);
                }
            }
        } else if (Type.isString(types)) {
            _types = [];
            _types.push(types);
        }

        Object.defineProperties(this, {
            email: {
                get: function() {
                    return _email;
                },
                set: function(v) {
                    if(Type.isString(v) && v.indexOf('@') > 0 &&
                            v.indexOf('@') !== (v.length - 1)) {
                            _email = v;
                    }
                },
                enumerable: true
            },
            isDefault: {
                get: function() {
                    return _isDefault;
                },
                set: function(v) {
                    _isDefault = Converter.toBoolean(v, false);
                },
                enumerable: true
            },
            types: {
                get: function() {
                    return _types;
                },
                set: function(v) {
                    if (Type.isArray(v)) {
                        _types = [];
                        for (var i = 0; i < v.length; ++i) {
                            if (Type.isString(v[i])) {
                                _types.push(v[i]);
                            }
                        }
                    } else if (Type.isString(v)) {
                        _types = [];
                        _types.push(v);
                    }
                },
                enumerable: true
            },
            label : {
                get: function() {
                    return _label;
                },
                set: function(v) {
                    _label = Converter.toString(v, false);
                },
                enumerable: true
            }
        });

        if (_editGuard.isEditEnabled()) {
            for (var prop in arguments[0]) {
                if (this.hasOwnProperty(prop)) {
                    this[prop] = arguments[0][prop];
                }
            }
        }
    };

    // class ContactPhoneNumber ////////////////////////////////////////////

    var ContactPhoneNumber = function (number, type, isDefault) {
        AV.validateConstructorCall(this, ContactPhoneNumber);

        var _isDefault = false;
        var _number = '';
        var _types = ['VOICE'];

        if (Type.isString(number)) {
            _number = number;
        }

        if (Type.isArray(type)) {
            _types = [];
            for (var i = 0; i < type.length; ++i) {
                _types.push(Converter.toString(type[i], false));
            }
        } else if (Type.isString(type)) {
            _types = [];
            _types.push(type, false);
        }

        if(Type.isBoolean(isDefault)) {
            _isDefault = isDefault;
        }

        Object.defineProperties(this, {
            number: {
                get: function()
                {
                    return _number;
                },
                set: function(v)
                {
                    // @todo normalization should be done by platform, but for some reason
                    // it doesn't works, so for now, we decided to do it here
                    _number = Converter.toString(v, false).split('-').join('');
                },
                enumerable: true
            },
            isDefault: {
                get: function () {
                    return _isDefault;
                },
                set: function (v) {
                    _isDefault = Converter.toBoolean(v, false);
                },
                enumerable: true
            },
            types: {
                get: function () {
                    return _types;
                },
                set: function (v) {
                    if (Type.isArray(v)) {
                        _types = [];
                        for (var i = 0; i < v.length; ++i) {
                            _types.push(Converter.toString(v[i], false));
                        }
                    } else if (Type.isString(v)) {
                        _types = [];
                        _types.push(v, false);
                    }
                },
                enumerable: true
            },
            label : {
                value: null,
                writable: true,
                enumerable: true
            }
        });

        if (_editGuard.isEditEnabled()) {
            for (var prop in arguments[0]) {
                if (this.hasOwnProperty(prop)) {
                    this[prop] = arguments[0][prop];
                }
            }
        }
    };

    // class ContactAddress ////////////////////////////////////////////////

    var ContactAddress = function (data) {
        AV.validateConstructorCall(this, ContactAddress);

        var _isDefault = false;
        var _types = ['HOME'];

        Object.defineProperties(this, {
            country: {
                value: null,
                writable: true,
                enumerable: true
            },
            region: {
                value: null,
                writable: true,
                enumerable: true
            },
            city: {
                value: null,
                writable: true,
                enumerable: true
            },
            streetAddress: {
                value: null,
                writable: true,
                enumerable: true
            },
            additionalInformation: {
                value: null,
                writable: true,
                enumerable: true
            },
            postalCode: {
                value: null,
                writable: true,
                enumerable: true
            },
            isDefault: {
                get: function () {
                    return _isDefault;
                },
                set: function (v) {
                    _isDefault = Converter.toBoolean(v, false);
                },
                enumerable: true
            },
            types: {
                get: function() {
                    return _types;
                },
                set: function(v) {
                    if (Type.isString(v)) {
                        _types = [];
                        _types.push(v);
                    } else if (Type.isArray(v)) {
                        _types = [];
                        for (var i = 0; i < v.length; ++i) {
                            if(Type.isString(v[i])) {
                                _types.push(v[i]);
                            }
                        }
                    }
                },
                enumerable: true
            },
            label : {
                value: null,
                writable: true,
                enumerable: true
            }
        });

        if (Type.isObject(data)) {
            for (var prop in data) {
                if (this.hasOwnProperty(prop)) {
                    this[prop] = data[prop];
                }
            }
        }
    };

    // class ContactAnniversary ////////////////////////////////////////////////

    var ContactAnniversary = function (anniversary_date, anniversary_label) {
        AV.validateConstructorCall(this, ContactAnniversary);

        var _anniversary_date = new Date();
        var _anniversary_label = null;

        Object.defineProperties(this, {
            date: {
                get: function () {
                    return _anniversary_date;
                },
                set: function (v) {
                    _anniversary_date = v instanceof Date ? v : new Date();
                },
                enumerable: true
            },
            label: {
                get: function () {
                    return _anniversary_label;
                },
                set: function (v) {
                    _anniversary_label = Converter.toString(v, true);
                },
                enumerable: true
            }
        });

        if (_editGuard.isEditEnabled()) {
            _anniversary_date = _fromJsonDate(arguments[0].date);
            _anniversary_label = arguments[0].label;
        } else {
            if (Type.isDate(anniversary_date)) {
                _anniversary_date = anniversary_date;
            }
            if (Type.isString(anniversary_label)) {
                _anniversary_label = anniversary_label;
            }
        }
    };

    // class ContactWebSite ////////////////////////////////////////////////////

    var ContactWebSite = function (contact_url, contact_type) {
        AV.validateConstructorCall(this, ContactWebSite);

        var _url = '';
        var _type = 'HOMEPAGE';

        if (Type.isString(contact_url)) {
            _url = contact_url;
        }
        if (Type.isString(contact_type)) {
            _type = contact_type;
        }

        Object.defineProperties(this, {
            url: {
                get: function() {
                    return _url;
                },
                set: function(v) {
                    _url = Converter.toString(v, false);
                },
                enumerable: true
            },
            type: {
                get: function() {
                    return _type;
                },
                set: function(v) {
                    _type = Converter.toString(v, false);
                },
                enumerable: true
            }
        });
    };

    // class ContactOrganization ///////////////////////////////////////////////

    var ContactOrganization = function (data) {
        AV.validateConstructorCall(this, ContactOrganization);
        Object.defineProperties(this, {
            name: {
                value: null,
                writable: true,
                enumerable: true
            },
            department: {
                value: null,
                writable: true,
                enumerable: true
            },
            title: {
                value: null,
                writable: true,
                enumerable: true
            },
            role: {
                value: null,
                writable: true,
                enumerable: true
            },
            logoURI: {
                value: null,
                writable: true,
                enumerable: true
            }
        });

        if (Type.isObject(data)) {
            for (var prop in data) {
                if (this.hasOwnProperty(prop)) {
                    this[prop] = data[prop];
                }
            }
        }
    };

    // class ContactName ///////////////////////////////////////////////////////

    var ContactName = function (data) {
        AV.validateConstructorCall(this, ContactName);

        var _displayName = null;

        Object.defineProperties(this, {
            prefix: {
                value: null,
                writable: true,
                enumerable: true
            },
            suffix: {
                value: null,
                writable: true,
                enumerable: true
            },
            firstName: {
                value: null,
                writable: true,
                enumerable: true
            },
            middleName: {
                value: null,
                writable: true,
                enumerable: true
            },
            lastName: {
                value: null,
                writable: true,
                enumerable: true
            },
            nicknames: {
                value: [],
                writable: true,
                enumerable: true
            },
            phoneticFirstName: {
                value: null,
                writable: true,
                enumerable: true
            },
            phoneticMiddleName: {
                value: null,
                writable: true,
                enumerable: true
            },
            phoneticLastName: {
                value: null,
                writable: true,
                enumerable: true
            },
            displayName: {
                get: function () {
                    return _displayName;
                },
                set: function (v) {
                    if (_editGuard.isEditEnabled()) {
                        _displayName = Converter.toString(v, true);
                    }
                },
                enumerable: true
            }
        });

        if (Type.isObject(data)) {
            for (var prop in data) {
                if (this.hasOwnProperty(prop)) {
                    this[prop] = data[prop];
                }
            }
        }
    };

    var ContactRelationship = function(relativeName, type) {
        AV.validateConstructorCall(this, ContactRelationship);

        Object.defineProperties(this, {
            relativeName: {
                value: Type.isString(relativeName) ? relativeName : null,
                writable: true,
                enumerable: true
            },
            type: {
                value: (Type.isNullOrUndefined(type) ? ContactRelationshipType.OTHER : type),
                writable: true,
                enumerable: true
            },
            label: {
                value: null,
                writable: true,
                enumerable: true
            }
        });

        if (_editGuard.isEditEnabled()) {
            for (var prop in arguments[0]) {
                if (this.hasOwnProperty(prop)) {
                    this[prop] = arguments[0][prop];
                }
            }
        }
    };

    var ContactInstantMessenger = function(imAddress, type) {
        AV.validateConstructorCall(this, ContactInstantMessenger);

        Object.defineProperties(this, {
            imAddress : {
                value: Type.isString(imAddress) ? imAddress : null,
                writable: true,
                enumerable: true
            },
            type : {
                value: (Type.isNullOrUndefined(type) ? ContactInstantMessengerType.OTHER : type),
                writable: true,
                enumerable: true
            },
            label : {
                value: null,
                writable: true,
                enumerable: true
            }
        });

        if (_editGuard.isEditEnabled()) {
            for (var prop in arguments[0]) {
                if (this.hasOwnProperty(prop)) {
                    this[prop] = arguments[0][prop];
                }
            }
        }
    };

    // exports /////////////////////////////////////////////////////////////////
    module.exports = {
        ref: ContactRef,
        name: ContactName,
        organization: ContactOrganization,
        webSite: ContactWebSite,
        anniversary: ContactAnniversary,
        address: ContactAddress,
        phoneNumber: ContactPhoneNumber,
        emailAddress: ContactEmailAddress,
        group: ContactGroup,
        relationship: ContactRelationship,
        instantMessenger: ContactInstantMessenger,
        relationshipType: ContactRelationshipType,
        instantMessengerType: ContactInstantMessengerType,
        editGuard: _editGuard,
        toJsonObject: _toJsonObject,
        getNextWatchId : _getNextWatchId,
        fromJsonDate: _fromJsonDate,
        promote: _promote
    };

})();
