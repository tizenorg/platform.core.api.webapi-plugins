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

var T = xwalk.utils.type;
var Converter = xwalk.utils.converter;
var AV = xwalk.utils.validator;
var Privilege = xwalk.utils.privilege;

var native = new xwalk.utils.NativeManager(extension);

var ApplicationControlLaunchMode = {
  SINGLE: 'SINGLE',
  GROUP: 'GROUP'
};

// helper functions ////////////////////////////////////////////////////
function _createApplicationControlData(object) {
  var ret;
  if (!T.isNullOrUndefined(object)) {
    ret = new tizen.ApplicationControlData(object.key, object.value);
  }
  return ret;
}

function _createApplicationControlDataArray(object) {
  var ret = [];
  if (!T.isNullOrUndefined(object) && T.isArray(object)) {
    object.forEach(function (o) {
      var data = _createApplicationControlData(o);
      if (!T.isNullOrUndefined(data)) {
        ret.push(data);
      }
    });
  }
  return ret;
}

function _createApplicationControl(object) {
  var ret;
  if (!T.isNullOrUndefined(object)) {
    ret = new tizen.ApplicationControl(object.operation,
        object.uri,
        object.mime,
        object.category,
        _createApplicationControlDataArray(object.data));
  }
  return ret;
}

function _createApplicationInformationArray(object) {
  var ret = [];
  if (!T.isNullOrUndefined(object) && T.isArray(object)) {
    object.forEach(function (o) {
      var data = new ApplicationInformation(o);
      if (!T.isNullOrUndefined(data)) {
        ret.push(data);
      }
    });
  }
  return ret;
}

// class ApplicationManager ////////////////////////////////////////////////////
var ApplicationManager = function() {
};

ApplicationManager.prototype.getCurrentApplication = function() {
  var result = native.callSync('ApplicationManager_getCurrentApplication', {});

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    return new Application(native.getResultObject(result));
  }
};

ApplicationManager.prototype.kill = function() {
  xwalk.utils.checkPrivilegeAccess(Privilege.APPMANAGER_KILL);

  var args = AV.validateMethod(arguments, [
      {
        name : 'contextId',
        type : AV.Types.STRING
      },
      {
        name : 'successCallback',
        type : AV.Types.FUNCTION,
        optional : true,
        nullable : true
      },
      {
        name : 'errorCallback',
        type : AV.Types.FUNCTION,
        optional : true,
        nullable : true
      }
  ]);

  var callback = function(result) {
    if (native.isFailure(result)) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(result));
    } else {
      native.callIfPossible(args.successCallback);
    }
  };

  var result = native.call('ApplicationManager_kill', {contextId: args.contextId}, callback);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
};

ApplicationManager.prototype.launch = function() {
  xwalk.utils.checkPrivilegeAccess(Privilege.APPLICATION_LAUNCH);

  var args = AV.validateMethod(arguments, [
      {
        name : 'id',
        type : AV.Types.STRING
      },
      {
        name : 'successCallback',
        type : AV.Types.FUNCTION,
        optional : true,
        nullable : true
      },
      {
        name : 'errorCallback',
        type : AV.Types.FUNCTION,
        optional : true,
        nullable : true
      }
  ]);

  var callback = function(result) {
    if (native.isFailure(result)) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(result));
    } else {
      native.callIfPossible(args.successCallback);
    }
  };

  var result = native.call('ApplicationManager_launch', {id: args.id}, callback);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
};

ApplicationManager.prototype.launchAppControl = function() {
  xwalk.utils.checkPrivilegeAccess(Privilege.APPLICATION_LAUNCH);

  var args = AV.validateMethod(arguments, [
      {
        name : 'appControl',
        type : AV.Types.PLATFORM_OBJECT,
        values : tizen.ApplicationControl
      },
      {
        name : 'id',
        type : AV.Types.STRING,
        optional : true,
        nullable : true
      },
      {
        name : 'successCallback',
        type : AV.Types.FUNCTION,
        optional : true,
        nullable : true
      },
      {
        name : 'errorCallback',
        type : AV.Types.FUNCTION,
        optional : true,
        nullable : true
      },
      {
        name : 'replyCallback',
        type : AV.Types.LISTENER,
        values : ['onsuccess', 'onfailure'],
        optional : true,
        nullable : true
      }
  ]);

  var replyCallbackId = 'ApplicationControlDataArrayReplyCallback_' + new Date().valueOf();
  var registeredReplyCallback = function(result) {
    if (native.isFailure(result)) {
      native.callIfPossible(args.replyCallback.onfailure);
    } else {
      native.callIfPossible(args.replyCallback.onsuccess,
          _createApplicationControlDataArray(result.data));
    }
    native.removeListener(replyCallbackId, registeredReplyCallback);
  };

  var callback = function(result) {
    if (native.isFailure(result)) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(result));
      native.removeListener(replyCallbackId, registeredReplyCallback);
    } else {
      native.callIfPossible(args.successCallback);
    }
  };

  var callArgs = {};
  callArgs.appControl = args.appControl;
  if (args.has.id) {
    callArgs.id = args.id;
  }
  if (args.has.replyCallback) {
    callArgs.replyCallback = replyCallbackId;
    native.addListener(replyCallbackId, registeredReplyCallback);
  }

  var result = native.call('ApplicationManager_launchAppControl', callArgs, callback);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
};

ApplicationManager.prototype.findAppControl = function() {
  var args = AV.validateMethod(arguments, [
      {
        name : 'appControl',
        type : AV.Types.PLATFORM_OBJECT,
        values : tizen.ApplicationControl
      },
      {
        name : 'successCallback',
        type : AV.Types.FUNCTION
      },
      {
        name : 'errorCallback',
        type : AV.Types.FUNCTION,
        optional : true,
        nullable : true
      }
  ]);

  var callback = function(result) {
    if (native.isFailure(result)) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(result));
    } else {
      var r = native.getResultObject(result);
      args.successCallback(_createApplicationInformationArray(r.informationArray),
          _createApplicationControl(r.appControl));
    }
  };

  var callArgs = {appControl: args.appControl};
  var result = native.call('ApplicationManager_findAppControl', callArgs, callback);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
};

ApplicationManager.prototype.getAppsContext = function() {
  var args = AV.validateMethod(arguments, [
      {
        name : 'successCallback',
        type : AV.Types.FUNCTION
      },
      {
        name : 'errorCallback',
        type : AV.Types.FUNCTION,
        optional : true,
        nullable : true
      }
  ]);

  var callback = function(result) {
    if (native.isFailure(result)) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(result));
    } else {
      var contexts = native.getResultObject(result).contexts;
      var c = [];
      contexts.forEach(function (i) {
        c.push(new ApplicationContext(i));
      });
      args.successCallback(c);
    }
  };

  var result = native.call('ApplicationManager_getAppsContext', {}, callback);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
};

ApplicationManager.prototype.getAppContext = function() {
  var args = AV.validateMethod(arguments, [
      {
        name : 'contextId',
        type : AV.Types.STRING,
        optional : true,
        nullable : true
      }
  ]);

  var callArgs = {};

  if (args.has.contextId) {
    callArgs.contextId = args.contextId;
  }

  var result = native.callSync('ApplicationManager_getAppContext', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    return new ApplicationContext(native.getResultObject(result));
  }
};

ApplicationManager.prototype.getAppsInfo = function() {
  var args = AV.validateMethod(arguments, [
      {
        name : 'successCallback',
        type : AV.Types.FUNCTION
      },
      {
        name : 'errorCallback',
        type : AV.Types.FUNCTION,
        optional : true,
        nullable : true
      }
  ]);

  var callback = function(result) {
    if (native.isFailure(result)) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(result));
    } else {
      args.successCallback(_createApplicationInformationArray(
          native.getResultObject(result).informationArray)
      );
    }
  };

  var result = native.call('ApplicationManager_getAppsInfo', {}, callback);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
};

ApplicationManager.prototype.getAppInfo = function() {
  var args = AV.validateMethod(arguments, [
      {
        name : 'id',
        type : AV.Types.STRING,
        optional : true,
        nullable : true
      }
  ]);

  var callArgs = {};

  if (args.has.id) {
    callArgs.id = args.id;
  }

  var result = native.callSync('ApplicationManager_getAppInfo', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    return new ApplicationInformation(native.getResultObject(result));
  }
};

ApplicationManager.prototype.getAppCerts = function() {
  xwalk.utils.checkPrivilegeAccess(Privilege.APPMANAGER_CERTIFICATE);

  var args = AV.validateMethod(arguments, [
      {
        name : 'id',
        type : AV.Types.STRING,
        optional : true,
        nullable : true
      }
  ]);

  var callArgs = {};

  if (args.has.id) {
    callArgs.id = args.id;
  }

  var result = native.callSync('ApplicationManager_getAppCerts', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    var certificates = native.getResultObject(result);
    var c = [];
    certificates.forEach(function (i) {
      c.push(new ApplicationCertificate(i));
    });
    return c;
  }
};

ApplicationManager.prototype.getAppSharedURI = function() {
  var args = AV.validateMethod(arguments, [
      {
        name : 'id',
        type : AV.Types.STRING,
        optional : true,
        nullable : true
      }
  ]);

  var callArgs = {};

  if (args.has.id) {
    callArgs.id = args.id;
  }

  var result = native.callSync('ApplicationManager_getAppSharedURI', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    return native.getResultObject(result);
  }
};

ApplicationManager.prototype.getAppMetaData = function() {
  xwalk.utils.checkPrivilegeAccess(Privilege.APPLICATION_INFO);

  var args = AV.validateMethod(arguments, [
      {
        name : 'id',
        type : AV.Types.STRING,
        optional : true,
        nullable : true
      }
  ]);

  var callArgs = {};

  if (args.has.id) {
    callArgs.id = args.id;
  }

  var result = native.callSync('ApplicationManager_getAppMetaData', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    var metaData = native.getResultObject(result);
    var md = [];
    metaData.forEach(function (i) {
      md.push(new ApplicationMetaData(i));
    });
    return md;
  }
};

function ListenerManager(native, listenerName) {
  this.listeners = {};
  this.nextId = 1;
  this.nativeSet = false;
  this.native = native;
  this.listenerName = listenerName;
};

ListenerManager.prototype.onListenerCalled = function(msg) {
  var d = null;

  switch (msg.action) {
  case 'oninstalled':
  case 'onupdated':
    d = new ApplicationInformation(msg.data);
    break;

  case 'onuninstalled':
    d = msg.data;
    break;

  default:
    console.logd('Unknown mode: ' + msg.action);
  return;
  }

  for (var watchId in this.listeners) {
    if (this.listeners.hasOwnProperty(watchId) && this.listeners[watchId][msg.action]) {
      this.listeners[watchId][msg.action](d);
    }
  }
};

ListenerManager.prototype.addListener = function(callback) {
  console.log('ListenerManager.prototype.addListener');
  var id = this.nextId;
  if (!this.nativeSet) {
    this.native.addListener(this.listenerName, this.onListenerCalled.bind(this));
    var result = this.native.callSync('ApplicationManager_addAppInfoEventListener');
    if (this.native.isFailure(result)) {
      throw this.native.getErrorObject(result);
    }
    this.nativeSet = true;
  }

  this.listeners[id] = callback;
  ++this.nextId;

  return id;
};

ListenerManager.prototype.removeListener = function(watchId) {
  if (this.listeners.hasOwnProperty(watchId)) {
    delete this.listeners[watchId];
  }

  if (this.nativeSet && T.isEmptyObject(this.listeners)) {
      this.native.callSync('ApplicationManager_removeAppInfoEventListener');
      this.native.removeListener(this.listenerName);
      this.nativeSet = false;
  }
};

var APPLICATION_EVENT_LISTENER = 'ApplicationEventListener';
var applicationEventListener = new ListenerManager(native, APPLICATION_EVENT_LISTENER);

ApplicationManager.prototype.addAppInfoEventListener = function() {
  var args = AV.validateMethod(arguments, [
      {
        name : 'eventCallback',
        type : AV.Types.LISTENER,
        values : ['oninstalled', 'onupdated', 'onuninstalled']
      }
  ]);

  return applicationEventListener.addListener(args.eventCallback);
};

ApplicationManager.prototype.removeAppInfoEventListener = function() {
  var args = AV.validateMethod(arguments, [
      {
        name : 'watchId',
        type : AV.Types.LONG
      }
  ]);

  applicationEventListener.removeListener(args.watchId);
};

// class Application ////////////////////////////////////////////////////

function Application(data) {
  Object.defineProperties(this, {
    appInfo : {
      value : new ApplicationInformation(data.appInfo),
      writable : false,
      enumerable : true
    },
    contextId : {
      value : Converter.toString(data.contextId),
      writable : false,
      enumerable : true
    }
  });
}

Application.prototype.exit = function() {
  native.sendRuntimeMessage('tizen://exit');
};

Application.prototype.hide = function() {
  native.sendRuntimeMessage('tizen://hide');
};

Application.prototype.getRequestedAppControl = function() {
  var result = native.callSync('Application_getRequestedAppControl', {});

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  } else {
    result = native.getResultObject(result);
    if (result) {
      return new RequestedApplicationControl(result);
    } else {
      return null;
    }
  }
};

// class ApplicationInformation ////////////////////////////////////////////////////

function ApplicationInformation(data) {
  var size;
  var sizeException;

  function sizeGetter() {
    xwalk.utils.checkPrivilegeAccess(Privilege.APPLICATION_INFO);
    if (undefined === size) {
      var callArgs = { packageId : this.packageId }; // jshint ignore:line
      var result = native.callSync('ApplicationInformation_getSize', callArgs);

      if (native.isFailure(result)) {
        sizeException = native.getErrorObject(result);
        size = 0;
      } else {
        size = native.getResultObject(result).size;
      }
    }

    if (undefined !== sizeException) {
      throw sizeException;
    }

    return size;
  }

  Object.defineProperties(this, {
    id : {
      value : data.id,
      writable : false,
      enumerable : true
    },
    name : {
      value : data.name,
      writable : false,
      enumerable : true
    },
    iconPath : {
      value : data.iconPath,
      writable : false,
      enumerable : true
    },
    version : {
      value : data.version,
      writable : false,
      enumerable : true
    },
    show : {
      value : data.show,
      writable : false,
      enumerable : true
    },
    categories : {
      value : data.categories,
      writable : false,
      enumerable : true
    },
    installDate : {
      value : new Date(data.installDate),
      writable : false,
      enumerable : true
    },
    size : {
      enumerable : true,
      set : function() {
      },
      get : sizeGetter
    },
    packageId : {
      value : data.packageId,
      writable : false,
      enumerable : true
    }
  });
}

// class ApplicationContext ////////////////////////////////////////////////////

function ApplicationContext(data) {
  Object.defineProperties(this, {
    id    : {value: data.id, writable: false, enumerable: true},
    appId : {value: data.appId, writable: false, enumerable: true}
  });
}

// class ApplicationControlData ////////////////////////////////////////////////////

tizen.ApplicationControlData = function(k, v) {
  AV.validateConstructorCall(this, tizen.ApplicationControlData);

  var valid = (arguments.length >= 2) && T.isArray(v);

  var key;
  function keySetter(k) {
    key = Converter.toString(k);
  }
  if (valid) {
    keySetter(k);
  }

  var value;
  function valueSetter(v) {
    if (T.isArray(v)) {
      value = [];
      for (var i = 0; i < v.length; ++i) {
        value.push(Converter.toString(v[i]));
      }
    }
  }
  if (valid) {
    valueSetter(v);
  }

  Object.defineProperties(this, {
    key : {
      enumerable : true,
      set : keySetter,
      get : function() {
        return key;
      }
    },
    value : {
      enumerable : true,
      set : valueSetter,
      get : function() {
        return value;
      }
    }
  });
}

// class ApplicationControl ////////////////////////////////////////////////////

tizen.ApplicationControl = function(o, u, m, c, d, mode) {
  AV.validateConstructorCall(this, tizen.ApplicationControl);

  var valid = (arguments.length >= 1);

  var operation;
  function operationSetter(o) {
    operation = Converter.toString(o);
  }
  if (valid) {
    operationSetter(o);
  }

  var uri;
  function uriSetter(u) {
    if (T.isNull(u)) {
      uri = u;
    } else {
      uri = Converter.toString(u);
    }
  }
  if (valid) {
    uriSetter(T.isUndefined(u) ? null : u);
  }

  var mime;
  function mimeSetter(m) {
    if (T.isNull(m)) {
      mime = m;
    } else {
      mime = Converter.toString(m);
    }
  }
  if (valid) {
    mimeSetter(T.isUndefined(m) ? null : m);
  }

  var category;
  function categorySetter(c) {
    if (T.isNull(c)) {
      category = c;
    } else {
      category = Converter.toString(c);
    }
  }
  if (valid) {
    categorySetter(T.isUndefined(c) ? null : c);
  }

  var data;
  function dataSetter(d) {
    if (T.isArray(d)) {
      for (var i = 0; i < d.length; ++i) {
        if (!(d[i] instanceof tizen.ApplicationControlData)) {
          return;
        }
      }
      data = d;
    }
  }
  if (valid) {
    dataSetter(T.isNullOrUndefined(d) ? [] : d);
  }

  var launchMode;
  function launchModeSetter(mode) {
    if (ApplicationControlLaunchMode[mode]) {
       launchMode = ApplicationControlLaunchMode[mode];
    }
  }
  if (valid) {
    if (T.isNullOrUndefined(mode) || !ApplicationControlLaunchMode[mode]) {
      launchMode = ApplicationControlLaunchMode['SINGLE'];
    } else {
      launchModeSetter(mode);
    }
  }

  Object.defineProperties(this, {
    operation : {
      enumerable : true,
      set : operationSetter,
      get : function() {
        return operation;
      }
    },
    uri : {
      enumerable : true,
      set : uriSetter,
      get : function() {
        return uri;
      }
    },
    mime : {
      enumerable : true,
      set : mimeSetter,
      get : function() {
        return mime;
      }
    },
    category : {
      enumerable : true,
      set : categorySetter,
      get : function() {
        return category;
      }
    },
    data : {
      enumerable : true,
      set : dataSetter,
      get : function() {
        return data;
      }
    },
    launchMode : {
      enumerable : true,
      set : launchModeSetter,
      get : function() {
        return launchMode;
      }
    }
  });
}

// class RequestedApplicationControl ////////////////////////////////////////////////////

function RequestedApplicationControl(data) {
  Object.defineProperties(this, {
    appControl : {
      value: _createApplicationControl(data.appControl),
      writable: false,
      enumerable: true
    },
    callerAppId : {
      value: data.callerAppId,
      writable: false,
      enumerable: true
    }
  });
}

RequestedApplicationControl.prototype.replyResult = function() {
  var args = AV.validateMethod(arguments, [
      {
        name : 'data',
        type : AV.Types.ARRAY,
        values : tizen.ApplicationControlData,
        optional: true,
        nullable: true
      }
  ]);


  var callArgs = {};

  if (args.has.data) {
    callArgs.data = args.data;
  } else {
    callArgs.data = [];
  }

  var result = native.callSync('RequestedApplicationControl_replyResult', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
};

RequestedApplicationControl.prototype.replyFailure = function() {
  var result = native.callSync('RequestedApplicationControl_replyFailure', {});

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
};

// class ApplicationCertificate ////////////////////////////////////////////////////

function ApplicationCertificate(data) {
  Object.defineProperties(this, {
    type : {
      value : data.type,
      writable : false,
      enumerable : true
    },
    value : {
      value : data.value,
      writable : false,
      enumerable : true
    }
  });
}

// class ApplicationMetaData ////////////////////////////////////////////////////
function ApplicationMetaData(data) {
  Object.defineProperties(this, {
    key : {
      value : data.key,
      writable : false,
      enumerable : true
    },
    value : {
      value : data.value,
      writable : false,
      enumerable : true
    }
  });
}

// exports ////////////////////////////////////////////////////
exports = new ApplicationManager();
