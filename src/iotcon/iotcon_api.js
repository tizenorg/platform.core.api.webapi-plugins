/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd All Rights Reserved
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
var T = xwalk.utils.type;

var ResponseResult = {
  SUCCESS: 'SUCCESS',
  ERROR: 'ERROR',
  RESOURCE_CREATED: 'RESOURCE_CREATED',
  RESOURCE_DELETED: 'RESOURCE_DELETED',
  SLOW: 'SLOW'
};

var PresenceResponseResultType = {
  SUCCESS: 'SUCCESS',
  STOPPED: 'STOPPED',
  TIMEOUT: 'TIMEOUT'
};

var PresenceTriggerType = {
  CREATED: 'CREATED',
  UPDATED: 'UPDATED',
  DESTROYED: 'DESTROYED'
};

var ConnectivityType = {
  IPV4: 'IPV4',
  IPV6: 'IPV6',
  BT_EDR: 'BT_EDR',
  BT_LE: 'BT_LE',
  ALL: 'ALL'
};

var ResourceInterface = {
  DEFAULT: 'DEFAULT',
  LINK: 'LINK',
  BATCH: 'BATCH',
  GROUP: 'GROUP'
};

var ObservePolicy = {
  IGNORE_OUT_OF_ORDER: 'IGNORE_OUT_OF_ORDER',
  ACCEPT_OUT_OF_ORDER: 'ACCEPT_OUT_OF_ORDER'
};

var PresenceTriggerType = {
  NO_TYPE: 'NO_TYPE',
  REGISTER: 'REGISTER',
  DEREGISTER: 'DEREGISTER'
};

function Client() {
}

Client.prototype.findResource = function() {
  var args = validator.validateMethod(arguments, [{
    name: 'hostAddress',
    type: types.STRING,
    nullable: true
  }, {
    name: 'resourceType',
    type: types.STRING,
    nullable: true
  }, {
    name: 'connectivityType',
    type: types.ENUM,
    values: T.getValues(ConnectivityType)
  }, {
    name: 'successCallback',
    type: types.FUNCTION
  }, {
    name: 'errorCallback',
    type: types.FUNCTION,
    optional: true,
    nullable: true
  }]);

  var callArgs = {};
  callArgs.hostAddress = args.hostAddress;
  callArgs.resourceType = args.resourceType;
  callArgs.connectivityType = args.connectivityType;

  var callback = function(result) {
    if (native.isFailure(result)) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(result));
    } else {
      // TODO: implement
      args.successCallback();
    }
  };

  var result = native.call('IotconClient_findResource', callArgs, callback);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
};

var presenceEventListener = (function() {
  var listenerName = 'PresenceEventListener';
  var listeners = {};
  var jsListenerRegistered = false;

  function callback(response) {
    if (listeners[response.id]) {
      // TODO: implement
      listeners[response.id]();
    }
  }

  function addListener(id, func) {
    if (!jsListenerRegistered) {
      native.addListener(listenerName, callback);
      jsListenerRegistered = true;
    }

    listeners[id] = func;
  }

  function removeListener(id) {
    if (listeners[id]) {
      delete listeners[id];
    }

    if (jsListenerRegistered && T.isEmptyObject(listeners)) {
      native.removeListener(listenerName, callback);
      jsListenerRegistered = false;
    }
  }

  return {
    addListener: addListener,
    removeListener: removeListener
  };
})();

Client.prototype.addPresenceEventListener = function() {
  var args = validator.validateMethod(arguments, [{
    name: 'hostAddress',
    type: types.STRING,
    nullable: true
  }, {
    name: 'resourceType',
    type: types.STRING
  }, {
    name: 'connectivityType',
    type: types.ENUM,
    values: T.getValues(ConnectivityType)
  }, {
    name: 'successCallback',
    type: types.FUNCTION
  }]);

  var callArgs = {};
  callArgs.hostAddress = args.hostAddress;
  callArgs.resourceType = args.resourceType;
  callArgs.connectivityType = args.connectivityType;

  var result = native.callSync('IotconClient_addPresenceEventListener', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    var id = native.getResultObject(result);
    presenceEventListener.addListener(id, args.successCallback);
    return id;
  }
};

Client.prototype.removePresenceEventListener = function() {
  var args = validator.validateMethod(arguments, [{
    name: 'watchId',
    type: types.LONG
  }]);

  var callArgs = {};
  callArgs.id = args.watchId;

  var result = native.callSync('IotconClient_removePresenceEventListener', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    presenceEventListener.removeListener(args.watchId);
  }
};

Client.prototype.getDeviceInfo = function() {
  var args = validator.validateMethod(arguments, [{
    name: 'hostAddress',
    type: types.STRING
  }, {
    name: 'connectivityType',
    type: types.ENUM,
    values: T.getValues(ConnectivityType)
  }, {
    name: 'successCallback',
    type: types.FUNCTION
  }, {
    name: 'errorCallback',
    type: types.FUNCTION,
    optional: true,
    nullable: true
  }]);

  var callArgs = {};
  callArgs.hostAddress = args.hostAddress;
  callArgs.connectivityType = args.connectivityType;

  var callback = function(result) {
    if (native.isFailure(result)) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(result));
    } else {
      // TODO: implement
      args.successCallback();
    }
  };

  var result = native.call('IotconClient_getDeviceInfo', callArgs, callback);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
};

Client.prototype.getPlatformInfo = function() {
  var args = validator.validateMethod(arguments, [{
    name: 'hostAddress',
    type: types.STRING
  }, {
    name: 'connectivityType',
    type: types.ENUM,
    values: T.getValues(ConnectivityType)
  }, {
    name: 'successCallback',
    type: types.FUNCTION
  }, {
    name: 'errorCallback',
    type: types.FUNCTION,
    optional: true,
    nullable: true
  }]);

  var callArgs = {};
  callArgs.hostAddress = args.hostAddress;
  callArgs.connectivityType = args.connectivityType;

  var callback = function(result) {
    if (native.isFailure(result)) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(result));
    } else {
      // TODO: implement
      args.successCallback();
    }
  };

  var result = native.call('IotconClient_getPlatformInfo', callArgs, callback);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
};

function Resource() {
}

function Server() {
}

Server.prototype.createResource = function() {
  var args = validator.validateMethod(arguments, [{
    name: 'dictionary',
    type: types.DICTIONARY
  }, {
    name: 'successCallback',
    type: types.FUNCTION
  }, {
    name: 'errorCallback',
    type: types.FUNCTION,
    optional: true,
    nullable: true
  }]);

  var callArgs = args.dictionary;

  var callback = function(result) {
    if (native.isFailure(result)) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(result));
    } else {
      // TODO: implement
      args.successCallback();
    }
  };

  var result = native.call('IotconServer_createResource', callArgs, callback);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
};

Server.prototype.removeResource = function() {
  var args = validator.validateMethod(arguments, [{
    name: 'resource',
    type: types.PLATFORM_OBJECT,
    values: Resource
  }, {
    name: 'successCallback',
    type: types.FUNCTION
  }, {
    name: 'errorCallback',
    type: types.FUNCTION,
    optional: true,
    nullable: true
  }]);

  var callArgs = {};
  callArgs.id = args.resource._id;  // TODO: check if this is correct

  var callback = function(result) {
    if (native.isFailure(result)) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(result));
    } else {
      args.successCallback();
    }
  };

  var result = native.call('IotconServer_removeResource', callArgs, callback);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
};

Server.prototype.updateResource = function() {
  var args = validator.validateMethod(arguments, [{
    name: 'resource',
    type: types.PLATFORM_OBJECT,
    values: Resource
  }, {
    name: 'successCallback',
    type: types.FUNCTION
  }, {
    name: 'errorCallback',
    type: types.FUNCTION,
    optional: true,
    nullable: true
  }]);

  var callArgs = {};
  callArgs.id = args.resource._id;  // TODO: check if this is correct

  var callback = function(result) {
    if (native.isFailure(result)) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(result));
    } else {
      args.successCallback(args.resource);
    }
  };

  var result = native.call('IotconServer_updateResource', callArgs, callback);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
};

var client = new Client();
var server = new Server();

function Iotcon() {
}

Iotcon.prototype.getClient = function() {
  return client;
};

Iotcon.prototype.getServer = function() {
  return server;
};

Iotcon.prototype.getTimeout = function() {
  var result = native.callSync('Iotcon_getTimeout', {});

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    return native.getResultObject(result);
  }
};

Iotcon.prototype.setTimeout = function() {
  var args = validator.validateMethod(arguments, [{
    name: 'timeout',
    type: types.LONG
  }]);

  var callArgs = {};
  callArgs.timeout = args.timeout;

  var result = native.callSync('Iotcon_setTimeout', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
};

// Exports
exports = new Iotcon();
