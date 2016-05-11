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

PushManager.prototype.registerService = function() {
  validator.validateArgs(arguments, [
    {
      name: 'appControl',
      type: validator.Types.PLATFORM_OBJECT,
      values: tizen.ApplicationControl
    }
  ]);
  console.warn('Method registerService() is deprecated, use register() instead.');
  this.register.apply(this, Array.prototype.slice.call(arguments, 1));
};

PushManager.prototype.register = function() {
  var data = validator.validateArgs(arguments, [
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

  var ret = native.call('Push_registerApplication', {}, function(msg) {
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

PushManager.prototype.unregisterService = function() {
  console.warn('Method unregisterService() is deprecated, use unregister() instead.');
  this.unregister.apply(this, arguments);
};

PushManager.prototype.unregister = function() {
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
  var result = native.call('Push_unregisterApplication', {}, function(msg) {
    if (msg.error) {
      if (validatorType.isFunction(data.errorCallback)) {
        data.errorCallback(native.getErrorObject(msg));
      }
    } else if (validatorType.isFunction(data.successCallback)) {
      data.successCallback();
    }
  });

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
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

PushManager.prototype.getPushMessage = function() {

  var ret = native.callSync('Push_getPushMessage', {});
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  } else {
    return new PushMessage(native.getResultObject(ret));
  }
};

exports = new PushManager();
