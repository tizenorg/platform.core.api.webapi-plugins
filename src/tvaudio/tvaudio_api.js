/* global xwalk, extension, tizen */

// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.



var native = new xwalk.utils.NativeManager(extension);
var validator = xwalk.utils.validator;
var types = validator.Types;



/**
 * This class provides access to the API functionalities through the tizen.tvaudiocontrol interface.
 * @constructor
 */
function AudioControlManager() {
  if (!(this instanceof AudioControlManager)) {
    throw new TypeError;
  }
}


/**
 * Turns on or off the silent mode.
 * @param {!boolean} mute  The mute state
 *     (true = turn on the silent mode, false = turn off the silent mode)
 */

AudioControlManager.prototype.setMute = function(mute) {
  var args = validator.validateArgs(arguments, [
    {name: 'mute', type: types.BOOLEAN}
  ]);

  var ret = native.callSync('AudioControlManager_setMute', {
    mute: args.mute
  });

  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
};


/**
 * Gets the mute state.
 */
AudioControlManager.prototype.isMute = function() {
  var ret = native.callSync('AudioControlManager_isMute');

  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }

  return native.getResultObject(ret);
};


/**
 * Changes the volume level.
 * @param {!number} volume The value of volume
 *     (the available volume range is 0 ~ 100)
 */
AudioControlManager.prototype.setVolume = function(volume) {

  var args = validator.validateArgs(arguments, [
    {name: 'volume', type: types.UNSIGNED_LONG}
  ]);

  if (args.volume < 0 || args.volume > 100) {
    throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR,
        'Volume is out of range: ' + args.volume, 'InvalidValuesError');
  }

  var ret = native.callSync('AudioControlManager_setVolume', {
    volume: args.volume
  });

  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
};


/**
 * Increases the volume by 1 level.
 */
AudioControlManager.prototype.setVolumeUp = function() {
  var ret = native.callSync('AudioControlManager_setVolumeUp');

  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
};


/**
 * Decreases the volume by 1 level.
 */
AudioControlManager.prototype.setVolumeDown = function() {
  var ret = native.callSync('AudioControlManager_setVolumeDown');

  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
};


/**
 * Gets the current volume level.
 * @return {number} The current volume (the volume range is 0 ~ 100)
 */
AudioControlManager.prototype.getVolume = function() {
  var ret = native.callSync('AudioControlManager_getVolume');
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
  return native.getResultObject(ret);
};


/**
 * Registers a volume change callback for getting notified when TV volume has been changed.
 * @param {!function} listener The method to invoke when the volume has been changed.
 */
AudioControlManager.prototype.setVolumeChangeListener = function(listener) {
  return undefined;
};


/**
 * Unregisters the volume change callback for detecting the volume changes.
 */
AudioControlManager.prototype.unsetVolumeChangeListener = function() {
  return undefined;
};


/**
 * Gets the current audio output mode.
 * @return {AudioOutputMode} The current audio output mode
 */
AudioControlManager.prototype.getOutputMode = function() {
  var ret = native.callSync('AudioControlManager_getOutputMode');
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
  return native.getResultObject(ret);
};


/**
 * Plays the sound of a specific beep.
 */
AudioControlManager.prototype.playSound = function() {
  return undefined;
};


// Exports
exports = new AudioControlManager();
