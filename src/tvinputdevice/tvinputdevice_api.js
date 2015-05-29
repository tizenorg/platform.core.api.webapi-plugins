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
var types = validator.Types;


function InputDeviceKey(dict) {
  for (var key in dict) {
    if (dict.hasOwnProperty(key)) {
      Object.defineProperty(this, key, {
        value: dict[key],
        enumerable: true
      });
    }
  }
  Object.freeze(this);
}


function dictListToInputDeviceKeyList(list) {
  var result = [], listLength = list.length;
  for (var i = 0; i < listLength; ++i) {
    result.push(new InputDeviceKey(list[i]));
  }
  return result;
}



/**
 * This class provides access to the API functionalities through the tizen.tvinputdevice interface.
 * @constructor
 */
function TVInputDeviceManager() {
  if (!(this instanceof TVInputDeviceManager)) {
    throw new TypeError;
  }
}


/**
 * Retrieves the list of keys can be registered with the registerKey() method.
 * @return {array} Array of keys
 */
TVInputDeviceManager.prototype.getSupportedKeys = function() {
  var ret = native.callSync('TVInputDeviceManager_getSupportedKeys');
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
  return dictListToInputDeviceKeyList(native.getResultObject(ret));
};


/**
 * Returns information about the key which has the given name.
 * @param {!string} keyName  The key name
 * @return {object} Key object
 */
TVInputDeviceManager.prototype.getKey = function(keyName) {
  var args = validator.validateArgs(arguments, [
    {name: 'keyName', type: types.STRING}
  ]);

  var ret = native.callSync('TVInputDeviceManager_getKey', {
    keyName: args.keyName
  });

  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
  return native.getResultObject(ret);
};


/**
 * Registers an input device key to receive DOM keyboard event when it is pressed or released.
 * @param {!string} keyName  The key name
 */
TVInputDeviceManager.prototype.registerKey = function(keyName) {
  var args = validator.validateArgs(arguments, [
    {name: 'keyName', type: types.STRING}
  ]);

  var ret = native.callSync('TVInputDeviceManager_registerKey', {
    keyName: args.keyName
  });

  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
};


/**
 * Unregisters an input device key.
 * @param {!string} keyName  The key name
 */
TVInputDeviceManager.prototype.unregisterKey = function(keyName) {
  var args = validator.validateArgs(arguments, [
    {name: 'keyName', type: types.STRING}
  ]);

  var ret = native.callSync('TVInputDeviceManager_unregisterKey', {
    keyName: args.keyName
  });

  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
};


// Exports
exports = new TVInputDeviceManager();
