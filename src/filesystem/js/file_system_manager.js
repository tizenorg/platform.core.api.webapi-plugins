// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var _PRIVILEGE_FILESYSTEM_READ = 'http://tizen.org/privilege/filesystem.read';
var _PRIVILEGE_FILESYSTEM_WRITE = 'http://tizen.org/privilege/filesystem.write';

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
    maxPathLength: {value: PATH_MAX, writable: false, enumerable: true}
  });
}

FileSystemManager.prototype.resolve = function(location, onsuccess, onerror, mode) {
  xwalk.utils.checkPrivilegeAccess(_PRIVILEGE_FILESYSTEM_READ);

  var args = validator_.validateArgs(arguments, [
    {name: 'location', type: types_.STRING},
    {name: 'onsuccess', type: types_.FUNCTION},
    {name: 'onerror', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'mode', type: types_.ENUM, values: Object.keys(FileMode), optional: true, nullable: true}
  ]);

  if (!args.has.mode) {
    args.mode = 'rw';
  }
  commonFS_.initCache();

  if (args.location[0] === '/') {
    setTimeout(function() {
      native_.callIfPossible(args.onerror,
          new WebAPIException(WebAPIException.NOT_FOUND_ERR,
          'Global path without \'file://\' prefix is not valid.'));
    }, 0);
    return;
  }
  var _realPath = commonFS_.toRealPath(args.location);
  var _isLocationAllowed = commonFS_.isLocationAllowed(_realPath);

  if (args.mode !== 'r' && !_isLocationAllowed) {
    setTimeout(function() {
      native_.callIfPossible(args.onerror,
          new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
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
    var _result = commonFS_.getFileInfo(aStatObj, false, args.mode);
    if (_result.readOnly && args.mode !== 'r') {
      throw new WebAPIException(WebAPIException.IO_ERR);
    } else {
      native_.callIfPossible(args.onsuccess, new File(_result));
    }
  };

  native_.call('File_stat', data, callback);
};

FileSystemManager.prototype.getStorage = function(label, onsuccess, onerror) {
  xwalk.utils.checkPrivilegeAccess(_PRIVILEGE_FILESYSTEM_READ);

  var args = validator_.validateArgs(arguments, [
    {name: 'label', type: types_.STRING},
    {name: 'onsuccess', type: types_.FUNCTION},
    {name: 'onerror', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  commonFS_.initCache();

  var storage, i;
  setTimeout(function() {
    for (i = 0; i < commonFS_.cacheStorages.length; i++) {
      if (commonFS_.cacheStorages[i].label === args.label) {
        storage = new FileSystemStorage(commonFS_.cacheStorages[i]);
      }
    }

    if (storage === undefined) {
      native_.callIfPossible(args.onerror,
          new WebAPIException(WebAPIException.NOT_FOUND_ERR, 'Storage not found.'));
    } else {
      native_.callIfPossible(args.onsuccess, storage);
    }
  }, 0);
};

FileSystemManager.prototype.listStorages = function(onsuccess, onerror) {
  xwalk.utils.checkPrivilegeAccess(_PRIVILEGE_FILESYSTEM_READ);

  var args = validator_.validateArgs(arguments, [
    {name: 'onsuccess', type: types_.FUNCTION},
    {name: 'onerror', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  commonFS_.initCache();

  var storages = [], i;
  setTimeout(function() {
    for (i = 0; i < commonFS_.cacheStorages.length; i++) {
      storages.push(new FileSystemStorage(commonFS_.cacheStorages[i]));
    }

    native_.callIfPossible(args.onsuccess, storages);
  }, 0);
};

var callbackId = 0;
var callbacks = {};

function nextCallbackId() {
  return callbackId++;
}

function _StorageStateChangeListener(result) {
  var storage = new FileSystemStorage(result);
  for (var id in callbacks) {
    native_.callIfPossible(callbacks[id], storage);
  }
}

FileSystemManager.prototype.addStorageStateChangeListener = function(onsuccess, onerror) {
  xwalk.utils.checkPrivilegeAccess(_PRIVILEGE_FILESYSTEM_WRITE);

  var args = validator_.validateArgs(arguments, [
    {name: 'onsuccess', type: types_.FUNCTION},
    {name: 'onerror', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  commonFS_.initCache();

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
  xwalk.utils.checkPrivilegeAccess(_PRIVILEGE_FILESYSTEM_WRITE);

  var args = validator_.validateArgs(arguments, [
    {name: 'watchId', type: types_.LONG}
  ]);

  if (!arguments.length) {
    throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR,
        'Missing watchId');
  }
  var id = args.watchId;

  if (type_.isNullOrUndefined(callbacks[id])) {
    throw new WebAPIException(WebAPIException.NOT_FOUND_ERR, 'Watch ID not found.');
  }

  delete callbacks[id];

  if (type_.isEmptyObject(callbacks)) {
    native_.callSync('FileSystemManager_removeStorageStateChangeListener', {});
  }
};

var filesystem = new FileSystemManager();

function onStorageStateChanged() {
  commonFS_.clearCache();
  commonFS_.initCache();
}

filesystem.addStorageStateChangeListener(onStorageStateChanged);

exports = filesystem;
