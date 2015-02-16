// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


function File(data) {
  function fileSizeGetter() {
    var _realPath = commonFS_.toRealPath(this.fullPath);
    var _result = native_.callSync('File_statSync', {location: _realPath});
    var _aStatObj = native_.getResultObject(_result);
    return _aStatObj.isFile ? _aStatObj.size : undefined;
  }

  Object.defineProperties(this, {
    parent: {
      value: (function(data) {
        try {
          if (data.parent) { // prevent recursive - only one parent
            var _parentPath = data.path.substr(0, data.path.length - 1);
            var _location = {location: commonFS_.toRealPath(_parentPath)};
            var _result = native_.callSync('File_statSync', _location);
            var _statObj = native_.getResultObject(_result);
            var _info = commonFS_.getFileInfo(_parentPath, _statObj, true);
            return new File(_info);
          } else {
            return null;
          }
        } catch (err) {
          console.log(err.name, err.message);
          return null;
        }
      }(data)),
      writable: false,
      enumerable: true
    },
    readOnly: {value: data.readOnly, writable: false, enumerable: true},
    isFile: {value: data.isFile, writable: false, enumerable: true},
    isDirectory: {value: data.isDirectory, writable: false, enumerable: true},
    created: {value: data.created, writable: false, enumerable: true},
    modified: {value: data.modified, writable: false, enumerable: true},
    path: {value: data.path, writable: false, enumerable: true},
    name: {value: data.name, writable: false, enumerable: true},
    fullPath: {value: data.fullPath, writable: false, enumerable: true},
    fileSize: {enumerable: true, set: function() {
    }, get: fileSizeGetter},
    length: {value: data.length, writable: false, enumerable: true},
    mode: {value: data.mode, writable: false},
    f_isSubDir: {value: function(fullPathToCheck) {
      return (-1 !== fullPathToCheck.indexOf(commonFS_.toRealPath(this.fullPath)));
    }, writable: false},
    f_isCorrectRelativePath: {value: function(relativePath) {
      return ((-1 === relativePath.indexOf('/')) &&
          (-1 === relativePath.indexOf('\\')) &&
          (-1 === relativePath.indexOf('?')) &&
          (-1 === relativePath.indexOf('*')) &&
          (-1 === relativePath.indexOf(':')) &&
          (-1 === relativePath.indexOf('"')) &&
          (-1 === relativePath.indexOf('<')) &&
          (-1 === relativePath.indexOf('>')));
    }}
  });
}

File.prototype.toURI = function() {
  return 'file://' + commonFS_.toRealPath(this.fullPath);
};

File.prototype.listFiles = function(onsuccess, onerror, filter) {
  var args = validator_.validateArgs(arguments, [
    {name: 'onsuccess', type: types_.FUNCTION},
    {name: 'onerror', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'filter', type: types_.DICTIONARY, optional: true, nullable: true}
  ]);

  var data = {
    filter: args.filter
  };

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.onerror, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.onsuccess);
  };

  native_.call('File_listFiles', data, callback);
};

File.prototype.openStream = function(mode, onsuccess, onerror, encoding) {
  var args = validator_.validateArgs(arguments, [
    {name: 'mode', type: types_.ENUM, values: ['r', 'rw', 'w', 'a']},
    {name: 'onsuccess', type: types_.FUNCTION},
    {name: 'onerror', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'encoding', type: types_.STRING, optional: true, nullable: true}
  ]);

  var data = {
    mode: args.mode,
    encoding: args.encoding
  };

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.onerror, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.onsuccess, new FileStream(native_.getResultObject(result)));
  };

  native_.call('File_openStream', data, callback);
};

File.prototype.readAsText = function(onsuccess, onerror, encoding) {
  var args = validator_.validateArgs(arguments, [
    {name: 'onsuccess', type: types_.FUNCTION},
    {name: 'onerror', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'encoding', type: types_.STRING, optional: true, nullable: true}
  ]);

  var data = {
    encoding: args.encoding
  };

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.onerror, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.onsuccess);
  };

  native_.call('File_readAsText', data, callback);
};

File.prototype.copyTo = function(originFilePath, destinationFilePath, overwrite, onsuccess, onerror) {
  var args = validator_.validateArgs(arguments, [
    {name: 'originFilePath', type: types_.STRING},
    {name: 'destinationFilePath', type: types_.STRING},
    {name: 'overwrite', type: types_.BOOLEAN},
    {name: 'onsuccess', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'onerror', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var data = {
    originFilePath: args.originFilePath,
    destinationFilePath: args.destinationFilePath,
    overwrite: args.overwrite
  };

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.onerror, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.onsuccess);
  };

  native_.call('File_copyTo', data, callback);
};

File.prototype.moveTo = function(originFilePath, destinationFilePath, overwrite, onsuccess, onerror) {
  var args = validator_.validateArgs(arguments, [
    {name: 'originFilePath', type: types_.STRING},
    {name: 'destinationFilePath', type: types_.STRING},
    {name: 'overwrite', type: types_.BOOLEAN},
    {name: 'onsuccess', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'onerror', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var data = {
    originFilePath: args.originFilePath,
    destinationFilePath: args.destinationFilePath,
    overwrite: args.overwrite
  };

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.onerror, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.onsuccess);
  };

  native_.call('File_moveTo', data, callback);
};

File.prototype.createDirectory = function(dirPath) {
  var args = validator_.validateArgs(arguments, [
    {name: 'dirPath', type: types_.STRING}
  ]);

  var data = {
    dirPath: args.dirPath
  };

  var result = native_.callSync('File_createDirectory', data);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  var returnObject = new File(native_.getResultObject(result));
  return returnObject;

};

File.prototype.createFile = function(relativeFilePath) {
  var args = validator_.validateArgs(arguments, [
    {name: 'relativeFilePath', type: types_.STRING}
  ]);

  var data = {
    relativeFilePath: args.relativeFilePath
  };

  var result = native_.callSync('File_createFile', data);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  var returnObject = new File(native_.getResultObject(result));
  return returnObject;

};

File.prototype.resolve = function(filePath) {
  var args = validator_.validateArgs(arguments, [
    {name: 'filePath', type: types_.STRING}
  ]);

  var data = {
    filePath: args.filePath
  };

  var result = native_.callSync('File_resolve', data);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  var returnObject = new File(native_.getResultObject(result));
  return returnObject;

};

File.prototype.deleteDirectory = function(directoryPath, recursive, onsuccess, onerror) {
  var args = validator_.validateArgs(arguments, [
    {name: 'directoryPath', type: types_.STRING},
    {name: 'recursive', type: types_.BOOLEAN},
    {name: 'onsuccess', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'onerror', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var data = {
    directoryPath: args.directoryPath,
    recursive: args.recursive
  };

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.onerror, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.onsuccess);
  };

  native_.call('File_deleteDirectory', data, callback);

};

File.prototype.deleteFile = function(filePath, onsuccess, onerror) {
  var args = validator_.validateArgs(arguments, [
    {name: 'filePath', type: types_.STRING},
    {name: 'onsuccess', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'onerror', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var data = {
    filePath: args.filePath
  };

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.onerror, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.onsuccess);
  };

  native_.call('File_deleteFile', data, callback);
};
