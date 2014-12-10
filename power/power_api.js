/* global xwalk, extension, tizen */

// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var listener_ = undefined;

function throwException_(err) {
    throw new tizen.WebAPIException(err.code, err.name, err.message);
}

function callSync_(msg) {
    var ret = extension.internal.sendSyncMessage(JSON.stringify(msg));
    var obj = JSON.parse(ret);
    if (obj.error)
        throwException_(obj.error);
    return obj.result;
}

extension.setMessageListener(function(msg) {
    var m = JSON.parse(msg);
    if (m.cmd == 'ScreenStateChanged') {
        if (listener_) {
            listener_(m.prev_state, m.new_state);
        }
    }
});

/**
 * An enumerator that defines power resources with values aligned with
 * SystemInfo property values.
 * @enum {string}
 */
var PowerResource = {
    SCREEN : 'SCREEN',
    CPU : 'CPU'
};

/**
 * An enumerator that indicates the power state for screen resource.
 * @enum {string}
 */
var PowerScreenState = {
    SCREEN_OFF : 'SCREEN_OFF',
    SCREEN_DIM : 'SCREEN_DIM',
    SCREEN_NORMAL : 'SCREEN_NORMAL',
    SCREEN_BRIGHT : 'SCREEN_BRIGHT'
};

/**
 * An enumerator that indicates the power state for cpu resource.
 * @enum {string}
 */
var PowerCpuState = {
    CPU_AWAKE : 'CPU_AWAKE'
};

/**
 * This class provides functions to request and release power resource.
 * @constructor
 */
function PowerManager() {
}

/**
 * Requests the minimum-state for a power resource.
 * @param {!PowerResource} resource The power resource for which the request
 *     is made.
 * @param {!PowerState} state The minimal state in which the power resource
 *     is desired to be.
 */
PowerManager.prototype.request = function() {
    var args = xwalk.utils.validator.validateArgs(arguments, [
        {name: 'resource', type: 'string'},
        {name: 'state', type: 'string'}
    ]);

    if (!PowerResource.hasOwnProperty(args.resource))
        throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR);

    if (args.resource == 'SCREEN' && !PowerScreenState.hasOwnProperty(args.state))
        throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR);

    if (args.resource == 'CPU' && !PowerCpuState.hasOwnProperty(args.state))
        throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR);

    callSync_({
        'cmd': 'PowerManager_request',
        'resource': args.resource,
        'state': args.state
    });
};

/**
 * Releases the power state request for the given resource.
 * @param {!PowerResource} resource The resource for which requests are to
 *     be removed.
 */
PowerManager.prototype.release = function() {
    var args = xwalk.utils.validator.validateArgs(arguments, [
        {name: 'resource', type: 'string'}
    ]);

    if (!PowerResource.hasOwnProperty(args.resource))
        throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR);

    callSync_({
        'cmd': 'PowerManager_release',
        'resource': args.resource
    });
};

/**
 * Sets the screen state change callback and monitors its state changes.
 * @param {!function} listener The screen state change callback.
 */
PowerManager.prototype.setScreenStateChangeListener = function() {
    var args = xwalk.utils.validator.validateArgs(arguments, [
        {name: 'listener', type: 'function'}
    ]);

    listener_ = args.listener;
};

/**
 * Unsets the screen state change callback and stop monitoring it.
 */
PowerManager.prototype.unsetScreenStateChangeListener = function() {
    listener_ = undefined;
};

/**
 * Gets the screen brightness level of an application, from 0 to 1.
 * @return {number} Current screen brightness value.
 */
PowerManager.prototype.getScreenBrightness = function() {
    var ret = callSync_({
        'cmd': 'PowerManager_getScreenBrightness'
    });

    return ret;
};

/**
 * Sets the screen brightness level for an application, from 0 to 1.
 * @param {!number} brightness The screen brightness value to set.
 */
PowerManager.prototype.setScreenBrightness = function() {
    var args = xwalk.utils.validator.validateArgs([
        {name: 'brightness', type: 'double'}
    ]);

    if (brightness < 0 || brightness > 1)
        throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR);

    callSync_({
        'cmd': 'PowerManager_setScreenBrightness',
        'brightness': args.brightness
    });
};

/**
 * Returns true if the screen is on.
 * @return {boolean} true if screen is on.
 */
PowerManager.prototype.isScreenOn = function() {
    var ret = callSync_({
        'cmd': 'PowerManager_isScreenOn'
    });

    return ret;
};

/**
 * Restores the screen brightness to the system default setting value.
 */
PowerManager.prototype.restoreScreenBrightness = function() {
    callSync_({
        'cmd': 'PowerManager_restoreScreenBrightness'
    });
};

/**
 * Turns on the screen.
 */
PowerManager.prototype.turnScreenOn = function() {
    callSync_({
        'cmd': 'PowerManager_setScreenState',
        'on': true
    });
};

/**
 * Turns off the screen.
 */
PowerManager.prototype.turnScreenOff = function() {
    callSync_({
        'cmd': 'PowerManager_setScreenState',
        'on': false
    });
};

// Exports
exports = new PowerManager();
