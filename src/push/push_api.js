/* global xwalk, extension, tizen */

// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


var native = new xwalk.utils.NativeManager(extension);
var validator = xwalk.utils.validator;
var validatorType = xwalk.utils.type;


/**
 * @const
 * @type {string}
 */
var NOTIFICATION_LISTENER = 'Push_Notification_Listener';

function PushMessage(dict) {
  for (var key in dict) {
    if (dict.hasOwnProperty(key)) {
      Object.defineProperty(this, key, {
        value: key === 'date' ? new Date(dict[key] * 1000) : dict[key],
        enumerable: true
      });
    }
  }
  Object.freeze(this);
}



/**
 * @constructor
 */
function PushManager() {
  if (!(this instanceof PushManager)) {
    throw new TypeError;
  }
}

PushManager.prototype.registerService = function(appControl, successCallback, errorCallback) {
  var data = validator.validateArgs(arguments, [
    {
      name: 'appControl',
      type: validator.Types.PLATFORM_OBJECT,
      values: tizen.ApplicationControl
    },
    {
      name: 'successCallback',
      type: validator.Types.FUNCTION
    },
    {
      name: 'errorCallback',
      type: validator.Types.FUNCTION,
      optional: true,
      nullable: true
    }
  ]);
  var ret = native.call('Push_registerService', {
    operation: data.appControl.operation,
    uri: data.appControl.uri,
    mime: data.appControl.mime,
    category: data.appControl.category,
    data: data.appControl.data
  }, function(msg) {
    if (msg.error) {
      if (validatorType.isFunction(data.errorCallback)) {
        data.errorCallback(native.getErrorObject(msg.error));
      }
    } else {
      data.successCallback(msg.registrationId);
    }
  });
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
};

PushManager.prototype.unregisterService = function(successCallback, errorCallback) {
  var data = validator.validateArgs(arguments, [
    {
      name: 'successCallback',
      type: validator.Types.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'errorCallback',
      type: validator.Types.FUNCTION,
      optional: true,
      nullable: true
    }
  ]);
  native.call('Push_unregisterService', {}, function(msg) {
    if (msg.error) {
      if (validatorType.isFunction(data.errorCallback)) {
        data.errorCallback(native.getErrorObject(msg.error));
      }
    } else if (validatorType.isFunction(data.successCallback)) {
      data.successCallback();
    }
  });
};

PushManager.prototype.connectService = function(notificationCallback) {
  var data = validator.validateArgs(arguments, [
    {
      name: 'notificationCallback',
      type: validator.Types.FUNCTION
    }
  ]);
  var ret = native.callSync('Push_connectService', {});
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
  native.addListener(NOTIFICATION_LISTENER, function(msg) {
    data.notificationCallback(new PushMessage(msg.pushMessage));
  });
};

PushManager.prototype.disconnectService = function() {
  var ret = native.callSync('Push_disconnectService', {});
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
  native.removeListener(NOTIFICATION_LISTENER);
};

PushManager.prototype.getRegistrationId = function() {
  var ret = native.callSync('Push_getRegistrationId', {});
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
  return native.getResultObject(ret);
};

PushManager.prototype.getUnreadNotifications = function() {
  var ret = native.callSync('Push_getUnreadNotifications', {});
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
};

exports = new PushManager();
