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

function stripPemString(str) {
  // remove new line characters
  // remove BEGIN and END lines
  return str.replace(/(\r\n|\r|\n)/g, '').replace(/-----[^-]*-----/g, '');
}

function InternalData(data) {
  if (!(this instanceof InternalData)) {
    return new InternalData(data);
  }

  for (var key in data) {
    if (data.hasOwnProperty(key)) {
      this[key] = data[key];
    }
  }
}

function updateInternalData(internal, data) {
  var values = InternalData(data);

  for (var key in data) {
    if (values.hasOwnProperty(key) && internal.hasOwnProperty(key)) {
      internal[key] = values;
    }
  }
}

function Key(name, password, extractable, keyType, rawKey) {
  var _internal = {
    name: converter.toString(name),
    password: (password ? converter.toString(password) : null),
    extractable: !!extractable,  // make sure it is boolean
    keyType: (KeyType.hasOwnProperty(keyType) ? keyType : KeyType.KEY_NONE),
    rawKey: (rawKey ? converter.toString(rawKey) : '')
  };

  Object.defineProperties(this, {
    name: {
      get: function () { return _internal.name; },
      set: function () {},
      enumerable: true
    },
    password: {
      get: function () { return _internal.password; },
      set: function (value) {
        if (value instanceof InternalData) {
          _internal.password = value.password;
        }
      },
      enumerable: true
    },
    extractable: {
      get: function () { return _internal.extractable; },
      set: function () {},
      enumerable: true
    },
    keyType: {
      get: function () { return _internal.keyType; },
      set: function (value) {
        if (value instanceof InternalData && KeyType.hasOwnProperty(value.keyType)) {
          _internal.keyType = value.keyType;
        }
      },
      enumerable: true
    },
    rawKey: {
      get: function () { return _internal.rawKey; },
      set: function (value) {
        if (value instanceof InternalData) {
          _internal.rawKey = value.rawKey;
        }
      },
      enumerable: true
    }
  });
}

Key.prototype.save = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.KEYMANAGER);
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

  var that = this;

  native.call('KeyManager_saveKey', {
    key: this,
    rawKey: stripPemString(args.rawKey)
  }, function(msg) {
    if (native.isFailure(msg)) {
      if (type.isFunction(args.errorCallback)) {
        args.errorCallback(native.getErrorObject(msg));
      }
    } else {
      updateInternalData(that, {rawKey: stripPemString(args.rawKey), keyType: msg.keyType});
      native.callIfPossible(args.successCallback);
    }
  });
};

Key.prototype.remove = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.KEYMANAGER);
  var ret = native.callSync('KeyManager_removeAlias', {
    alias: this.name
  });
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
};

function Certificate(name, password, extractable, rawCert) {
  var _internal = {
    name: converter.toString(name),
    password: (password ? converter.toString(password) : null),
    extractable: !!extractable,  // make sure it is boolean
    rawCert: (rawCert ? converter.toString(rawCert) : '')
  };

  Object.defineProperties(this, {
    name: {
      get: function () { return _internal.name; },
      set: function () {},
      enumerable: true
    },
    password: {
      get: function () { return _internal.password; },
      set: function (value) {
        if (value instanceof InternalData) {
          _internal.password = value.password;
        }
      },
      enumerable: true
    },
    extractable: {
      get: function () { return _internal.extractable; },
      set: function () {},
      enumerable: true
    },
    rawCert: {
      get: function () { return _internal.rawCert; },
      set: function (value) {
        if (value instanceof InternalData) {
          _internal.rawCert = value.rawCert;
        }
      },
      enumerable: true
    }
  });
}

Certificate.prototype.save = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.KEYMANAGER);
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

  var that = this;

  native.call('KeyManager_saveCertificate', {
    certificate: this,
    rawCert: stripPemString(args.rawCert)
  }, function(msg) {
    if (native.isFailure(msg)) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(msg));
    } else {
      updateInternalData(that, {rawCert: args.rawCert});
      native.callIfPossible(args.successCallback);
    }
  });
};

Certificate.prototype.loadFromFile = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.KEYMANAGER);
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

  var that = this;

  native.call('KeyManager_loadCertificateFromFile', {
    certificate: this,
    fileURI: args.fileURI,
    password: args.password
  }, function(msg) {
    if (native.isFailure(msg)) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(msg));
    } else {
      updateInternalData(that, {password: args.password, rawCert: native.getResultObject(msg)});
      native.callIfPossible(args.successCallback);
    }
  });
};

Certificate.prototype.remove = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.KEYMANAGER);
  var ret = native.callSync('KeyManager_removeAlias', {
    alias: this.name
  });
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
};

function Data(name, password, extractable, rawData) {
  var _internal = {
    name: converter.toString(name),
    password: (password ? converter.toString(password) : null),
    extractable: !!extractable, // make sure it is boolean
    rawData: (rawData ? converter.toString(rawData) : '')
  };

  Object.defineProperties(this, {
    name: {
      get: function () { return _internal.name; },
      set: function () {},
      enumerable: true
    },
    password: {
      get: function () { return _internal.password; },
      set: function (value) {
        if (value instanceof InternalData) {
          _internal.password = value.password;
        }
      },
      enumerable: true
    },
    extractable: {
      get: function () { return _internal.extractable; },
      set: function () {},
      enumerable: true
    },
    rawData: {
      get: function () { return _internal.rawData; },
      set: function (value) {
        if (value instanceof InternalData) {
          _internal.rawData = value.rawData;
        }
      },
      enumerable: true
    }
  });
}

Data.prototype.save = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.KEYMANAGER);
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

  var that = this;

  native.call('KeyManager_saveData', {
    data: this,
    rawData: args.rawData
  }, function(msg) {
    if (native.isFailure(msg)) {
      if (type.isFunction(args.errorCallback)) {
        args.errorCallback(native.getErrorObject(msg));
      }
    } else {
      updateInternalData(that, {rawData: args.rawData});
      native.callIfPossible(args.successCallback);
    }
  });
};

Data.prototype.remove = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.KEYMANAGER);
  var ret = native.callSync('KeyManager_removeAlias', {
    alias: this.name
  });
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
};

function KeyManager() {

}

KeyManager.prototype.generateKeyPair = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.KEYMANAGER);
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
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.KEYMANAGER);
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
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.KEYMANAGER);
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
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.KEYMANAGER);
  var ret = native.callSync('KeyManager_getKeyAliasList', {});
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
  return native.getResultObject(ret);
};

KeyManager.prototype.getCertificate = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.KEYMANAGER);
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
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.KEYMANAGER);
  var ret = native.callSync('KeyManager_getCertificatesAliasList', {});
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
  return native.getResultObject(ret);
};

KeyManager.prototype.getData = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.KEYMANAGER);
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
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.KEYMANAGER);
  var ret = native.callSync('KeyManager_getDataAliasList', {});
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
  return native.getResultObject(ret);
};

KeyManager.prototype.allowAccessControl = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.KEYMANAGER);
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

  var ret = native.callSync('KeyManager_isDataNameFound', {dataName : args.dataName});
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }

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
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.KEYMANAGER);
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

  var ret = native.callSync('KeyManager_isDataNameFound', {dataName : args.dataName});
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }

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
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.KEYMANAGER);
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
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.KEYMANAGER);
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
    xwalk.utils.validator.isConstructorCall(this, tizen.Key);
    Key.call(this, name, password, extractable, KeyType.KEY_NONE, "");
};
tizen.Key.prototype = Object.create(Key.prototype);
tizen.Certificate = function(name, password, extractable) {
    xwalk.utils.validator.isConstructorCall(this, tizen.Certificate);
    Certificate.call(this, name, password, extractable, "");
};
tizen.Certificate.prototype = Object.create(Certificate.prototype);
tizen.Data = function(name, password, extractable) {
    xwalk.utils.validator.isConstructorCall(this, tizen.Data);
    Data.call(this, name, password, extractable, "");
};
tizen.Data.prototype = Object.create(Data.prototype);

exports = new KeyManager();
