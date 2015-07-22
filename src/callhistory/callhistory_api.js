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

var validator_ = xwalk.utils.validator;
var converter_ = xwalk.utils.converter;
var types_ = validator_.Types;
var T_ = xwalk.utils.type;
var native_ = new xwalk.utils.NativeManager(extension);

function _createCallHistoryEntries(e) {
    var entries_array = [];
    var entries = e.data;

    entries.forEach(function (data) {
        entries_array.push(new CallHistoryEntry(data));
    });

    return entries_array;
};

function _getUidFromCallHistoryEntry(entries) {
    var uid = [];

    entries.forEach(function (data) {
        uid.push(data.uid);
    });

    return uid;
};

function ListenerManager(native, listenerName) {
    this.listeners = {};
    this.nextId = 1;
    this.nativeSet = false;
    this.native = native;
    this.listenerName = listenerName;
};

ListenerManager.prototype.onListenerCalled = function(msg) {
    var d = undefined;
    switch (msg.action) {
    case 'onadded':
    case 'onchanged':
        d = _createCallHistoryEntries(msg);
        break;

    case 'onremoved':
        d = msg.data;
        break;

    default:
        console.log('Unknown mode: ' + msg.action);
        return;
    }

    for (var watchId in this.listeners) {
        if (this.listeners.hasOwnProperty(watchId) && this.listeners[watchId][msg.action]) {
            this.listeners[watchId][msg.action](d);
        }
    }
};

ListenerManager.prototype.addListener = function(callback) {
    var id = this.nextId;
    if (!this.nativeSet) {
      this.native.addListener(this.listenerName, this.onListenerCalled.bind(this));
      this.nativeSet = true;
    }
    this.listeners[id] = callback;
    ++this.nextId;
    return id;
};

ListenerManager.prototype.removeListener = function(watchId) {
    if (this.listeners[watchId] === null || this.listeners[watchId] === undefined) {
        throw new WebAPIException(0, 'Watch id not found.', 'InvalidValuesError');
    }

    if (this.listeners.hasOwnProperty(watchId)) {
        delete this.listeners[watchId];
    }
};

var CALL_HISTORY_CHANGE_LISTENER = 'CallHistoryChangeCallback';
var callHistoryChangeListener = new ListenerManager(native_, CALL_HISTORY_CHANGE_LISTENER);

function CallHistory() {
};

CallHistory.prototype.find = function() {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.CALLHISTORY_READ);

    var args = validator_.validateArgs(arguments, [
        {
            name : 'successCallback',
            type : types_.FUNCTION,
        },
        {
            name : 'errorCallback',
            type : types_.FUNCTION,
            optional : true,
            nullable : true
        },
        {
            name : 'filter',
            type : types_.PLATFORM_OBJECT,
            optional : true,
            nullable : true,
            values : [tizen.AttributeFilter,
                      tizen.AttributeRangeFilter,
                      tizen.CompositeFilter]
        },
        {
            name : 'sortMode',
            type : types_.PLATFORM_OBJECT,
            optional : true,
            nullable : true,
            values : tizen.SortMode
        },
        {
            name : 'limit',
            type : types_.UNSIGNED_LONG,
            optional : true,
            nullable : true
        },
        {
            name : 'offset',
            type : types_.UNSIGNED_LONG,
            optional : true,
            nullable : true
        }
    ]);

    var callback = function(result) {
        if (native_.isFailure(result)) {
            native_.callIfPossible(args.errorCallback, native.getErrorObject(result));
        } else {
            var entries = _createCallHistoryEntries(result);
            args.successCallback(entries);
        }
    };

    var callArgs = {};
    callArgs.filter = args.filter;
    callArgs.sortMode = args.sortMode;
    callArgs.limit = args.limit;
    callArgs.offset = args.offset;

    native_.call('CallHistory_find', callArgs, callback);
};

CallHistory.prototype.remove = function() {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.CALLHISTORY_WRITE);

    var args = validator_.validateArgs(arguments, [
        {
            name : 'entry',
            type : types_.PLATFORM_OBJECT,
            values : CallHistoryEntry
        }
    ]);

    var callArgs = {};
    callArgs.uid = args.entry.uid;

    var result = native_.callSync('CallHistory_remove', callArgs);
    if (native_.isFailure(result)) {
        throw new WebAPIException(
                WebAPIException.INVALID_VALUES_ERR, 'Watch id not found.');
    }

    return;
};

CallHistory.prototype.removeBatch = function() {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.CALLHISTORY_WRITE);

    var args = validator_.validateArgs(arguments, [
        {
            name : 'entries',
            type : types_.ARRAY,
            values : CallHistoryEntry
        },
        {
            name : 'successCallback',
            type : types_.FUNCTION,
            optional : true,
            nullable : true
        },
        {
            name : 'errorCallback',
            type : types_.FUNCTION,
            optional : true,
            nullable : true
        }
    ]);

    var callback = function(result) {
        if (native_.isFailure(result)) {
            native_.callIfPossible(args.errorCallback, native.getErrorObject(result));
        } else {
            native_.callIfPossible(args.successCallback);
        }
    };

    var uid = _getUidFromCallHistoryEntry(args.entries);
    var callArgs = {};
    callArgs.uid = uid;

    native_.call('CallHistory_removeBatch', callArgs, callback);
};

CallHistory.prototype.removeAll = function() {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.CALLHISTORY_WRITE);

    var args = validator_.validateArgs(arguments, [
        {
            name : 'successCallback',
            type : types_.FUNCTION,
            optional : true,
            nullable : true
        },
        {
            name : 'errorCallback',
            type : types_.FUNCTION,
            optional : true,
            nullable : true
        }
    ]);

    var callback = function(result) {
        if (native_.isFailure(result)) {
            native_.callIfPossible(args.errorCallback, native.getErrorObject(result));
        } else {
            native_.callIfPossible(args.successCallback);
        }
    };

    native_.call('CallHistory_removeAll', {}, callback);
};

CallHistory.prototype.addChangeListener = function() {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.CALLHISTORY_READ);

    var args = validator_.validateArgs(arguments, [
        {
            name : 'eventCallback',
            type : types_.LISTENER,
            values : ['onadded', 'onchanged', 'onremoved']
        }
    ]);

    if (T_.isEmptyObject(callHistoryChangeListener.listeners)) {
        native_.callSync('CallHistory_addChangeListener');
    }

    return callHistoryChangeListener.addListener(args.eventCallback);
};

CallHistory.prototype.removeChangeListener = function() {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.CALLHISTORY_READ);

    var args = validator_.validateArgs(arguments, [
        {
            name : 'watchId',
            type : types_.LONG
        }
    ]);

    callHistoryChangeListener.removeListener(args.watchId);

    if (T_.isEmptyObject(callHistoryChangeListener.listeners)) {
        native_.callSync('CallHistory_removeChangeListener');
    }
};

function RemoteParty(data) {
    Object.defineProperties(this, {
        remoteParty: {
            value: data.remoteParty ? data.remoteParty : null,
            writable: false,
            enumerable: true
        },
        personId: {
            value: data.personId ? converter_.toString(data.personId) : null,
            writable: false,
            enumerable: true
        }
    });
};

function CallHistoryEntry(data) {
    function directionSetter(val) {
        if (direction === 'MISSEDNEW' && val === 'MISSED') {
            var result = native_.callSync('CallHistory_setMissedDirection', {uid : this.uid});
            if (native_.isSuccess(result)) {
                direction = 'MISSED';
            }
        }
    }

    function createRemoteParties(parties) {
        var parties_array = [];
        parties.forEach(function (data) {
            parties_array.push(new RemoteParty(data));
        });
        return parties_array;
    }

    var direction;
    if (data) {
        direction = converter_.toString(data.direction, false);
    }

    Object.defineProperties(this, {
        uid: {value: converter_.toString(data.uid), writable: false, enumerable: true},
        type: {value: data.type, writable: false, enumerable: true},
        features : {
            value: data.features ? data.features : null,
            writable: false,
            enumerable: true
        },
        remoteParties : {
            value : createRemoteParties(data.remoteParties),
            writable: false,
            enumerable: true
        },
        startTime: {value: new Date(Number(data.startTime) * 1000),
            writable: false,
            enumerable: true
        },
        duration: {value: data.duration, writable: false, enumerable: true},
        direction: {
            enumerable: true,
            set : directionSetter,
            get : function() { return direction; }
        },
        callingParty: {
            value: data.callingParty ? data.callingParty : null,
            writable: false,
            enumerable: true
        },
    });
};

// Exports
exports = new CallHistory();
