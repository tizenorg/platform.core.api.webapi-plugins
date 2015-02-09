/* global tizen, xwalk, extension */

// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


var validator_ = xwalk.utils.validator;
var types_ = validator_.Types;

var callbackId = 0;
var callbacks = {};

extension.setMessageListener(function(json) {
  var result = JSON.parse(json);
  var callback = callbacks[result['callbackId']];
  callback(result);
});

function nextCallbackId() {
  return callbackId++;
}

function callNative(cmd, args) {
  var json = {'cmd': cmd, 'args': args};
  var argjson = JSON.stringify(json);
  var resultString = extension.internal.sendSyncMessage(argjson);
  var result = JSON.parse(resultString);

  if (typeof result !== 'object') {
    throw new tizen.WebAPIException(tizen.WebAPIException.UNKNOWN_ERR);
  }

  if (result['status'] === 'success') {
    if (result['result']) {
      return result['result'];
    }
    return true;
  } else if (result['status'] === 'error') {
    var err = result['error'];
    if (err) {
      throw new tizen.WebAPIException(err.name, err.message);
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
  Object.defineProperty(obj, n, {'value': v, 'writable': false});
}

function ApplicationManager() {
  // constructor of ApplicationManager
}

function getAppInfoWithReadOnly(obj) {
  var appInfo = new ApplicationInformation();
  SetReadOnlyProperty(appInfo, 'id', obj.id);
  SetReadOnlyProperty(appInfo, 'name', obj.name);
  SetReadOnlyProperty(appInfo, 'iconPath', obj.iconPath);
  SetReadOnlyProperty(appInfo, 'version', obj.version);
  SetReadOnlyProperty(appInfo, 'show', obj.show);
  SetReadOnlyProperty(appInfo, 'categories', obj.categories);
  SetReadOnlyProperty(appInfo, 'installDate', new Date(obj.installDate));
  SetReadOnlyProperty(appInfo, 'size', obj.size);
  SetReadOnlyProperty(appInfo, 'packageId', obj.packageId);

  return appInfo;
}
ApplicationManager.prototype.getCurrentApplication = function() {
  var nativeParam = {
  };

  try {
    var syncResult = callNative('ApplicationManager_getCurrentApplication', nativeParam);
  } catch (e) {
    throw e;
  }

  var appInfo = getAppInfoWithReadOnly(syncResult.appInfo);
  var app = new Application();
  SetReadOnlyProperty(app, 'appInfo', appInfo);
  SetReadOnlyProperty(app, 'contextId', syncResult.contextId);
  return app;
};

ApplicationManager.prototype.kill = function(contextId, successCallback, errorCallback) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'contextId', 'type': types_.STRING},
    {'name': 'successCallback', 'type': types_.FUNCTION, 'optional': true, 'nullable': true},
    {'name': 'errorCallback', 'type': types_.FUNCTION, 'optional': true, 'nullable': true}
  ]);

  var nativeParam = {
    'contextId': args.contextId
  };

  try {
    var syncResult =
        callNativeWithCallback('ApplicationManager_kill', nativeParam, function(result) {
      if (result.status === 'success') {
        if (args.successCallback) {
          args.successCallback();
        }
      }
      if (result.status === 'error') {
        if (args.errorCallback) {
          args.errorCallback(result.error);
        }
      }
      delete callbacks[result['callbackId']];
    });
  } catch (e) {
    throw e;
  }
};

ApplicationManager.prototype.launch = function(id, successCallback, errorCallback) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'id', 'type': types_.STRING},
    {'name': 'successCallback', 'type': types_.FUNCTION, 'optional': true, 'nullable': true},
    {'name': 'errorCallback', 'type': types_.FUNCTION, 'optional': true, 'nullable': true}
  ]);

  var nativeParam = {
    'id': args.id
  };

  try {
    var syncResult =
        callNativeWithCallback('ApplicationManager_launch', nativeParam, function(result) {
      if (result.status === 'success') {
        if (args.successCallback) {
          args.successCallback();
        }
      }
      if (result.status === 'error') {
        if (args.errorCallback) {
          args.errorCallback(result.error);
        }
      }
      delete callbacks[result['callbackId']];
    });
  } catch (e) {
    throw e;
  }
};

ApplicationManager.prototype.launchAppControl = function(appControl, id, successCallback,
    errorCallback, replyCallback) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'appControl', 'type': types_.PLATFORM_OBJECT, 'values': tizen.ApplicationControl},
    {'name': 'id', 'type': types_.STRING, 'optional': true, 'nullable': true},
    {'name': 'successCallback', 'type': types_.FUNCTION, 'optional': true, 'nullable': true},
    {'name': 'errorCallback', 'type': types_.FUNCTION, 'optional': true, 'nullable': true},
    {'name': 'replyCallback', 'type': types_.LISTENER, 'values': ['onsuccess', 'onfailure'],
      'optional': true, 'nullable': true}
  ]);

  var nativeParam = {
  };
  if (args['id']) {
    nativeParam['id'] = args.id;
  }
  if (args['appControl']) {
    nativeParam['appControl'] = args.appControl;
  }
  try {
    var syncResult =
        callNativeWithCallback('ApplicationManager_launchAppControl', nativeParam, function(ret) {
      // In case of reply, can have onsuccess or onfailure with result.status
      // It should be checked first of all
      if (ret.type === 'onsuccess') {
        if (args.replyCallback) {
          args.replyCallback.onsuccess(ret.data);
        }
      }
      else if (ret.type === 'onfailure') {
        if (args.replyCallback) {
          args.replyCallback.onfailure();
        }
      }
      else if (ret.status === 'success') {
        if (args.successCallback) {
          args.successCallback();
        }
      }
      else if (ret.status === 'error') {
        if (args.errorCallback) {
          args.errorCallback(ret.error);
        }
      }
      delete callbacks[result['callbackId']];
    });
  } catch (e) {
    throw e;
  }
};

ApplicationManager.prototype.findAppControl = function(appControl, successCallback, errorCallback) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'appControl', 'type': types_.PLATFORM_OBJECT, 'values': tizen.ApplicationControl},
    {'name': 'successCallback', 'type': types_.FUNCTION},
    {'name': 'errorCallback', 'type': types_.FUNCTION, 'optional': true, 'nullable': true}
  ]);

  var nativeParam = {
  };
  if (args['appControl']) {
    nativeParam['appControl'] = args.appControl;
  }
  try {
    var syncResult =
        callNativeWithCallback('ApplicationManager_findAppControl', nativeParam, function(result) {
      if (result.status === 'success') {
        var returnArray = new Array();
        for (var i = 0; i < result.informationArray.length; i++) {
          var appInfo = getAppInfoWithReadOnly(result.informationArray[i]);
          returnArray.push(appInfo);
        }

        args.successCallback(returnArray, result.appControl);
      } else if (result.status === 'error') {
        if (args.errorCallback) {
          args.errorCallback(result.error);
        }
      }
      delete callbacks[result['callbackId']];
    });
  } catch (e) {
    throw e;
  }
};

ApplicationManager.prototype.getAppsContext = function(successCallback, errorCallback) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'successCallback', 'type': types_.FUNCTION},
    {'name': 'errorCallback', 'type': types_.FUNCTION, 'optional': true, 'nullable': true}
  ]);

  var nativeParam = {
  };
  try {
    var syncResult =
        callNativeWithCallback('ApplicationManager_getAppsContext', nativeParam, function(result) {
      if (result.status === 'success') {
        var returnArray = new Array();
        for (var index = 0; index < result.contexts.length; index++) {
          var appContext = new ApplicationContext();
          SetReadOnlyProperty(appContext, 'id', result.contexts[index].id);
          SetReadOnlyProperty(appContext, 'appId', result.contexts[index].appId);
          returnArray.push(appContext);
        }
        args.successCallback(returnArray);
      }
      else if (result.status === 'error') {
        if (args.errorCallback) {
          args.errorCallback(result.error);
        }
      }
      delete callbacks[result['callbackId']];
    });
  } catch (e) {
    throw e;
  }
};

ApplicationManager.prototype.getAppContext = function(contextId) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'contextId', 'type': types_.STRING, 'optional': true, 'nullable': true}
  ]);

  var nativeParam = {
  };
  if (args['contextId']) {
    nativeParam['contextId'] = args.contextId;
  }
  try {
    var syncResult = callNative('ApplicationManager_getAppContext', nativeParam);
  } catch (e) {
    throw e;
  }

  var returnObject = new ApplicationContext();
  SetReadOnlyProperty(returnObject, 'id', syncResult.id); // read only property
  SetReadOnlyProperty(returnObject, 'appId', syncResult.appId); // read only property

  return returnObject;
};

ApplicationManager.prototype.getAppsInfo = function(successCallback, errorCallback) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'successCallback', 'type': types_.FUNCTION},
    {'name': 'errorCallback', 'type': types_.FUNCTION, 'optional': true, 'nullable': true}
  ]);

  var nativeParam = {
  };
  try {
    var syncResult =
        callNativeWithCallback('ApplicationManager_getAppsInfo', nativeParam, function(result) {
      if (result.status === 'success') {
        var returnArray = new Array();
        for (var i = 0; i < result.informationArray.length; i++) {
          var appInfo = getAppInfoWithReadOnly(result.informationArray[i]);
          returnArray.push(appInfo);
        }
        args.successCallback(returnArray);
      }
      else if (result.status === 'error') {
        if (args.errorCallback) {
          args.errorCallback(result.error);
        }
      }
      delete callbacks[result['callbackId']];
    });
  } catch (e) {
    throw e;
  }
};

ApplicationManager.prototype.getAppInfo = function(id) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'id', 'type': types_.STRING, 'optional': true, 'nullable': true}
  ]);

  var nativeParam = {
  };
  if (args['id']) {
    nativeParam['id'] = args.id;
  }
  try {
    var syncResult = callNative('ApplicationManager_getAppInfo', nativeParam);
  } catch (e) {
    throw e;
  }
  var appInfo = getAppInfoWithReadOnly(syncResult);
  return appInfo;
};

ApplicationManager.prototype.getAppCerts = function(id) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'id', 'type': types_.STRING, 'optional': true, 'nullable': true}
  ]);

  var nativeParam = {
  };
  if (args['id']) {
    nativeParam['id'] = args.id;
  }
  try {
    var syncResult = callNative('ApplicationManager_getAppCerts', nativeParam);
  } catch (e) {
    throw e;
  }

  var returnArrayObject = new Array();

  for (var i = 0; i < syncResult.length; i++) {
    var returnObject = new ApplicationCertificate();
    SetReadOnlyProperty(returnObject, 'type', syncResult[i].type); // read only property
    SetReadOnlyProperty(returnObject, 'value', syncResult[i].value); // read only property
    returnArrayObject.push(returnObject);
  }
  return returnArrayObject;
};

ApplicationManager.prototype.getAppSharedURI = function() {
  var args = validator_.validateArgs(arguments, [
    {'name': 'id', 'type': types_.STRING, optional: true, nullable: true}
  ]);

  var nativeParam = {
  };
  if (args['id']) {
    nativeParam['id'] = args.id;
  }
  try {
    var syncResult = callNative('ApplicationManager_getAppSharedURI', nativeParam);
  } catch (e) {
    throw e;
  }

  return syncResult;
};

ApplicationManager.prototype.getAppMetaData = function(id) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'id', 'type': types_.STRING, 'optional': true, 'nullable': true}
  ]);

  var nativeParam = {
  };
  if (args['id']) {
    nativeParam['id'] = args.id;
  }
  try {
    var syncResult = callNative('ApplicationManager_getAppMetaData', nativeParam);
  } catch (e) {
    throw e;
  }

  var returnArrayObject = new Array();

  for (var i = 0; i < syncResult.length; i++) {
    var returnObject = new ApplicationMetaData();
    SetReadOnlyProperty(returnObject, 'key', syncResult[i].key); // read only property
    SetReadOnlyProperty(returnObject, 'value', syncResult[i].value); // read only property
    returnArrayObject.push(returnObject);
  }
  return returnArrayObject;
};

ApplicationManager.prototype.addAppInfoEventListener = function(eventCallback) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'eventCallback', 'type': types_.LISTENER,
      'values': ['oninstalled', 'onupdated', 'onuninstalled']}
  ]);

  var param = {
  };
  try {
    var syncResult =
        callNativeWithCallback('ApplicationManager_addAppInfoEventListener', param, function(ret) {
      if (ret.type === 'oninstalled') {
        var appInfo = getAppInfoWithReadOnly(ret.info);
        args.eventCallback.oninstalled(appInfo);
      } else if (ret.type === 'onupdated') {
        var appInfo = getAppInfoWithReadOnly(ret.info);
        args.eventCallback.oninstalled(appInfo);
      } else if (ret.type === 'onuninstalled') {
        args.eventCallback.onuninstalled(ret.id);
      }
      delete callbacks[result['callbackId']];
    });
  } catch (e) {
    throw e;
  }

  return syncResult;
};

ApplicationManager.prototype.removeAppInfoEventListener = function(watchId) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'watchId', 'type': types_.LONG}
  ]);

  var nativeParam = {
    'watchId': args.watchId
  };
  try {
    var syncResult = callNative('ApplicationManager_removeAppInfoEventListener', nativeParam);
  } catch (e) {
    throw e;
  }
};

function Application() {
  // constructor of Application
}

Application.prototype.exit = function() {
  var nativeParam = {
  };
  try {
    var syncResult = callNative('Application_exit', nativeParam);
  } catch (e) {
    throw e;
  }
};

Application.prototype.hide = function() {
  var nativeParam = {
  };
  try {
    var syncResult = callNative('Application_hide', nativeParam);
  } catch (e) {
    throw e;
  }
};

Application.prototype.getRequestedAppControl = function() {
  var nativeParam = {
  };
  try {
    var syncResult = callNative('Application_getRequestedAppControl', nativeParam);
  } catch (e) {
    throw e;
  }

  var returnObject = new RequestedApplicationControl();
  SetReadOnlyProperty(returnObject, 'appControl', syncResult.appControl); // read only property
  SetReadOnlyProperty(returnObject, 'callerAppId', syncResult.callerAppId); // read only property

  return returnObject;
};

function ApplicationInformation() {
  // constructor of ApplicationInformation
}

function ApplicationContext() {
  // constructor of ApplicationContext
}

tizen.ApplicationControlData = function(key, value) {
  if (this && this.constructor === tizen.ApplicationControlData &&
      (typeof(key) === 'string' || key instanceof String) &&
      (value && value instanceof Array)) {

    Object.defineProperties(this, {
      'key': { writable: true, enumerable: true, value: key },
      'value': { writable: true, enumerable: true, value: value }
    });

  } else {
    throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR);
  }
};

tizen.ApplicationControl = function(operation, uri, mime, category, data) {
  if (this && this.constructor === tizen.ApplicationControl &&
      (typeof(operation) === 'string' || operation instanceof String)) {

    Object.defineProperties(this, {
      'operation': { writable: true, enumerable: true, value: operation },
      'uri': { writable: true, enumerable: true, value: uri === undefined ? null : uri },
      'mime': { writable: true, enumerable: true, value: mime === undefined ? null : mime },
      'category': { writable: true, enumerable: true,
        value: category === undefined ? null : category },
      'data': { writable: true, enumerable: true, value: data === undefined ? null : data }
    });

  } else {
    throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR);
  }
};

function RequestedApplicationControl() {
  // constructor of RequestedApplicationControl
}

RequestedApplicationControl.prototype.replyResult = function(data) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'data', 'type': types_.PLATFORM_OBJECT,
      'values': tizen.ApplicationControlData, 'optional': true}
  ]);

  var nativeParam = {
  };
  if (args['data']) {
    nativeParam['data'] = args.data;
  }
  if (this.callerAppId)
    nativeParam['callerAppId'] = this.callerAppId;
  try {
    var syncResult = callNative('RequestedApplicationControl_replyResult', nativeParam);
  } catch (e) {
    throw e;
  }
};

RequestedApplicationControl.prototype.replyFailure = function() {
  var nativeParam = {
  };
  if (this.callerAppId)
    nativeParam['callerAppId'] = this.callerAppId;
  try {
    var syncResult = callNative('RequestedApplicationControl_replyFailure', nativeParam);
  } catch (e) {
    throw e;
  }
};

function ApplicationCertificate() {
  // constructor of ApplicationCertificate
}

function ApplicationMetaData() {
  // constructor of ApplicationMetaData
}

exports = new ApplicationManager();
