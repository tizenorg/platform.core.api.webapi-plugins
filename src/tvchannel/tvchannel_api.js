//Copyright (c) 2013 Intel Corporation. All rights reserved.
//Use of this source code is governed by a BSD-style license that can be
//found in the LICENSE file.

var native = new xwalk.utils.NativeManager(extension);
var validator = xwalk.utils.validator;
var validatorType = xwalk.utils.type;


/**
 * An enumerator that defines window types.
 * @enum {string}
 */
var WindowType = {
  MAIN: 'MAIN'
};

function ListenerManager(native, listenerName) {
  this.listeners = {};
  this.nextId = 1;
  native.addListener(listenerName, this.onListenerCalled.bind(this));
}

ListenerManager.prototype.onListenerCalled = function(msg) {
  for (var key in this.listeners) {
    if (this.listeners.hasOwnProperty(key)) {
      this.listeners[key](msg.channel);
    }
  }
};

ListenerManager.prototype.addListener = function(callback) {
  var id = this.nextId;
  this.listeners[id] = callback;
  ++this.nextId;
  return id;
};

ListenerManager.prototype.removeListener = function(watchId) {
  if (this.listeners.hasOwnProperty(watchId)) {
    delete this.listeners[watchId];
  }
};


/**
 * @const
 * @type {string}
 */
var CHANNEL_CHANGE_LISTENER = 'ChannelChanged';

var channelListener = new ListenerManager(native, CHANNEL_CHANGE_LISTENER);

//TVChannelManager interface
function TVChannelManager() {
  if (!(this instanceof TVChannelManager)) {
    throw new TypeError;
  }
}


TVChannelManager.prototype.tune = function(tuneOption, successCallback, errorCallback, windowType) {
  return undefined;
};

TVChannelManager.prototype.tuneUp = function(successCallback, errorCallback, tuneMode, windowType) {
  return undefined;
};

TVChannelManager.prototype.tuneDown = function(successCallback,
    errorCallback, tuneMode, windowType) {
  return undefined;
};

TVChannelManager.prototype.findChannel = function(major, minor, successCallback, errorCallback) {
  return undefined;
};

TVChannelManager.prototype.getChannelList = function(successCallback,
    errorCallback, tuneMode, start, number) {
  return undefined;
};

TVChannelManager.prototype.getCurrentChannel = function(windowType) {
  var args = validator.validateArgs(arguments, [
    {
      name: 'windowType',
      optional: true,
      nullable: true,
      type: validator.Types.ENUM,
      values: validatorType.getValues(WindowType)
    }
  ]);
  var ret = native.callSync('TVChannelManager_getCurrentChannel', {
    windowType: args.windowType ? args.windowType : WindowType.MAIN
  });
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
  return native.getResultObject(ret);
};

TVChannelManager.prototype.getProgramList = function(channelInfo,
    startTime, successCallback, errorCallback, duration) {
  return undefined;
};

TVChannelManager.prototype.getCurrentProgram = function(windowType) {
  var args = validator.validateArgs(arguments, [
    {
      name: 'windowType',
      optional: true,
      nullable: true,
      type: validator.Types.ENUM,
      values: validatorType.getValues(WindowType)
    }
  ]);
  var ret = native.callSync('TVChannelManager_getCurrentProgram', {
    windowType: args.windowType ? args.windowType : WindowType.MAIN
  });
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
  return native.getResultObject(ret);
};

TVChannelManager.prototype.addChannelChangeListener = function(callback, windowType) {
  var args = validator.validateArgs(arguments, [
    {
      name: 'callback',
      type: validator.Types.FUNCTION
    },
    {
      name: 'windowType',
      optional: true,
      nullable: true,
      type: validator.Types.ENUM,
      values: validatorType.getValues(WindowType)
    }
  ]);

  return channelListener.addListener(args.callback);
};

TVChannelManager.prototype.removeChannelChangeListener = function(listenerId) {
  channelListener.removeListener(listenerId);
};

TVChannelManager.prototype.addProgramChangeListener = function(successCallback, windowType) {
  return undefined;
};

TVChannelManager.prototype.removeProgramChangeListener = function(listenerId) {
  return undefined;
};

TVChannelManager.prototype.getNumOfAvailableTuner = function()
    {
  var ret = native.callSync('TVChannelManager_getNumOfAvailableTuner');
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
  return native.getResultObject(ret);
};

exports = new TVChannelManager();
