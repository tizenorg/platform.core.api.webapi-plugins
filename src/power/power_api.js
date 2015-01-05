/* global xwalk, extension, tizen */

// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var validator_ = xwalk.utils.validator;
var types_ = validator_.Types;
var native_ = new xwalk.utils.NativeManager(extension);


/**
 * @type {string}
 * @const
 */
var SCREEN_STATE_LISTENER = 'ScreenStateChanged';


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
    var args = validator_.validateArgs(arguments, [
        {name: 'resource', type: types_.STRING},
        {name: 'state', type: types_.STRING}
    ]);

    if (!PowerResource.hasOwnProperty(args.resource))
        throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR);

    if (args.resource == 'SCREEN' && !PowerScreenState.hasOwnProperty(args.state))
        throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR);

    if (args.resource == 'CPU' && !PowerCpuState.hasOwnProperty(args.state))
        throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR);

    native_.callSync('PowerManager_request', {
        resource: args.resource,
        state: args.state
    });
};

/**
 * Releases the power state request for the given resource.
 * @param {!PowerResource} resource The resource for which requests are to
 *     be removed.
 */
PowerManager.prototype.release = function() {
    var args = validator_.validateArgs(arguments, [
        {name: 'resource', type: types_.STRING}
    ]);

    if (!PowerResource.hasOwnProperty(args.resource))
        throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR);

    native_.callSync('PowerManager_release', {
        resource: args.resource
    });
};

/**
 * Sets the screen state change callback and monitors its state changes.
 * @param {!function} listener The screen state change callback.
 */
PowerManager.prototype.setScreenStateChangeListener = function() {
    var args = validator_.validateArgs(arguments, [
        {name: 'listener', type: types_.FUNCTION}
    ]);

    native_.addListener(SCREEN_STATE_LISTENER, args.listener);
};

/**
 * Unsets the screen state change callback and stop monitoring it.
 */
PowerManager.prototype.unsetScreenStateChangeListener = function() {
    native_.removeListener(SCREEN_STATE_LISTENER);
};

/**
 * Gets the screen brightness level of an application, from 0 to 1.
 * @return {number} Current screen brightness value.
 */
PowerManager.prototype.getScreenBrightness = function() {
    var ret = native_.callSync('PowerManager_getScreenBrightness');

    if (native_.isFailure(ret)) {
        throw native_.getErrorObject(ret);
    }

    return native_.getResultObject(ret);
};

/**
 * Sets the screen brightness level for an application, from 0 to 1.
 * @param {!number} brightness The screen brightness value to set.
 */
PowerManager.prototype.setScreenBrightness = function() {
    var args = validator_.validateArgs(arguments, [
        {name: 'brightness', type: types_.DOUBLE}
    ]);

    if (args.brightness < 0 || args.brightness > 1)
        throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR);

    native_.callSync('PowerManager_setScreenBrightness', {
        brightness: args.brightness
    });
};

/**
 * Returns true if the screen is on.
 * @return {boolean} true if screen is on.
 */
PowerManager.prototype.isScreenOn = function() {
    var ret = native_.callSync('PowerManager_isScreenOn');

    if (native_.isFailure(ret)) {
        throw native_.getErrorObject(ret);
    }

    return native_.getResultObject(ret);
};

/**
 * Restores the screen brightness to the system default setting value.
 */
PowerManager.prototype.restoreScreenBrightness = function() {
    var ret = native_.callSync('PowerManager_restoreScreenBrightness');

    if (native_.isFailure(ret)) {
        throw native_.getErrorObject(ret);
    }
};

/**
 * Turns on the screen.
 */
PowerManager.prototype.turnScreenOn = function() {
    var ret = native_.callSync('PowerManager_setScreenState', {
        on: true
    });

    if (native_.isFailure(ret)) {
        throw native_.getErrorObject(ret);
    }
};

/**
 * Turns off the screen.
 */
PowerManager.prototype.turnScreenOff = function() {
    var ret = native_.callSync('PowerManager_setScreenState', {
        on: false
    });

    if (native_.isFailure(ret)) {
        throw native_.getErrorObject(ret);
    }
};

exports = new PowerManager();
