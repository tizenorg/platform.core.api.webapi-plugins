// Download

var validator_ = xwalk.utils.validator;
var types_ = validator_.Types;
var check_ = xwalk.utils.type;


var callbackId = 0;
var callbacks = {};
var requests = {};


extension.setMessageListener(function(json) {

  var result = JSON.parse(json);
  var callback = callbacks[result['callbackId']];
  //console.log("PostMessage received: " + result.status);

  if (result.status == 'progress') {
    var receivedSize = result.receivedSize;
    var totalSize = result.totalSize;
    callback.onprogress(result.callbackId, receivedSize, totalSize);
  }
  else if (result.status == 'paused') {
    callback.onpaused(result.callbackId);
  }
  else if (result.status == 'canceled') {
    callback.oncanceled(result.callbackId);
  }
  else if (result.status == 'completed') {
    var fullPath = result.fullPath;
    callback.oncompleted(result.callbackId, fullPath);
  }
  else if (result.status == 'error') {
    callback.onfailed(
        result.callbackId, new tizen.WebAPIError(result['error'].name, result['error'].message));
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
      throw new WebAPIException(err.name, err.message);
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

var DownloadState = {
  'QUEUED': 'QUEUED',
  'DOWNLOADING': 'DOWNLOADING',
  'PAUSED': 'PAUSED',
  'CANCELED': 'CANCELED',
  'COMPLETED': 'COMPLETED',
  'FAILED': 'FAILED'
};

var DownloadNetworkType = {
  'CELLULAR': 'CELLULAR',
  'WIFI': 'WIFI',
  'ALL': 'ALL'
};

tizen.DownloadRequest = function(url, destination, fileName, networkType, httpHeader) {
  validator_.isConstructorCall(this, tizen.DownloadRequest);
  var args = validator_.validateArgs(arguments, [
    {'name' : 'url', 'type': types_.STRING, 'nullable': false, 'optional': false},
    {'name' : 'destination', 'type': types_.STRING, 'nullable': true, 'optional': true},
    {'name' : 'fileName', 'type': types_.STRING, 'nullable': true, 'optional': true},
    {'name' : 'networkType', 'type': types_.ENUM, 'values': ['CELLULAR', 'WIFI', 'ALL'],
    'nullable' : true, 'optional': true},
    {'name' : 'httpHeader', 'type': types_.Dictionary, 'nullable': true, 'optional': true}
  ]);

  var url_ = url;
  var networkType_;

  if (networkType === undefined) networkType_ = 'ALL';
  else if (networkType in DownloadNetworkType) networkType_ = networkType;

  Object.defineProperties(this, {
    'url': { enumerable: true,
      get: function() { return url_;},
      set: function(value) { if (value != null) { url_ = value; }} },
    'destination': { writable: true, enumerable: true,
      value: destination === undefined ? '' : destination },
    'fileName': { writable: true, enumerable: true,
      value: fileName === undefined ? '' : fileName },
    'networkType': { enumerable: true,
      get: function() { return networkType_;},
      set: function(value) {
        if (value === null || value in DownloadNetworkType) { networkType_ = value; }} },
    'httpHeader': { writable: true, enumerable: true,
      value: httpHeader === undefined ? {} : httpHeader }
  });
};


function DownloadManager() {
  // constructor of DownloadManager
}

DownloadManager.prototype.start = function(downloadRequest) {
  var args = validator_.validateArgs(arguments, [
    {'name' : 'downloadRequest', 'type': types_.PLATFORM_OBJECT, 'values': tizen.DownloadRequest},
    {'name' : 'downloadCallback', 'type': types_.LISTENER,
      'values' : ['onprogress', 'onpaused', 'oncanceled', 'oncompleted', 'onfailed'],
      optional: true, nullable: true}
  ]);

  var nativeParam = {
    'url': args.downloadRequest.url,
    'destination': args.downloadRequest.destination,
    'fileName': args.downloadRequest.fileName,
    'networkType': args.downloadRequest.networkType,
    'httpHeader': args.downloadRequest.httpHeader,
    'callbackId': nextCallbackId()
  };

  if (args.downloadCallback) {
    this.setListener(nativeParam.callbackId, args.downloadCallback);
  }

  try {
    var syncResult = callNative('DownloadManager_start', nativeParam);
  } catch (e) {
    throw e;
  }

  requests[nativeParam.callbackId] = args.downloadRequest;

  return nativeParam.callbackId;
};

DownloadManager.prototype.cancel = function(downloadId) {
  var args = validator_.validateArgs(arguments, [
    {name: 'downloadId', type: types_.LONG, 'nullable': false, 'optional': false}
  ]);

  var nativeParam = {
    'downloadId': args.downloadId
  };

  if (typeof requests[downloadId] === 'undefined')
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
        'the identifier does not match any download operation in progress');

  try {
    var syncResult = callNative('DownloadManager_cancel', nativeParam);
  } catch (e) {
    throw e;
  }
};

DownloadManager.prototype.pause = function(downloadId) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'downloadId', 'type': types_.LONG, 'nullable': false, 'optional': false}
  ]);

  var nativeParam = {
    'downloadId': args.downloadId
  };

  if (typeof requests[downloadId] === 'undefined')
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
        'the identifier does not match any download operation in progress');

  try {
    var syncResult = callNative('DownloadManager_pause', nativeParam);
  } catch (e) {
    throw e;
  }
};

DownloadManager.prototype.resume = function(downloadId) {
  var args = validator_.validateArgs(arguments, [
    {'name' : 'downloadId', 'type': types_.LONG, 'nullable': false, 'optional': false}
  ]);

  var nativeParam = {
    'downloadId': args.downloadId
  };

  if (typeof requests[downloadId] === 'undefined')
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
        'the identifier does not match any download operation in progress');

  try {
    var syncResult = callNative('DownloadManager_resume', nativeParam);
  } catch (e) {
    throw e;
  }
};

DownloadManager.prototype.getState = function(downloadId) {
  var args = validator_.validateArgs(arguments, [
    {'name' : 'downloadId', 'type': types_.LONG, 'nullable': false, 'optional': false}
  ]);

  var nativeParam = {
    'downloadId': args.downloadId
  };

  if (typeof requests[downloadId] === 'undefined')
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
        'the identifier does not match any download operation in progress');

  try {
    var syncResult = callNative('DownloadManager_getState', nativeParam);
  } catch (e) {
    throw e;
  }

  return syncResult;
};

DownloadManager.prototype.getDownloadRequest = function(downloadId) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'downloadId', 'type': types_.LONG, 'nullable': false, 'optional': false}
  ]);

  if (typeof requests[downloadId] === 'undefined')
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
        'the identifier does not match any download operation in progress');

  return requests[args.downloadId];
};

DownloadManager.prototype.getMIMEType = function(downloadId) {
  var args = validator_.validateArgs(arguments, [
    {'name' : 'downloadId', 'type': types_.LONG, 'nullable': false, 'optional': false}
  ]);

  var nativeParam = {
    'downloadId': args.downloadId
  };

  if (typeof requests[downloadId] === 'undefined')
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
        'the identifier does not match any download operation in progress');

  try {
    var syncResult = callNative('DownloadManager_getMIMEType', nativeParam);
  } catch (e) {
    throw e;
  }

  return syncResult;
};

DownloadManager.prototype.setListener = function(downloadId, downloadCallback) {
  var args = validator_.validateArgs(arguments, [
    {'name' : 'downloadId', 'type': types_.LONG},
    {'name' : 'downloadCallback', 'type': types_.LISTENER,
      'values' : ['onprogress', 'onpaused', 'oncanceled', 'oncompleted', 'onfailed']}
  ]);

  callbacks[args.downloadId] = args.downloadCallback;
};



exports = new DownloadManager();

