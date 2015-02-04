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

var _soundModeChangeListener;

function _soundModeChangeListenerCallback(result) {
  native_.callIfPossible(_soundModeChangeListener, native_.getResultObject(result));
}

SoundManager.prototype.setSoundModeChangeListener = function(callback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'callback', type: types_.FUNCTION}
  ]);

  _soundModeChangeListener = args.callback;
  native_.addListener('SoundModeChangeListener', _soundModeChangeListenerCallback);

  var result = native_.callSync('SoundManager_setSoundModeChangeListener', {});

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

SoundManager.prototype.unsetSoundModeChangeListener = function() {
  native_.removeListener('SoundModeChangeListener', _soundModeChangeListenerCallback);

  var result = native_.callSync('SoundManager_unsetSoundModeChangeListener', {});

  _soundModeChangeListener = undefined;

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

var _volumeChangeListener;

function _volumeChangeListenerCallback(result) {
  native_.callIfPossible(_volumeChangeListener, native_.getResultObject(result));
}

SoundManager.prototype.setVolumeChangeListener = function(callback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'callback', type: types_.FUNCTION}
  ]);

  _volumeChangeListener = args.callback;
  native_.addListener('VolumeChangeListener', _volumeChangeListenerCallback);

  var result = native_.callSync('SoundManager_setVolumeChangeListener', {});

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

SoundManager.prototype.unsetVolumeChangeListener = function() {
  native_.removeListener('VolumeChangeListener', _volumeChangeListenerCallback);

  var result = native_.callSync('SoundManager_unsetVolumeChangeListener', {});

  _volumeChangeListener = undefined;

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};


exports = new SoundManager();
