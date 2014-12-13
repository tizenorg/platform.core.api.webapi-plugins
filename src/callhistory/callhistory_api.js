// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var Common = function() {
function _getException(type, msg) {
    return new tizen.WebAPIException(type, msg || 'Unexpected exception');
}

function _getTypeMismatch(msg) {
    return _getException(WebAPIException.TYPE_MISMATCH_ERR,
            msg || 'Provided arguments are not valid.');
}

function _throwTypeMismatch(msg) {
    throw _getTypeMismatch(msg);
}

var _type = xwalk.utils.type;
var _converter = xwalk.utils.converter;
var _validator = xwalk.utils.validator;

function Common() {}

function _prepareRequest(cmd, args) {
    var request = {
            cmd : cmd,
            args : args || {}
    };

    return request;
}

Common.prototype.callSync = function (cmd, args) {
    var request = _prepareRequest(cmd, args);
    var ret = extension.internal.sendSyncMessage(JSON.stringify(request));
    var obj = JSON.parse(ret);
    if (obj.error) {
        throw new tizen.throwException(obj.error);
    }

    return obj.result;
};

Common.prototype.getCall = function (module) {
    return function _call(method, args, callback) {
        return JSON.parse(native.call(_prepareRequest(module, method, args), function (result) {
            if (typeof callback === 'function') {
                callback(JSON.parse(result));
            }
        }));
    };
};

Common.prototype.isSuccess = function (result) {
    return (result.status !== 'error');
};

Common.prototype.isFailure = function (result) {
    return !this.isSuccess(result);
};

Common.prototype.getErrorObject = function (result) {
    return new tizen.WebAPIException(0, result.error.message, result.error.name);
};

Common.prototype.getResultObject = function (result) {
    return result.result;
};

Common.prototype.callIfPossible = function(callback) {
    if (!_type.isNullOrUndefined(callback)) {
        callback.apply(callback, [].slice.call(arguments, 1));
    }
};

Common.prototype.getTypeMismatch = function(msg) {
    _getTypeMismatch(msg);
};

Common.prototype.throwTypeMismatch = function(msg) {
    _throwTypeMismatch(msg);
};

Common.prototype.getInvalidValues = function(msg) {
    return _getException('InvalidValuesError',
            msg || 'There\'s a problem with input value.');
};

Common.prototype.throwInvalidValues = function(msg) {
    throw this.getInvalidValues(msg);
};

Common.prototype.getIOError = function (msg) {
    return _getException('IOError', msg || 'Unexpected IO error.');
};

Common.prototype.throwIOError = function (msg) {
    throw this.getIOError(msg);
};

Common.prototype.getNotSupported = function (msg) {
    return _getException(WebAPIException.NOT_SUPPORTED_ERR, msg || 'Not supported.');
};

Common.prototype.throwNotSupported = function (msg) {
    throw this.getNotSupported(msg);
};

Common.prototype.getNotFound = function (msg) {
    return _getException('NotFoundError', msg || 'Not found.');
};

Common.prototype.throwNotFound = function (msg) {
    throw this.getNotFound(msg);
};

Common.prototype.getUnknownError = function (msg) {
    return _getException('UnknownError', msg || 'Unknown error.');
};

Common.prototype.throwUnknownError = function (msg) {
    throw this.getUnknownError(msg);
};

Common.prototype.throwTypeMismatch = function(msg) {
    _throwTypeMismatch(msg);
};

Common.prototype.sort = function (arr, sortMode) {
    var _getSortProperty = function (obj, props) {
        for (var i = 0; i < props.length; ++i) {
            if (!obj.hasOwnProperty(props[i])) {
                return null;
            }
            obj = obj[props[i]];
        }
        return obj;
    };

    if (sortMode instanceof tizen.SortMode) {
        var props = sortMode.attributeName.split('.');
        arr.sort(function (a, b) {
            var aValue = _getSortProperty(a, props);
            var bValue = _getSortProperty(b, props);

            if (sortMode.order === 'DESC') {
                return aValue < bValue;
            }
            return bValue < aValue;
        });
    }
    return arr;
};

Common.prototype.filter = function (arr, filter) {
    if (_type.isNullOrUndefined(arr))
        return arr;
    if (filter instanceof tizen.AttributeFilter ||
            filter instanceof tizen.AttributeRangeFilter ||
            filter instanceof tizen.CompositeFilter) {
        arr = arr.filter(function(element) {
            return filter._filter(element);
        });
    }
    return arr;
};

Common.prototype.repackFilter = function (filter) {
    if (filter instanceof tizen.AttributeFilter) {
        return {
            filterType: "AttributeFilter",
            attributeName: filter.attributeName,
            matchFlag: filter.matchFlag,
            matchValue: filter.matchValue,
        };
    }
    if (filter instanceof tizen.AttributeRangeFilter) {
        return {
            filterType: "AttributeRangeFilter",
            attributeName: filter.attributeName,
            initialValue: _type.isNullOrUndefined(filter.initialValue) ? null : filter.initialValue,
                    endValue: _type.isNullOrUndefined(filter.endValue) ? null : filter.endValue
        };
    }
    if (filter instanceof tizen.CompositeFilter) {
        var _f = [];
        var filters = filter.filters;

        for (var i = 0; i < filters.length; ++i) {
            _f.push(this.repackFilter(filters[i]));
        }

        return {
            filterType: "CompositeFilter",
            type: filter.type,
            filters: _f
        };
    }

    return null;
}
    var _common = new Common();

    return {
        Type : _type,
        Converter : _converter,
        ArgumentValidator : _validator,
        Common : _common
    };
};

var _common = new Common();
var T = _common.Type;
var Converter = _common.Converter;
var AV = _common.ArgumentValidator;
var C = _common.Common;

var _listeners = {};
var _listenersId = 0;

function _createCallHistoryEntries(e) {
    var entries_array = [];
    var entries = e.data;

    entries.forEach(function (data) {
        entries_array.push(new CallHistoryEntry(data));
    });

    return entries_array;
};

extension.setMessageListener(function(msg) {
    var m = JSON.parse(msg);
    if (m.cmd == 'CallHistoryChangeCallback') {
        var d = null;

        switch (m.action) {
        case 'onadded':
        case 'onchanged':
            d = _createCallHistoryEntries(m);
            break;

        case 'onremoved':
            d = m.data;
            break;

        default:
            console.log('Unknown mode: ' + m.action);
            return;
        }

        for (var watchId in _listeners) {
            if (_listeners.hasOwnProperty(watchId) && _listeners[watchId][m.action]) {
                _listeners[watchId][m.action](d);
            }
        }
    }
});

function CallHistory() {
};

CallHistory.prototype.find = function() {

};

CallHistory.prototype.remove = function() {

};

CallHistory.prototype.removeBatch = function() {

};

CallHistory.prototype.removeAll = function() {

};

CallHistory.prototype.addChangeListener = function() {
    var args = AV.validateArgs(arguments, [
        {
            name : 'eventCallback',
            type : AV.Types.LISTENER,
            values : ['onadded', 'onchanged', 'onremoved']
        }
    ]);

    if (T.isEmptyObject(_listeners)) {
        C.callSync('CallHistory_addChangeListener');
    }

    var watchId = ++_listenersId;
    _listeners[watchId] = args.eventCallback;

     return watchId;
};

CallHistory.prototype.removeChangeListener = function() {
    var args = AV.validateArgs(arguments, [
        {
            name : 'watchId',
            type : AV.Types.LONG
        }
    ]);

    var id = args.watchId;

    if (T.isNullOrUndefined(_listeners[id])) {
        throw new tizen.WebAPIException(0, 'NotFoundError', 'Watch id not found.');
    }

    delete _listeners[id];

    if (T.isEmptyObject(_listeners)) {
       C.callSync('CallHistory_removeChangeListener');
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
            value: data.personId ? Converter.toString(data.personId) : null,
            writable: false,
            enumerable: true
        }
    });
};

function CallHistoryEntry(data) {

    function directionSetter(val) {
        direction = Converter.toString(val, false);
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
        directionSetter(data.direction);
    }

    Object.defineProperties(this, {
        uid: {value: Converter.toString(data.uid), writable: false, enumerable: true},
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
