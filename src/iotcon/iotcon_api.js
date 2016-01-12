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

function createListener(name, c) {
  var listenerName = name;
  var callback = c || function(response) {
    return response;
  };
  var listeners = {};
  var jsListenerRegistered = false;

  function internalCallback(response) {
    if (listeners[response.id]) {
      listeners[response.id](callback(response));
    }
  }

  function addListener(id, func) {
    if (!jsListenerRegistered) {
      native.addListener(listenerName, internalCallback);
      jsListenerRegistered = true;
    }

    listeners[id] = func;
  }

  function removeListener(id) {
    if (listeners[id]) {
      delete listeners[id];
    }

    if (jsListenerRegistered && T.isEmptyObject(listeners)) {
      native.removeListener(listenerName, internalCallback);
      jsListenerRegistered = false;
    }
  }

  return {
    addListener: addListener,
    removeListener: removeListener
  };
}

function InternalData(d) {
  for (var prop in d) {
    if (d.hasOwnProperty(prop)) {
      this[prop] = d[prop];
    }
  }
}

InternalData.prototype.update = function(dst) {
  for (var prop in this) {
    if (this.hasOwnProperty(prop) && dst.hasOwnProperty(prop)) {
      dst[prop] = this;
    }
  }
};

InternalData.prototype.decorate = function(dst) {
  var that = this;
  function getBuilder(prop) {
    if (T.isArray(that[prop])) {
      return function() {
        return that[prop].slice();
      };
    } else {
      return function() {
        return that[prop];
      };
    }
  }
  function setBuilder(prop) {
    return function(d) {
      if (d instanceof InternalData) {
        that[prop] = d[prop];
      }
    };
  }
  for (var prop in this) {
    if (this.hasOwnProperty(prop)) {
      Object.defineProperty(dst, prop, {
        get: getBuilder(prop),
        set: setBuilder(prop),
        enumerable: true
      });
    }
  }
};

function updateWithInternalData(src, dst) {
  new InternalData(src).update(dst);
}

function decorateWithData(data, dst) {
  for (var prop in data) {
    if (data.hasOwnProperty(prop)) {
      Object.defineProperty(dst, prop, {
        value: data[prop],
        writable: false,
        enumerable: true
      });
    }
  }
}

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

function DeviceInfo(data) {
  decorateWithData(data, this);
}

function IotconOption(id, data) {
  validator.isConstructorCall(this, tizen.IotconOption);

  Object.defineProperties(this, {
    id: {
      value: id,
      writable: false,
      enumerable: true
    },
    data: {
      value: data,
      writable: false,
      enumerable: true
    }
  });
}

function PlatformInfo(data) {
  decorateWithData(data, this);
}

function PresenceResponse(data) {
  decorateWithData(data, this);
}

function Query(resourceTypes, resourceInterface, filters) {
  validator.isConstructorCall(this, tizen.Query);

  Object.defineProperties(this, {
    resourceTypes: {
      value: resourceTypes || null,
      writable: true,
      enumerable: true
    },
    resourceInterface: {
      value: resourceInterface || null,
      writable: true,
      enumerable: true
    },
    filters: {
      value: filters || null,
      writable: true,
      enumerable: true
    }
  });
}

function QueryFilter(key, value) {
  validator.isConstructorCall(this, tizen.QueryFilter);

  Object.defineProperties(this, {
    key: {
      value: key,
      writable: true,
      enumerable: true
    },
    value: {
      value: value,
      writable: true,
      enumerable: true
    }
  });
}

function Representation(uriPath) {
  validator.isConstructorCall(this, tizen.Representation);

  Object.defineProperties(this, {
    uriPath: {
      value: uriPath,
      writable: true,
      enumerable: true
    },
    resourceTypes: {
      value: [],
      writable: true,
      enumerable: true
    },
    resourceInterfaces: {
      value: [],
      writable: true,
      enumerable: true
    },
    states: {
      value: null,
      writable: true,
      enumerable: true
    },
    representations: {
      value: null,
      writable: true,
      enumerable: true
    }
  });
}

function createRepresentation(data) {
  var r = new tizen.Representation(data.uriPath);
  var props = ['resourceTypes', 'resourceInterfaces', 'states'];

  for (var p = 0; p < props.length; ++p) {
    if (data[props[p]]) {
      r[props[p]] = data[props[p]];
    }
  }

  if (data.representations) {
    r.representations = [];
    for (var i = 0; i < data.representations.length; ++i) {
      r.representations.push(createRepresentation(data.representations[i]));
    }
  }

  return r;
}

function Request(data) {
  Object.defineProperty(this, '_id', {
    value: data.id,
    writable: false,
    enumerable: false
  });

  delete data.id;

  decorateWithData(data, this);
}

function Response(request) {
  validator.isConstructorCall(this, tizen.Response);

  Object.defineProperties(this, {
    request: {
      value: request,
      writable: false,
      enumerable: true
    },
    result: {
      value: null,
      writable: true,
      enumerable: true
    },
    representation: {
      value: null,
      writable: true,
      enumerable: true
    },
    options: {
      value: null,
      writable: true,
      enumerable: true
    }
  });
}

Response.prototype.send = function() {
  var callArgs = {};
  callArgs.id = this.request._id;
  callArgs.result = this.result;
  callArgs.representation = this.representation;
  callArgs.options = this.options;

  var result = native.callSync('IotconResponse_send', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
};

function State(key, state) {
  validator.isConstructorCall(this, tizen.State);

  Object.defineProperties(this, {
    key: {
      value: key,
      writable: false,
      enumerable: true
    },
    state: {
      value: state,
      writable: false,
      enumerable: true
    }
  });
}

function RemoteResource(data) {
  Object.defineProperties(this, {
    _id: {
      value: data.id,
      writable: false,
      enumerable: false
    },
    cachedRepresentation: {
      get: function() {
        var callArgs = {};
        callArgs.id = data.id;
        var result = native.callSync('IotconRemoteResource_getCachedRepresentation', callArgs);
        return createRepresentation(native.getResultObject(result));
      },
      set: function() {},
      enumerable: true
    }
  });

  delete data.id;

  var internal = new InternalData(data);
  internal.decorate(this);
}

RemoteResource.prototype.methodGet = function() {
  var args = validator.validateMethod(arguments, [{
    name: 'query',
    type: types.PLATFORM_OBJECT,
    values: Query
  }, {
    name: 'responseCallback',
    type: types.FUNCTION
  }, {
    name: 'errorCallback',
    type: types.FUNCTION,
    optional: true,
    nullable: true
  }]);

  var callArgs = {};
  callArgs.id = this._id;
  callArgs.query = args.query;

  var callback = function(result) {
    if (native.isFailure(result)) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(result));
    } else {
      // TODO: implement
      args.responseCallback();
    }
  };

  var result = native.call('IotconRemoteResource_methodGet', callArgs, callback);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
};

RemoteResource.prototype.methodPut = function() {
  var args = validator.validateMethod(arguments, [{
    name: 'representation',
    type: types.PLATFORM_OBJECT,
    values: Representation
  }, {
    name: 'query',
    type: types.PLATFORM_OBJECT,
    values: Query
  }, {
    name: 'responseCallback',
    type: types.FUNCTION
  }, {
    name: 'errorCallback',
    type: types.FUNCTION,
    optional: true,
    nullable: true
  }]);

  var callArgs = {};
  callArgs.id = this._id;
  callArgs.representation = args.representation;
  callArgs.query = args.query;

  var callback = function(result) {
    if (native.isFailure(result)) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(result));
    } else {
      // TODO: implement
      args.responseCallback();
    }
  };

  var result = native.call('IotconRemoteResource_methodPut', callArgs, callback);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
};

RemoteResource.prototype.methodPost = function() {
  var args = validator.validateMethod(arguments, [{
    name: 'representation',
    type: types.PLATFORM_OBJECT,
    values: Representation
  }, {
    name: 'query',
    type: types.PLATFORM_OBJECT,
    values: Query
  }, {
    name: 'responseCallback',
    type: types.FUNCTION
  }, {
    name: 'errorCallback',
    type: types.FUNCTION,
    optional: true,
    nullable: true
  }]);

  var callArgs = {};
  callArgs.id = this._id;
  callArgs.representation = args.representation;
  callArgs.query = args.query;

  var callback = function(result) {
    if (native.isFailure(result)) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(result));
    } else {
      // TODO: implement
      args.responseCallback();
    }
  };

  var result = native.call('IotconRemoteResource_methodPost', callArgs, callback);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
};

RemoteResource.prototype.methodDelete = function() {
  var args = validator.validateMethod(arguments, [{
    name: 'responseCallback',
    type: types.FUNCTION
  }, {
    name: 'errorCallback',
    type: types.FUNCTION,
    optional: true,
    nullable: true
  }]);

  var callArgs = {};
  callArgs.id = this._id;

  var callback = function(result) {
    if (native.isFailure(result)) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(result));
    } else {
      // TODO: implement
      args.responseCallback();
    }
  };

  var result = native.call('IotconRemoteResource_methodDelete', callArgs, callback);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
};

var stateChangeListener = createListener('RemoteResourceStateChangeListener');

RemoteResource.prototype.setStateChangeListener = function() {
  var args = validator.validateMethod(arguments, [{
    name: 'query',
    type: types.PLATFORM_OBJECT,
    values: Query
  }, {
    name: 'observePolicy',
    type: types.ENUM,
    values: T.getValues(ObservePolicy)
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
  callArgs.id = this._id;
  callArgs.query = args.query;
  callArgs.observePolicy = args.observePolicy;
  var that = this;

  var listener = function(result) {
    if (native.isFailure(result)) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(result));
    } else {
      updateWithInternalData(native.getResultObject(result), that);
      args.successCallback(that);
    }
  };

  var result = native.callSync('IotconRemoteResource_setStateChangeListener', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    stateChangeListener.addListener(this._id, listener);
  }
};

RemoteResource.prototype.unsetStateChangeListener = function() {
  var callArgs = {};
  callArgs.id = this._id;

  var result = native.callSync('IotconRemoteResource_unsetStateChangeListener', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    stateChangeListener.removeListener(this._id);
  }
};

RemoteResource.prototype.startCaching = function() {
  var callArgs = {};
  callArgs.id = this._id;

  var result = native.callSync('IotconRemoteResource_startCaching', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
};

RemoteResource.prototype.stopCaching = function() {
  var callArgs = {};
  callArgs.id = this._id;

  var result = native.callSync('IotconRemoteResource_stopCaching', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
};

var connectionChangeListener = createListener('RemoteResourceConnectionChangeListener');

RemoteResource.prototype.setConnectionChangeListener = function() {
  var args = validator.validateMethod(arguments, [{
    name: 'successCallback',
    type: types.FUNCTION
  }, {
    name: 'errorCallback',
    type: types.FUNCTION,
    optional: true,
    nullable: true
  }]);

  var callArgs = {};
  callArgs.id = this._id;

  var listener = function(result) {
    if (native.isFailure(result)) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(result));
    } else {
      args.successCallback(native.getResultObject(result));
    }
  };

  var result = native.callSync('IotconRemoteResource_setConnectionChangeListener', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    connectionChangeListener.addListener(this._id, listener);
  }
};

RemoteResource.prototype.unsetConnectionChangeListener = function() {
  var callArgs = {};
  callArgs.id = this._id;

  var result = native.callSync('IotconRemoteResource_unsetConnectionChangeListener', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    connectionChangeListener.removeListener(this._id);
  }
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
      var rr = new RemoteResource(native.getResultObject(result));
      args.successCallback(rr);
    }
  };

  var result = native.call('IotconClient_findResource', callArgs, callback);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
};

var presenceEventListener = createListener('PresenceEventListener', function(response) {
  return new PresenceResponse(native.getResultObject(response));
});

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
      args.successCallback(new DeviceInfo(native.getResultObject(result)));
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
      args.successCallback(new PlatformInfo(native.getResultObject(result)));
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
tizen.IotconOption = IotconOption;
tizen.Query = Query;
tizen.QueryFilter = QueryFilter;
tizen.Representation = Representation;
tizen.Response = Response;
tizen.State = State;
exports = new Iotcon();
