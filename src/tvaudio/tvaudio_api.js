/* global xwalk, extension, tizen */

// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.



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
  return undefined;
};


/**
 * Gets the mute state.
 */
AudioControlManager.prototype.isMute = function() {
  return undefined;
};


/**
 * Changes the volume level.
 * @param {!number} volume The value of volume
 *     (the available volume range is 0 ~ 100)
 */
AudioControlManager.prototype.setVolume = function(volume) {
  return undefined;
};


/**
 * Increases the volume by 1 level.
 */
AudioControlManager.prototype.setVolumeUp = function() {
  return undefined;
};


/**
 * Decreases the volume by 1 level.
 */
AudioControlManager.prototype.setVolumeDown = function() {
  return undefined;
};


/**
 * Gets the current volume level.
 * @return {number} The current volume (the volume range is 0 ~ 100)
 */
AudioControlManager.prototype.getVolume = function() {
  return undefined;
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
  return undefined;
};


// Exports
exports = new AudioControlManager();
