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
var kIdKey = Symbol();

function createListener(name, c) {
  var listenerName = name;
  var callback = c || function(response) {
    return response;
  };
  var listeners = {};
  var jsListenerRegistered = false;

  function internalCallback(response) {
    if (native.isSuccess(response)) {
      response = native.getResultObject(response);
      if (listeners[response.id]) {
        listeners[response.id](callback(response));
      }
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
  SLOW: 'SLOW',
  FORBIDDEN: 'FORBIDDEN',
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

var QosLevel = {
  LOW: 'LOW',
  HIGH: 'HIGH'
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
  this[kIdKey] = data.id;

  delete data.id;

  if (data.representation) {
    data.representation = createRepresentation(data.representation);
  } else {
    data.representation = null;
  }

  if (data.options) {
    var options = [];
    for (var i = 0; i < data.options.length; ++i) {
      options.push(new IotconOption(data.options[i].id, data.options[i].data));
    }
    data.options = options;
  }

  decorateWithData(data, this);
}

function Resource(data) {
  Object.defineProperties(this, {
    observerIds: {
      get: function() {
        var callArgs = {};
        callArgs.id = this[kIdKey];
        var result = native.callSync('IotconResource_getObserverIds', callArgs);
        return native.getResultObject(result);
      }.bind(this),
      set: function() {},
      enumerable: true
    }
  });

  this[kIdKey] = data.id;

  delete data.id;

  var internal = new InternalData(data);
  internal.decorate(this);

  this.states = null;
}

Resource.prototype.notify = function() {
  var args = validator.validateMethod(arguments, [{
    name: 'qos',
    type: types.ENUM,
    values: T.getValues(QosLevel)
  }, {
    name: 'observerIds',
    type: types.ARRAY,
    values: types.LONG,
    optional: true,
    nullable: true
  }]);

  var states = {};
  function getStates(r) {
    states[r[kIdKey]] = r.states;
    for (var i = 0; i < r.resources.length; ++i) {
      getStates(r.resources[i]);
    }
  }
  getStates(this);

  var callArgs = {};
  callArgs.id = this[kIdKey];
  callArgs.qos = args.qos;
  callArgs.observerIds = args.observerIds;
  callArgs.states = states;

  var result = native.callSync('IotconResource_notify', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
};

Resource.prototype.addResourceTypes = function() {
  var args = validator.validateMethod(arguments, [{
    name: 'types',
    type: types.ARRAY,
    values: types.STRING
  }]);

  var callArgs = {};
  callArgs.id = this[kIdKey];
  callArgs.types = args.types;

  var result = native.callSync('IotconResource_addResourceTypes', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    var t = this.resourceTypes;
    t = t.concat(args.types);
    updateWithInternalData({ resourceTypes: t }, this);
  }
};

Resource.prototype.addResourceInterface = function() {
  var args = validator.validateMethod(arguments, [{
    name: 'iface',
    type: types.ENUM,
    values: T.getValues(ResourceInterface)
  }]);

  var callArgs = {};
  callArgs.id = this[kIdKey];
  callArgs.iface = args.iface;

  var result = native.callSync('IotconResource_addResourceInterface', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    var interfaces = this.resourceInterfaces;
    interfaces.push(args.iface);
    updateWithInternalData({ resourceInterfaces: interfaces }, this);
  }
};

Resource.prototype.addChildResource = function() {
  var args = validator.validateMethod(arguments, [{
    name: 'resource',
    type: types.PLATFORM_OBJECT,
    values: Resource
  }]);

  var callArgs = {};
  callArgs.id = this[kIdKey];
  callArgs.childId = args.resource[kIdKey];

  var result = native.callSync('IotconResource_addChildResource', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    var children = this.resources;
    children.push(args.resource);
    updateWithInternalData({ resources: children }, this);
  }
};

Resource.prototype.removeChildResource = function() {
  var args = validator.validateMethod(arguments, [{
    name: 'resource',
    type: types.PLATFORM_OBJECT,
    values: Resource
  }]);

  var callArgs = {};
  callArgs.id = this[kIdKey];
  callArgs.childId = args.resource[kIdKey];

  var result = native.callSync('IotconResource_removeChildResource', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    var children = this.resources;
    var position = children.indexOf(args.resource);
    if (-1 !== position) {
      children.splice(position, 1);
      updateWithInternalData({ resources: children }, this);
    }
  }
};

var resourceRequestListener = createListener('ResourceRequestListener', function(response) {
  return new Request(response.data);
});

Resource.prototype.setRequestListener = function() {
  var args = validator.validateMethod(arguments, [{
    name: 'successCallback',
    type: types.FUNCTION
  }]);

  var callArgs = {};
  callArgs.id = this[kIdKey];

  var listener = function(result) {
    args.successCallback(result);
  };

  var result = native.callSync('IotconResource_setRequestListener', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    resourceRequestListener.addListener(this[kIdKey], listener);
  }
};

Resource.prototype.unsetRequestListener = function() {
  var callArgs = {};
  callArgs.id = this[kIdKey];

  var result = native.callSync('IotconResource_unsetRequestListener', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    resourceRequestListener.removeListener(this[kIdKey]);
  }
};


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
  var args = validator.validateMethod(arguments, [{
    name: 'iface',
    type: types.ENUM,
    values: T.getValues(ResourceInterface)
  }]);

  var callArgs = {};
  callArgs.id = this.request[kIdKey];
  callArgs.result = this.result;
  callArgs.representation = this.representation;
  callArgs.options = this.options;
  callArgs.iface = args.iface;

  var result = native.callSync('IotconResponse_send', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
};

function RemoteResponse(data) {
  if (data.representation) {
    data.representation = createRepresentation(data.representation);
  } else {
    data.representation = null;
  }

  if (data.options) {
    var options = [];
    for (var i = 0; i < data.options.length; ++i) {
      options.push(new IotconOption(data.options[i].id, data.options[i].data));
    }
    data.options = options;
  }

  decorateWithData(data, this);
}

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

function prepareResourceInfo(that){
  var callArgs = {};
  callArgs.id = that[kIdKey];
  if (!callArgs.id) {
    console.log("RemoteResource is not already stored in C++ layer, adding all members");
    callArgs.hostAddress = that.hostAddress;
    callArgs.connectivityType = that.connectivityType;
    callArgs.uriPath = that.uriPath;
    //properties flags
    callArgs.isObservable = that.isObservable;
    callArgs.isDiscoverable = that.isDiscoverable;
    callArgs.isActive = that.isActive;
    callArgs.isSlow = that.isSlow;
    callArgs.isSecure = that.isSecure;
    callArgs.isExplicitDiscoverable = that.isExplicitDiscoverable;
    callArgs.resourceTypes = that.resourceTypes;
    callArgs.resourceInterfaces = that.resourceInterfaces;
  } else {
    console.log("Already stored in C++, all needed info is id");
  }
  return callArgs;
}

function manageId(that, result) {
  if (result.keepId) {
    that[kIdKey] = result.id;
    console.log("Keep id of resource: " + that[kIdKey]);
  } else {
    console.log("Clear id of resource");
    delete that[kIdKey];
  }
  delete result.keepId;
  delete result.id;
}

function RemoteResource(data) {
  Object.defineProperties(this, {
    cachedRepresentation: {
      get: function() {
        var callArgs = {};
        callArgs.id = this[kIdKey];
        var result = native.callSync('IotconRemoteResource_getCachedRepresentation', callArgs);
        if (native.isSuccess(result)) {
          return createRepresentation(native.getResultObject(result));
        } else {
          return null;
        }
      }.bind(this),
      set: function() {},
      enumerable: true
    }
  });

  this[kIdKey] = data.id;

  delete data.id;

  var internal = new InternalData(data);
  internal.decorate(this);
}

RemoteResource.prototype.methodGet = function() {
  var args = validator.validateMethod(arguments, [{
    name: 'query',
    type: types.DICTIONARY
  }, {
    name: 'responseCallback',
    type: types.FUNCTION
  }, {
    name: 'errorCallback',
    type: types.FUNCTION,
    optional: true,
    nullable: true
  }]);

  var callArgs = prepareResourceInfo(this);
  callArgs.query = args.query;

  var callback = function(result) {
    result = native.getResultObject(result);
    manageId(this, result);
    if (!result.data) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(result));
    } else {
      args.responseCallback(new RemoteResponse(result.data));
    }
  }.bind(this);

  var result = native.call('IotconRemoteResource_methodGet', callArgs, callback);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    manageId(this, native.getResultObject(result));
  }
};

RemoteResource.prototype.methodPut = function() {
  var args = validator.validateMethod(arguments, [{
    name: 'representation',
    type: types.PLATFORM_OBJECT,
    values: Representation
  }, {
    name: 'query',
    type: types.DICTIONARY
  }, {
    name: 'responseCallback',
    type: types.FUNCTION
  }, {
    name: 'errorCallback',
    type: types.FUNCTION,
    optional: true,
    nullable: true
  }]);

  var callArgs = prepareResourceInfo(this);
  callArgs.representation = args.representation;
  callArgs.query = args.query;

  var callback = function(result) {
    result = native.getResultObject(result);
    manageId(this, result);
    if (!result.data) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(result));
    } else {
      args.responseCallback(new RemoteResponse(result.data));
    }
  }.bind(this);

  var result = native.call('IotconRemoteResource_methodPut', callArgs, callback);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    manageId(this, native.getResultObject(result));
  }
};

RemoteResource.prototype.methodPost = function() {
  var args = validator.validateMethod(arguments, [{
    name: 'representation',
    type: types.PLATFORM_OBJECT,
    values: Representation
  }, {
    name: 'query',
    type: types.DICTIONARY
  }, {
    name: 'responseCallback',
    type: types.FUNCTION
  }, {
    name: 'errorCallback',
    type: types.FUNCTION,
    optional: true,
    nullable: true
  }]);

  var callArgs = prepareResourceInfo(this);
  callArgs.representation = args.representation;
  callArgs.query = args.query;

  var callback = function(result) {
    result = native.getResultObject(result);
    manageId(this, result);
    if (!result.data) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(result));
    } else {
      args.responseCallback(new RemoteResponse(result.data));
    }
  }.bind(this);

  var result = native.call('IotconRemoteResource_methodPost', callArgs, callback);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    manageId(this, native.getResultObject(result));
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

  var callArgs = prepareResourceInfo(this);

  var callback = function(result) {
    result = native.getResultObject(result);
    manageId(this, result);
    if (!result.data) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(result));
    } else {
      args.responseCallback(new RemoteResponse(result.data));
    }
  }.bind(this);

  var result = native.call('IotconRemoteResource_methodDelete', callArgs, callback);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    manageId(this, native.getResultObject(result));
  }
};

var stateChangeListener = createListener('RemoteResourceStateChangeListener');

RemoteResource.prototype.setStateChangeListener = function() {
  var args = validator.validateMethod(arguments, [{
    name: 'query',
    type: types.DICTIONARY
  }, {
    name: 'observePolicy',
    type: types.ENUM,
    values: T.getValues(ObservePolicy)
  }, {
    name: 'successCallback',
    type: types.FUNCTION
  }]);

  var callArgs = prepareResourceInfo(this);
  callArgs.query = args.query;
  callArgs.observePolicy = args.observePolicy;
  var that = this;

  var listener = function(result) {
    //TODO check what should be updated
    //updateWithInternalData(result, that);
    args.successCallback(new RemoteResponse(result.data));
  };

  var result = native.callSync('IotconRemoteResource_setStateChangeListener', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    manageId(this, native.getResultObject(result));
    stateChangeListener.addListener(this[kIdKey], listener);
  }
};

RemoteResource.prototype.unsetStateChangeListener = function() {
  var callArgs = prepareResourceInfo(this);

  var result = native.callSync('IotconRemoteResource_unsetStateChangeListener', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    manageId(this, native.getResultObject(result));
    stateChangeListener.removeListener(this[kIdKey]);
  }
};

RemoteResource.prototype.startCaching = function() {
  var callArgs = prepareResourceInfo(this);

  var result = native.callSync('IotconRemoteResource_startCaching', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    manageId(this, native.getResultObject(result));
  }
};

RemoteResource.prototype.stopCaching = function() {
  var callArgs = prepareResourceInfo(this);

  var result = native.callSync('IotconRemoteResource_stopCaching', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    manageId(this, native.getResultObject(result));
  }
};

var connectionChangeListener = createListener('RemoteResourceConnectionChangeListener');

RemoteResource.prototype.setConnectionChangeListener = function() {
  var args = validator.validateMethod(arguments, [{
    name: 'successCallback',
    type: types.FUNCTION
  }]);

  var callArgs = prepareResourceInfo(this);

  var listener = function(result) {
    args.successCallback(result.data);
  };

  var result = native.callSync('IotconRemoteResource_setConnectionChangeListener', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    manageId(this, native.getResultObject(result));
    connectionChangeListener.addListener(this[kIdKey], listener);
  }
};

RemoteResource.prototype.unsetConnectionChangeListener = function() {
  var callArgs = prepareResourceInfo(this);

  var result = native.callSync('IotconRemoteResource_unsetConnectionChangeListener', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    manageId(this, native.getResultObject(result));
    connectionChangeListener.removeListener(this[kIdKey]);
  }
};

function Client() {
}

var findResourceListener = createListener('FindResourceListener');
var globalFindResourceId = 0;

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
  }, {
    name: 'isSecure',
    type: types.BOOLEAN
  }]);

  var callArgs = {};
  callArgs.id = ++globalFindResourceId;
  callArgs.hostAddress = args.hostAddress;
  callArgs.resourceType = args.resourceType;
  callArgs.connectivityType = args.connectivityType;
  callArgs.isSecure = args.isSecure;

  var callback = function(result) {
    if (native.isFailure(result)) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(result));
    } else {
      var rr = new RemoteResource(native.getResultObject(result));
      args.successCallback(rr);
    }
  };

  var result = native.callSync('IotconClient_findResource', callArgs);
  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    findResourceListener.addListener(callArgs.id, callback);
  }
};

var presenceEventListener = createListener('PresenceEventListener', function(response) {
  return new PresenceResponse(response.data);
});

Client.prototype.addPresenceEventListener = function() {
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

function Server() {
}

var serverResources = {};

Server.prototype.createResource = function() {
  var args = validator.validateMethod(arguments, [{
    name: 'uriPath',
    type: types.STRING
  }, {
    name: 'resourceTypes',
    type: types.ARRAY,
    values: types.STRING
  }, {
    name: 'dictionary',
    type: types.DICTIONARY,
    optional: true,
    nullable: true
  }]);

  var callArgs = args.dictionary || {};
  callArgs.uriPath = args.uriPath;
  callArgs.resourceTypes = args.resourceTypes;

  var result = native.callSync('IotconServer_createResource', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    var resource = new Resource(native.getResultObject(result));
    serverResources[resource[kIdKey]] = resource;
    return resource;
  }
};

Server.prototype.removeResource = function() {
  var args = validator.validateMethod(arguments, [{
    name: 'resource',
    type: types.PLATFORM_OBJECT,
    values: Resource
  }]);

  var callArgs = {};
  callArgs.id = args.resource[kIdKey];

  var result = native.callSync('IotconServer_removeResource', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    delete serverResources[callArgs.id];
  }
};

Server.prototype.getResources = function() {
  var result = [];

  for (var id in serverResources) {
    if (serverResources.hasOwnProperty(id)) {
      result.push(serverResources[id]);
    }
  }

  return result;
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
tizen.Representation = Representation;
tizen.Response = Response;
tizen.State = State;
exports = new Iotcon();
