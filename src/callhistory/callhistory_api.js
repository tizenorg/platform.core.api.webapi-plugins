// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var Common = function() {
function _getException(type, msg) {
    return new WebAPIException(type, msg || 'Unexpected exception');
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

function _prepareRequest(module, method, args) {
    var request = {
            module : module
    };
    request.data = {
            method : method,
            args : args
    };
    return request;
}

Common.prototype.getCallSync = function (msg) {
    var ret = extension.internal.sendSyncMessage(JSON.stringify(msg));
    var obj = JSON.parse(ret);
    if (obj.error) {
        throwException_(obj.error);
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
    return new WebAPIException(result.error);
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
var _nextId = 0;

extension.setMessageListener(function(msg) {

});

function CallHistory() {
}

CallHistory.prototype.find = function() {

}

CallHistory.prototype.remove = function() {

}

CallHistory.prototype.removeBatch = function() {

}

CallHistory.prototype.removeAll = function() {

}

CallHistory.prototype.addChangeListener  = function() {

}

CallHistory.prototype.removeChangeListener = function() {

}

function RemoteParty(data) {

}

function CallHistoryEntry(data) {

}

// Exports
var CallHistoryObject = new CallHistory();

exports.find = CallHistoryObject.find;
exports.remove = CallHistoryObject.remove;
exports.removeBatch = CallHistoryObject.removeBatch;
exports.removeAll = CallHistoryObject.removeAll;
exports.addChangeListener = CallHistoryObject.addChangeListener;
exports.removeChangeListener = CallHistoryObject.removeChangeListener;
