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

    var _dataStructures = require('./tizen.contact.ContactDataStructures');
    var _editGuard = _dataStructures.editGuard;
    var _toJsonObject = _dataStructures.toJsonObject;
    var _getNextWatchId = _dataStructures.getNextWatchId;
    var _promote = _dataStructures.promote;
    _dataStructures = undefined;

    function _checkError(result) {
        if (Common.isFailure(result)) {
            throw Common.getErrorObject(result);
        }
    }

    var _registered = false;
    var _listenerId = 'ContactPersonChangeListener';
    var _personCallbackMap = {};
    var _personChangeListener = function (result) {
        result = JSON.parse(result);
        for (var key in _personCallbackMap) {
            if (_personCallbackMap.hasOwnProperty(key)) {
                if (result.added.length) {
                    Common.callIfPossible(_personCallbackMap[key].onpersonsadded,
                                          _promote(result.added, Person));
                }
                if (result.updated.length) {
                    Common.callIfPossible(_personCallbackMap[key].onpersonsupdated,
                                          _promote(result.updated, Person));
                }
                if (result.removed.length) {
                    Common.callIfPossible(_personCallbackMap[key].onpersonsremoved,
                                          result.removed);
                }
            }
        }
    };

    // import AddressBook class ////////////////////////////////////////////////
    var AddressBook = require('./tizen.contact.AddressBook');

    // import Person class /////////////////////////////////////////////////////
    var Person = require('./tizen.contact.Person');

    // class ContactManager ////////////////////////////////////////////////////

    var ContactManager = function () {};

    // Gets the available address books
    ContactManager.prototype.getAddressBooks = function () {
        var args = AV.validateMethod(arguments, [
            {
                name: 'successCallback',
                type: AV.Types.FUNCTION,
                optional: false,
                nullable: false
            },
            {
                name: 'errorCallback',
                type: AV.Types.FUNCTION,
                optional: true,
                nullable: true
            }
        ]);

        var callback = function (result) {
            if (Common.isFailure(result)) {
                Common.callIfPossible(args.errorCallback, Common.getErrorObject(result));
            } else {
                var books = Common.getResultObject(result);
                var tmp = [];

                books.forEach(function (data) {
                    tmp.push(new AddressBook(data.id, data.name, data.readOnly));
                });

                Common.callIfPossible(args.successCallback, tmp);
            }
        };

        var result = _call('ContactManager_getAddressBooks', {}, callback);

        if (Common.isFailure(result)) {
            throw Common.getErrorObject(result);
        }
    };

    // Gets the aggregation of all address books.
    ContactManager.prototype.getUnifiedAddressBook = function () {
        // validation
        var result = _callSync('CheckReadPrivileges', {});
        if (Common.isFailure(result)) {
            throw new WebAPIException(WebAPIException.SECURITY_ERR,
                                      'You do not have privileges for this operation');
        }

        return new AddressBook(-1, 'Unified address book', false);
    };

    // Gets the default address book.
    ContactManager.prototype.getDefaultAddressBook = function () {
        //privileges are checked in getAddressBook function
        return this.getAddressBook(0);
    };

    // Gets the address book with the specified identifier.
    ContactManager.prototype.getAddressBook = function () {
        // validation
        var args = AV.validateMethod(arguments, [
            {
                name: 'addressBookId',
                type: AV.Types.STRING,
                optional: false,
                nullable: false
            }
        ]);

        var result = _callSync('ContactManager_getAddressBook', {
            addressBookID: args.addressBookId
        });
        _checkError(result);
        return new AddressBook(args.addressBookId, result.name,
            Boolean(result.readOnly === 'true' ? true : false));
    };

    // Gets the person with the specified identifier.
    ContactManager.prototype.get = function () {
        // validation
        var args = AV.validateMethod(arguments, [
            {
                name: 'personId',
                type: AV.Types.STRING,
                optional: false,
                nullable: false
            }
        ]);

        var result = _callSync('ContactManager_get', {
            personID: args.personId
        });
        _checkError(result);

        return _editGuard.run(function() {
            return new Person(Common.getResultObject(result));
        });
    };

    // Updates a person in the address book synchronously.
    ContactManager.prototype.update = function () {
        // validation
        var args = AV.validateMethod(arguments, [
            {
                name: 'person',
                type: AV.Types.PLATFORM_OBJECT,
                values: Person,
                optional: false,
                nullable: false
            }
        ]);
        var result = _callSync('ContactManager_update', { person: args.person });
        _checkError(result);

        result = Common.getResultObject(result);
        for (var prop in result) {
            if (args.person.hasOwnProperty(prop)) {
                args.person[prop] = result[prop];
            }
        }
    };

    // Updates several existing persons in the contact DB asynchronously.
    ContactManager.prototype.updateBatch = function () {
        var args = AV.validateMethod(arguments, [
            {
                name: 'persons',
                type: AV.Types.ARRAY,
                values: Person,
                optional: false,
                nullable: false
            },
            {
                name: 'successCallback',
                type: AV.Types.FUNCTION,
                optional: true,
                nullable: true
            },
            {
                name: 'errorCallback',
                type: AV.Types.FUNCTION,
                optional: true,
                nullable: true
            }
        ]);

        var callback = function (result) {
            if (Common.isFailure(result)) {
                Common.callIfPossible(args.errorCallback, Common.getErrorObject(result));
                return;
            }

            Common.callIfPossible(args.successCallback);
        };

        var result = _call('ContactManager_updateBatch',
                           {addressBook: {}, batchArgs: _toJsonObject(args.persons) },
                           callback);

        if (Common.isFailure(result)) {
            throw Common.getErrorObject(result);
        }
    };

    // Removes a person from the contact DB synchronously.
    ContactManager.prototype.remove = function () {
        // validation
        var args = AV.validateMethod(arguments, [
            {
                name: 'personId',
                type: AV.Types.STRING,
                optional: false,
                nullable: false
            }
        ]);

        var result = _callSync('ContactManager_remove', {personId: args.personId});
        _checkError(result);
    };

    // Removes persons from contact DB asynchronously.
    ContactManager.prototype.removeBatch = function () {
        var args = AV.validateMethod(arguments, [
            {
                name: 'personIds',
                type: AV.Types.ARRAY,
                values: AV.Types.STRING,
                optional: false,
                nullable: false
            },
            {
                name: 'successCallback',
                type: AV.Types.FUNCTION,
                optional: true,
                nullable: true
            },
            {
                name: 'errorCallback',
                type: AV.Types.FUNCTION,
                optional: true,
                nullable: true
            }
        ]);

        var callback = function (result) {
            if (Common.isFailure(result)) {
                Common.callIfPossible(args.errorCallback, Common.getErrorObject(result));
                return;
            }

            Common.callIfPossible(args.successCallback);
        };

        var result = _call('ContactManager_removeBatch',
                           {addressBook: {}, batchArgs: _toJsonObject(args.personIds) },
                           callback);

        if (Common.isFailure(result)) {
            throw Common.getErrorObject(result);
        }
    };

    // Gets an array of all Person objects from the contact DB or the ones that match the
    // optionally supplied filter.
    ContactManager.prototype.find = function () {
        var args = AV.validateMethod(arguments, [
            {
                name: 'successCallback',
                type: AV.Types.FUNCTION,
                optional: false,
                nullable: false
            },
            {
                name: 'errorCallback',
                type: AV.Types.FUNCTION,
                optional: true,
                nullable: true
            },
            {
                name: 'filter',
                type: AV.Types.PLATFORM_OBJECT,
                values: [tizen.AttributeFilter,
                         tizen.AttributeRangeFilter,
                         tizen.CompositeFilter],
                optional: true,
                nullable: true
            },
            {
                name: 'sortMode',
                type: AV.Types.PLATFORM_OBJECT,
                values: tizen.SortMode,
                optional: true,
                nullable: true
            }
        ]);

        // TODO implement contact sorting.

        var data = {
            filter: Common.repackFilter(args.filter),
            sortMode: args.sortMode
        };

        var self = this;

        var callback = function(result) {
            if (Common.isSuccess(result)) {
                var _result = Common.getResultObject(result);
                var retval = [];
                for (var i=0; i<_result.length; ++i) {
                    retval.push(self.get(String(_result[i])));
                }
                //TODO: Move sorting to native code
                retval = Common.sort(retval, args.sortMode);
                args.successCallback(retval);
            } else {
                Common.callIfPossible(args.errorCallback, Common.getErrorObject(result));
            }
        };

        var result = _call('ContactManager_find', data, callback);

        if (Common.isFailure(result)) {
            throw Common.getErrorObject(result);
        }
    };

    // Subscribes to receive notifications about persons' changes.
    ContactManager.prototype.addChangeListener = function () {
        var args = AV.validateMethod(arguments, [
            {
                name: 'successCallback',
                type: AV.Types.LISTENER,
                values: ['onpersonsadded', 'onpersonsupdated', 'onpersonsremoved'],
                optional: false,
                nullable: false
            }
        ]);

        if (Type.isEmptyObject(_personCallbackMap)) {
            var result = _callSync('ContactManager_startListening', {});

            if (Common.isFailure(result)) {
                throw Common.getErrorObject(result);
            }
        }

        if (!_registered) {
            native.addListener(_listenerId, _personChangeListener);
            _registered = true;
        }

        var currentWatchId = _getNextWatchId();

        _personCallbackMap[currentWatchId] = args.successCallback;

        return currentWatchId;
    };

    // Unsubscribes a persons' changes watch operation.
    ContactManager.prototype.removeChangeListener = function () {
        var args = AV.validateMethod(arguments, [
            {
                name: 'watchId',
                type: AV.Types.LONG,
                optional: false,
                nullable: false
            }
        ]);

        // This makes UTC_contact_removeChangeListenerPerson_N_001 pass.
        // watch id's start at 1
        if (args.watchId === 0) {
            Common.throwInvalidValues('id is null or undefined');
        }

        if (args.watchId < 0) {
            Common.throwInvalidValues('Negative watch id');
        }

        if (!_personCallbackMap.hasOwnProperty(args.watchId)) {
            Common.throwNotFound('watch id not found');
        }

        delete _personCallbackMap[args.watchId];

        if (Type.isEmptyObject(_personCallbackMap)) {
            native.removeListener(_listenerId, _personChangeListener);
            _registered = false;

            var result = _callSync('ContactManager_stopListening', {});

            if (Common.isFailure(result)) {
                throw Common.getErrorObject(result);
            }
        }
    };

    // exports /////////////////////////////////////////////////////////////////
    module.exports = ContactManager;

})();
