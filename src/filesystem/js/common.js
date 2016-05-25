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

var tizen24home = "/opt/usr/media";

//this variable need to match same variable in common/filesystem/filesystem_provider_storage.cc
var kVirtualRootImages = "images";

var commonFS_ = (function() {
  var cacheReady = false;
  var listenerRegistered = false;
  var cacheVirtualToReal = {};
  var cacheStorages = [];
  var uriPrefix = 'file://';
  // special condition for previous versions paths
  // (global paths usage issue workaround)
  var isAppForEarlierVersion = privUtils_.isAppVersionEarlierThan("3.0");
  var homeDir = undefined;

  function clearCache() {
    cacheVirtualToReal = {};
    cacheStorages = [];
    cacheReady = false;
  }

  // initalize home directory for correct mapping global paths from tizen 2.4
  // (global paths usage issue workaround)
  function initHomeDir(aPath) {
    if (homeDir || !isAppForEarlierVersion) {
      return;
    }
    var imagesPath = cacheVirtualToReal[kVirtualRootImages].path;

    if (imagesPath[imagesPath.length-1] === "/") {
      homeDir = imagesPath.split("/").slice(0, -2).join("/");
    } else {
      homeDir = imagesPath.split("/").slice(0, -1).join("/");
    }
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
    // initalize home directory for correct mapping global paths from tizen 2.4
    // (global paths usage issue workaround)
    initHomeDir();

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

  function removeDotsFromPath(str) {
    if(str === undefined){
        return str;
    }

    var _pathTokens = str.split('/');
    var _correctDir = [];
    var _fileRealPath = _pathTokens[0];
    _correctDir.push(_pathTokens[0]);
    for (var i = 1; i < _pathTokens.length; ++i) {
      if(_pathTokens[i] == "..") {
        if (_fileRealPath == '') {
          _fileRealPath = undefined;
          break;
        }
        var _lastDir = _correctDir.pop();
        _fileRealPath = _fileRealPath.substring(0, _fileRealPath.length - _lastDir.length - 1);
      } else if(_pathTokens[i] != "."){
        _fileRealPath += '/' + _pathTokens[i];
        _correctDir.push(_pathTokens[i]);
      }
    }
    return _fileRealPath;
  }

  function checkPathWithoutDots(aPath) {
    if (-1 !== aPath.indexOf('/../')) {
      return false;
    }
    if (-1 !== aPath.indexOf('/./')) {
      return false;
    }
    // check if path ends with '/.' or '/..'
    if (aPath.match(/\/\.\.?$/)) {
      return false;
    }
    // check if path starts with './' or '../'
    if (aPath.match(/^\.\.?\//)) {
      return false;
    }
    return true;
  }

  function convertForEarlierVersionPath(aPath) {
    if (isAppForEarlierVersion) {
      if (aPath && aPath.indexOf(tizen24home) === 0) {
        console.log("Converting 2.4 style path to 3.0 pattern");
        aPath = homeDir + aPath.substr(tizen24home.length);
      }
    }
    return aPath;
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
    // this line makes that '.' and '..' is supported in paths, but each method handle those cases
    // and return error (see commonFS_.checkPathWithoutDots() method)
    _fileRealPath = removeDotsFromPath(_fileRealPath);
    // convert path to be compatibile with previous version of Tizen
    // (global paths usage issue workaround)
    _fileRealPath = convertForEarlierVersionPath(_fileRealPath);
    // if path is valid try to cut last '/' if it is present
    if (_fileRealPath) {
      _fileRealPath = mergeMultipleSlashes(_fileRealPath);
      var lastCharIndex = _fileRealPath.length-1;
      if (_fileRealPath[lastCharIndex] === '/') {
        _fileRealPath = _fileRealPath.substr(0,lastCharIndex);
      }
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
    checkPathWithoutDots: checkPathWithoutDots,
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
