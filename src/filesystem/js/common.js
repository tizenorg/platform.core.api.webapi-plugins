// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


var utils_ = xwalk.utils;
var type_ = utils_.type;
var converter_ = utils_.converter;
var validator_ = utils_.validator;
var types_ = validator_.Types;
var native_ = new xwalk.utils.NativeManager(extension);

function SetReadOnlyProperty(obj, n, v) {
  Object.defineProperty(obj, n, {value: v, writable: false});
}

var FileSystemStorageType = {
  INTERNAL: 'INTERNAL',
  EXTERNAL: 'EXTERNAL'
};

var FileSystemStorageState = {
  MOUNTED: 'MOUNTED',
  REMOVED: 'REMOVED',
  UNMOUNTABLE: 'UNMOUNTABLE'
};

var FileMode = {
  r: 'r',
  rw: 'rw',
  w: 'w',
  a: 'a'
};

function CommonFS() {
  Object.defineProperties(this, {
    PRIVILEGE_FILESYSTEM_READ: {
      value: 'http://tizen.org/privilege/filesystem.read',
      writable: false,
      enumerable: true
    },
    PRIVILEGE_FILESYSTEM_WRITE: {
      value: 'http://tizen.org/privilege/filesystem.write',
      writable: false,
      enumerable: true
    }
  });

  this.cacheVirtualToReal = {};
  this.cacheStorages = [];
}

CommonFS.prototype.getFileInfo = function(aStatObj, secondIter, aMode) {
  var _result = {},
      _pathTokens,
      _fileParentPath = '',
      i;
  var aPath = this.toVirtualPath(aStatObj.path);

  _result.readOnly = aStatObj.readOnly;
  _result.isFile = aStatObj.isFile;
  _result.isDirectory = aStatObj.isDirectory;
  _result.created = new Date(aStatObj.ctime * 1000);
  _result.modified = new Date(aStatObj.mtime * 1000);
  _result.fullPath = aPath;
  _result.fileSize = aStatObj.size;
  _result.mode = aMode;
  if (_result.isDirectory) {
    try {
      _result.length = aStatObj.nlink;
    } catch (err) {
      _result.length = 0;
    }
  } else {
    _result.length = undefined;
  }

  _pathTokens = aPath.split('/');
  if (_pathTokens.length > 1) {
    for (i = 0; i < _pathTokens.length - 1; ++i) {
      _fileParentPath += _pathTokens[i] + '/';
    }
    _result.path = _fileParentPath;
    _result.name = _pathTokens[_pathTokens.length - 1];
    _result.parent = (secondIter) ? null : _fileParentPath;
  } else {
    _result.parent = null;
    _result.path = aPath;
    _result.name = '';
  }
  return _result;
};

CommonFS.prototype.isLocationAllowed = function(aPath) {
  if (!aPath) {
    return false;
  }
  if (aPath.indexOf(this.cacheVirtualToReal.ringtones.path) === 0) {
    return false;
  }
  if (aPath.indexOf(this.cacheVirtualToReal['wgt-package'].path) === 0) {
    return false;
  }

  return true;
};

CommonFS.prototype.f_isSubDir = function(fullPathToCheck, fullPath) {
  return (-1 !== fullPathToCheck.indexOf(this.toRealPath(fullPath)));
};

CommonFS.prototype.f_isCorrectRelativePath = function(relativePath) {
  return ((-1 === relativePath.indexOf('/')) &&
      (-1 === relativePath.indexOf('\\')) &&
      (-1 === relativePath.indexOf('?')) &&
      (-1 === relativePath.indexOf('*')) &&
      (-1 === relativePath.indexOf(':')) &&
      (-1 === relativePath.indexOf('"')) &&
      (-1 === relativePath.indexOf('<')) &&
      (-1 === relativePath.indexOf('>')));
};

CommonFS.prototype.toRealPath = function(aPath) {
  var _fileRealPath = '',
      _uriPrefix = 'file://',
      i;
  if (aPath.indexOf(_uriPrefix) === 0) {
    _fileRealPath = aPath.substr(_uriPrefix.length);
  } else if (aPath[0] !== '/') {
    //virtual path
    var _pathTokens = aPath.split('/');
    if (this.cacheVirtualToReal[_pathTokens[0]] && (
        this.cacheVirtualToReal[_pathTokens[0]].state === undefined ||
        this.cacheVirtualToReal[_pathTokens[0]].state === FileSystemStorageState.MOUNTED)) {
      _fileRealPath = this.cacheVirtualToReal[_pathTokens[0]].path;
      for (i = 1; i < _pathTokens.length; ++i) {
        _fileRealPath += '/' + _pathTokens[i];
      }
    } else {
      //If path token is not present in cache then it is invalid
      _fileRealPath = undefined;
    }
  } else {
    _fileRealPath = aPath;
  }

  return _fileRealPath;
};

CommonFS.prototype.toVirtualPath = function(aPath) {
  var _virtualPath = aPath;
  if (_virtualPath.indexOf('file://') === 0) {
    _virtualPath = _virtualPath.substr('file://'.length);
  }

  for (var virtual_root in this.cacheVirtualToReal) {
    var real_root_path = this.cacheVirtualToReal[virtual_root].path;
    if (aPath.indexOf(real_root_path, 0) === 0) {
      return aPath.replace(real_root_path, virtual_root);
    }
  }

  return aPath;
};

CommonFS.prototype.initCache = function() {
  if (this.cacheStorages.length > 0) {
    return;
  }

  var result = native_.callSync('Filesystem_fetchVirtualRoots', {});
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  var virtualRoots = native_.getResultObject(result);

  for (var i = 0; i < virtualRoots.length; ++i) {
    this.cacheVirtualToReal[virtualRoots[i].name] = {
      path: virtualRoots[i].path,
      type: FileSystemStorageType.INTERNAL,
      state: FileSystemStorageState.MOUNTED
    };
  }

  var result = native_.callSync('FileSystemManager_fetchStorages', {});
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  var storages = native_.getResultObject(result);
  for (var i = 0; i < storages.length; ++i) {
    this.cacheStorages.push({
      label: storages[i].name,
      type: storages[i].type,
      state: storages[i].state,
      storage_id: storages[i].storage_id
    });
  }
};

CommonFS.prototype.clearCache = function() {
  this.cacheVirtualToReal = {};
  this.cacheStorages = [];
};

var commonFS_ = new CommonFS();
