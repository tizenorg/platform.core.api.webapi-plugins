// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var validator = xwalk.utils.validator;
var converter = xwalk.utils.converter;

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

};

Key.prototype.remove = function() {

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

};

Certificate.prototype.loadFromFile = function() {

};

Certificate.prototype.remove = function() {

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

};

Data.prototype.remove = function() {

};

function KeyManager() {

}

KeyManager.prototype.generateKeyPair = function() {

};

KeyManager.prototype.loadFromPKCS12File = function() {

};

KeyManager.prototype.getKey = function() {

};

KeyManager.prototype.getKeyAliasList = function() {

};

KeyManager.prototype.getCertificate = function() {

};

KeyManager.prototype.getCertificatesAliasList = function() {

};

KeyManager.prototype.getData = function() {

};

KeyManager.prototype.getDataAliasList = function() {

};

KeyManager.prototype.allowAccessControl = function() {

};

KeyManager.prototype.denyAccessControl = function() {

};

KeyManager.prototype.createSignature = function() {

};

KeyManager.prototype.verifySignature = function() {

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