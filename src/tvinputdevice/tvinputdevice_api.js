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
var mandatoryMap = {
  "ArrowLeft" : {
    keyName : "Left",
    keyCode : 37
  },
  "ArrowUp" : {
    keyName : "Up",
    keyCode : 38
  },
  "ArrowRight" : {
    keyName : "Right",
    keyCode : 39
  },
  "ArrowDown" : {
    keyName : "Down",
    keyCode : 40
  },
  "Enter" : {
    keyName : "Return",
    keyCode : 13
  },
  "Back" : {
    keyName : "XF86Back",
    keyCode : 10009
  },
};
var map = {
  "VolumeUp": {
    keyName: "XF86AudioRaiseVolume",
    keyCode: 447
  },
  "VolumeDown": {
    keyName: "XF86AudioLowerVolume",
    keyCode: 448
  },
  "VolumeMute": {
    keyName: "XF86AudioMute",
    keyCode: 449
  },
  "ChannelUp": {
    keyName: "XF86RaiseChannel",
    keyCode: 427
  },
  "ChannelDown": {
    keyName: "XF86LowerChannel",
    keyCode: 428
  },
  "ColorF0Red": {
    keyName: "XF86Red",
    keyCode: 403
  },
  "ColorF1Green": {
    keyName: "XF86Green",
    keyCode: 404
  },
  "ColorF2Yellow": {
    keyName: "XF86Yellow",
    keyCode: 405
  },
  "ColorF3Blue": {
    keyName: "XF86Blue",
    keyCode: 406
  },
  "Menu": {
    keyName: "XF86SysMenu",
    keyCode: 10133
  },
  "Tools": {
    keyName: "XF86SimpleMenu",
    keyCode: 10135
  },
  "Info": {
    keyName: "XF86Info",
    keyCode: 457
  },
  "Exit": {
    keyName: "XF86Exit",
    keyCode: 10182
  },
  "Search": {
    keyName: "XF86Search",
    keyCode: 10225
  },
  "Guide": {
    keyName: "XF86ChannelGuide",
    keyCode: 458
  },
  "MediaRewind": {
    keyName: "XF86AudioRewind",
    keyCode: 412
  },
  "MediaPause": {
    keyName: "XF86AudioPause",
    keyCode: 19
  },
  "MediaFastForward": {
    keyName: "XF86AudioNext",
    keyCode: 417
  },
  "MediaRecord": {
    keyName: "XF86AudioRecord",
    keyCode: 416
  },
  "MediaPlay": {
    keyName: "XF86AudioPlay",
    keyCode: 415
  },
  "MediaStop": {
    keyName: "XF86AudioStop",
    keyCode: 413
  },
  "MediaPlayPause": {
    keyName: "XF86PlayBack",
    keyCode: 10252
  },
  "MediaTrackPrevious": {
    keyName: "XF86PreviousChapter",
    keyCode: 10232
  },
  "MediaTrackNext": {
    keyName: "XF86NextChapter",
    keyCode: 10233
  },
  "Source": {
    keyName: "XF86Display",
    keyCode: 10072
  },
  "PictureSize": {
    keyName: "XF86PictureSize",
    keyCode: 10140
  },
  "PreviousChannel": {
    keyName: "XF86PreviousChannel",
    keyCode: 10190
  },
  "ChannelList": {
    keyName: "XF86ChannelList",
    keyCode: 10073
  },
  "E-Manual": {
    keyName: "XF86EManual",
    keyCode: 10146
  },
  "MTS": {
    keyName: "XF86MTS",
    keyCode: 10195
  },
  "3D": {
    keyName: "XF863D",
    keyCode: 10199
  },
  "Soccer": {
    keyName: "XF86SoccerMode",
    keyCode: 10228
  },
  "Caption": {
    keyName: "XF86Caption",
    keyCode: 10221
  },
  "Teletext": {
    keyName: "XF86TTXMIX",
    keyCode: 10200
  },
  "Extra": {
    keyName: "XF86ExtraApp",
    keyCode: 10253
  },
  "0": {
    keyName: "0",
    keyCode: 48
  },
  "1": {
    keyName: "1",
    keyCode: 49
  },
  "2": {
    keyName: "2",
    keyCode: 50
  },
  "3": {
    keyName: "3",
    keyCode: 51
  },
  "4": {
    keyName: "4",
    keyCode: 52
  },
  "5": {
    keyName: "5",
    keyCode: 53
  },
  "6": {
    keyName: "6",
    keyCode: 54
  },
  "7": {
    keyName: "7",
    keyCode: 55
  },
  "8": {
    keyName: "8",
    keyCode: 56
  },
  "9": {
    keyName: "9",
    keyCode: 57
  },
  "Minus": {
    keyName: "minus",
    keyCode: 189
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
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.TV_INPUT_DEVICE);
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
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.TV_INPUT_DEVICE);
  var args = validator.validateArgs(arguments, [
    {name: 'keyName', type: types.STRING}
  ]);

  if (map[args.keyName]) {
    return new TVInputDeviceKey( { name: args.keyName, code: map[args.keyName].keyCode } );
  } else if (mandatoryMap[args.keyName]) {
    return new TVInputDeviceKey( { name: args.keyName, code: mandatoryMap[args.keyName].keyCode } );
  } else {
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
    'Parameter "keyName" is invalid.');
  }

};


/**
 * Registers an input device key to receive DOM keyboard event when it is pressed or released.
 * @param {!string} keyName  The key name
 */
TVInputDeviceManager.prototype.registerKey = function(keyName) {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.TV_INPUT_DEVICE);
  var args = validator.validateArgs(arguments, [
    {name: 'keyName', type: types.STRING}
  ]);
  if (!map[args.keyName]) {
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
    'Parameter "keyName" is invalid.');
  }

  var ret = native.sendRuntimeSyncMessage('tizen://api/inputdevice/registerKey', map[args.keyName].keyName);
  if (ret === 'error') {
    throw new WebAPIException(WebAPIException.UNKNOWN_ERR, 'UnknownError');
  }
};


/**
 * Unregisters an input device key.
 * @param {!string} keyName  The key name
 */
TVInputDeviceManager.prototype.unregisterKey = function(keyName) {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.TV_INPUT_DEVICE);
  var args = validator.validateArgs(arguments, [
    {name: 'keyName', type: types.STRING}
  ]);

  if (!map[args.keyName]) {
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
    'Parameter "keyName" is invalid.');
  }

  var ret = native.sendRuntimeSyncMessage('tizen://api/inputdevice/unregisterKey', map[args.keyName].keyName);
  if (ret === 'error') {
    throw new WebAPIException(WebAPIException.UNKNOWN_ERR, 'UnknownError');
  }
};

TVInputDeviceManager.prototype.registerKeyBatch = function() {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.TV_INPUT_DEVICE);
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

  TVInputDeviceManager.prototype.unregisterKeyBatch = function() {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.TV_INPUT_DEVICE);
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
exports = new TVInputDeviceManager();
