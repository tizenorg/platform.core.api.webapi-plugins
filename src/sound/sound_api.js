// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


var utils_ = xwalk.utils;
var type_ = utils_.type;
var converter_ = utils_.converter;
var validator_ = utils_.validator;
var privilege_ = utils_.privilege;
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

function _createSoundDeviceInfoArray(e) {
  var devices_array = [];

  e.forEach(function (data) {
    devices_array.push(new SoundDeviceInfo(data));
  });

  return devices_array;
};

function ListenerManager(native, listenerName) {
  this.listeners = {};
  this.nextId = 1;
  this.nativeSet = false;
  this.native = native;
  this.listenerName = listenerName;
};

ListenerManager.prototype.onListenerCalled = function(msg) {
  var obj = new SoundDeviceInfo(msg);
  for (var watchId in this.listeners) {
    if (this.listeners.hasOwnProperty(watchId)) {
      this.listeners[watchId](obj);
    }
  }
};

ListenerManager.prototype.addListener = function(callback) {
  var id = this.nextId;
  if (!this.nativeSet) {
    this.native.addListener(this.listenerName, this.onListenerCalled.bind(this));
    this.native.callSync('SoundManager_addDeviceStateChangeListener');
    this.nativeSet = true;
  }

  this.listeners[id] = callback;
  ++this.nextId;

  return id;
};

ListenerManager.prototype.removeListener = function(watchId) {
  if (this.listeners.hasOwnProperty(watchId)) {
    delete this.listeners[watchId];
  }

  if (this.nativeSet && type_.isEmptyObject(this.listeners)) {
      this.native.callSync('SoundManager_removeDeviceStateChangeListener');
      this.native.removeListener(this.listenerName);
      this.nativeSet = false;
  }
};

var DEVICE_STATE_CHANGE_LISTENER = 'SoundDeviceStateChangeCallback';
var soundDeviceStateChangeListener = new ListenerManager(native_, DEVICE_STATE_CHANGE_LISTENER);

function SoundManager() {}

SoundManager.prototype.getSoundMode = function() {
  var result = native_.callSync('SoundManager_getSoundMode');
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  return native_.getResultObject(result);
};

SoundManager.prototype.setVolume = function(type, volume) {
  utils_.checkPrivilegeAccess(privilege_.VOLUME_SET);

  var args = validator_.validateArgs(arguments, [
    {name: 'type', type: types_.ENUM, values: Object.keys(SoundType)},
    {name: 'volume', type: types_.DOUBLE}
  ]);

  if (args.volume < 0 || args.volume > 1) {
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR);
  }
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
  native_.callIfPossible(_volumeChangeListener, result.type, result.volume);
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

SoundManager.prototype.getConnectedDeviceList = function() {
  var result = native_.callSync('SoundManager_getConnectedDeviceList', {});
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  var devices = _createSoundDeviceInfoArray(native_.getResultObject(result));
  return devices;
};

SoundManager.prototype.getActivatedDeviceList = function() {
  var result = native_.callSync('SoundManager_getActivatedDeviceList', {});
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  var devices = _createSoundDeviceInfoArray(native_.getResultObject(result));
  return devices;
};

SoundManager.prototype.addDeviceStateChangeListener = function() {
  var args = validator_.validateArgs(arguments, [
    {
       name : 'eventCallback',
       type : types_.FUNCTION
     }
  ]);

  return soundDeviceStateChangeListener.addListener(args.eventCallback);
};

SoundManager.prototype.removeDeviceStateChangeListener = function() {
  var args = validator_.validateArgs(arguments, [
    {
       name : 'watchId',
       type : types_.LONG
    }
  ]);

  soundDeviceStateChangeListener.removeListener(args.watchId);
};

function SoundDeviceInfo(data) {
  Object.defineProperties(this, {
    id: {value: data.id, writable: false, enumerable: true},
    name: {value: data.name, writable: false, enumerable: true},
    device : {value: data.device, writable: false, enumerable: true},
    direction : {value: data.direction, writable: false, enumerable: true},
    isConnected: {value: data.isConnected, writable: false, enumerable: true},
    isActivated: {value: data.isActivated, writable: false, enumerable: true},
  });
};

exports = new SoundManager();
