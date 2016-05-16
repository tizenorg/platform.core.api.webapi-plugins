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
var utils_ = xwalk.utils;
var native_ = new utils_.NativeManager(extension);
var T_ = utils_.type;

// Flag showing if the discovery procedure has started
var discoveryStarted = false;

// Currently available devices
var convergenceDevices = [];

var ConvergenceServiceConnectListeners = {};
var nextConnectListenerId = 0;
var ConvergenceServicesWaitingConnection = {};

var localServices = [];

// TODO: make arrays of listeners for each device.service
var convergenceServiceCommandListeners = {};
var nextCommandListenerId = 0;

var ServiceType = {
  APP_TO_APP_COMMUNICATION: 'APP_TO_APP_COMMUNICATION',
  REMOTE_APP_CONTROL: 'REMOTE_APP_CONTROL'
};

var ConnectionState = {
  CONNECTED: 'CONNECTED',
  NOT_CONNECTED: 'NOT_CONNECTED',
  CONNECTING: 'CONNECTING'
};

var PayloadType = {
  STRING: 'STRING',
  RAW_BYTES: 'RAW_BYTES',
  APP_CONTROL: 'APP_CONTROL',
};



function SetReadOnlyProperty(obj, n, v) {
  if (arguments.length > 2)
    Object.defineProperty(
      obj, n, {
        value: v,
        writable: false,
        enumerable: true,
        configurable: true
      });
  else
    Object.defineProperty(obj, n, {
      writable: false,
      enumerable: true,
      configurable: true
    });
}

function getServiceTypeName(typeNumber) {
  switch(typeNumber) {
  case 0:
    return ServiceType.APP_TO_APP_COMMUNICATION;
  case 1:
    return ServiceType.REMOTE_APP_CONTROL;
  default:
    console.log('ERROR: Unknown service type'); // TODO throw exception
    return 'Unknown service type';
  }
}

function getServiceTypeNumber(s) {
  if(!s) {
    console.log('ERROR: Unknown service type name');
    return -1; // TODO throw exception
  }
  switch(s.serviceType) {
  case ServiceType.APP_TO_APP_COMMUNICATION:
    return 0;
  case ServiceType.REMOTE_APP_CONTROL:
    return 1;
  default:
    console.log('ERROR: Unknown service type name');
    return -1; // TODO throw exception
  }
}

function getServiceConnectionStateName(connectionStateNumber) {
  switch(connectionStateNumber) {
  case 0:
    return ConnectionState.CONNECTED;
  case 1:
    return ConnectionState.NOT_CONNECTED;
  case 2:
    return ConnectionState.CONNECTING;
  default:
    log.console('ERROR: Unknown connection state');
    return -1; // TODO throw exception
  }
}

function getDeviceId(s) {
  console.log('Entered getDeviceId()');
  for(var i = 0; i < convergenceDevices.length; i ++) {
    var curDevice = convergenceDevices[i];
    for(var j = 0; j < curDevice.services.length; j ++) {
      var curSvc = curDevice.services[j];
      if(s === curSvc)
        return curDevice.id;
    }
  }
  return 'localhost';
}

function isUnhandledLocalService(s) {
  console.log('Entered isUnhandledLocalService()');
  if(getDeviceId() != 'localhost')
    return false; // This is a remote service

  for(var i = 0; i < localServices.length; i ++)
    if(s === localServices[i])
      return false; // This service is already handled (registered in D2D Manager)

  return true;
}

function registerLocalService(s) {
  console.log('Entered registerLocalService()');
  localServices.push(s);
  var result = native_.callSync('Service_createLocalService', {
    service: s,
    serviceTypeNumber: getServiceTypeNumber(s)
  });
  if(native_.isFailure(result))
    throw native_.getErrorObject(result);
}

function Device(id, name, type, services) {
  validator_.isConstructorCall(this, Device);
  this.id = id;
  this.name = name;
  this.type = type;
  this.services = services;
}

function ConvergenceManager() {
  // constructor of ConvergenceManager
}

ConvergenceManager.prototype.startDiscovery = function(successCallback,
  errorCallback, timeout) {
  console.log('Entered ConvergenceManager.startDiscovery()');
  var args = validator_.validateArgs(arguments, [
    {name: 'successCallback', type: types_.LISTENER, values: ['onfound', 'onfinished' ]},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'timeout', type: types_.LONG, optional: true, nullable: true}
  ]);

  // Indicate, that discovery procedure is on
  discoveryStarted = true;

  // Reset currently available device list
  convergenceDevices = [];

  native_.addListener('CONVERGENCE_DISCOVERY_LISTENER', function(result) {
    console.log('Entered discovery listener');

    if (native_.isFailure(result)) {
      native_.callIfPossible(errorCallback, native_.getErrorObject(result));
    } else {
      if(result.discovery_status == 'device_found') {

        // Create an instance of the device
        var d = new Device('', '', '', []);

        // Prepare service array
        if(result && result.device && result.device.services) {
          d.id = result.device.id;
          d.name = result.device.name;
          d.type = result.device.type;
          for (var i = 0; i < result.device.services.length; ++i) {
            var s = new Service(getServiceTypeName(result.device.services[i].serviceType));
            s.connectionState = getServiceConnectionStateName(result.device.services[i].connectionState);
            s.properties = result.device.services[i].properties;
            d.services.push(s);
          }
        }

        // Store newly found device internally
        convergenceDevices.push(d);

        // Invoke user callback retrieving newly found device
	native_.callIfPossible(successCallback.onfound, d);

      } else if(result.discovery_status == 'discovery_finished') {
        // Unregister discovery listener, because Convergence Manager is a
        // singleton object and no one else can receive discovery results
        native_.removeListener('CONVERGENCE_DISCOVERY_LISTENER');

        // Notify the customer about discovery results
	native_.callIfPossible(successCallback.onfinished, convergenceDevices);

      } else {
        console.log('UNKNOWN discovery state exception'); // TODO throw exception
      }
    }
  });

  // Start the discovery using Native API
  var result = native_.call('ConvergenceManager_startDiscovery', {
      timeout: (args.timeout) ? args.timeout : 0
    }, function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(errorCallback, native_.getErrorObject(result));
    }
  });
  if(native_.isFailure(result))
    throw native_.getErrorObject(result);
};

ConvergenceManager.prototype.stopDiscovery = function() {
  console.log('Entered ConvergenceManager.stopDiscovery()');

  if(!discoveryStarted)
    throw new WebAPIException('InvalidStateError', 'Discovery has not started yet.');

  discoveryStarted = false;

  var result = native_.callSync('ConvergenceManager_stopDiscovery', null);
  if(native_.isFailure(result))
    throw native_.getErrorObject(result);
};

function Service(serviceType) {
  console.log('Entered Service.constructor()');

  validator_.isConstructorCall(this, Service);

  SetReadOnlyProperty(this, 'serviceType', serviceType); // read only property
  this.connectionState = ConnectionState.NOT_CONNECTED;
  this.properties = [];
}

native_.addListener('CONVERGENCE_SERVICE_CONNECT_LISTENER', function(result) {
  if (native_.isFailure(result)) {
    native_.callIfPossible(errorCallback, native_.getErrorObject(result));
  } else {
    // Invoke corresponding callback
    var lid = result.curListenerId;
    if (!lid || !ConvergenceServiceConnectListeners.hasOwnProperty(lid))
      return; // SOmething is wrong: listener MUST be there

    // Update connection state of the service
    var s = ConvergenceServicesWaitingConnection[lid];
    if(s) // Service MUST NOT be null here
      s.connectionState = ConnectionState.CONNECTED;

    // Invoke user-defined connection callback
    native_.callIfPossible(ConvergenceServiceConnectListeners[lid], result.payload);

    // Release listener (it is a one-time listener and it is not needed anymore)
    delete ConvergenceServiceConnectListeners[lid];
  }
});

Service.prototype.connect = function(successCallback, errorCallback) {
  console.log('Entered Service.connect()');
  var args = validator_.validateArgs(arguments, [
    {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  if(this.serviceState != ConnectionState.NOT_CONNECTED)
    throw new WebAPIException('InvalidStateError', 'Service is connected already.');

  if(isUnhandledLocalService(this))
    registerLocalService(this);

  var lid = ++nextConnectListenerId;
  ConvergenceServiceConnectListeners[lid] = successCallback;
  ConvergenceServicesWaitingConnection[lid] = this;

  var result = native_.call('Service_connect', {
      deviceId: getDeviceId(this),
      serviceTypeNumber: getServiceTypeNumber(this),
      curListenerId: lid
    }, function(result) {
    if (native_.isFailure(result))
      native_.callIfPossible(errorCallback, native_.getErrorObject(result));
  });
  if(native_.isFailure(result))
    throw native_.getErrorObject(result);
};

Service.prototype.disconnect = function() {
  console.log('Entered Service.disconnect()');

  if(this.serviceState != ConnectionState.CONNECTED)
    throw new WebAPIException('InvalidStateError', 'Service is not connected yet.');

  var result = native_.callSync('Service_disconnect', {
      serviceTypeNumber: getServiceTypeNumber(this),
      deviceId: getDeviceId(this)
    });

  if (native_.isFailure(result))
    throw native_.getErrorObject(result);
  else
    connectionState = ConnectionState.DISCONNECTED;
};

Service.prototype.start = function(channel, payload, successCallback, errorCallback) {
  console.log('Entered Service.start()');
  var args = validator_.validateArgs(arguments, [
    {name: 'channel', type: types_.PLATFORM_OBJECT, values: tizen.Channel, optional: true, nullable: true},
    {name: 'payload', type: types_.PLATFORM_OBJECT, values: [tizen.PayloadString, tizen.PayloadRawBytes, tizen.PayloadAppControl], optional: true, nullable: true},
    {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  if(isUnhandledLocalService(this))
    registerLocalService(this);

  var result = native_.call('Service_start', {
      serviceTypeNumber: getServiceTypeNumber(this),
      deviceId: getDeviceId(this),
      channel: args.channel,
      payload: args.payload
    }, function(result) {
    if (native_.isFailure(result))
      native_.callIfPossible(errorCallback, native_.getErrorObject(result));
    else
      native_.callIfPossible(successCallback);
  });
  if(native_.isFailure(result))
    throw native_.getErrorObject(result);
};

Service.prototype.read = function(channel, payload, successCallback, errorCallback) {
  console.log('Entered Service.read()');
  var args = validator_.validateArgs(arguments, [
    {name: 'channel', type: types_.PLATFORM_OBJECT, values: tizen.Channel, optional: true, nullable: true},
    {name: 'payload', type: types_.PLATFORM_OBJECT, values: [tizen.PayloadString, tizen.PayloadRawBytes, tizen.PayloadAppControl], optional: true, nullable: true},
    {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  if(isUnhandledLocalService(this))
    registerLocalService(this);

  var result = native_.call('Service_read', {
      serviceTypeNumber: getServiceTypeNumber(this),
      deviceId: getDeviceId(this),
      channel: args.channel,
      payload: args.payload
    }, function(result) {
    if (native_.isFailure(result))
      native_.callIfPossible(errorCallback, native_.getErrorObject(result));
    else
      native_.callIfPossible(successCallback);
  });
  if(native_.isFailure(result))
    throw native_.getErrorObject(result);
};

Service.prototype.send = function(channel, payload, successCallback, errorCallback) {
  console.log('Entered Service.send()');
  var args = validator_.validateArgs(arguments, [
    {name: 'channel', type: types_.PLATFORM_OBJECT, values: tizen.Channel, optional: true, nullable: true},
    {name: 'payload', type: types_.PLATFORM_OBJECT, values: [tizen.PayloadString, tizen.PayloadRawBytes, tizen.PayloadAppControl], optional: true, nullable: true},
    {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  if(isUnhandledLocalService(this))
    registerLocalService(this);

  var result = native_.call('Service_send', {
      serviceTypeNumber: getServiceTypeNumber(this),
      deviceId: getDeviceId(this),
      channel: args.channel,
      payload: args.payload
    }, function(result) {
    if (native_.isFailure(result))
      native_.callIfPossible(errorCallback, native_.getErrorObject(result));
    else
      native_.callIfPossible(successCallback);
  });
  if(native_.isFailure(result))
    throw native_.getErrorObject(result);
};

Service.prototype.stop = function(channel, payload, successCallback, errorCallback) {
  console.log('Entered Service.stop()');
  var args = validator_.validateArgs(arguments, [
    {name: 'channel', type: types_.PLATFORM_OBJECT, values: tizen.Channel, optional: true, nullable: true},
    {name: 'payload', type: types_.PLATFORM_OBJECT, values: [tizen.PayloadString, tizen.PayloadRawBytes, tizen.PayloadAppControl], optional: true, nullable: true},
    {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  if(isUnhandledLocalService(this))
    registerLocalService(this);

  var result = native_.call('Service_stop', {
      serviceTypeNumber: getServiceTypeNumber(this),
      deviceId: getDeviceId(this),
      channel: args.channel,
      payload: args.payload
    }, function(result) {
    if (native_.isFailure(result))
      native_.callIfPossible(errorCallback, native_.getErrorObject(result));
    else
      native_.callIfPossible(successCallback);
  });
  if(native_.isFailure(result))
    throw native_.getErrorObject(result);
};

native_.addListener('CONVERGENCE_SERVICE_COMMAND_LISTENER', function(result) {
  console.log('On service command listener');

  if (native_.isFailure(result)) {
    native_.callIfPossible(errorCallback, native_.getErrorObject(result));
  } else {
    // Invoke corresponding callback
    if (convergenceServiceCommandListeners.hasOwnProperty(result.curListenerId)) {

      console.log('');
      console.log(JSON.stringify(result.channel));
      console.log('');
      console.log(JSON.stringify(result.payload));
      console.log('');

      var c = new Channel(result.channel.id, result.channel.options);
      //var p = result.payload;
      var p = [];
      for(var i = 0; i < result.payload.length; i ++) {
        var curPl = result.payload[i];
        switch(curPl.type) {
        case PayloadType.STRING:
          p.push(new PayloadString(curPl.key, curPl.value));
          break;
        case PayloadType.RAW_BYTES:
          p.push(new PayloadRawBytes(curPl.key, curPl.value));
          break;
        case PayloadType.APP_CONTROL:
          p.push(new PayloadAppControl(curPl.appId, curPl.key, curPl.value));
          break;
        default:
          console.log('ERROR: Unknown payload type');
          break;
        }
      }
      convergenceServiceCommandListeners[result.curListenerId](c, p);
    }
  }
});

Service.prototype.addListener = function(listenerCallback) {
  console.log('Entered Service.addListener()');
  var args = validator_.validateArgs(arguments, [
    {name: 'listenerCallback', type: types_.FUNCTION}
  ]);

  if(isUnhandledLocalService(this))
    registerLocalService(this);

  var lid = ++nextCommandListenerId;

  convergenceServiceCommandListeners[lid] = listenerCallback;

  var result = native_.callSync('Service_addListener', {
      serviceTypeNumber: getServiceTypeNumber(this),
      deviceId: getDeviceId(this),
      curListenerId: lid
    });
  if(native_.isFailure(result))
    throw native_.getErrorObject(result);

  return args.curListenerId;
};

Service.prototype.removeListener = function(id) {
  console.log('Entered Service.removeListener()');
  var args = validator_.validateArgs(arguments, [
    {name: 'id', type: types_.LONG, optional: false}
  ]);

  var result = native_.callSync('Service_removeListener', {
      serviceTypeNumber: getServiceTypeNumber(this),
      deviceId: getDeviceId(this)
      //curListenerId: id // not needed in below layers
    });
  if(native_.isFailure(result))
    throw native_.getErrorObject(result);

  if (convergenceServiceCommandListeners.hasOwnProperty(id)) {
    delete convergenceServiceCommandListeners[id];
  }
};

function Channel(id, options) {
  validator_.isConstructorCall(this, Channel);
  this.id = id;
  this.options = options;
}

function PayloadString(key, value) {
  validator_.isConstructorCall(this, PayloadString);
  this.key = key;
  this.value = value;
  SetReadOnlyProperty(this, 'type', PayloadType.STRING); // read only property
}

function PayloadRawBytes(key, value) {
  validator_.isConstructorCall(this, PayloadRawBytes);
  this.key = key;
  this.value = value;
  SetReadOnlyProperty(this, 'type', PayloadType.RAW_BYTES); // read only property
}

function PayloadAppControl(appId, key, value) {
  validator_.isConstructorCall(this, PayloadAppControl);
  this.appId = appId;
  this.key = key;
  this.value = value;
  SetReadOnlyProperty(this, 'type', PayloadType.APP_CONTROL); // read only property
}

exports = new ConvergenceManager();
tizen.Service = Service;
tizen.Channel = Channel;
tizen.PayloadString = PayloadString;
tizen.PayloadRawBytes = PayloadRawBytes;
tizen.PayloadAppControl = PayloadAppControl;
