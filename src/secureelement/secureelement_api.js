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

function Reader() {
    var handle = null;
    Object.defineProperties(this, {
        isPresent:   {value: false, writable: false, enumerable: true}
    });
}

Reader.prototype.getName = function() {
};

Reader.prototype.openSession = function() {
    var args = validator_.validateArgs(arguments, [
        { name: "successCallback", type: types_.FUNCTION },
        { name: "errorCallback", type: types_.FUNCTION, optional: true, nullable: true }
    ]);
}

Reader.prototype.closeSessions = function() {
}

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
