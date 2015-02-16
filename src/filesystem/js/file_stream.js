// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


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
