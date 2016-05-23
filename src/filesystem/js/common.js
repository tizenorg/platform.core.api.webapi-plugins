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

var privUtils_ = xwalk.utils;
var privilege_ = privUtils_.privilege;
var type_ = privUtils_.type;
var converter_ = privUtils_.converter;
var validator_ = privUtils_.validator;
var types_ = validator_.Types;
var native_ = new xwalk.utils.NativeManager(extension);

function SetReadOnlyProperty(obj, n, v) {
  Object.defineProperty(obj, n, {
    value: v,
    writable: false
  });
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

var commonFS_ = (function() {
  var cacheReady = false;
  var listenerRegistered = false;
  var cacheVirtualToReal = {};
  var cacheStorages = [];
  var uriPrefix = 'file://';

  function clearCache() {
    cacheVirtualToReal = {};
    cacheStorages = [];
    cacheReady = false;
  }

  function initCache() {
    if (cacheReady) {
      return;
    }

    var result = native_.callSync('Filesystem_fetchVirtualRoots', {});
    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }
    var virtualRoots = native_.getResultObject(result);

    for (var i = 0; i < virtualRoots.length; ++i) {
      cacheVirtualToReal[virtualRoots[i].name] = {
        path: virtualRoots[i].path,
        label: virtualRoots[i].name,
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
      cacheStorages.push({
        path: storages[i].path,
        label: storages[i].name,
        type: storages[i].type,
        state: storages[i].state,
        storage_id: storages[i].storage_id
      });
    }

    if (!listenerRegistered) {
      try {
        tizen.filesystem.addStorageStateChangeListener(function() {
          clearCache();
        });
        listenerRegistered = true;
      } catch (e) {
        console.log('Failed to register storage change listener, '
            + 'storage information may be corrupted: ' + e.message);
      }
    }

    cacheReady = true;
  }

  function mergeMultipleSlashes(str) {
    var retStr = str.replace(/(^(file\:\/\/\/)|^(file\:\/\/)|\/)\/{0,}/g, '$1');
    return retStr;
  }

  function toRealPath(aPath) {
    var _fileRealPath = '';

    aPath = mergeMultipleSlashes(aPath);

    if (aPath.indexOf(uriPrefix) === 0) {
      _fileRealPath = aPath.substr(uriPrefix.length);
    } else if (aPath[0] !== '/') {
      // virtual path
      initCache();

      var _pathTokens = aPath.split('/');

      if (cacheVirtualToReal[_pathTokens[0]]) {
        _fileRealPath = cacheVirtualToReal[_pathTokens[0]].path;
        for (var i = 1; i < _pathTokens.length; ++i) {
          _fileRealPath += '/' + _pathTokens[i];
        }
      } else {
        // If path token is not present in cache then it is invalid
        _fileRealPath = undefined;
        // check storages
        for (var j = 0; j < cacheStorages.length; ++j) {
          if (cacheStorages[j].label === _pathTokens[0]) {
            _fileRealPath = cacheStorages[j].path;
            for (var i = 1; i < _pathTokens.length; ++i) {
              _fileRealPath += '/' + _pathTokens[i];
            }
            break;
          }
        }
      }
    } else {
      _fileRealPath = aPath;
    }

    return _fileRealPath;
  }

  function toVirtualPath(aPath) {
    aPath = mergeMultipleSlashes(aPath);
    var _virtualPath = aPath;

    if (_virtualPath.indexOf(uriPrefix) === 0) {
      _virtualPath = _virtualPath.substr(uriPrefix.length);
    }

    initCache();

    for (var virtual_root in cacheVirtualToReal) {
      var real_root_path = cacheVirtualToReal[virtual_root].path;
      if (_virtualPath.indexOf(real_root_path, 0) === 0) {
        return _virtualPath.replace(real_root_path, virtual_root);
      }
    }

    return _virtualPath;
  }

  function getFileInfo(aStatObj, secondIter, aMode) {
    var _result = {}, _pathTokens, _fileParentPath = '', i;
    var aPath = toVirtualPath(aStatObj.path);

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
      var last = _pathTokens.length - 1;
      var lastToken = '';
      if (_pathTokens[last] === '') {
        // 'abc/d/e/' case with trailing '/' sign
        last = _pathTokens.length - 2;
        lastToken = '/';
      }
      for (i = 0; i < last; ++i) {
        _fileParentPath += _pathTokens[i] + '/';
      }
      if (last > 0) {
        _result.path = _fileParentPath;
        _result.name = (secondIter) ? _pathTokens[last] : _pathTokens[last] + lastToken;
        _result.parent = (secondIter) ? null : _fileParentPath;
      } else {
        // '/' dir case
        _result.path = _pathTokens[last] + lastToken;;
        _result.name = '';
        _result.parent = (secondIter) ? null : _fileParentPath;
      }
    } else {
      _result.parent = null;
      _result.path = aPath;
      _result.name = '';
    }
    return _result;
  }

  function isLocationAllowed(aPath) {
    if (!aPath) {
      return false;
    }
    initCache();
    if (aPath.indexOf(cacheVirtualToReal.ringtones.path) === 0) {
      return false;
    }
    if (aPath.indexOf(cacheVirtualToReal['wgt-package'].path) === 0) {
      return false;
    }

    return true;
  }

  function f_isSubDir(fullPathToCheck, fullPath) {
    var realFullPath = toRealPath(fullPath);
    return ((-1 !== fullPathToCheck.indexOf(realFullPath)) && (fullPathToCheck !== realFullPath));
  };

  function f_isCorrectRelativePath(relativePath) {
    return ((0 !== relativePath.indexOf('/'))
        && (0 !== relativePath.indexOf('\\'))
        && (-1 === relativePath.indexOf('?'))
        && (-1 === relativePath.indexOf('*'))
        && (-1 === relativePath.indexOf(':'))
        && (-1 === relativePath.indexOf('"'))
        && (-1 === relativePath.indexOf('<')) && (-1 === relativePath
        .indexOf('>')));
  };

  function cloneStorage(storage) {
    return {
      label: storage.label,
      type: storage.type,
      state: storage.state
    };
  }

  function getStorage(label) {
    initCache();
    for (var i = 0; i < cacheStorages.length; ++i) {
      if (cacheStorages[i].label === label) {
        return cloneStorage(cacheStorages[i]);
      }
    }

    for (var key in cacheVirtualToReal) {
      if (cacheVirtualToReal.hasOwnProperty(key)) {
        if (cacheVirtualToReal[key].label === label) {
          return cloneStorage(cacheVirtualToReal[key]);
        }
      }
    }

    return null;
  }

  function getAllStorages() {
    var ret = [];
    initCache();
    for (var i = 0; i < cacheStorages.length; ++i) {
      ret.push(cloneStorage(cacheStorages[i]));
    }

    for (var key in cacheVirtualToReal) {
      if (cacheVirtualToReal.hasOwnProperty(key)) {
        ret.push(cloneStorage(cacheVirtualToReal[key]));
      }
    }

    return ret;
  }

  return {
    clearCache: clearCache,
    toRealPath: toRealPath,
    toVirtualPath: toVirtualPath,
    getFileInfo: getFileInfo,
    isLocationAllowed: isLocationAllowed,
    f_isSubDir: f_isSubDir,
    f_isCorrectRelativePath: f_isCorrectRelativePath,
    getStorage: getStorage,
    getAllStorages: getAllStorages
  };
})();