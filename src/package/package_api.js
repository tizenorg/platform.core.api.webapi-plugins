// Package

var validator_ = xwalk.utils.validator;
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
    if (result['result']) {
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
  SetReadOnlyProperty(obj, 'id'); // read only property
  SetReadOnlyProperty(obj, 'name'); // read only property
  SetReadOnlyProperty(obj, 'iconPath'); // read only property
  SetReadOnlyProperty(obj, 'version'); // read only property
  SetReadOnlyProperty(obj, 'totalSize'); // read only property
  SetReadOnlyProperty(obj, 'dataSize'); // read only property
  SetReadOnlyProperty(obj, 'lastModified'); // read only property
  SetReadOnlyProperty(obj, 'author'); // read only property
  SetReadOnlyProperty(obj, 'description'); // read only property
  SetReadOnlyProperty(obj, 'appIds'); // read only property

  return obj;
}

function PackageManager() {
  // constructor of PackageManager
}


PackageManager.prototype.install = function(packageFileURI, progressCallback) {
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
              result.informationArray[i] = PackageInformation(result.informationArray[i]);
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
    return PackageInformation(syncResult);
  } catch (e) {
    throw e;
  }
};

PackageManager.prototype.setPackageInfoEventListener = function(eventCallback) {
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
            args.eventCallback.oninstalled(PackageInformation(result.info));
          } else if (result.status == 'updated') {
            args.eventCallback.onupdated(PackageInformation(result.info));
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

