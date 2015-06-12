/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

var native = new xwalk.utils.NativeManager(extension);
var validator = xwalk.utils.validator;
var validatorType = xwalk.utils.type;
var Privilege = xwalk.utils.privilege;

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
  xwalk.utils.checkPrivilegeAccess(Privilege.PUSH);
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
        data.errorCallback(native.getErrorObject(msg));
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
  xwalk.utils.checkPrivilegeAccess(Privilege.PUSH);
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
        data.errorCallback(native.getErrorObject(msg));
      }
    } else if (validatorType.isFunction(data.successCallback)) {
      data.successCallback();
    }
  });
};

PushManager.prototype.connectService = function(notificationCallback) {
  xwalk.utils.checkPrivilegeAccess(Privilege.PUSH);
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
  xwalk.utils.checkPrivilegeAccess(Privilege.PUSH);
  var ret = native.callSync('Push_disconnectService', {});
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
  native.removeListener(NOTIFICATION_LISTENER);
};

PushManager.prototype.getRegistrationId = function() {
  xwalk.utils.checkPrivilegeAccess(Privilege.PUSH);
  var ret = native.callSync('Push_getRegistrationId', {});
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
  return native.getResultObject(ret);
};

PushManager.prototype.getUnreadNotifications = function() {
  xwalk.utils.checkPrivilegeAccess(Privilege.PUSH);
  var ret = native.callSync('Push_getUnreadNotifications', {});
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
};

exports = new PushManager();
