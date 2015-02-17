/* global xwalk, extension, tizen */

// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


var native = new xwalk.utils.NativeManager(extension);
var validator = xwalk.utils.validator;
var types = validator.Types;


/**
 * @type {string}
 * @const
 */
var VOLUME_CHANGE_LISTENER = 'VolumeChangeCallback';



/**
 * This class provides access to the API functionalities through
 * the tizen.tvaudiocontrol interface.
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
 * @return {boolean} 'true' if sound is muted else 'false'
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

  if (arguments.length < 1) {
    throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR,
        'no volume argument: ' + arguments.length, 'InvalidValuesError');
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
 * Registers a volume change callback for getting notified
 * when TV volume has been changed.
 *
 * @param {!function} listener The method to invoke
 *                    when the volume has been changed.
 */
AudioControlManager.prototype.setVolumeChangeListener = function(listener) {
  var args = validator.validateArgs(arguments, [
    {name: 'listener', type: types.FUNCTION}
  ]);

  native.removeListener(VOLUME_CHANGE_LISTENER);

  var ret = native.callSync('AudioControlManager_setVolumeChangeListener');

  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }

  native.addListener(VOLUME_CHANGE_LISTENER, args.listener);
};


/**
 * Unregisters the volume change callback for detecting the volume changes.
 */
AudioControlManager.prototype.unsetVolumeChangeListener = function() {
    var ret = native.callSync('AudioControlManager_unsetVolumeChangeListener');

    if (native.isFailure(ret)) {
      throw native.getErrorObject(ret);
    }

  native.removeListener(VOLUME_CHANGE_LISTENER);
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
 * Allowed types of sound
 * They should correspond to values in native layer and .pcm files
 */
var AudioBeepType = [
  'MOVE', // indented use the same sound
    'UP',
    'DOWN',
    'LEFT',
    'RIGHT',
    'PAGE_LEFT',
    'PAGE_RIGHT',
  'BACK',
  'SELECT',
  'CANCEL',
  'WARNING',
  'KEYPAD',
  'KEYPAD_ENTER',
  'KEYPAD_DEL',
  'PREPARING'
];
Object.freeze(AudioBeepType);


/**
 * Plays one of specific sounds.
 * @param {!AudioBeepType} beep The Sound to play.
 */
AudioControlManager.prototype.playSound = function(beep) {
  var args = validator.validateArgs(arguments, [{
    name: 'type',
    type: validator.Types.ENUM,
    values: AudioBeepType // AudioBeepType
  }]);

  var ret = native.callSync('AudioControlManager_playSound',
                            {type: args.type});
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
  return;
};

// Exports
exports = new AudioControlManager();
