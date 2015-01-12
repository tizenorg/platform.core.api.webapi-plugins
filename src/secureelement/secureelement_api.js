// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var validator_ = xwalk.utils.validator;
var types_ = validator_.Types;
var native_ = new xwalk.utils.NativeManager(extension);

//////////////////SEService/////////////////

function SEService() {
}

SEService.prototype.getReaders = function() {
    var args = validator_.validateArgs(arguments, [
        { name: "successCallback", type: types_.FUNCTION },
        { name: "errorCallback", type: types_.FUNCTION, optional: true, nullable: true }
    ]);
};

SEService.prototype.registerSEListener = function() {
    var args = validator_.validateArgs(arguments, [
        { name: "listener", type: types_.LISTENER, values: ['onSEReady', 'onSENotReady'] },
    ]);
};

SEService.prototype.unregisterSEListener = function() {
    var args = validator_.validateArgs(arguments, [
        { name: "id", type: types_.UNSIGNED_LONG },
    ]);
}

SEService.prototype.shutdown = function() {
    console.log('Shutdown');
    return 'Shutdown';
};

//////////////////Reader/////////////////

function Reader(reader_handle) {
    Object.defineProperties(this, {
        isPresent:  {   configurable: false,
                        enumerable: true,
                        set: function() {},
                        get: function() {
                            var callArgs = { handle: reader_handle };
                            return native_.callSync('SEReader_isPresent', callArgs);
                        }},
        _handle:    {   configurable: false,
                        enumerable: false,
                        set: function() {},
                        get: function() { return reader_handle }}
    });
}

Reader.prototype.getName = function() {
    var callArgs = { handle: this._handle };
    var result = native_.callSync('SEReader_getName', callArgs);

    if(native_.isFailure(result)) {
        throw native_.getErrorObject(result);
    }

    return native_.getResultObject(result).name;
};

Reader.prototype.openSession = function() {
    var args = validator_.validateArgs(arguments, [
        { name: "successCallback", type: types_.FUNCTION },
        { name: "errorCallback", type: types_.FUNCTION, optional: true, nullable: true }
    ]);

    var callback = function(result) {
        if(native_.isFailure(result)) {
            native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
        } else {
            var result_obj = native_.getResultObject(result);
            var session = new Session(result_obj.handle);
            args.successCallback(session);
        }
    };

    var callArgs = { handle: this._handle };

    native_.call('SEReader_openSession', callArgs, callback);
};

Reader.prototype.closeSessions = function() {
    var callArgs = { handle: this._handle };
    native_.call('SEReader_closeSessions', callArgs);
};

//////////////////Session/////////////////

function Session() {
    var handle = null;
    Object.defineProperties(this, {
        isClosed:   {value: false, writable: false, enumerable: true}
    });
}

Session.prototype.openBasicChannel = function() {
    var args = validator_.validateArgs(arguments, [
        { name: "aid", type: types_.ARRAY, values: types_.BYTE },
        { name: "successCallback", type: types_.FUNCTION },
        { name: "errorCallback", type: types_.FUNCTION, optional: true, nullable: true }
    ]);
};

Session.prototype.openLogicalChannel = function() {
    var args = validator_.validateArgs(arguments, [
        { name: "aid", type: types_.ARRAY, values: types_.BYTE },
        { name: "successCallback", type: types_.FUNCTION },
        { name: "errorCallback", type: types_.FUNCTION, optional: true, nullable: true }
    ]);
}

Session.prototype.getATR = function() {
}

Session.prototype.close = function() {
}

Session.prototype.closeChannels = function() {
}

//////////////////Channel/////////////////

function Channel() {
    var handle = null;
    Object.defineProperties(this, {
        isBasicChannel:   {value: false, writable: false, enumerable: true}
    });
}

Channel.prototype.close = function() {
};

Channel.prototype.transmit = function() {
    var args = validator_.validateArgs(arguments, [
        { name: "command", type: types_.ARRAY, values: types_.BYTE },
        { name: "successCallback", type: types_.FUNCTION },
        { name: "errorCallback", type: types_.FUNCTION, optional: true, nullable: true }
    ]);
}

Channel.prototype.getSelectResponse = function() {
}

exports = new SEService();
