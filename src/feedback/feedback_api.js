/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the 'License");
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
 
var validator_ = xwalk.utils.validator;
var types_ = validator_.Types;
var type_ = xwalk.utils.type;
var native_ = new xwalk.utils.NativeManager(extension);

var ExceptionMap = {
  'UnknownError' : WebAPIException.UNKNOWN_ERR,
  'TypeMismatchError' : WebAPIException.TYPE_MISMATCH_ERR,
  'InvalidValuesError' : WebAPIException.INVALID_VALUES_ERR,
  'IOError' : WebAPIException.IO_ERR,
  'ServiceNotAvailableError' : WebAPIException.SERVICE_NOT_AVAILABLE_ERR,
  'SecurityError' : WebAPIException.SECURITY_ERR,
  'NetworkError' : WebAPIException.NETWORK_ERR,
  'NotSupportedError' : WebAPIException.NOT_SUPPORTED_ERR,
  'NotFoundError' : WebAPIException.NOT_FOUND_ERR,
  'InvalidAccessError' : WebAPIException.INVALID_ACCESS_ERR,
  'AbortError' : WebAPIException.ABORT_ERR,
  'QuotaExceededError' : WebAPIException.QUOTA_EXCEEDED_ERR
};

function callNative(cmd, args) {
  var json = {'cmd': cmd, 'args': args};
  var argjson = JSON.stringify(json);
  var resultString = extension.internal.sendSyncMessage(argjson);
  var result = JSON.parse(resultString);

  if (typeof result !== 'object') {
    throw new WebAPIException(WebAPIException.UNKNOWN_ERR);
  }

  if (result['status'] == 'success') {
    if (result['result']) {
      return result['result'];
    }
    return true;
  }
  else if (result['status'] == 'error') {
    var err = result['error'];
    if (err) {
      if (ExceptionMap[err.name]) {
        throw new WebAPIException(ExceptionMap[err.name], err.message);
      } else {
        throw new WebAPIException(WebAPIException.UNKNOWN_ERR, err.message);
      }
    }
    return false;
  }
}

function SetReadOnlyProperty(obj, n, v) {
  Object.defineProperty(obj, n, {value: v, writable: false});
}

var FeedbackType = {
    TYPE_SOUND: 'TYPE_SOUND',
    TYPE_VIBRATION: 'TYPE_VIBRATION',
    NONE: 'NONE'
}

var FeedbackPattern = {
  TAP: 'TAP',
  SIP: 'SIP',
  KEY0: 'KEY0',
  KEY1: 'KEY1',
  KEY2: 'KEY2',
  KEY3: 'KEY3',
  KEY4: 'KEY4',
  KEY5: 'KEY5',
  KEY6: 'KEY6',
  KEY7: 'KEY7',
  KEY8: 'KEY8',
  KEY9: 'KEY9',
  KEY_STAR: 'KEY_STAR',
  KEY_SHARP: 'KEY_SHARP',
  KEY_BACK: 'KEY_BACK',
  HOLD: 'HOLD',
  HW_TAP: 'HW_TAP',
  HW_HOLD: 'HW_HOLD',
  MESSAGE: 'MESSAGE',
  EMAIL: 'EMAIL',
  WAKEUP: 'WAKEUP',
  SCHEDULE: 'SCHEDULE',
  TIMER: 'TIMER',
  GENERAL: 'GENERAL',
  POWERON: 'POWERON',
  POWEROFF: 'POWEROFF',
  CHARGERCONN: 'CHARGERCONN',
  CHARGING_ERROR: 'CHARGING_ERROR',
  FULLCHARGED: 'FULLCHARGED',
  LOWBATT: 'LOWBATT',
  LOCK: 'LOCK',
  UNLOCK: 'UNLOCK',
  VIBRATION_ON: 'VIBRATION_ON',
  SILENT_OFF: 'SILENT_OFF',
  BT_CONNECTED: 'BT_CONNECTED',
  BT_DISCONNECTED: 'BT_DISCONNECTED',
  LIST_REORDER: 'LIST_REORDER',
  LIST_SLIDER: 'LIST_SLIDER',
  VOLUME_KEY: 'VOLUME_KEY'
  };

function FeedbackManager() {
  // constructor of FeedbackManager
}


FeedbackManager.prototype.isPatternSupported = function(pattern, type) {
  var args = validator_.validateArgs(arguments, [
    {name: 'pattern', type: types_.ENUM, values: Object.keys(FeedbackPattern)},
    {name: 'type', type: types_.ENUM, values: Object.keys(FeedbackType)},
  ]);

  if ('' === args.pattern) {
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
                              'Pattern name cannot be empty.');
  }

  if ('' === args.type) {
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
                              'Pattern type name cannot be empty.');
  }

    var result = callNative('FeedbackManager_isPatternSupported', args);

    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }
    return native_.getResultObject(result);
};

FeedbackManager.prototype.play = function(pattern, type) {
  var args = validator_.validateArgs(arguments, [
    {name: 'pattern', type: types_.ENUM, values: Object.keys(FeedbackPattern)},
    {name: 'type', type: types_.ENUM, values: Object.keys(FeedbackType), 'optional' : true},
  ]);

  if ('' === args.pattern) {
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
                              'Pattern name cannot be empty.');
  }
  if ('' === args.type) {
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
                              'Pattern type name cannot be empty.');
  }

  var nativeParam = {
      'pattern': args.pattern,
      'type': args.type ? args.type : 'any'
    };

  var result = callNative('FeedbackManager_play', nativeParam);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  return;
};

FeedbackManager.prototype.stop = function() {
  native_.removeListener(TAG_LISTENER);

  var result = callNative('FeedbackManager_stop');
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  return;
};

exports = new FeedbackManager();
