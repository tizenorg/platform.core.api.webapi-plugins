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
var privilege_ = xwalk.utils.privilege;
var types_ = validator_.Types;


var callbackId = 0;
var callbacks = {};
var infoEventListenerId = -1;

function invokeListener(result) {
  if (result.listener === 'infoEvent') {
    var listener = callbacks[infoEventListenerId];
    listener(result);
  }
}

extension.setMessageListener(function(json) {
  var result = JSON.parse(json);

  if (result.hasOwnProperty('listener')) {
    invokeListener(result);
  } else {
    var callback = callbacks[result['callbackId']];
    callback(result);
  }
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
    throw new WebAPIException(WebAPIException.UNKNOWN_ERR);
  }

  if (result['status'] == 'success') {
    if (undefined !== result['result']) {
      return result['result'];
    }
    return true;
  } else if (result['status'] == 'error') {
    var err = result['error'];
    if (err) {
      throw new WebAPIException(err);
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
  if (arguments.length > 2)
    Object.defineProperty(
        obj, n, {value: v, writable: false, enumerable: true, configurable: true});
  else
    Object.defineProperty(obj, n, {writable: false, enumerable: true, configurable: true});
}

function PackageInformation(obj) {
  var lastModified = obj.lastModified;
  obj.lastModified = new Date(lastModified);

  SetReadOnlyProperty(this, 'id', obj.id); // read only property
  SetReadOnlyProperty(this, 'name', obj.name); // read only property
  SetReadOnlyProperty(this, 'iconPath', obj.iconPath); // read only property
  SetReadOnlyProperty(this, 'version', obj.version); // read only property
  SetReadOnlyProperty(this, 'lastModified', obj.lastModified); // read only property
  SetReadOnlyProperty(this, 'author', obj.author); // read only property
  SetReadOnlyProperty(this, 'description', obj.description); // read only property
  SetReadOnlyProperty(this, 'appIds', obj.appIds); // read only property

  var totalSize;
  var dataSize;

  Object.defineProperty(this, 'totalSize', {
    enumerable: true,
    set: function() {},
    get: function() {
      if (undefined === totalSize) {
        try {
          totalSize = callNative('PackageManager_getTotalSize', {id: this.id});
        } catch (e) {
          totalSize = -1;
        }
      }
      return totalSize;
    }
  });

  Object.defineProperty(this, 'dataSize', {
    enumerable: true,
    set: function() {},
    get: function() {
      if (undefined === dataSize) {
        try {
          dataSize = callNative('PackageManager_getDataSize', {id: this.id});
        } catch (e) {
          dataSize = -1;
        }
      }
      return dataSize;
    }
  });
}

function PackageManager() {
  // constructor of PackageManager
}


PackageManager.prototype.install = function(packageFileURI, progressCallback) {
  xwalk.utils.checkPrivilegeAccess(privilege_.PACKAGEMANAGER_INSTALL);

  var args = validator_.validateArgs(arguments, [
    {'name' : 'packageFileURI', 'type' : types_.STRING},
    {'name' : 'progressCallback',
      'type' : types_.LISTENER,
      'values' : ['onprogress', 'oncomplete']},
    {'name' : 'errorCallback', 'type' : types_.FUNCTION, 'optional' : true, 'nullable' : true}
  ]);

  var nativeParam = {
    'packageFileURI': args.packageFileURI
  };

  try {
    var syncResult = callNativeWithCallback(
        'PackageManager_install',
        nativeParam,
        function(result) {
          if (result.status == 'progress') {
            args.progressCallback.onprogress(result.id, result.progress);
          } else if (result.status == 'complete') {
            args.progressCallback.oncomplete(result.id);
          } else if (result.status == 'error') {
            var err = result['error'];
            if (err) {
              args.errorCallback(new WebAPIException(err));
              return;
            }
          }

          if (result.status == 'complete' || result.status == 'error') {
            delete callbacks[result['callbackId']];
          }
        });
  } catch (e) {
    throw e;
  }
};

PackageManager.prototype.uninstall = function(id, progressCallback) {
  xwalk.utils.checkPrivilegeAccess(privilege_.PACKAGEMANAGER_INSTALL);

  var args = validator_.validateArgs(arguments, [
    {'name' : 'id', 'type' : types_.STRING},
    {'name' : 'progressCallback',
      'type' : types_.LISTENER,
      'values' : ['onprogress', 'oncomplete']},
    {'name' : 'errorCallback', 'type' : types_.FUNCTION, 'optional' : true, 'nullable' : true}
  ]);

  var nativeParam = {
    'id': args.id
  };

  try {
    var syncResult = callNativeWithCallback(
        'PackageManager_uninstall',
        nativeParam,
        function(result) {
          if (result.status == 'progress') {
            args.progressCallback.onprogress(result.id, result.progress);
          } else if (result.status == 'complete') {
            args.progressCallback.oncomplete(result.id);
          } else if (result.status == 'error') {
            var err = result['error'];
            if (err) {
              args.errorCallback(new WebAPIException(err));
              return;
            }
          }

          if (result.status == 'complete' || result.status == 'error') {
            delete callbacks[result['callbackId']];
          }
        });
  } catch (e) {
    throw e;
  }

};

PackageManager.prototype.getPackagesInfo = function(successCallback, errorCallback) {
  xwalk.utils.checkPrivilegeAccess(privilege_.PACKAGE_INFO);

  var args = validator_.validateArgs(arguments, [
    {'name' : 'successCallback', 'type' : types_.FUNCTION},
    {'name' : 'errorCallback', 'type' : types_.FUNCTION, 'optional' : true, 'nullable' : true}
  ]);

  var nativeParam = {
  };

  try {
    var syncMsg = callNativeWithCallback(
        'PackageManager_getPackagesInfo',
        nativeParam,
        function(result) {
          if (result.status == 'success') {
            for (var i = 0; i < result.informationArray.length; i++) {
              result.informationArray[i] = new PackageInformation(result.informationArray[i]);
            }
            args.successCallback(result.informationArray);
          } else if (result.status == 'error') {
            var err = result['error'];
            if (err) {
              args.errorCallback(new tizen.WebAPIError(err.name, err.message));
              return;
            }
          }

          delete callbacks[result['callbackId']];
        });
  } catch (e) {
    throw e;
  }
};

PackageManager.prototype.getPackageInfo = function() {
  xwalk.utils.checkPrivilegeAccess(privilege_.PACKAGE_INFO);

  var args = validator_.validateArgs(arguments, [
    {'name': 'id', 'type': types_.STRING, 'optional' : true, 'nullable' : true}
  ]);

  var nativeParam = {
  };

  if (args['id']) {
    nativeParam['id'] = args.id;
  }

  try {
    var syncResult = callNative('PackageManager_getPackageInfo', nativeParam);
    return new PackageInformation(syncResult);
  } catch (e) {
    throw e;
  }
};

PackageManager.prototype.setPackageInfoEventListener = function(eventCallback) {
  xwalk.utils.checkPrivilegeAccess(privilege_.PACKAGE_INFO);

  var args = validator_.validateArgs(
      arguments,
      [
        {'name' : 'eventCallback',
          'type' : types_.LISTENER,
          'values' : ['oninstalled', 'onupdated', 'onuninstalled']}
      ]);

  var nativeParam = {
  };

  try {
    var syncResult = callNativeWithCallback(
        'PackageManager_setPackageInfoEventListener',
        nativeParam,
        function(result) {
          if (result.status == 'installed') {
            args.eventCallback.oninstalled(new PackageInformation(result.info));
          } else if (result.status == 'updated') {
            args.eventCallback.onupdated(new PackageInformation(result.info));
          } else if (result.status == 'uninstalled') {
            args.eventCallback.onuninstalled(result.id);
          }
        });

    if (infoEventListenerId === -1) {
      infoEventListenerId = nativeParam.callbackId;
    } else {
      delete callbacks[infoEventListenerId];
      infoEventListenerId = nativeParam.callbackId;
    }

  } catch (e) {
    throw e;
  }
};

PackageManager.prototype.unsetPackageInfoEventListener = function() {
  xwalk.utils.checkPrivilegeAccess(privilege_.PACKAGE_INFO);

  var nativeParam = {
  };

  try {
    var syncResult = callNative('PackageManager_unsetPackageInfoEventListener', nativeParam);
    if (syncResult === true) {
      delete callbacks[infoEventListenerId];
      infoEventListenerId = -1;
    }
  } catch (e) {
    throw e;
  }
};

exports = new PackageManager();

