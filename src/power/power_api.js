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
var native_ = new xwalk.utils.NativeManager(extension);

function ListenerManager(native, listenerName) {
  this.listener;
  this.native = native;
  this.listenerName = listenerName;
}

ListenerManager.prototype.onListenerCalled = function(msg) {
  if(this.listener) {
    this.listener(msg.prev_state, msg.new_state);
  }
};

ListenerManager.prototype.addListener = function(callback) {
  this.native.addListener(this.listenerName, this.onListenerCalled.bind(this));
  this.listener = callback;
};

ListenerManager.prototype.removeListener = function() {
  this.native.removeListener(this.listenerName);
  delete this.listener;
};

var screenStateChangeListener = new ListenerManager(native_, "SCREEN_STATE_LISTENER");

function callNative(cmd, args) {
    var json = {'cmd':cmd, 'args':args};
    var argjson = JSON.stringify(json);
    var resultString = extension.internal.sendSyncMessage(argjson);
    var result = JSON.parse(resultString);

    if (typeof result !== 'object') {
        throw new WebAPIException(WebAPIException.UNKNOWN_ERR);
    }

    if (result['status'] == 'success') {
        if('result' in result) {
            return result['result'];
        }
        return true;
    } else if (result['status'] == 'error') {
        var err = result['error'];
        if(err) {
            throw new WebAPIException(err);
        }
        return false;
    }
}

function callNativeWithCallback(cmd, args, callback) {
    if(callback) {
        var id = nextCallbackId();
        args['callbackId'] = id;
        callbacks[id] = callback;
    }

    return callNative(cmd, args);
}

function SetReadOnlyProperty(obj, n, v){
    Object.defineProperty(obj, n, {value:v, writable: false});
}

var PowerResource = {
    'SCREEN': 'SCREEN',
    'CPU': 'CPU'
};

/**
 * An enumerator that indicates the power state for screen resource.
 * @enum {string}
 */
var PowerScreenState = {
    'SCREEN_OFF': 'SCREEN_OFF',
    'SCREEN_DIM': 'SCREEN_DIM',
    'SCREEN_NORMAL': 'SCREEN_NORMAL',
    'SCREEN_BRIGHT': 'SCREEN_BRIGHT'
};

/**
 * An enumerator that indicates the power state for cpu resource.
 * @enum {string}
 */
var PowerCpuState = {
    'CPU_AWAKE': 'CPU_AWAKE'
};

/**
 * This class provides functions to request and release power resource.
 * @constructor
 */
function PowerManager() {
    // constructor of PowerManager
}

/**
 * Requests the minimum-state for a power resource.
 * @param {!PowerResource} resource The power resource for which the request
 *     is made.
 * @param {!PowerState} state The minimal state in which the power resource
 *     is desired to be.
 */
PowerManager.prototype.request = function(resource, state) {
   xwalk.utils.checkPrivilegeAccess(privilege_.POWER);

   var args = validator_.validateArgs(arguments, [
        {'name' : 'resource', 'type': types_.ENUM, 'values' : ['SCREEN', 'CPU']},
        {'name' : 'state', 'type': types_.ENUM, 'values' : ['SCREEN_OFF', 'SCREEN_DIM', 'SCREEN_NORMAL', 'SCREEN_BRIGHT', 'CPU_AWAKE']}
    ]);

    var nativeParam = {
    };

    if (args['resource']) {
        nativeParam['resource'] = args.resource;
    }
    if (args['state']) {
        nativeParam['state'] = args.state;
    }

    try {
        var syncResult = callNative('PowerManager_request', nativeParam);
        // if you need synchronous result from native function using 'syncResult'.
    } catch(e) {
        throw e;
    }
};

/**
 * Releases the power state request for the given resource.
 * @param {!PowerResource} resource The resource for which requests are to
 *     be removed.
 */
PowerManager.prototype.release = function(resource) {
    var args = validator_.validateArgs(arguments, [
        {'name' : 'resource', 'type': types_.ENUM, 'values' : ['SCREEN', 'CPU']}
    ]);

    if (!PowerResource.hasOwnProperty(args.resource))
        throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR);

    var nativeParam = {
    };
    if (args['resource']) {
        nativeParam['resource'] = args.resource;
    }
    try {
        var syncResult = callNative('PowerManager_release', nativeParam);
        // if you need synchronous result from native function using 'syncResult'.
    } catch(e) {
        throw e;
    }

};

/**
 * Sets the screen state change callback and monitors its state changes.
 * @param {!function} listener The screen state change callback.
 */
PowerManager.prototype.setScreenStateChangeListener = function(listener) {
    var args = validator_.validateArgs(arguments, [
        {name: 'listener', type: types_.FUNCTION}
    ]);

    screenStateChangeListener.addListener(args.listener);
};

/**
 * Unsets the screen state change callback and stop monitoring it.
 */
PowerManager.prototype.unsetScreenStateChangeListener = function() {
    screenStateChangeListener.removeListener();
};

/**
 * Gets the screen brightness level of an application, from 0 to 1.
 * @return {number} Current screen brightness value.
 */
PowerManager.prototype.getScreenBrightness = function() {
    var nativeParam = {
    };
	var syncResult = 0;

    try {
        syncResult = callNative('PowerManager_getScreenBrightness', nativeParam);
        // if you need synchronous result from native function using 'syncResult'.
    } catch(e) {
        throw e;
    }

	return syncResult;
}

/**
 * Sets the screen brightness level for an application, from 0 to 1.
 * @param {!number} brightness The screen brightness value to set.
 */
PowerManager.prototype.setScreenBrightness = function(brightness) {
    xwalk.utils.checkPrivilegeAccess(privilege_.POWER);

    var args = validator_.validateArgs(arguments, [
        {'name' : 'brightness', 'type': types_.DOUBLE}
    ]);

    if (args.brightness < 0 || args.brightness > 1)
        throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR);

    var nativeParam = {
            'brightness': args.brightness
    };
    try {
        var syncResult = callNative('PowerManager_setScreenBrightness', nativeParam);
        // if you need synchronous result from native function using 'syncResult'.
    } catch(e) {
        throw e;
    }
}

/**
 * Returns true if the screen is on.
 * @return {boolean} true if screen is on.
 */
PowerManager.prototype.isScreenOn = function() {
    var nativeParam = {
    };
	var syncResult = 0;

    try {
        syncResult = callNative('PowerManager_isScreenOn', nativeParam);
        // if you need synchronous result from native function using 'syncResult'.
    } catch(e) {
        throw e;
    }

	return syncResult;
}

/**
 * Restores the screen brightness to the system default setting value.
 */
PowerManager.prototype.restoreScreenBrightness = function() {
    var nativeParam = {
    };

    try {
        var syncResult = callNative('PowerManager_restoreScreenBrightness', nativeParam);
        // if you need synchronous result from native function using 'syncResult'.
    } catch(e) {
        throw e;
    }
}

/**
 * Turns on the screen.
 */
PowerManager.prototype.turnScreenOn = function() {
    xwalk.utils.checkPrivilegeAccess(privilege_.POWER);

    var nativeParam = {
    };

    try {
        var syncResult = callNative('PowerManager_turnScreenOn', nativeParam);
        // if you need synchronous result from native function using 'syncResult'.
    } catch(e) {
        throw e;
    }
}

/**
 * Turns off the screen.
 */
PowerManager.prototype.turnScreenOff = function() {
    xwalk.utils.checkPrivilegeAccess(privilege_.POWER);

    var nativeParam = {
    };

    try {
        var syncResult = callNative('PowerManager_turnScreenOff', nativeParam);
        // if you need synchronous result from native function using 'syncResult'.
    } catch(e) {
        throw e;
    }

}


exports = new PowerManager();

