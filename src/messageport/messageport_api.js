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
 
var validator_ = xwalk.utils.validator;
var types_ = validator_.Types;


var callbackId = 0;
var callbacks = {};
var ports = [];

extension.setMessageListener(function(json) {
  var msg = JSON.parse(json);
  var listeners = callbacks[msg['local_port_id']];
  var rmp;

  console.log('Listeners length:' + listeners.length);

  if (!msg.hasOwnProperty('remotePort'))
    rmp = null;
  else
    rmp = new RemoteMessagePort(msg.remotePort, msg.remoteAppId, msg.trusted);
  for (var i = 0; i < listeners.length; i++) {
    var func = listeners[i][0];
    setTimeout(function() {
      func(msg.message, rmp);
    }, 0);
  }

});

function nextCallbackId() {
  return callbackId++;
}

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


function callNativeWithCallback(cmd, args, callback) {
  if (callback) {
    var id = nextCallbackId();
    args['callbackId'] = id;
    callbacks[id] = callback;
  }

  return callNative(cmd, args);
}

function SetReadOnlyProperty(obj, n, v) {
  Object.defineProperty(obj, n, {value: v, writable: false});
}


function MessagePortManager() {
  // constructor of MessagePortManager
}


MessagePortManager.prototype.requestLocalMessagePort = function(localMessagePortName) {
  var args = validator_.validateArgs(arguments, [
    {'name' : 'localMessagePortName', 'type': types_.STRING}
  ]);

  if ('' === args.localMessagePortName) {
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
                              'Port name cannot be empty.');
  }

  var localPortId;
  var nativeParam = {
    'localMessagePortName': args.localMessagePortName
  };

  try {

    localPortId = callNative('MessagePortManager_requestLocalMessagePort', nativeParam);

  } catch (e) {
    throw e;
  }

  var returnObject = new LocalMessagePort(args.localMessagePortName, false);
  ports[nativeParam.localMessagePortName] = localPortId;

  return returnObject;
};

MessagePortManager.prototype.requestTrustedLocalMessagePort = function(localMessagePortName) {
  var args = validator_.validateArgs(arguments, [
    {'name' : 'localMessagePortName', 'type': types_.STRING}
  ]);

  if ('' === args.localMessagePortName) {
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
                              'Port name cannot be empty.');
  }

  var nativeParam = {
    'localMessagePortName': args.localMessagePortName
  };

  try {

    var localPortId = callNative('MessagePortManager_requestTrustedLocalMessagePort', nativeParam);

  } catch (e) {
    throw e;
  }

  var returnObject = new LocalMessagePort(args.localMessagePortName, true);
  ports[nativeParam.localMessagePortName] = localPortId;

  return returnObject;
};

MessagePortManager.prototype.requestRemoteMessagePort =
    function(appId, remoteMessagePortName) {
  var args = validator_.validateArgs(arguments, [
    {'name' : 'appId', 'type': types_.STRING},
    {'name' : 'remoteMessagePortName', 'type': types_.STRING}
  ]);

  var nativeParam = {
    'appId': args.appId,
    'remoteMessagePortName': args.remoteMessagePortName
  };

  try {

    var syncResult = callNative('MessagePortManager_requestRemoteMessagePort', nativeParam);

  } catch (e) {
    throw e;
  }

  var returnObject = new RemoteMessagePort(args.remoteMessagePortName, args.appId, false);

  return returnObject;
};

MessagePortManager.prototype.requestTrustedRemoteMessagePort =
    function(appId, remoteMessagePortName) {
  var args = validator_.validateArgs(arguments, [
    {'name' : 'appId', 'type': types_.STRING},
    {'name' : 'remoteMessagePortName', 'type': types_.STRING}
  ]);

  var nativeParam = {
    'appId' : args.appId,
    'remoteMessagePortName': args.remoteMessagePortName
  };

  try {

    var syncResult = callNative('MessagePortManager_requestTrustedRemoteMessagePort', nativeParam);

  } catch (e) {
    throw e;
  }

  var returnObject = new RemoteMessagePort(args.remoteMessagePortName, args.appId, true);

  return returnObject;
};


function LocalMessagePort(messagePortName, isTrusted) {
  Object.defineProperties(this, {
    'messagePortName': { value: messagePortName, writable: false, enumerable: true },
    'isTrusted': { value: !!isTrusted, writable: false, enumerable: true }
  });
}


LocalMessagePort.prototype.addMessagePortListener = function(listener) {
  var args = validator_.validateArgs(arguments, [
    {'name' : 'listener', 'type': types_.FUNCTION, 'nullable': false}
  ]);

  var portId = ports[this.messagePortName];

  if (!callbacks.hasOwnProperty(portId)) callbacks[portId] = [];

  callbackId++;
  callbacks[portId].push([listener, callbackId]);

  return callbackId;

};

LocalMessagePort.prototype.removeMessagePortListener = function(watchId) {
  var args = validator_.validateArgs(arguments, [
    {'name' : 'watchId', 'type': types_.LONG, 'nullable': false, 'optional': false }
  ]);

  var to_delete;
  var listeners = callbacks[ports[this.messagePortName]];

  for (var i = 0, j = listeners.length; i < j; i++) {
    var listener_id = listeners[i][1];
    if (watchId == listener_id) {
      to_delete = i;
      break;
    }
  }

  if (typeof to_delete === 'undefined')
    throw new WebAPIException(WebAPIException.NOT_FOUND_ERR,
        'The port of the target application is not found.');

  listeners.splice(to_delete, 1);

};


function RemoteMessagePort(messagePortName, appId, isTrusted) {
  Object.defineProperties(this, {
    'messagePortName': { value: messagePortName, writable: false },
    'appId': { value: appId, writable: false },
    'isTrusted': { value: !!isTrusted, writable: false }
  });
}

RemoteMessagePort.prototype.sendMessage = function() {
  var args = validator_.validateArgs(arguments, [
    {'name' : 'data', 'type': types_.ARRAY},
    {'name' : 'localMessagePort', 'type': types_.PLATFORM_OBJECT, 'optional' : true,
      'nullable' : true, 'values' : LocalMessagePort }
  ]);

  var filtered_data = new Array(args.data.length);
  var unique_data_key = {};

  for (var i = 0, j = args.data.length; i < j; i++) {
    if (!args.data[i].hasOwnProperty('key')) {
      throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
          'MessagePortDataItem should contain \'key\' property.');
    }
    var key = args.data[i].key;
    if ('' === key) {
      throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
          'Property \'key\' should not be empty.');
    }
    if (true === unique_data_key[key]) {
      throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
          'Property \'key\' should not be duplicated.');
    }
    filtered_data[i] = { key: key, value: args.data[i].value };
    unique_data_key[key] = true;
  }

  var nativeParam = {
    'appId': this.appId,
    'messagePortName': this.messagePortName,
    'data': filtered_data,
    'trusted': this.isTrusted,
    'local_port_id': args.localMessagePort ? ports[args.localMessagePort.messagePortName] : -1
  };

  var syncResult = callNative('RemoteMessagePort_sendMessage', nativeParam);
};

exports = new MessagePortManager();
