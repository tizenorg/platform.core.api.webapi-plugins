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
var privilege_ = xwalk.utils.privilege;
var types_ = validator_.Types;
var type_utils = xwalk.utils.type;
var native_ = new xwalk.utils.NativeManager(extension);

//////////////////SEService/////////////////

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
    case 'onSEReady':
    case 'onSENotReady':
        d = new Reader(msg.handle);
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
    xwalk.utils.checkPrivilegeAccess(privilege_.SECUREELEMENT);

    var id = this.nextId;
    if (!this.nativeSet) {
        this.native.addListener(this.listenerName, this.onListenerCalled.bind(this));
        this.native.callSync('SEService_registerSEListener');
        this.nativeSet = true;
    }

    this.listeners[id] = callback;
    ++this.nextId;

    return id;
};

ListenerManager.prototype.removeListener = function(watchId) {
    xwalk.utils.checkPrivilegeAccess(privilege_.SECUREELEMENT);

    if (this.listeners.hasOwnProperty(watchId)) {
      delete this.listeners[watchId];
    }

    if (this.nativeSet && type_utils.isEmptyObject(this.listeners)) {
        this.native.callSync('SEService_unregisterSEListener');
        this.native.removeListener(this.listenerName);
        this.nativeSet = false;
    }
};

var SE_CHANGE_LISTENER = 'SecureElementChangeListener';
var SEChangeListener = new ListenerManager(native_, SE_CHANGE_LISTENER);

function SEService() {
}

SEService.prototype.getReaders = function() {
    xwalk.utils.checkPrivilegeAccess(privilege_.SECUREELEMENT);

    var args = validator_.validateArgs(arguments, [
        { name: "successCallback", type: types_.FUNCTION },
        { name: "errorCallback", type: types_.FUNCTION, optional: true, nullable: true }
    ]);

    var callback = function(result) {
        if(native_.isFailure(result)) {
            native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
        } else {
            var result_obj = native_.getResultObject(result);
            var readers_array = [];

            result_obj.forEach(function (data) {
                readers_array.push(new Reader(data));
            });

            args.successCallback(readers_array);
        }
    };

    native_.call('SEService_getReaders', {}, callback);
};

SEService.prototype.registerSEListener = function() {
    var args = validator_.validateArgs(arguments, [
        {
            name : 'eventCallback',
            type : types_.LISTENER,
            values: ['onSEReady', 'onSENotReady']
        }
    ]);

    return SEChangeListener.addListener(args.eventCallback);
};

SEService.prototype.unregisterSEListener = function() {
    var args = validator_.validateArgs(arguments, [
        {
            name : 'id',
            type : types_.UNSIGNED_LONG
        }
    ]);

    SEChangeListener.removeListener(args.id);
}

SEService.prototype.shutdown = function() {
    xwalk.utils.checkPrivilegeAccess(privilege_.SECUREELEMENT);

    var result = native_.callSync('SEService_shutdown', {});

    if (native_.isFailure(result)) {
        throw native_.getErrorObject(result);
    }
};

//////////////////Reader/////////////////

function Reader(reader_handle) {
    Object.defineProperties(this, {
        isPresent:  {   configurable: false,
                        enumerable: true,
                        set: function() {},
                        get: function() {
                            var callArgs = { handle: reader_handle };
                            var result = native_.callSync('SEReader_isPresent', callArgs);
                            if (native_.isFailure(result)) {
                              console.log('SEReader_isPresent error: ' + native_.getErrorObject(result));
                              return false;
                            } else {
                              return native_.getResultObject(result).isPresent;
                            }
                        }},
        _handle:    {   configurable: false,
                        enumerable: false,
                        set: function() {},
                        get: function() { return reader_handle }}
    });
}

Reader.prototype.getName = function() {
    xwalk.utils.checkPrivilegeAccess(privilege_.SECUREELEMENT);

    var callArgs = { handle: this._handle };
    var result = native_.callSync('SEReader_getName', callArgs);

    if(native_.isFailure(result)) {
        throw native_.getErrorObject(result);
    }

    return native_.getResultObject(result).name;
};

Reader.prototype.openSession = function() {
    xwalk.utils.checkPrivilegeAccess(privilege_.SECUREELEMENT);

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
    xwalk.utils.checkPrivilegeAccess(privilege_.SECUREELEMENT);
    var callArgs = { handle: this._handle };
    var result = native_.callSync('SEReader_closeSessions', callArgs);
    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }
};

//////////////////Channel/////////////////

function Channel( channel_handle, is_basic_channel) {
    Object.defineProperties(this, {
        _handle:    { enumerable: false, configurable: false, set: function() {}, get: function() { return channel_handle }},
        isBasicChannel:   { enumerable: true, configurable: false, set: function() {}, get: function() { return is_basic_channel }}
    });
}

Channel.prototype.close = function() {
    xwalk.utils.checkPrivilegeAccess(privilege_.SECUREELEMENT);
    var callArgs = { handle: this._handle };
    native_.callSync('SEChannel_close', callArgs);
};

Channel.prototype.transmit = function() {
    xwalk.utils.checkPrivilegeAccess(privilege_.SECUREELEMENT);

    var args = validator_.validateArgs(arguments, [
        { name: "command", type: types_.ARRAY, values: types_.BYTE },
        { name: "successCallback", type: types_.FUNCTION },
        { name: "errorCallback", type: types_.FUNCTION, optional: true, nullable: true }
    ]);

    var callback = function(result) {
        if ( native_.isFailure(result)) {
            native_.callIfPossible( args.errorCallback, native_.getErrorObject(result));
        } else {
            var result_obj = native_.getResultObject(result);
            args.successCallback(result_obj.response);
        }
    }

    var callArgs = {
        handle: this._handle,
        command: args.command
    };

    native_.call('SEChannel_transmit', callArgs, callback);
}

Channel.prototype.getSelectResponse = function() {
    xwalk.utils.checkPrivilegeAccess(privilege_.SECUREELEMENT);
    var callArgs = { handle: this._handle };
    native_.callSync('SEChannel_getSelectResponse', callArgs);
}

//////////////////Session/////////////////

function Session(session_handle) {
    Object.defineProperties(this, {
        isClosed:   { configurable: false,
                      enumerable: true,
                      set: function() {},
                      get: function() {
                        var callArgs = { handle: session_handle };
                        var result = native_.callSync('SESession_isClosed', callArgs);
                        if (native_.isFailure(result)) {
                          console.log('SESession_isClosed error: ' + native_.getErrorObject(result));
                          return true;
                        } else {
                          return native_.getResultObject(result).isClosed;
                        }
                      }
                    },
        _handle:    { enumerable: false,
                      configurable: false,
                      set: function() {},
                      get: function() { return session_handle }}
    });
}

Session.prototype.openBasicChannel = function() {
    xwalk.utils.checkPrivilegeAccess(privilege_.SECUREELEMENT);

    var args = validator_.validateArgs(arguments, [
        { name: "aid", type: types_.ARRAY, values: types_.BYTE },
        { name: "successCallback", type: types_.FUNCTION },
        { name: "errorCallback", type: types_.FUNCTION, optional: true, nullable: true }
    ]);

    var callback = function(result) {
        if ( native_.isFailure(result)) {
            native_.callIfPossible( args.errorCallback, native_.getErrorObject(result));
        } else {
            var result_obj = native_.getResultObject(result);
            var channel = new Channel( result_obj.handle, result_obj.isBasicChannel);
            args.successCallback(channel);
        }
    }

    var callArgs = {
        handle: this._handle,
        aid: args.aid
    };

    native_.call('SESession_openBasicChannel', callArgs, callback);
};

Session.prototype.openLogicalChannel = function() {
    xwalk.utils.checkPrivilegeAccess(privilege_.SECUREELEMENT);

    var args = validator_.validateArgs(arguments, [
        { name: "aid", type: types_.ARRAY, values: types_.BYTE },
        { name: "successCallback", type: types_.FUNCTION },
        { name: "errorCallback", type: types_.FUNCTION, optional: true, nullable: true }
    ]);

    var callback = function(result) {
        if ( native_.isFailure(result)) {
            native_.callIfPossible( args.errorCallback, native_.getErrorObject(result));
        } else {
            var result_obj = native_.getResultObject(result);
            var channel = new Channel( result_obj.handle, result_obj.isBasicChannel);
            args.successCallback(channel);
        }
    }

    var callArgs = {
        handle: this._handle,
        aid: args.aid
    };

    native_.call('SESession_openLogicalChannel', callArgs, callback);
}

Session.prototype.getATR = function() {
    xwalk.utils.checkPrivilegeAccess(privilege_.SECUREELEMENT);
    var callArgs = { handle: this._handle };
    var result = native_.callSync('SESession_getATR', callArgs);
    if (native_.isFailure(result)) {
        throw native_.getErrorObject(result);
    } else {
        return native_.getResultObject(result);
    }
}

Session.prototype.close = function() {
    xwalk.utils.checkPrivilegeAccess(privilege_.SECUREELEMENT);
    var callArgs = { handle: this._handle };
    native_.callSync('SESession_close', callArgs);
}

Session.prototype.closeChannels = function() {
    xwalk.utils.checkPrivilegeAccess(privilege_.SECUREELEMENT);
    var callArgs = { handle: this._handle };
    native_.callSync('SESession_closeChannels', callArgs);
}


exports = new SEService();
