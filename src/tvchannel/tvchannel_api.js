//Copyright (c) 2013 Intel Corporation. All rights reserved.
//Use of this source code is governed by a BSD-style license that can be
//found in the LICENSE file.

extension.setMessageListener(function(msg) {

});

// TVChannelManager interface
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
  return undefined;
};

TVChannelManager.prototype.getProgramList = function(channelInfo,
    startTime, successCallback, errorCallback, duration) {
  return undefined;
};

TVChannelManager.prototype.getCurrentProgram = function(windowType) {
  return undefined;
};

TVChannelManager.prototype.addChannelChangeListener = function(successCallback, windowType) {
  return undefined;
};

TVChannelManager.prototype.removeChannelChangeListener = function(listenerId) {
  return undefined;
};

TVChannelManager.prototype.addProgramChangeListener = function(successCallback, windowType) {
  return undefined;
};

TVChannelManager.prototype.removeProgramChangeListener = function(listenerId) {
  return undefined;
};

TVChannelManager.prototype.getNumOfAvailableTuner = function() {
  return undefined;
};

exports = new TVChannelManager();
