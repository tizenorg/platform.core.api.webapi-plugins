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
var map = {
  "VolumeUp": {
      keyName: "XF86AudioRaiseVolume",
      keyCode: 447
  },
  "VolumeDown": {
      keyName: "XF86AudioLowerVolume",
      keyCode: 448
  },
};


function TVInputDeviceKey(dict) {
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

  var re = [];
  for (var key in map) {
      if (map.hasOwnProperty(key)) {
          re.push(new TVInputDeviceKey({name: key, code: map[key].keyCode}));
      }
  }
  
  return re;
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

  if (!map[args.keyName]) {
      throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
      'Parameter "keyName" is invalid.');
  }
  
  return new TVInputDeviceKey( { name: args.keyName, code: map[args.keyName].keyCode } );

};


/**
 * Registers an input device key to receive DOM keyboard event when it is pressed or released.
 * @param {!string} keyName  The key name
 */
TVInputDeviceManager.prototype.registerKey = function(keyName) {
  var args = validator.validateArgs(arguments, [
    {name: 'keyName', type: types.STRING}
  ]);
  if (!map[args.keyName]) {
      throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
      'Parameter "keyName" is invalid.');
  }

  var ret = native.sendRuntimeSyncMessage('tizen://api/inputdevice/registerKey',map[args.keyName].keyName);

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
  
  if (!map[args.keyName]) {
      throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
      'Parameter "keyName" is invalid.');
  }
  
  var ret = native.sendRuntimeSyncMessage('tizen://api/inputdevice/unregisterKey',map[args.keyName].keyName);  

  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
};


// Exports
exports = new TVInputDeviceManager();
