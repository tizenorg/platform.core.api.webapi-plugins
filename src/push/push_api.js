/* global xwalk, extension, tizen */

// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


var native = new xwalk.utils.NativeManager(extension);
var validator = xwalk.utils.validator;



/**
 * @constructor
 */
function PushManager() {
  if (!(this instanceof PushManager)) {
    throw new TypeError;
  }
}

PushManager.prototype.registerService = function(appControl, successCallback, errorCallback) {

};

PushManager.prototype.unregisterService = function(successCallback, errorCallback) {

};

PushManager.prototype.connectService = function(notificationCallback) {

};

PushManager.prototype.disconnectService = function() {

};

PushManager.prototype.getRegistrationId = function() {

};

PushManager.prototype.getUnreadNotifications = function() {

};

exports = new PushManager();
