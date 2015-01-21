// Application
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

  if (result['status'] == 'success') {
    if (result['result']) {
      return result['result'];
    }
    return true;
  } else if (result['status'] == 'error') {
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
  Object.defineProperty(obj, n, {value: v, writable: false});
}


function ApplicationManager() {
  // constructor of ApplicationManager
}


ApplicationManager.prototype.getCurrentApplication = function() {
  var nativeParam = {};

  try {
    var syncResult = callNative('ApplicationManager_getCurrentApplication', nativeParam);
  } catch (e) {
    throw e;
  }

  var appInfo = new ApplicationInformation();
  SetReadOnlyProperty(appInfo, 'id', syncResult.appInfo.id);
  SetReadOnlyProperty(appInfo, 'name', syncResult.appInfo.name);
  SetReadOnlyProperty(appInfo, 'iconPath', syncResult.appInfo.iconPath);
  SetReadOnlyProperty(appInfo, 'version', syncResult.appInfo.version);
  SetReadOnlyProperty(appInfo, 'show', syncResult.appInfo.show);
  SetReadOnlyProperty(appInfo, 'categories', syncResult.appInfo.categories);
  SetReadOnlyProperty(appInfo, 'installDate', syncResult.appInfo.installDate);
  SetReadOnlyProperty(appInfo, 'size', syncResult.appInfo.size);
  SetReadOnlyProperty(appInfo, 'packageId', syncResult.appInfo.packageId);

  var app = new Application();
  SetReadOnlyProperty(app, 'appInfo', appInfo);
  SetReadOnlyProperty(app, 'contextId', syncResult.contextId);
  return app;
};

ApplicationManager.prototype.kill = function(contextId) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'contextId', 'type': types_.STRING},
    {'name': 'successCallback', 'type': types_.FUCTION, optional: true, nullable: true},
    {'name': 'errorCallback', 'type': types_.FUNCTION, optional: true, nullable: true}
  ]);

  var nativeParam = {
  };
  if (args['contextId']) {
    nativeParam['contextId'] = args.contextId;

    try {
      var syncResult = callNativeWithCallback('ApplicationManager_kill', nativeParam, function(result) {
        if (result.status == 'success') {
          if (args.successCallback) {
            args.successCallback();
          }
        }
        if (result.status == 'error') {
          if (args.errorCallback) {
            args.errorCallback(result.error);
          }
        }
      });
    } catch (e) {
      throw e;
    }
  }
};

ApplicationManager.prototype.launch = function(id) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'id', 'type': types_.STRING},
    {'name': 'successCallback', 'type': types_.FUNCTION, optional: true, nullable: true},
    {'name': 'errorCallback', 'type': types_.FUNCTION, optional: true, nullable: true}
  ]);

  var nativeParam = {
  };
  if (args['id']) {
    nativeParam['id'] = args.id;
  }

  try {
    var syncResult = callNativeWithCallback('ApplicationManager_launch', nativeParam, function(result) {
      if (result.status == 'success') {
        if (args.successCallback) {
          args.successCallback();
        }
      }
      if (result.status == 'error') {
        if (args.errorCallback) {
          args.errorCallback(result.error);
        }
      }
    });
  } catch (e) {
    throw e;
  }
};

ApplicationManager.prototype.launchAppControl = function(appControl) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'appControl', 'type': types_.DICTIONARY},
    {'name': 'id', 'type': types_.STRING, optional: true, nullable: true},
    {'name': 'successCallback', 'type': types_.FUNCTION, optional: true, nullable: true},
    {'name': 'errorCallback', 'type': types_.FUNCTION, optional: true, nullable: true},
    {'name': 'replyCallback', 'type': types_.LISTENER, 'values': ['onsuccess', 'onfailure'], optional: true, nullable: true}
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
    var syncResult = callNativeWithCallback('ApplicationManager_launchAppControl', nativeParam, function(result) {
      // In case of reply, can have onsuccess or onfailure with result.status
      // It should be checked first of all
      if (result.type == 'onsuccess') {
        if (args.replyCallback) {
          args.replyCallback.onsuccess(result.data);
        }
      }
      else if (result.type == 'onfailure') {
        if (args.replyCallback) {
          args.replyCallback.onfailure();
        }
      }
      else if (result.status == 'success') {
        if (args.successCallback) {
          args.successCallback();
        }
      }
      else if (result.status == 'error') {
        if (args.errorCallback) {
          args.errorCallback(result.error);
        }
      }
    });
  } catch (e) {
    throw e;
  }
};

ApplicationManager.prototype.findAppControl = function(appControl, successCallback) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'appControl', 'type': types_.DICTIONARY},
    {'name': 'successCallback', 'type': types_.FUNCTION},
    {'name': 'errorCallback', 'type': types_.FUNCTION, optional: true, nullable: true}
  ]);

  var nativeParam = {
  };
  if (args['appControl']) {
    nativeParam['appControl'] = args.appControl;
  }
  try {
    var syncResult = callNativeWithCallback('ApplicationManager_findAppControl', nativeParam, function(result) {
      if (result.status == 'success') {
        args.successCallback(result.informationArray, result.appControl);
      } else if (result.status == 'error') {
        if (args.errorCallback) {
          args.errorCallback(result.error);
        }
      }
    });
  } catch (e) {
    throw e;
  }
};

ApplicationManager.prototype.getAppsContext = function(successCallback) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'successCallback', 'type': types_.FUNCTION},
    {'name': 'errorCallback', 'type': types_.FUNCTION, optional: true, nullable: true}
  ]);

  var nativeParam = {
  };
  try {
    var syncResult = callNativeWithCallback('ApplicationManager_getAppsContext', nativeParam, function(result) {
      if (result.status == 'success') {
        args.successCallback(result.contexts);
      }
      else if (result.status == 'error') {
        if (args.errorCallback) {
          args.errorCallback(result.error);
        }
      }
    });
  } catch (e) {
    throw e;
  }
};

ApplicationManager.prototype.getAppContext = function() {
  var args = validator_.validateArgs(arguments, [
    {'name': 'contextId', 'type': types_.STRING, optional: true, nullable: true}
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

ApplicationManager.prototype.getAppsInfo = function(successCallback) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'successCallback', 'type': types_.FUNCTION},
    {'name': 'errorCallback', 'type': types_.FUNCTION, optional: true, nullable: true}
  ]);

  var nativeParam = {
  };
  try {
    var syncResult = callNativeWithCallback('ApplicationManager_getAppsInfo', nativeParam, function(result) {
      if (result.status == 'success') {
        args.successCallback(result.informationArray);
      }
      else if (result.status == 'error') {
        if (args.errorCallback) {
          args.errorCallback(result.error);
        }
      }
    });
  } catch (e) {
    throw e;
  }
};

ApplicationManager.prototype.getAppInfo = function() {
  var args = validator_.validateArgs(arguments, [
    {'name': 'id', 'type': types_.STRING, optional: true, nullable: true}
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

  var returnObject = new ApplicationInformation();
  SetReadOnlyProperty(returnObject, 'id', syncResult.id);
  SetReadOnlyProperty(returnObject, 'name', syncResult.name);
  SetReadOnlyProperty(returnObject, 'iconPath', syncResult.iconPath);
  SetReadOnlyProperty(returnObject, 'version', syncResult.version);
  SetReadOnlyProperty(returnObject, 'show', syncResult.show);
  SetReadOnlyProperty(returnObject, 'categories', syncResult.categories);
  SetReadOnlyProperty(returnObject, 'installDate', syncResult.installDate);
  SetReadOnlyProperty(returnObject, 'size', syncResult.size);
  SetReadOnlyProperty(returnObject, 'packageId', syncResult.packageId);

  return returnObject;
};

ApplicationManager.prototype.getAppCerts = function() {
  var args = validator_.validateArgs(arguments, [
    {'name': 'id', 'type': types_.STRING, optional: true, nullable: true}
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

ApplicationManager.prototype.getAppMetaData = function() {
  var args = validator_.validateArgs(arguments, [
    {'name': 'id', 'type': types_.STRING, optional: true, nullable: true}
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
    {'name': 'eventCallback', 'type': types_.LISTENER, 'values': ['oninstalled', 'onupdated', 'onuninstalled']}
  ]);

  var nativeParam = {
  };
  try {
    var syncResult = callNativeWithCallback('ApplicationManager_addAppInfoEventListener', nativeParam, function(result) {
      if (result.type == 'oninstalled') {
        args.eventCallback.oninstalled(result.info);
      }
      if (result.type == 'onupdated') {
        args.eventCallback.onupdated(result.info);
      }
      if (result.type == 'onuninstalled') {
        args.eventCallback.onuninstalled(result.id);
      }
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
  var nativeParam = {};
  try {
    var syncResult = callNative('Application_exit', nativeParam);
  } catch (e) {
    throw e;
  }
};

Application.prototype.hide = function() {
  var nativeParam = {};
  try {
    var syncResult = callNative('Application_hide', nativeParam);
  } catch (e) {
    throw e;
  }
};

Application.prototype.getRequestedAppControl = function() {
  var nativeParam = {};
  try {
    var syncResult = callNative('Application_getRequestedAppControl', nativeParam);
  } catch (e) {
    throw e;
  }

  var returnObject = new RequestedApplicationControl();
  SetReadOnlyProperty(returnObject, 'appControl', returnObject.appControl); // read only property
  SetReadOnlyProperty(returnObject, 'callerAppId', returnObject.callerAppId); // read only property

  return returnObject;
};

function ApplicationInformation() {
  // constructor of ApplicationInformation
}

function ApplicationContext() {
  // constructor of ApplicationContext
}

tizen.ApplicationControlData = function(key, value) {
  if (this && this.constructor == tizen.ApplicationControlData &&
      (typeof(key) == 'string' || key instanceof String) &&
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

  if (this && this.constructor == tizen.ApplicationControl &&
      (typeof(operation) == 'string' || operation instanceof String) &&
      data) {

    Object.defineProperties(this, {
      'operation': { writable: true, enumerable: true, value: operation },
      'uri': { writable: true, enumerable: true, value: uri === undefined ? null : uri },
      'mime': { writable: true, enumerable: true, value: mime === undefined ? null : mime },
      'category': { writable: true, enumerable: true, value: category === undefined ? null : category },
      'data': { writable: true, enumerable: true, value: data }
    });

  } else {
    throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR);
  }
};

function RequestedApplicationControl() {
  // constructor of RequestedApplicationControl
}

RequestedApplicationControl.prototype.replyResult = function() {
  var args = validator_.validateArgs(arguments, [
    {'name': 'data', 'type': types_.ARRAY, optional: true}
  ]);

  var nativeParam = {
  };
  if (args['data']) {
    nativeParam['data'] = args.data;
  }
  try {
    var syncResult = callNative('RequestedApplicationControl_replyResult', nativeParam);
  } catch (e) {
    throw e;
  }
};

RequestedApplicationControl.prototype.replyFailure = function() {

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
