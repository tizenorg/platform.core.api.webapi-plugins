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

var ServiceType = {
  SMART_VIEW_APP_COMMUNICATION: 'SMART_VIEW_APP_COMMUNICATION',
  REMOTE_CONTROL: 'REMOTE_CONTROL'
};

var ConnectionState = {
  CONNECTED: 'CONNECTED',
  NOT_CONNECTED: 'NOT_CONNECTED',
  CONNECTING: 'CONNECTING'
};

function ConvergenceManager() {
  // constructor of ConvergenceManager
}

// Currently available devices
var convergence_devices = [];

ConvergenceManager.prototype.startDiscovery = function(successCallback,
  errorCallback, timeout) {
  console.log('Entered ConvergenceManager.startDiscovery()');
  var args = validator_.validateArgs(arguments, [
    {name: 'successCallback', type: types_.LISTENER, values: ['onfound', 'onfinished' ]},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'timeout', type: types_.LONG, optional: true}
  ]);
  console.log(JSON.stringify(args));

  if(!args.timeout)
    args.timeout = 0; // Default value of timeout

  args.listenerId = 'CONVERGENCE_DISCOVERY_LISTENER';

  native_.addListener(args.listenerId, function(result) {
    console.log('Entered discovery listener');
    console.log(JSON.stringify(result));

    if (native_.isFailure(result)) {
      if (!T_.isNullOrUndefined(errorCallback)) {
        errorCallback(native_.getErrorObject(result));
      }
    } else {
      if(result.discovery_status == 'device_found') {
        // Prepare service array
        if(result && result.device && result.device.services) {
          var services = [];
          for (var i = 0; i < result.device.services.length; ++i) {
            var s = new Service(result.device.services[i].serviceType);
            s.connectionState = result.device.services[i].connectionState;
            s.deviceId = result.device.id;
            services.push(s);
          }
          result.device.services = services;
        }

        // Store newly found device internally
        convergence_devices.push(result.device);

        // Invoke user callback retrieving newly found device
        if (!T_.isNullOrUndefined(successCallback.onfound))
          successCallback.onfound(result.device);
      } else if(result.discovery_status == 'discovery_finished') {
        // Unregister discovery listener, because Convergence Manager is a
        // singleton object and no one else can receive discovery results
        native_.removeListener('CONVERGENCE_DISCOVERY_LISTENER');

        // Notify the customer about discovery results
        if (!T_.isNullOrUndefined(successCallback.onfinished))
          successCallback.onfinished(convergence_devices);

      } else {
        console.log('UNKNOWN discovery state exception');
      }
    }
  });

  // Reset currently available device list
  convergence_devices = [];

  native_.call('ConvergenceManager_startDiscovery', args, function(result) {
    if (native_.isFailure(result)) {
      if (!T_.isNullOrUndefined(errorCallback)) {
        errorCallback(native_.getErrorObject(result));
      }
    }
  });
};

ConvergenceManager.prototype.stopDiscovery = function() {
  console.log('Entered ConvergenceManager.stopDiscovery()');
  var args = validator_.validateArgs(arguments, [
  ]);
  console.log(JSON.stringify(args));

  native_.call('ConvergenceManager_stopDiscovery', args, function(result) {
    if (native_.isFailure(result)) {
      if (!T_.isNullOrUndefined(errorCallback)) {
        errorCallback(native_.getErrorObject(result));
      }
    }
  });
};

function Service(serviceType) {
  console.log('Entered Service.constructor()');

  validator_.isConstructorCall(this, Service);

  SetReadOnlyProperty(this, 'serviceType', serviceType); // read only property
  this.connectionState = null;
  this.properties = [];
}

var ConvergenceServiceConnectListeners = {};
var nextConnectListenerId = 0;

native_.addListener('CONVERGENCE_SERVICE_CONNECT_LISTENER', function(result) {
  if (native_.isFailure(result)) {
    if (!T_.isNullOrUndefined(errorCallback)) {
      errorCallback(native_.getErrorObject(result));
    }
  } else {
    // Invoke corresponding callback
    if (ConvergenceServiceConnectListeners.hasOwnProperty(result.curListenerId)) {
      ConvergenceServiceConnectListeners[result.curListenerId](result.payload);
      delete ConvergenceServiceConnectListeners[result.curListenerId];
    }
  }
});

Service.prototype.connect = function(successCallback, errorCallback) {
  console.log('Entered Service.connect()');
  var args = validator_.validateArgs(arguments, [
    {name: 'successCallback', type: types_.FUNCTION},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);
  console.log(JSON.stringify(args));

  args.service = this;
  args.deviceId = args.service.deviceId;

  if (!T_.isNullOrUndefined(successCallback)) {
    args.curListenerId = ++nextConnectListenerId;
    ConvergenceServiceConnectListeners[args.curListenerId] = successCallback;
  }

  native_.call('Service_connect', args, function(result) {
    if (native_.isFailure(result)) {
      if (!T_.isNullOrUndefined(errorCallback)) {
        errorCallback(native_.getErrorObject(result));
      }
    }
  });
};

Service.prototype.disconnect = function() {
  console.log('Entered Service.disconnect()');
  var args = validator_.validateArgs(arguments, [
  ]);
  console.log(JSON.stringify(args));

  args.service = this;
  args.deviceId = args.service.deviceId;

  var result = native_.callSync('Service_disconnect', args);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

Service.prototype.start = function(channel, payload, successCallback, errorCallback) {
  console.log('Entered Service.start()');
  var args = validator_.validateArgs(arguments, [
    {name: 'channel', type: types_.PLATFORM_OBJECT, values: tizen.Channel,
      optional: true, nullable: true},
    {name: 'payload', type: types_.PLATFORM_OBJECT, values: [
      tizen.PayloadString, tizen.PayloadRawBytes, tizen.PayloadAppControl],
      optional: true, nullable: true},
    {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);
  console.log(JSON.stringify(args));

  args.service = this;
  args.deviceId = args.service.deviceId;

  native_.call('Service_start', args, function(result) {
    if (native_.isFailure(result)) {
      if (!T_.isNullOrUndefined(errorCallback)) {
        errorCallback(native_.getErrorObject(result));
      }
    } else
      successCallback();
  });
};

Service.prototype.read = function(channel, payload, successCallback, errorCallback) {
  console.log('Entered Service.read()');
  var args = validator_.validateArgs(arguments, [
    {name: 'channel', type: types_.PLATFORM_OBJECT, values: tizen.Channel,
      optional: true, nullable: true},
    {name: 'payload', type: types_.PLATFORM_OBJECT, values: [
      tizen.PayloadString, tizen.PayloadRawBytes, tizen.PayloadAppControl],
      optional: true, nullable: true},
    {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);
  console.log(JSON.stringify(args));

  args.service = this;
  args.deviceId = args.service.deviceId;

  native_.call('Service_read', args, function(result) {
    if (native_.isFailure(result)) {
      if (!T_.isNullOrUndefined(errorCallback)) {
        errorCallback(native_.getErrorObject(result));
      }
    } else
      successCallback();
  });
};

Service.prototype.send = function(channel, payload, successCallback, errorCallback) {
  console.log('Entered Service.send()');
  var args = validator_.validateArgs(arguments, [
    {name: 'channel', type: types_.PLATFORM_OBJECT, values: tizen.Channel,
      optional: true, nullable: true},
    {name: 'payload', type: types_.PLATFORM_OBJECT, values:
      [tizen.PayloadString, tizen.PayloadRawBytes, tizen.PayloadAppControl],
      optional: true, nullable: true},
    {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);
  console.log(JSON.stringify(args));

  args.service = this;
  args.deviceId = args.service.deviceId;

  native_.call('Service_send', args, function(result) {
    if (native_.isFailure(result)) {
      if (!T_.isNullOrUndefined(errorCallback)) {
        errorCallback(native_.getErrorObject(result));
      }
    } else
      successCallback();
  });

};

Service.prototype.stop = function(channel, payload, successCallback, errorCallback) {
  console.log('Entered Service.stop()');
  var args = validator_.validateArgs(arguments, [
    {name: 'channel', type: types_.PLATFORM_OBJECT, values: tizen.Channel,
      optional: true, nullable: true},
    {name: 'payload', type: types_.PLATFORM_OBJECT, values:
      [tizen.PayloadString, tizen.PayloadRawBytes, tizen.PayloadAppControl],
      optional: true, nullable: true},
    {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);
  console.log(JSON.stringify(args));

  args.service = this;
  args.deviceId = args.service.deviceId;

  native_.call('Service_stop', args, function(result) {
    if (native_.isFailure(result)) {
      if (!T_.isNullOrUndefined(errorCallback)) {
        errorCallback(native_.getErrorObject(result));
      }
    } else
      successCallback();
  });
};

// TODO: make arrays of listeners for each device.service
var convergenceServiceCommandListeners = {};
var nextCommandListenerId = 0;

native_.addListener('CONVERGENCE_SERVICE_COMMAND_LISTENER', function(result) {
  console.log('On service command listener');
  console.log(JSON.stringify(result));

  if (native_.isFailure(result)) {
    if (!T_.isNullOrUndefined(errorCallback)) {
      errorCallback(native_.getErrorObject(result));
    }
  } else {
    // Invoke corresponding callback
    if (convergenceServiceCommandListeners.hasOwnProperty(result.curListenerId)) {
      convergenceServiceCommandListeners[result.curListenerId](
        result.channel, result.payload);
    }
  }
});

Service.prototype.addListener = function(listenerCallback) {
  console.log('Entered Service.addListener()');
  var args = validator_.validateArgs(arguments, [
    {name: 'listenerCallback', type: types_.FUNCTION}
  ]);
  console.log(JSON.stringify(args));

  args.service = this;
  args.deviceId = args.service.deviceId;

  args.curListenerId = ++nextCommandListenerId;
  convergenceServiceCommandListeners[args.curListenerId] = listenerCallback;

  native_.callSync('Service_addListener', args);

  return args.curListenerId;
};

Service.prototype.removeListener = function(id) {
  console.log('Entered Service.removeListener()');
  var args = validator_.validateArgs(arguments, [
    {name: 'id', type: types_.LONG, optional: false}
  ]);
  console.log(JSON.stringify(args));

  args.service = this;
  args.deviceId = args.service.deviceId;

  native_.callSync('Service_removeListener', args);

  if (convergenceServiceCommandListeners.hasOwnProperty(result.curListenerId)) {
    delete convergenceServiceCommandListeners[result.curListenerId];
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
}

function PayloadRawBytes(key, value) {
  validator_.isConstructorCall(this, PayloadRawBytes);
  this.key = key;
  this.value = value;
}

function PayloadAppControl(key, value) {
  validator_.isConstructorCall(this, PayloadAppControl);
  this.key = key;
  this.value = value;
}


exports = new ConvergenceManager();
tizen.Service = Service;
tizen.Channel = Channel;
tizen.PayloadString = PayloadString;
tizen.PayloadRawBytes = PayloadRawBytes;
tizen.PayloadAppControl = PayloadAppControl;
