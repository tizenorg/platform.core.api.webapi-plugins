/* global tizen, xwalk, extension */

// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


var validator_ = xwalk.utils.validator;
var types_ = validator_.Types;
var native_ = new xwalk.utils.NativeManager(extension);

var ApplicationControlLaunchMode = {
  SINGLE: 'SINGLE',
  GROUP: 'GROUP'
};

function callNative(cmd, args) {
  var result = native_.callSync(cmd, args);

  if (typeof result !== 'object') {
    throw new WebAPIException(WebAPIException.UNKNOWN_ERR);
  }

  if (native_.isSuccess(result)) {
    if (native_.getResultObject(result)) {
      return native_.getResultObject(result);
    }
    return true;
  } else {
    if (result.error) {
      throw native_.getErrorObject(result);
    }
    return false;
  }
}

function callNativeWithCallback(cmd, args, callback) {
  return native_.call(cmd, args, callback);
}

function setReadOnlyProperty(obj, n, v) {
  Object.defineProperty(obj, n, {'value': v, 'writable': false});
}

function defineReadWriteProperty(object, key, value) {
  Object.defineProperty(object, key, {
    enumerable: true,
    writable: true,
    value: value
  });
}

function defineReadWriteNonNullProperty(object, key, value) {
  var hvalue = value;
  Object.defineProperty(object, key, {
    enumerable: true,
    set: function(val) {
      if (val !== null && val !== undefined) {
        hvalue = val;
      }
    },
    get: function() {
      return hvalue;
    }
  });
}

function ApplicationManager() {
  // constructor of ApplicationManager
}

function getAppInfoWithReadOnly(obj) {
  var appInfo = new ApplicationInformation();
  setReadOnlyProperty(appInfo, 'id', obj.id);
  setReadOnlyProperty(appInfo, 'name', obj.name);
  setReadOnlyProperty(appInfo, 'iconPath', obj.iconPath);
  setReadOnlyProperty(appInfo, 'version', obj.version);
  setReadOnlyProperty(appInfo, 'show', obj.show);
  setReadOnlyProperty(appInfo, 'categories', obj.categories);
  setReadOnlyProperty(appInfo, 'installDate', new Date(obj.installDate));
  setReadOnlyProperty(appInfo, 'packageId', obj.packageId);

  Object.defineProperty(appInfo, 'size', {
    get: function () {
      xwalk.utils.checkPrivilegeAccess('http://tizen.org/privilege/application.info');
      return obj.size;
    },
    set: function (value) {},
    enumerable: true
  });

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
  setReadOnlyProperty(app, 'appInfo', appInfo);
  setReadOnlyProperty(app, 'contextId', syncResult.contextId);
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
      if (native_.isSuccess(result)) {
        native_.callIfPossible(args.successCallback);
      } else {
        native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      }
    });
    if (native_.isFailure(syncResult)){
      var error = native_.getErrorObject(syncResult);
      if (error.code === WebAPIException.NOT_FOUND_ERR
            || error.code === WebAPIException.INVALID_VALUES_ERR
            || error.code === WebAPIException.UNKNOWN_ERR) {
        setTimeout(function() {
          native_.callIfPossible(args.errorCallback, error);
        }, 0);
        return;
      }
      throw error;
    }
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
      if (native_.isSuccess(result)) {
          native_.callIfPossible(args.successCallback);
      } else {
          native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      }
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
  if (args.appControl['launchMode'] ) {
      nativeParam['launchMode'] = args.appControl['launchMode'];
  }

  //Try to find application
  try {
      if (args['id']) {
        this.getAppInfo(args.id);
      }
  } catch(e) {
      //If app cannot be found, it should all errorCallback with NotFoundException.
      setTimeout(function() {
          native_.callIfPossible(args.errorCallback, e);
      }, 0);
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
        native_.callIfPossible(args.successCallback);
      }
      else if (ret.status === 'error') {
        native_.callIfPossible(args.errorCallback, native_.getErrorObject(ret));
      }
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
      if (native_.isSuccess(result)) {
        var returnArray = [];
        for (var i = 0; i < result.informationArray.length; i++) {
          var appInfo = getAppInfoWithReadOnly(result.informationArray[i]);
          returnArray.push(appInfo);
        }

        args.successCallback(returnArray, result.appControl);
      } else if (native_.isFailure(result)) {
        native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      }
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
      if (native_.isSuccess(result)) {
        var returnArray = [];
        for (var index = 0; index < result.contexts.length; index++) {
          var appContext = new ApplicationContext();
          setReadOnlyProperty(appContext, 'id', result.contexts[index].id);
          setReadOnlyProperty(appContext, 'appId', result.contexts[index].appId);
          returnArray.push(appContext);
        }
        args.successCallback(returnArray);
      }
      else if (result.status === 'error') {
        native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      }
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
  setReadOnlyProperty(returnObject, 'id', syncResult.id); // read only property
  setReadOnlyProperty(returnObject, 'appId', syncResult.appId); // read only property

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
      if (native_.isSuccess(result)) {
        var returnArray = [];
        for (var i = 0; i < result.informationArray.length; i++) {
          var appInfo = getAppInfoWithReadOnly(result.informationArray[i]);
          returnArray.push(appInfo);
        }
        args.successCallback(returnArray);
      }
      else if (native_.isFailure(result)) {
        native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      }
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

  var returnArrayObject = [];

  for (var i = 0; i < syncResult.length; i++) {
    var returnObject = new ApplicationCertificate();
    setReadOnlyProperty(returnObject, 'type', syncResult[i].type); // read only property
    setReadOnlyProperty(returnObject, 'value', syncResult[i].value); // read only property
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

  var returnArrayObject = [];

  for (var i = 0; i < syncResult.length; i++) {
    var returnObject = new ApplicationMetaData();
    setReadOnlyProperty(returnObject, 'key', syncResult[i].key); // read only property
    setReadOnlyProperty(returnObject, 'value', syncResult[i].value); // read only property
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
  return native_.sendRuntimeMessage('Application_exit');
};

Application.prototype.hide = function() {
  return native_.sendRuntimeMessage('Application_hide');
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
  setReadOnlyProperty(returnObject, 'appControl', syncResult.appControl); // read only property
  setReadOnlyProperty(returnObject, 'callerAppId', syncResult.callerAppId); // read only property

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
    defineReadWriteNonNullProperty(this, 'key', key);
    defineReadWriteNonNullProperty(this, 'value', value);
  } else {
    throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR);
  }
};

tizen.ApplicationControl = function(operation, uri, mime, category, data, launchMode) {
  if (this && this.constructor === tizen.ApplicationControl &&
      (typeof(operation) === 'string' || operation instanceof String)) {

    defineReadWriteNonNullProperty(this, 'operation', operation);
    defineReadWriteProperty(this, 'uri', uri);
    defineReadWriteProperty(this, 'mime', mime);
    defineReadWriteProperty(this, 'category', category);
    defineReadWriteProperty(this, 'data', data);
    defineReadWriteProperty(this, 'data', launchMode);

  } else {
    throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR);
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
