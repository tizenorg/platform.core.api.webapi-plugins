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


/**
 * This class provides access to the API functionalities through the tizen.tvinputdevice interface.
 * @constructor
 */
function InputDeviceManager() {
  if (!(this instanceof InputDeviceManager)) {
    throw new TypeError;
  }
}

/**
 * Retrieves the list of keys can be registered with the registerKey() method.
 * @return {array} Array of keys
 */
InputDeviceManager.prototype.getSupportedKeys = function() {

  var re = [];
  for (var key in map) {
      if (map.hasOwnProperty(key)) {
          re.push(new InputDeviceKey({name: key, code: map[key].keyCode}));
      }
  }
  
  return re;
};


/**
 * Returns information about the key which has the given name.
 * @param {!string} keyName  The key name
 * @return {object} Key object
 */
InputDeviceManager.prototype.getKey = function(keyName) {
  var args = validator.validateArgs(arguments, [
    {name: 'keyName', type: types.STRING}
  ]);

  if (!map[args.keyName]) {
      throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
      'Parameter "keyName" is invalid.');
  }
  
  return new InputDeviceKey( { name: args.keyName, code: map[args.keyName].keyCode } );

};


/**
 * Registers an input device key to receive DOM keyboard event when it is pressed or released.
 * @param {!string} keyName  The key name
 */
InputDeviceManager.prototype.registerKey = function(keyName) {
  var args = validator.validateArgs(arguments, [
    {name: 'keyName', type: types.STRING}
  ]);
  if (!map[args.keyName]) {
      throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
      'Parameter "keyName" is invalid.');
  }

  var ret = native.sendRuntimeSyncMessage('tizen://api/inputdevice/registerKey',map[args.keyName].keyName);

  if (ret === 'error') {
    throw new WebAPIException(WebAPIException.UNKNOWN_ERR, 'UnknownError');
  }
};


/**
 * Unregisters an input device key.
 * @param {!string} keyName  The key name
 */
InputDeviceManager.prototype.unregisterKey = function(keyName) {
  var args = validator.validateArgs(arguments, [
    {name: 'keyName', type: types.STRING}
  ]);
  
  if (!map[args.keyName]) {
      throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
      'Parameter "keyName" is invalid.');
  }
  
  var ret = native.sendRuntimeSyncMessage('tizen://api/inputdevice/unregisterKey',map[args.keyName].keyName);

  if (ret === 'error') {
    throw new WebAPIException(WebAPIException.UNKNOWN_ERR, 'UnknownError');
  }
};

InputDeviceManager.prototype.registerKeyBatch = function() {
  var args = validator.validateMethod(arguments, [
    {
      name: 'keyNames',
      type: types.ARRAY,
      values: types.STRING
    },
    {
      name: 'successCallback',
      type: types.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'errorCallback',
      type: types.FUNCTION,
      optional: true,
      nullable: true
    }
  ]);

  var keysList = "";
  for (var i = 0; i < args.keyNames.length; ++i) {
    if (!map[args.keyNames[i]]) {
      throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
                                'Invalid key name: "' + args.keyNames[i] + '"');
    }
    keysList += map[args.keyNames[i]].keyName + ((i < args.keyNames.length - 1) ? "," : "");
  }

  setTimeout(function() {
    var ret = native.sendRuntimeSyncMessage('tizen://api/inputdevice/registerKeyBatch', keysList);
    if (ret === 'error') {
      native.callIfPossible(args.errorCallback, new WebAPIException(
          WebAPIException.UNKNOWN_ERR, 'Failed to register keys.'));
    } else {
      native.callIfPossible(args.successCallback);
    }
  }.bind(this), 0);
};

InputDeviceManager.prototype.unregisterKeyBatch = function() {
  var args = validator.validateMethod(arguments, [
    {
      name: 'keyNames',
      type: types.ARRAY,
      values: types.STRING
    },
    {
      name: 'successCallback',
      type: types.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'errorCallback',
      type: types.FUNCTION,
      optional: true,
      nullable: true
    }
  ]);

  var keysList = "";
  for (var i = 0; i < args.keyNames.length; ++i) {
    if (!map[args.keyNames[i]]) {
      throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
                                'Invalid key name: "' + args.keyNames[i] + '"');
    }
    keysList += map[args.keyNames[i]].keyName + ((i < args.keyNames.length - 1) ? "," : "");
  }

  setTimeout(function() {
    var ret = native.sendRuntimeSyncMessage('tizen://api/inputdevice/unregisterKeyBatch', keysList);
    if (ret === 'error') {
      native.callIfPossible(args.errorCallback, new WebAPIException(
          WebAPIException.UNKNOWN_ERR, 'Failed to unregister keys.'));
    } else {
      native.callIfPossible(args.successCallback);
    }
  }.bind(this), 0);
};

// Exports
exports = new InputDeviceManager();
