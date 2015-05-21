//Copyright (c) 2013 Intel Corporation. All rights reserved.
//Use of this source code is governed by a BSD-style license that can be
//found in the LICENSE file.

extension.setMessageListener(function(msg) {

});

//TVWindowManager interface
function TVWindowManager() {
  if (!(this instanceof TVWindowManager)) {
    throw new TypeError;
  }
}


TVWindowManager.prototype.getAvailableWindows = function(successCallback, errorCallback)
    {
  return undefined;
};

TVWindowManager.prototype.setSource = function(videosource, successCallback, errorCallback, type)
    {
  return undefined;
};

TVWindowManager.prototype.getSource = function(type)
    {
  return undefined;
};

TVWindowManager.prototype.show = function(successCallback, errorCallback, rectangle, type)
    {
  return undefined;
};

TVWindowManager.prototype.hide = function(successCallback, errorCallback, type)
    {
  return undefined;
};

TVWindowManager.prototype.getRect = function(successCallback, errorCallback, unit, type)
    {
  return undefined;
};

exports = new TVWindowManager();
