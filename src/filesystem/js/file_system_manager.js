// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function FileSystemStorage(data) {
  Object.defineProperties(this, {
    label: {value: data.label, writable: false, enumerable: true},
    type: {value: data.type, writable: false, enumerable: true},
    state: {value: data.state, writable: false, enumerable: true}
  });
}

var PATH_MAX = 4096;

function FileSystemManager() {
  Object.defineProperties(this, {
    maxPathLength: {value: PATH_MAX, writable: false, enumerable: true},
    _isWidgetPathFound: {value: false, writable: true}
  });
}

FileSystemManager.prototype.resolve = function(location, onsuccess, onerror, mode) {
  var args = validator_.validateArgs(arguments, [
    {name: 'location', type: types_.STRING},
    {name: 'onsuccess', type: types_.FUNCTION},
    {name: 'onerror', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'mode', type: types_.ENUM, values: Object.keys(FileMode), optional: true, nullable: true}
  ]);

  if (!args.has.mode) {
    args.mode = 'rw';
  }
  commonFS_.initCache(this);

  if (args.location[0] === '/') {
    setTimeout(function() {
      native_.callIfPossible(args.onerror,
          new tizen.WebAPIException(tizen.WebAPIException.NOT_FOUND_ERR,
          'Global path without \'file://\' prefix is not valid.'));
    }, 0);
    return;
  }
  var _realPath = commonFS_.toRealPath(args.location);
  var _isLocationAllowed = commonFS_.isLocationAllowed(_realPath);

  if (args.mode !== 'r' && !_isLocationAllowed) {
    setTimeout(function() {
      native_.callIfPossible(args.onerror,
          new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR,
          'Provided arguments are not valid.'));
    }, 0);
    return;
  }

  var data = {
    location: _realPath
  };

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.onerror, native_.getErrorObject(result));
      return;
    }

    var aStatObj = native_.getResultObject(result);

    var _result;
    var _path = (args.location.indexOf('file://') === 0) ?
            commonFS_.toVirtualPath(args.location) : args.location;
    if (_path[_path.length - 1] === '/') {
      _path = _path.substr(0, _path.length - 1);
    }
    _result = commonFS_.getFileInfo(_path, aStatObj, false, args.mode);
    if (_result.readOnly && args.mode !== 'r') {
      throw new tizen.WebAPIException(tizen.WebAPIException.IO_ERR);
    } else {
      native_.callIfPossible(args.onsuccess, new File(_result));
    }
  };

  native_.call('File_stat', data, callback);
};

FileSystemManager.prototype.getStorage = function(label, onsuccess, onerror) {
  var args = validator_.validateArgs(arguments, [
    {name: 'label', type: types_.STRING},
    {name: 'onsuccess', type: types_.FUNCTION},
    {name: 'onerror', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  commonFS_.initCache(this);

  var cachedObj = commonFS_.cacheVirtualToReal[args.label];
  if (undefined === cachedObj) {
    setTimeout(function() {
      native_.callIfPossible(args.onerror,
          new tizen.WebAPIException(tizen.WebAPIException.NOT_FOUND_ERR,
          'Storage not found.'));
    }, 0);
  } else {
    setTimeout(function() {
      var storage = new FileSystemStorage({
        label: args.label,
        type: cachedObj.type,
        state: cachedObj.state
      });
      native_.callIfPossible(args.onsuccess, storage);
    }, 0);
  }
};

FileSystemManager.prototype.listStorages = function(onsuccess, onerror) {
  var args = validator_.validateArgs(arguments, [
    {name: 'onsuccess', type: types_.FUNCTION},
    {name: 'onerror', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  commonFS_.initCache(this);

  var _storages = [];

  for (var _storage in commonFS_.cacheVirtualToReal) {
    var storageObj = commonFS_.cacheVirtualToReal[_storage];
    _storages.push(new FileSystemStorage({
      label: _storage,
      type: storageObj.type ? storageObj.type : FileSystemStorageType.INTERNAL,
      state: storageObj.state ? storageObj.state : FileSystemStorageState.MOUNTED
    }));
  }

  setTimeout(function() {args.onsuccess(_storages);}, 0);

};

var callbackId = 0;
var callbacks = {};

function nextCallbackId() {
  return callbackId++;
}

function _StorageStateChangeListener(result) {
  var storage = new FileSystemStorage(native_.getResultObject(result));
  for (var id in callbacks) {
    if (callbacks.hasOwnProperty(id)) {
      native_.callIfPossible(callbacks[id].onsuccess, storage);
    }
  }
}

FileSystemManager.prototype.addStorageStateChangeListener = function(onsuccess, onerror) {
  var args = validator_.validateArgs(arguments, [
    {name: 'onsuccess', type: types_.FUNCTION},
    {name: 'onerror', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var register = false;
  if (type_.isEmptyObject(callbacks)) {
    register = true;
  }

  var id = nextCallbackId();
  callbacks[id] = args.onsuccess;

  if (register) {
    native_.addListener('StorageStateChangeListener', _StorageStateChangeListener);

    var result = native_.callSync('FileSystemManager_addStorageStateChangeListener', {});

    if (native_.isFailure(result)) {
      native_.callIfPossible(args.onerror, native_.getErrorObject(result));
      return;
    }
  }

  return id;
};

FileSystemManager.prototype.removeStorageStateChangeListener = function(watchId) {
  var args = validator_.validateArgs(arguments, [
    {name: 'watchId', type: types_.LONG}
  ]);

  var id = args.watchId;

  if (type_.isNullOrUndefined(callbacks[id])) {
    throw new tizen.WebAPIException(tizen.WebAPIException.NOT_FOUND_ERR, 'Watch ID not found.');
  }

  delete callbacks[id];

  if (type_.isEmptyObject(callbacks)) {
    native_.callSync('FileSystemManager_removeStorageStateChangeListener', id);
  }
};

exports = new FileSystemManager();
