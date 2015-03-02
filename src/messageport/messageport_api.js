// MessagePort

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
    func(msg.message, rmp);
  }

});

function nextCallbackId() {
  return callbackId++;
}

var ExceptionMap = {
  'UnknownError' : tizen.WebAPIException.UNKNOWN_ERR,
  'TypeMismatchError' : tizen.WebAPIException.TYPE_MISMATCH_ERR,
  'InvalidValuesError' : tizen.WebAPIException.INVALID_VALUES_ERR,
  'IOError' : tizen.WebAPIException.IO_ERR,
  'ServiceNotAvailableError' : tizen.WebAPIException.SERVICE_NOT_AVAILABLE_ERR,
  'SecurityError' : tizen.WebAPIException.SECURITY_ERR,
  'NetworkError' : tizen.WebAPIException.NETWORK_ERR,
  'NotSupportedError' : tizen.WebAPIException.NOT_SUPPORTED_ERR,
  'NotFoundError' : tizen.WebAPIException.NOT_FOUND_ERR,
  'InvalidAccessError' : tizen.WebAPIException.INVALID_ACCESS_ERR,
  'AbortError' : tizen.WebAPIException.ABORT_ERR,
  'QuotaExceededError' : tizen.WebAPIException.QUOTA_EXCEEDED_ERR
};

function callNative(cmd, args) {
  var json = {'cmd': cmd, 'args': args};
  var argjson = JSON.stringify(json);
  var resultString = extension.internal.sendSyncMessage(argjson);
  var result = JSON.parse(resultString);

  if (typeof result !== 'object') {
    throw new tizen.WebAPIException(tizen.WebAPIException.UNKNOWN_ERR);
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
        throw new tizen.WebAPIException(ExceptionMap[err.name], err.message);
      } else {
        throw new tizen.WebAPIException(tizen.WebAPIException.UNKNOWN_ERR, err.message);
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
    throw new tizen.WebAPIException(tizen.WebAPIException.NOT_FOUND_ERR,
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

RemoteMessagePort.prototype.sendMessage = function(data) {
  var args = validator_.validateArgs(arguments, [
    {'name' : 'data', 'type': types_.ARRAY},
    {'name' : 'localMessagePort', 'type': types_.PLATFORM_OBJECT, 'optional' : true,
      'nullable' : true, 'values' : LocalMessagePort }
  ]);

  var filtered_data = new Array(data.length);

  for (var i = 0, j = data.length; i < j; i++) {
    if (Object.hasOwnProperty(data[i], 'key'))
      throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR,
          'The input parameter contains an invalid value.');
    if (Object.hasOwnProperty(data[i], 'value'))
      throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR,
          'The input parameter contains an invalid value.');
    filtered_data[i] = { key: data[i].key, value: data[i].value };
  }


  var nativeParam = {
    'appId': this.appId,
    'messagePortName': this.messagePortName,
    'data': filtered_data,
    'trusted': this.isTrusted,
    'local_port_id': args.localMessagePort ? ports[args.localMessagePort.messagePortName] : -1
  };

  try {
    var syncResult = callNative('RemoteMessagePort_sendMessage', nativeParam);

  } catch (e) {
    throw e;
  }

};



exports = new MessagePortManager();

