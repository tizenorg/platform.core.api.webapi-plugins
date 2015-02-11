/* global tizen, xwalk, extension */

// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


var utils_ = xwalk.utils;
var type_ = utils_.type;
var converter_ = utils_.converter;
var validator_ = utils_.validator;
var types_ = validator_.Types;
var native_ = new xwalk.utils.NativeManager(extension);

var callbackId = 0;
var callbacks = {};

function nextCallbackId() {
  return callbackId++;
}


function SetReadOnlyProperty(obj, n, v) {
  Object.defineProperty(obj, n, {value: v, writable: false});
}

var FileMode = {
  r: 'r',
  rw: 'rw',
  w: 'w',
  a: 'a'
};
var FileSystemStorageType = {
  INTERNAL: 'INTERNAL',
  EXTERNAL: 'EXTERNAL'
};
var FileSystemStorageState = {
  MOUNTED: 'MOUNTED',
  REMOVED: 'REMOVED',
  UNMOUNTABLE: 'UNMOUNTABLE'
};


function FileSystemManager() {
  SetReadOnlyProperty(this, 'maxPathLength', null);
}


FileSystemManager.prototype.resolve = function(location, onsuccess, onerror, mode) {
  var args = validator_.validateArgs(arguments, [
    {name: 'location', type: types_.STRING},
    {name: 'onsuccess', type: types_.FUNCTION},
    {name: 'onerror', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'mode', type: types_.ENUM, values: ['r', 'rw', 'w', 'a'], optional: true, nullable: true}
  ]);

  var data = {
    location: args.location,
    mode: args.mode
  };

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.onerror, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.onsuccess);
  };

  native_.call('FileSystemManager_resolve', data, callback);
};

FileSystemManager.prototype.getStorage = function(label, onsuccess, onerror) {
  var args = validator_.validateArgs(arguments, [
    {name: 'label', type: types_.STRING},
    {name: 'onsuccess', type: types_.FUNCTION},
    {name: 'onerror', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var data = {
    label: args.label
  };

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.onerror, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.onsuccess);
  };

  native_.call('FileSystemManager_getStorage', data, callback);
};

FileSystemManager.prototype.listStorages = function(onsuccess, onerror) {
  var args = validator_.validateArgs(arguments, [
    {name: 'onsuccess', type: types_.FUNCTION},
    {name: 'onerror', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.onerror, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.onsuccess);
  };

  native_.call('FileSystemManager_listStorages', {}, callback);
};

FileSystemManager.prototype.addStorageStateChangeListener = function(onsuccess, onerror) {
  var args = validator_.validateArgs(arguments, [
    {name: 'onsuccess', type: types_.FUNCTION},
    {name: 'onerror', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.onerror, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.onsuccess);
  };

  native_.call('FileSystemManager_addStorageStateChangeListener', {}, callback);
};

FileSystemManager.prototype.removeStorageStateChangeListener = function(watchId) {
  var args = validator_.validateArgs(arguments, [
    {name: 'watchId', type: types_.LONG}
  ]);

  var data = {
    watchId: args.watchId
  };

  var result = native_.callSync('FileSystemManager_removeStorageStateChangeListener', data);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};



function File() {
  Object.defineProperties(this, {
    position: {
      get: function() {},
      set: function() {},
      enumerable: true
    },
    readOnly: {
      get: function() {},
      set: function() {},
      enumerable: true
    },
    isFile: {
      get: function() {},
      set: function() {},
      enumerable: true
    },
    isDirectory: {
      get: function() {},
      set: function() {},
      enumerable: true
    },
    created: {
      get: function() {},
      set: function() {},
      enumerable: true
    },
    modified: {
      get: function() {},
      set: function() {},
      enumerable: true
    },
    path: {
      get: function() {},
      set: function() {},
      enumerable: true
    },
    name: {
      get: function() {},
      set: function() {},
      enumerable: true
    },
    fullPath: {
      get: function() {},
      set: function() {},
      enumerable: true
    },
    length: {
      get: function() {},
      set: function() {},
      enumerable: true
    }
  });
}


File.prototype.toURI = function() {
  var result = native_.callSync('File_toURI', {});

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  return native_.getResultObject(result);
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


function FileStream(fileDescriptor, nodeMode, nodeEncoding) {
  Object.defineProperties(this, {
    position: {
      get: function() {},
      enumerable: true,
      writable: false
    },
    eof: {
      get: function() {},
      set: function() {},
      enumerable: true
    },
    bytesAvailable: {
      get: function() {},
      enumerable: true,
      writable: false
    }
  });
}

FileStream.prototype.close = function() {
  var result = native_.callSync('FileStream_close', {});

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

FileStream.prototype.read = function() {
  var args = validator_.validateArgs(arguments, [
    {
      name: 'charCount',
      type: types_.LONG
    }
  ]);

  if (args.charCount <= 0) {
    throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR,
        'Argument "charCount" must be greater than 0');
  }

  var result = native_.callSync('FileStream_read', {});

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

FileStream.prototype.readBytes = function() {
  var args = validator_.validateArgs(arguments, [
    {
      name: 'byteCount',
      type: types_.LONG
    }
  ]);

  if (args.byteCount <= 0) {
    throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR,
        'Argument "byteCount" must be greater than 0');
  }

  var result = native_.callSync('FileStream_readBytes', {});

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

FileStream.prototype.readBase64 = function() {
  var args = validator_.validateArgs(arguments, [
    {
      name: 'byteCount',
      type: types_.LONG
    }
  ]);

  if (args.byteCount <= 0) {
    throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR,
        'Argument "byteCount" must be greater than 0');
  }

  var result = native_.callSync('FileStream_readBase64', {});

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

FileStream.prototype.write = function() {
  var args = validator_.validateArgs(arguments, [
    {
      name: 'stringData',
      type: types_.STRING
    }
  ]);

  var result = native_.callSync('FileStream_write', {});

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

FileStream.prototype.writeBytes = function() {
  var args = validator_.validateArgs(arguments, [
    {
      name: 'byteData',
      type: types_.ARRAY,
      values: types_.OCTET
    }
  ]);

  var result = native_.callSync('FileStream_writeBytes', {});

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

FileStream.prototype.writeBase64 = function() {
  var args = validator_.validateArgs(arguments, [
    {
      name: 'base64Data',
      type: types_.STRING
    }
  ]);

  var result = native_.callSync('FileStream_writeBase64', {});

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};


exports = new FileSystemManager();
