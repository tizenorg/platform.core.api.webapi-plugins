// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var validator = xwalk.utils.validator;
var converter = xwalk.utils.converter;
var type = xwalk.utils.type;
var native = new xwalk.utils.NativeManager(extension);

var KeyPairType = {
  "RSA": "RSA",
  "ECDSA": "ECDSA",
  "DSA": "DSA"
};

var KeyStrength = {
  "1024": "1024",
  "2048": "2048",
  "4096": "4096"
};

var EllipticCurveType = {
  "EC_PRIME192V1": "EC_PRIME192V1",
  "EC_PRIME256V1": "EC_PRIME256V1",
  "EC_SECP384R1": "EC_SECP384R1"
};

var HashAlgorithmType = {
  "HASH_NONE": "HASH_NONE",
  "HASH_SHA1": "HASH_SHA1",
  "HASH_SHA256": "HASH_SHA256",
  "HASH_SHA384": "HASH_SHA384",
  "HASH_SHA512": "HASH_SHA512"
};

var RSAPaddingAlgorithm = {
  "PADDING_PKCS1": "PADDING_PKCS1",
  "PADDING_X931": "PADDING_X931"
};

var KeyType = {
  "KEY_NONE": "KEY_NONE",
  "KEY_RSA_PUBLIC": "KEY_RSA_PUBLIC",
  "KEY_RSA_PRIVATE": "KEY_RSA_PRIVATE",
  "KEY_ECDSA_PUBLIC": "KEY_ECDSA_PUBLIC",
  "KEY_ECDSA_PRIVATE": "KEY_ECDSA_PRIVATE",
  "KEY_DSA_PUBLIC": "KEY_DSA_PUBLIC",
  "KEY_DSA_PRIVATE": "KEY_DSA_PRIVATE",
  "KEY_AES": "KEY_AES"
};

var AccessControlType = {
  "READ": "READ",
  "READ_REMOVE": "READ_REMOVE"
};

function Key(name, password, extractable, keyType, rawKey) {
  Object.defineProperties(this, {
    name: {
      value: converter.toString(name),
      enumerable: true
    },
    password: {
      value: password ? converter.toString(password) : null,
      enumerable: true
    },
    extractable: {
      value: !!extractable,//make sure it is boolean
      enumerable: true
    },
    keyType: {
      value: KeyType.hasOwnProperty(keyType) ? keyType : KeyType.KEY_NONE,
      enumerable: true
    },
    rawKey: {
      value: converter.toString(rawKey),
      enumerable: true
    }
  });
}

Key.prototype.save = function() {
  var args = validator.validateArgs(arguments, [
    {
      name: 'rawKey',
      type: validator.Types.STRING,
      optional: true
    },
    {
      name: 'successCallback',
      type: validator.Types.FUNCTION,
      nullable: true
    },
    {
      name: 'errorCallback',
      type: validator.Types.FUNCTION,
      optional: true,
      nullable: true
    }
  ]);

  native.call('KeyManager_saveKey', {
    key: this,
    rawKey: args.rawKey
  }, function(msg) {
    if (native.isFailure(msg)) {
      if (type.isFunction(args.errorCallback)) {
        args.errorCallback(native.getErrorObject(msg));
      }
    } else {
      native.callIfPossible(args.successCallback);
    }
  });
};

Key.prototype.remove = function() {
  var ret = native.callSync('KeyManager_removeKey', {
    key: this
  });
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
};

function Certificate(name, password, extractable, rawCert) {
  Object.defineProperties(this, {
    name: {
      value: converter.toString(name),
      enumerable: true
    },
    password: {
      value: password ? converter.toString(password) : null,
      enumerable: true
    },
    extractable: {
      value: !!extractable,//make sure it is boolean
      enumerable: true
    },
    rawCert: {
      value: rawCert ? converter.toString(rawCert) : "",
      enumerable: true
    }
  });
}

Certificate.prototype.save = function() {
  var args = validator.validateArgs(arguments, [
    {
      name: 'rawCert',
      type: validator.Types.STRING
    },
    {
      name: 'successCallback',
      type: validator.Types.FUNCTION,
      nullable: true
    },
    {
      name: 'errorCallback',
      type: validator.Types.FUNCTION,
      optional: true,
      nullable: true
    }
  ]);

  native.call('KeyManager_saveCertificate', {
    certificate: this,
    rawCert: args.rawCert
  }, function(msg) {
    if (native.isFailure(msg)) {
      if (type.isFunction(args.errorCallback)) {
        args.errorCallback(native.getErrorObject(msg));
      }
    } else {
      native.callIfPossible(args.successCallback);
    }
  });
};

Certificate.prototype.loadFromFile = function() {
  var args = validator.validateArgs(arguments, [
    {
      name: 'fileURI',
      type: validator.Types.STRING
    },
    {
      name: 'successCallback',
      type: validator.Types.FUNCTION,
      nullable: true
    },
    {
      name: 'errorCallback',
      type: validator.Types.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'password',
      type: validator.Types.STRING,
      optional: true
    }
  ]);

  native.call('KeyManager_loadCertificateFromFile', {
    certificate: this,
    fileURI: args.fileURI,
    password: args.password
  }, function(msg) {
    if (native.isFailure(msg)) {
      if (type.isFunction(args.errorCallback)) {
        args.errorCallback(native.getErrorObject(msg));
      }
    } else {
      native.callIfPossible(args.successCallback);
    }
  });
};

Certificate.prototype.remove = function() {
  var ret = native.callSync('KeyManager_removeCertificate', {
    certificate: this
  });
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
};

function Data(name, password, extractable, rawData) {
  Object.defineProperties(this, {
    name: {
      value: converter.toString(name),
      enumerable: true
    },
    password: {
      value: password ? converter.toString(password) : null,
      enumerable: true
    },
    extractable: {
      value: !!extractable,//make sure it is boolean
      enumerable: true
    },
    rawData: {
      value: rawData ? converter.toString(rawData) : "",
      enumerable: true
    }
  });
}

Data.prototype.save = function() {
  var args = validator.validateArgs(arguments, [
    {
      name: 'rawData',
      type: validator.Types.STRING
    },
    {
      name: 'successCallback',
      type: validator.Types.FUNCTION,
      nullable: true
    },
    {
      name: 'errorCallback',
      type: validator.Types.FUNCTION,
      optional: true,
      nullable: true
    }
  ]);

  native.call('KeyManager_saveData', {
    data: this,
    rawData: args.rawData
  }, function(msg) {
    if (native.isFailure(msg)) {
      if (type.isFunction(args.errorCallback)) {
        args.errorCallback(native.getErrorObject(msg));
      }
    } else {
      native.callIfPossible(args.successCallback);
    }
  });
};

Data.prototype.remove = function() {
  var ret = native.callSync('KeyManager_removeData', {
    data: this
  });
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
};

function KeyManager() {

}

KeyManager.prototype.generateKeyPair = function() {
  var args = validator.validateArgs(arguments, [
    {
      name: "privKeyName",
      type: validator.Types.PLATFORM_OBJECT,
      values: Key
    },
    {
      name: "pubKeyName",
      type: validator.Types.PLATFORM_OBJECT,
      values: Key
    },
    {
      name: 'type',
      type: validator.Types.ENUM,
      values: Object.keys(KeyPairType)
    },
    {
      name: 'size',
      type: validator.Types.ENUM,
      values: Object.keys(KeyStrength)
    },
    {
      name: 'successCallback',
      type: validator.Types.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'errorCallback',
      type: validator.Types.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'ellipticCurveType',
      type: validator.Types.ENUM,
      values: Object.keys(EllipticCurveType),
      optional: true,
      nullable: true
    }
  ]);

  native.call('KeyManager_generateKeyPair', {
    privKeyName: args.privKeyName,
    pubKeyName: args.pubKeyName,
    type: args.type,
    size: args.size,
    ellipticCurveType: args.ellipticCurveType ? args.ellipticCurveType : null
  }, function(msg) {
    if (native.isFailure(msg)) {
      if (type.isFunction(args.errorCallback)) {
        args.errorCallback(native.getErrorObject(msg));
      }
    } else {
      native.callIfPossible(args.successCallback);
    }
  });
};

KeyManager.prototype.loadFromPKCS12File = function() {
  var args = validator.validateArgs(arguments, [
    {
      name: 'fileURI',
      type: validator.Types.STRING
    },
    {
      name: 'privKeyName',
      type: validator.Types.STRING
    },
    {
      name: 'certificateName',
      type: validator.Types.STRING
    },
    {
      name: 'successCallback',
      type: validator.Types.FUNCTION,
      nullable: true
    },
    {
      name: 'errorCallback',
      type: validator.Types.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'password',
      type: validator.Types.STRING,
      optional: true
    }
  ]);

  native.call('KeyManager_loadFromPKCS12File', {
    fileURI: args.fileURI,
    privKeyName: args.privKeyName,
    certificateName: args.certificateName,
    password: args.password ? args.password : null
  }, function(msg) {
    if (native.isFailure(msg)) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(msg));
    } else {
      native.callIfPossible(args.successCallback);
    }
  });
};

KeyManager.prototype.getKey = function() {
  var args = validator.validateArgs(arguments, [
    {
      name: "name",
      type: validator.Types.STRING
    },
    {
      name: "password",
      type: validator.Types.STRING,
      nullable: true,
      optional: true
    }
  ]);
  var ret = native.callSync('KeyManager_getKey', {
    name: args.name,
    password: args.password
  });
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
  var result = native.getResultObject(ret);
  return new Key(result.name, result.password, result.extractable, result.keyType, result.rawKey);
};

KeyManager.prototype.getKeyAliasList = function() {
  var ret = native.callSync('KeyManager_getKeyAliasList', {});
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
  return native.getResultObject(ret);
};

KeyManager.prototype.getCertificate = function() {
  var args = validator.validateArgs(arguments, [
    {
      name: "name",
      type: validator.Types.STRING
    },
    {
      name: "password",
      type: validator.Types.STRING,
      optional: true
    }
  ]);
  var ret = native.callSync('KeyManager_getCertificate', {
    name: args.name,
    password: args.password
  });
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
  var result = native.getResultObject(ret);
  return new Certificate(result.name, result.password, result.extractable, result.rawCert);
};

KeyManager.prototype.getCertificatesAliasList = function() {
  var ret = native.callSync('KeyManager_getCertificatesAliasList', {});
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
  return native.getResultObject(ret);
};

KeyManager.prototype.getData = function() {
  var args = validator.validateArgs(arguments, [
    {
      name: "name",
      type: validator.Types.STRING
    },
    {
      name: "password",
      type: validator.Types.STRING,
      optional: true
    }
  ]);
  var ret = native.callSync('KeyManager_getData', {
    name: args.name,
    password: args.password
  });
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
  var result = native.getResultObject(ret);
  return new Data(result.name, result.password, result.extractable, result.rawData);
};

KeyManager.prototype.getDataAliasList = function() {
  var ret = native.callSync('KeyManager_getDataAliasList', {});
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
  return native.getResultObject(ret);
};

KeyManager.prototype.allowAccessControl = function() {
  var args = validator.validateArgs(arguments, [
    {
      name: "dataName",
      type: validator.Types.STRING
    },
    {
      name: "id",
      type: validator.Types.STRING
    },
    {
      name: 'accessControlType',
      type: validator.Types.ENUM,
      values: Object.keys(AccessControlType)
    },
    {
      name: 'successCallback',
      type: validator.Types.FUNCTION,
      nullable: true
    },
    {
      name: 'errorCallback',
      type: validator.Types.FUNCTION,
      optional: true,
      nullable: true
    }
  ]);
  native.call('KeyManager_allowAccessControl', {
    dataName: args.dataName,
    id: args.id,
    accessControlType: args.accessControlType
  }, function(msg) {
    if (native.isFailure(msg)) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(msg));
    } else {
      native.callIfPossible(args.successCallback);
    }
  });
};

KeyManager.prototype.denyAccessControl = function() {
  var args = validator.validateArgs(arguments, [
    {
      name: "dataName",
      type: validator.Types.STRING
    },
    {
      name: "id",
      type: validator.Types.STRING
    },
    {
      name: 'successCallback',
      type: validator.Types.FUNCTION,
      nullable: true
    },
    {
      name: 'errorCallback',
      type: validator.Types.FUNCTION,
      optional: true,
      nullable: true
    }
  ]);
  native.call('KeyManager_denyAccessControl', {
    dataName: args.dataName,
    id: args.id
  }, function(msg) {
    if (native.isFailure(msg)) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(msg));
    } else {
      native.callIfPossible(args.successCallback);
    }
  });
};

KeyManager.prototype.createSignature = function() {
  var args = validator.validateArgs(arguments, [
    {
      name: "message",
      type: validator.Types.STRING
    },
    {
      name: "privKeyAlias",
      type: validator.Types.STRING
    },
    {
      name: 'hashAlgorithmType',
      type: validator.Types.ENUM,
      values: Object.keys(HashAlgorithmType)
    },
    {
      name: 'padding',
      type: validator.Types.ENUM,
      values: Object.keys(RSAPaddingAlgorithm)
    },
    {
      name: 'password',
      type: validator.Types.STRING,
      optional: true
    },
    {
      name: 'successCallback',
      type: validator.Types.FUNCTION,
      nullable: true
    },
    {
      name: 'errorCallback',
      type: validator.Types.FUNCTION,
      optional: true,
      nullable: true
    }
  ]);

  native.call('KeyManager_createSignature', {
    message: args.message,
    privKeyAlias: args.privKeyAlias,
    hashAlgorithmType: args.hashAlgorithmType,
    padding: args.padding,
    password: args.password ? args.password : null
  }, function(msg) {
    if (native.isFailure(msg)) {
      if (type.isFunction(args.errorCallback)) {
        args.errorCallback(native.getErrorObject(msg));
      }
    } else {
      native.callIfPossible(args.successCallback, native.getResultObject(msg));
    }
  });
};

KeyManager.prototype.verifySignature = function() {
  var args = validator.validateArgs(arguments, [
    {
      name: "signature",
      type: validator.Types.STRING
    },
    {
      name: "message",
      type: validator.Types.STRING
    },
    {
      name: 'pubKeyAlias',
      type: validator.Types.STRING
    },
    {
      name: 'hashAlgorithmType',
      type: validator.Types.ENUM,
      values: Object.keys(HashAlgorithmType)
    },
    {
      name: 'padding',
      type: validator.Types.ENUM,
      values: Object.keys(RSAPaddingAlgorithm)
    },
    {
      name: 'password',
      type: validator.Types.STRING,
      optional: true
    },
    {
      name: 'successCallback',
      type: validator.Types.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'errorCallback',
      type: validator.Types.FUNCTION,
      optional: true,
      nullable: true
    }
  ]);

  native.call('KeyManager_verifySignature', {
    signature: args.signature,
    message: args.message,
    pubKeyAlias: args.pubKeyAlias,
    hashAlgorithmType: args.hashAlgorithmType,
    padding: args.padding,
    password: args.password ? args.password : null
  }, function(msg) {
    if (native.isFailure(msg)) {
      if (type.isFunction(args.errorCallback)) {
        args.errorCallback(native.getErrorObject(msg));
      }
    } else {
      native.callIfPossible(args.successCallback);
    }
  });
};

// expose only basic constructors
tizen.Key = function(name, password, extractable) {
    Key.call(this, name, password, extractable, KeyType.KEY_NONE, "");
};
tizen.Key.prototype = Object.create(Key.prototype);
tizen.Certificate = function(name, password, extractable) {
    Certificate.call(this, name, password, extractable, "");
};
tizen.Certificate.prototype = Object.create(Certificate.prototype);
tizen.Data = function(name, password, extractable) {
    Data.call(this, name, password, extractable, "");
};
tizen.Data.prototype = Object.create(Data.prototype);

exports = new KeyManager();
