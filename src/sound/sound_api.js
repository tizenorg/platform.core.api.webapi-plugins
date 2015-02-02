// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


var utils_ = xwalk.utils;
var type_ = utils_.type;
var converter_ = utils_.converter;
var validator_ = utils_.validator;
var types_ = validator_.Types;
var native_ = new xwalk.utils.NativeManager(extension);


var SoundType = {
  SYSTEM: 'SYSTEM',
  NOTIFICATION: 'NOTIFICATION',
  ALARM: 'ALARM',
  MEDIA: 'MEDIA',
  VOICE: 'VOICE',
  RINGTONE: 'RINGTONE'
};

var SoundModeType = {
  SOUND: 'SOUND',
  VIBRATE: 'VIBRATE',
  MUTE: 'MUTE'
};


function SoundManager() {}

SoundManager.prototype.getSoundMode = function() {
  var result = native_.callSync('SoundManager_getSoundMode');
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  return native_.getResultObject(result);
};

SoundManager.prototype.setVolume = function(type, volume) {
  var args = validator_.validateArgs(arguments, [
    {name: 'type', type: types_.ENUM, values: Object.keys(SoundType)},
    {name: 'volume', type: types_.DOUBLE}
  ]);

  var result = native_.callSync('SoundManager_setVolume', args);
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  return native_.getResultObject(result);
};

SoundManager.prototype.getVolume = function(type) {
  var args = validator_.validateArgs(arguments, [
    {name: 'type', type: types_.ENUM, values: Object.keys(SoundType)}
  ]);

  var result = native_.callSync('SoundManager_getVolume', args);
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  return native_.getResultObject(result);
};

SoundManager.prototype.setSoundModeChangeListener = function(callback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'callback', type: types_.FUNCTION}
  ]);

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }

    var _result = native_.getResultObject(result);

    native_.callIfPossible(args.successCallback);
  };

  native_.call('SoundManager_setSoundModeChangeListener', args, callback);
};

SoundManager.prototype.unsetSoundModeChangeListener = function() {
  var result = native_.callSync('SoundManager_unsetSoundModeChangeListener');
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  return native_.getResultObject(result);
};

SoundManager.prototype.setVolumeChangeListener = function(callback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'callback', type: types_.FUNCTION}
  ]);

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }

    var _result = native_.getResultObject(result);

    native_.callIfPossible(args.successCallback);
  };

  native_.call('SoundManager_setVolumeChangeListener', args, callback);
};

SoundManager.prototype.unsetVolumeChangeListener = function() {
  var result = native_.callSync('SoundManager_unsetVolumeChangeListener');
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  return native_.getResultObject(result);
};


exports = new SoundManager();
