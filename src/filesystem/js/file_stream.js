// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


function FileStream(data, mode, encoding) {
  var _totalBytes = data.fileSize || 0;
  var _position = mode === 'a' ? _totalBytes : 0;

  Object.defineProperties(this, {
    eof: {
      value: false,
      enumerable: true,
      writable: false
    },
    position: {
      get: function() {
        return _position;
      },
      set: function(v) {
        _position = Math.max(0, v);
      },
      enumerable: true
    },
    bytesAvailable: {
      value: this.eof ? -1 : Math.max(0, _totalBytes - _position),
      enumerable: true,
      writable: false
    },
    _mode: {
      value: mode,
      writable: false,
      enumerable: false
    },
    _encoding: {
      value: encoding,
      writable: false,
      enumerable: false
    },
    _file: {
      value: data,
      writable: false,
      enumerable: false
    }
  });
}

FileStream.prototype.close = function() {
  var result = native_.callSync('FileStream_close', {});

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

function _checkReadAccess(mode) {
  if (mode !== 'r' && mode !== 'rw') {
    throw new tizen.WebAPIException(tizen.WebAPIException.IO_ERR, 'Stream is not in read mode.');
  }
}

function _checkWriteAccess(mode) {
  if (mode !== 'a' && mode !== 'w' && mode !== 'rw') {
    throw new tizen.WebAPIException(tizen.WebAPIException.IO_ERR, 'Stream is not in write mode.');
  }
}

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

  _checkReadAccess(this._mode);

  var _count = this.bytesAvailable;

  var data = {
    location: commonFS_.toRealPath(this._file.fullPath),
    offset: this.position || 0,
    length: args.charCount > _count ? _count : args.charCount
  };

  var result = native_.callSync('File_readSync', data);
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  var encoded = native_.getResultObject(result);
  var decoded = Base64.decode(encoded);

  return decoded;
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

  _checkReadAccess(this._mode);

  var _count = this.bytesAvailable;

  var data = {
    location: commonFS_.toRealPath(this._file.fullPath),
    offset: this.position || 0,
    length: args.byteCount > _count ? _count : args.byteCount
  };

  var result = native_.callSync('File_readSync', data);
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  var encoded = native_.getResultObject(result);
  var decoded = Base64.decode(encoded);
  var bytes = [];

  for (var i = 0; i < decoded.length; ++i) {
    bytes.push(decoded.charCodeAt(i));
  }

  return bytes;
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

  _checkReadAccess(this._mode);

  var _count = this.bytesAvailable;

  var data = {
    location: commonFS_.toRealPath(this._file.fullPath),
    offset: this.position || 0,
    length: args.byteCount > _count ? _count : args.byteCount
  };

  var result = native_.callSync('File_readSync', data);
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  var encoded = native_.getResultObject(result);

  return encoded;
};

FileStream.prototype.write = function() {
  var args = validator_.validateArgs(arguments, [
    {
      name: 'stringData',
      type: types_.STRING
    }
  ]);

  _checkWriteAccess(this._mode);

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

  _checkWriteAccess(this._mode);

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

  _checkWriteAccess(this._mode);

  var result = native_.callSync('FileStream_writeBase64', {});

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};
