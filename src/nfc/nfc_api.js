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

var validator_ = xwalk.utils.validator;
var types_ = validator_.Types;
var type_ = xwalk.utils.type;
var converter_ = xwalk.utils.converter;
var native_ = new xwalk.utils.NativeManager(extension);


var NDEFRecordTextEncoding = {
  UTF8: 'UTF8',
  UTF16: 'UTF16'
};

var NFCTagType = {
  GENERIC_TARGET: 'GENERIC_TARGET',
  ISO14443_A: 'ISO14443_A',
  ISO14443_4A: 'ISO14443_4A',
  ISO14443_3A: 'ISO14443_3A',
  MIFARE_MINI: 'MIFARE_MINI',
  MIFARE_1K: 'MIFARE_1K',
  MIFARE_4K: 'MIFARE_4K',
  MIFARE_ULTRA: 'MIFARE_ULTRA',
  MIFARE_DESFIRE: 'MIFARE_DESFIRE',
  ISO14443_B: 'ISO14443_B',
  ISO14443_4B: 'ISO14443_4B',
  ISO14443_BPRIME: 'ISO14443_BPRIME',
  FELICA: 'FELICA',
  JEWEL: 'JEWEL',
  ISO15693: 'ISO15693',
  UNKNOWN_TARGET: 'UNKNOWN_TARGET'
};

var CardEmulationMode = {
  ALWAYS_ON: 'ALWAYS_ON',
  OFF: 'OFF'
};

var SecureElementType = {
  ESE: 'ESE',
  UICC: 'UICC',
  HCE: 'HCE'
};

var CardEmulationCategoryType = {
  PAYMENT: 'PAYMENT',
  OTHER: 'OTHER'
};

var HCEEventType = {
  DEACTIVATED: 'DEACTIVATED',
  ACTIVATED: 'ACTIVATED',
  APDU_RECEIVED: 'APDU_RECEIVED'
};

function HCEEventData(data) {
  Object.defineProperties(this, {
    eventType: {
      value: data.eventType,
      writable: false,
      enumerable: true
    },
    apdu: {
      value: data.apdu || [],
      writable: false,
      enumerable: true
    },
    length: {
      value: data.length || 0,
      writable: false,
      enumerable: true
    }
  });
}

function ListenerManager(native, listenerName) {
  this.listeners = {};
  this.nextId = 1;
  this.nativeSet = false;
  this.native = native;
  this.listenerName = listenerName;
}

ListenerManager.prototype.onListenerCalled = function(msg) {
  for (var key in this.listeners) {
    if (this.listeners.hasOwnProperty(key)) {
      if ('CardElement' === msg.type) {
        this.listeners[key](msg.mode);
      } else if ('Transaction' === msg.type) {
        this.listeners[key](msg.aid, msg.data);
      } else if('HCEEventData' === msg.type) {
        var hceData = new HCEEventData(msg.result);
        this.listeners[key](hceData);
      }
    }
  }
};

ListenerManager.prototype.addListener = function(callback) {
  var id = this.nextId;
  if (!this.nativeSet) {
    this.native.addListener(this.listenerName, this.onListenerCalled.bind(this));
    this.nativeSet = true;
  }
  this.listeners[id] = callback;
  ++this.nextId;
  return id;
};

ListenerManager.prototype.removeListener = function(watchId) {
  if (this.listeners.hasOwnProperty(watchId)) {
    delete this.listeners[watchId];
  }
};

var PEER_LISTENER = 'PeerListener';
var RECEIVE_NDEF_LISTENER = 'ReceiveNDEFListener';
var CARD_EMULATION_MODE_LISTENER = 'CardEmulationModeChanged';
var ACTIVE_SECURE_ELEMENT_LISTENER = 'ActiveSecureElementChanged';
var TRANSACTION_EVENT_ESE_LISTENER = 'TransactionEventListener_ESE';
var TRANSACTION_EVENT_UICC_LISTENER = 'TransactionEventListener_UICC';
var HCE_EVENT_LISTENER = 'HCEEventListener';
var TAG_LISTENER = 'TagListener';
var cardEmulationModeListener = new ListenerManager(native_, CARD_EMULATION_MODE_LISTENER);
var activeSecureElementChangeListener = new ListenerManager(native_, ACTIVE_SECURE_ELEMENT_LISTENER);
var transactionEventListenerEse = new ListenerManager(native_, TRANSACTION_EVENT_ESE_LISTENER);
var transactionEventListenerUicc = new ListenerManager(native_, TRANSACTION_EVENT_UICC_LISTENER);
var HCEEventListener = new ListenerManager(native_, HCE_EVENT_LISTENER);


//////////////////NFCManager /////////////////

function NFCManager() {
  Object.defineProperties(this, {
    NFC_RECORD_TNF_EMPTY: {value: 0, writable: false, enumerable: true},
    NFC_RECORD_TNF_WELL_KNOWN: {value: 1, writable: false, enumerable: true},
    NFC_RECORD_TNF_MIME_MEDIA: {value: 2, writable: false, enumerable: true},
    NFC_RECORD_TNF_URI: {value: 3, writable: false, enumerable: true},
    NFC_RECORD_TNF_EXTERNAL_RTD: {value: 4, writable: false, enumerable: true},
    NFC_RECORD_TNF_UNKNOWN: {value: 5, writable: false, enumerable: true},
    NFC_RECORD_TNF_UNCHANGED: {value: 6, writable: false, enumerable: true}
  });
}

NFCManager.prototype.getDefaultAdapter = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_COMMON);

  // First check NFC suppor on C++ level
  var result = native_.callSync(
      'NFCManager_getDefaultAdapter',
      {}
      );
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  // If NFC is supported then return new NFCAdapter instance
  return new NFCAdapter();
};

NFCManager.prototype.setExclusiveMode = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_COMMON);

  var args = validator_.validateArgs(arguments, [
    {name: 'exclusiveMode', type: types_.BOOLEAN}
  ]);

  var result = native_.callSync('NFCManager_setExclusiveMode', {
    exclusiveMode: args.exclusiveMode
  });

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

//////////////////NFCAdapter /////////////////

function NFCAdapter() {
  function poweredGetter() {
    var ret = native_.callSync('NFCAdapter_getPowered');

    if (native_.isFailure(ret)) {
      return false;
    }

    return native_.getResultObject(ret);
  }

  function cardEmulationModeGetter() {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_CARDEMULATION);

    var result = native_.callSync('NFCAdapter_cardEmulationModeGetter');

    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }

    return native_.getResultObject(result);
  }

  function cardEmulationModeSetter(cem) {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_CARDEMULATION);

    var args = validator_.validateArgs(arguments, [
      {name: 'emulationMode', type: types_.STRING}
    ]);

    var result = native_.callSync(
        'NFCAdapter_cardEmulationModeSetter',
        { 'emulationMode': args.emulationMode}
        );

    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }
    return;
  }

  function activeSecureElementGetter() {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_CARDEMULATION);

    var result = native_.callSync('NFCAdapter_activeSecureElementGetter');

    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }

    return native_.getResultObject(result);
  }

  function activeSecureElementSetter(ase) {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_CARDEMULATION);

    var args = validator_.validateArgs(arguments, [
      {name: 'secureElement', type: types_.STRING}
    ]);

    var result = native_.callSync(
        'NFCAdapter_activeSecureElementSetter',
        { 'secureElement': args.secureElement}
        );

    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }
    return;
  }

  Object.defineProperties(this, {
    powered: {enumerable: true,
      set: function() {},
      get: poweredGetter
    },
    cardEmulationMode: {enumerable: true,
      set: cardEmulationModeSetter,
      get: cardEmulationModeGetter
    },
    activeSecureElement: {enumerable: true,
      set: activeSecureElementSetter,
      get: activeSecureElementGetter
    }
  });
}

NFCAdapter.prototype.setPowered = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_ADMIN);

  var args = validator_.validateArgs(arguments, [
    {
      name: 'powered',
      type: types_.BOOLEAN
    },
    {
      name: 'successCallback',
      type: types_.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'errorCallback',
      type: types_.FUNCTION,
      optional: true,
      nullable: true
    }
  ]);

  native_.call('NFCAdapter_setPowered', {
    powered: args.powered
  }, function(result) {
    if (native_.isFailure(result)) {
      args.errorCallback(result.error);
    } else {
      args.successCallback();
    }
  });
};

NFCAdapter.prototype.setTagListener = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_TAG);

  var args = validator_.validateArgs(arguments, [
    {
      name: 'listener',
      type: types_.LISTENER,
      values: ['onattach', 'ondetach']
    },
    {
      name: 'tagType',
      type: types_.ARRAY,
      values: types_.STRING,
      optional: true,
      nullable: true
    }
  ]);

  if (!type_.isNullOrUndefined(args.tagType)) {
    for (var i = 0; i < args.tagType.length; i++) {
      if (NFCTagType[args.tagType[i]] === undefined) {
        throw new WebAPIException(
            WebAPIException.TYPE_MISMATCH_ERR, 'Invalid tag type.');
      }
    }
  }

  // Listener object creation
  var listenerCallback = function(message) {
    var tagObject = undefined;

    if ('onattach' === message.action) {
      tagObject = new NFCTag(message.id);

      // If filter is set for listener but tag type is not searched one
      if (!type_.isNullOrUndefined(args.tagType) &&
          args.tagType.indexOf(tagObject.type) < 0) {
        return;
      }
    }
    args.listener[message.action](tagObject);
  };

  // Register (acivate) core listener if not done yet
  if (!native_.isListenerSet(TAG_LISTENER)) {
    var result = native_.callSync('NFCAdapter_setTagListener');
    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }
  }

  native_.addListener(TAG_LISTENER, listenerCallback);
  return;
};

NFCAdapter.prototype.setPeerListener = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_P2P);

  var args = validator_.validateArgs(arguments, [
    {
      name: 'listener',
      type: types_.LISTENER,
      values: ['onattach', 'ondetach']
    }
  ]);

  var listener = function(msg) {
    var data = undefined;
    if ('onattach' === msg.action) {
      data = new NFCPeer(msg.id);
    }
    args.listener[msg.action](data);
  };

  if (!native_.isListenerSet(PEER_LISTENER)) {
    var result = native_.callSync('NFCAdapter_setPeerListener');
    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }
  }

  native_.addListener(PEER_LISTENER, listener);
  return;
};

NFCAdapter.prototype.unsetTagListener = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_TAG);

  native_.removeListener(TAG_LISTENER);

  var result = native_.callSync('NFCAdapter_unsetTagListener');
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  return;
};

NFCAdapter.prototype.unsetPeerListener = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_P2P);

  native_.removeListener(PEER_LISTENER);

  var result = native_.callSync('NFCAdapter_unsetPeerListener');
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  return;
};

NFCAdapter.prototype.addCardEmulationModeChangeListener = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_CARDEMULATION);

  var args = validator_.validateArgs(arguments, [
    {
      name: 'callback',
      type: types_.FUNCTION
    }
  ]);

  if (type_.isEmptyObject(cardEmulationModeListener.listeners) &&
      type_.isEmptyObject(activeSecureElementChangeListener.listeners)) {
    var result = native_.callSync(
        'NFCAdapter_addCardEmulationModeChangeListener');
    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }
  }

  return cardEmulationModeListener.addListener(args.callback);
};

NFCAdapter.prototype.removeCardEmulationModeChangeListener = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_CARDEMULATION);

  var args = validator_.validateArgs(arguments, [
    {
      name: 'listenerId',
      type: types_.LONG
    }
  ]);
  cardEmulationModeListener.removeListener(args.listenerId);

  if (type_.isEmptyObject(cardEmulationModeListener.listeners) &&
      type_.isEmptyObject(activeSecureElementChangeListener.listeners)) {
    native_.callSync('NFCAdapter_removeCardEmulationModeChangeListener');
  }
};

NFCAdapter.prototype.addTransactionEventListener = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_CARDEMULATION);

  var args = validator_.validateArgs(arguments, [
    {
      name: 'type',
      type: types_.ENUM,
      values: type_.getValues(SecureElementType)
    },
    {
      name: 'callback',
      type: types_.FUNCTION
    }
  ]);

  var result;

  if (SecureElementType.ESE === args.type) {
    if (type_.isEmptyObject(transactionEventListenerEse.listeners)) {
      result = native_.callSync('NFCAdapter_addTransactionEventListener', {
        type: args.type});
      if (native_.isFailure(result)) {
        throw native_.getErrorObject(result);
      }
    }
    return transactionEventListenerEse.addListener(args.callback);
  } else {
    if (type_.isEmptyObject(transactionEventListenerUicc.listeners)) {
      result = native_.callSync('NFCAdapter_addTransactionEventListener', {
        type: args.type});
      if (native_.isFailure(result)) {
        throw native_.getErrorObject(result);
      }
    }
    return transactionEventListenerUicc.addListener(args.callback);
  }
};

NFCAdapter.prototype.removeTransactionEventListener = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_CARDEMULATION);

  var args = validator_.validateArgs(arguments, [
    {
      name: 'watchId',
      type: types_.LONG
    }
  ]);

  var ese_empty = type_.isEmptyObject(transactionEventListenerEse.listeners);
  var uicc_empty = type_.isEmptyObject(transactionEventListenerUicc.listeners);

  transactionEventListenerEse.removeListener(args.watchId);
  transactionEventListenerUicc.removeListener(args.watchId);

  if (type_.isEmptyObject(transactionEventListenerEse.listeners) && !ese_empty) {
    native_.callSync('NFCAdapter_removeTransactionEventListener', {
      type: SecureElementType.ESE});
  }

  if (type_.isEmptyObject(transactionEventListenerUicc.listeners)
            && !uicc_empty) {
    native_.callSync('NFCAdapter_removeTransactionEventListener', {
      type: SecureElementType.UICC});
  }

};

NFCAdapter.prototype.addActiveSecureElementChangeListener = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_CARDEMULATION);

  var args = validator_.validateArgs(arguments, [
    {
      name: 'callback',
      type: types_.FUNCTION
    }
  ]);

  if (type_.isEmptyObject(cardEmulationModeListener.listeners) &&
      type_.isEmptyObject(activeSecureElementChangeListener.listeners)) {
    var result = native_.callSync(
        'NFCAdapter_addActiveSecureElementChangeListener');
    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }
  }

  return activeSecureElementChangeListener.addListener(args.callback);
};

NFCAdapter.prototype.removeActiveSecureElementChangeListener = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_CARDEMULATION);

  var args = validator_.validateArgs(arguments, [
    {
      name: 'listenerId',
      type: types_.LONG
    }
  ]);
  activeSecureElementChangeListener.removeListener(args.listenerId);

  if (type_.isEmptyObject(cardEmulationModeListener.listeners) &&
      type_.isEmptyObject(activeSecureElementChangeListener.listeners)) {
    native_.callSync('NFCAdapter_removeCardEmulationModeChangeListener');
  }
};

NFCAdapter.prototype.getCachedMessage = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_COMMON);

  var result = native_.callSync('NFCAdapter_getCachedMessage');

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  if (!result.records) {
    return new tizen.NDEFMessage();
  }

  return new tizen.NDEFMessage(toRecordsArray(result.records));
};

NFCAdapter.prototype.setExclusiveModeForTransaction = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_CARDEMULATION);

  var args = validator_.validateArgs(arguments, [
    {
      name: 'transactionMode',
      type: types_.BOOLEAN
    }
  ]);

  var result = native_.callSync(
      'NFCAdapter_setExclusiveModeForTransaction',
      { 'transactionMode': args.transactionMode}
      );

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  return;
};

NFCAdapter.prototype.addHCEEventListener = function(eventCallback) {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_CARDEMULATION);

  var args = validator_.validateArgs(arguments, [
    {name: 'eventCallback', type: types_.FUNCTION}
  ]);

  if (!arguments.length || !type_.isFunction(arguments[0])) {
    throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR);
  }

  if (type_.isEmptyObject(HCEEventListener.listeners)) {
    var result = native_.callSync('NFCAdapter_addHCEEventListener');
    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }
  }

  return HCEEventListener.addListener(args.eventCallback);
};

NFCAdapter.prototype.removeHCEEventListener = function(watchId) {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_CARDEMULATION);

  var args = validator_.validateArgs(arguments, [
    {name: 'watchId', type: types_.LONG}
  ]);

  if (!arguments.length || !type_.isNumber(arguments[0])) {
    throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR);
  }

  HCEEventListener.removeListener(args.watchId);

  if (type_.isEmptyObject(HCEEventListener.listeners)) {
    var result = native_.callSync('NFCAdapter_removeHCEEventListener');
    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }
  }
};

NFCAdapter.prototype.sendHostAPDUResponse = function(apdu, successCallback, errorCallback) {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_CARDEMULATION);

  var args = validator_.validateArgs(arguments, [
    {name: 'apdu', type: types_.ARRAY, values: types_.BYTE},
    {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  if (!arguments.length || !type_.isArray(arguments[0])) {
    throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR);
  }

  var data = {
    apdu: args.apdu
  };

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.successCallback);
  };

  native_.call('NFCAdapter_sendHostAPDUResponse', data, callback);
};

NFCAdapter.prototype.isActivatedHandlerForAID = function(type, aid) {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_CARDEMULATION);

  var args = validator_.validateArgs(arguments, [
    {
      name: 'type',
      type: types_.ENUM,
      values: type_.getValues(SecureElementType)
    },
    {name: 'aid', type: types_.STRING}
  ]);

  if (arguments.length < 2) {
    throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR);
  }

  var data = {
    type: args.type,
    aid: args.aid
  };

  var result = native_.callSync('NFCAdapter_isActivatedHandlerForAID', data);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  return native_.getResultObject(result);
};

NFCAdapter.prototype.isActivatedHandlerForCategory = function(type, category) {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_CARDEMULATION);

  var args = validator_.validateArgs(arguments, [{
    name: 'type',
    type: types_.ENUM,
    values: type_.getValues(SecureElementType)
  }, {
    name: 'category',
    type: types_.ENUM,
    values: Object.keys(CardEmulationCategoryType)
  }]);

  if (arguments.length < 2) {
    throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR);
  }

  var data = {
    type: args.type,
    category: args.category
  };

  var result = native_.callSync('NFCAdapter_isActivatedHandlerForCategory', data);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  return native_.getResultObject(result);
};

NFCAdapter.prototype.registerAID = function(type, aid, category) {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_CARDEMULATION);

  var args = validator_.validateArgs(arguments, [{
    name: 'type',
    type: types_.ENUM,
    values: type_.getValues(SecureElementType)
  }, {
    name: 'aid',
    type: types_.STRING
  }, {
    name: 'category',
    type: types_.ENUM,
    values: Object.keys(CardEmulationCategoryType)
  }]);

  if (arguments.length < 3 || !type_.isString(arguments[0])) {
    throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR);
  }

  var data = {
    type: args.type,
    aid: args.aid,
    category: args.category
  };

  var result = native_.callSync('NFCAdapter_registerAID', data);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

NFCAdapter.prototype.unregisterAID = function(type, aid, category) {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_CARDEMULATION);

  var args = validator_.validateArgs(arguments, [
    {
      name: 'type',
      type: types_.ENUM,
      values: type_.getValues(SecureElementType)
    },
    {name: 'aid', type: types_.STRING},
    {name: 'category', type: types_.ENUM, values: Object.keys(CardEmulationCategoryType)}
  ]);

  if (arguments.length < 3 || !type_.isString(arguments[0])) {
    throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR);
  }

  var data = {
    type: args.type,
    aid: args.aid,
    category: args.category
  };

  var result = native_.callSync('NFCAdapter_unregisterAID', data);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

function AIDData(data) {
  Object.defineProperties(this, {
    type: {
      value: data.type,
      writable: false,
      enumerable: true
    },
    aid: {
      value: data.aid || [],
      writable: false,
      enumerable: true
    },
    readOnly: {
      value: data.readOnly || false,
      writable: false,
      enumerable: true
    }
  });
}

NFCAdapter.prototype.getAIDsForCategory = function(type, category, successCallback, errorCallback) {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_CARDEMULATION);

  var args = validator_.validateArgs(arguments, [{
    name: 'type',
    type: types_.ENUM,
    values: type_.getValues(SecureElementType)
  }, {
    name: 'category',
    type: types_.ENUM,
    values: Object.keys(CardEmulationCategoryType)
  }, {
    name: 'successCallback',
    type: types_.FUNCTION
  }, {
    name: 'errorCallback',
    type: types_.FUNCTION,
    optional: true,
    nullable: true
  }]);

  if (arguments.length < 3) {
    throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR);
  }

  var data = {
    type: args.type,
    category: args.category
  };

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }
    var aids = [];
    var r = native_.getResultObject(result);
    for (var i = 0; i < r.length; i++) {
      aids.push(new AIDData(r[i]));
    }
    native_.callIfPossible(args.successCallback, aids);
  };

  native_.call('NFCAdapter_getAIDsForCategory', data, callback);
};

function InternalRecordData(tnf, type, payload, id) {
  this.tnf = tnf;
  this.type = type;
  this.payload = payload;
  this.id = id;
};

var toRecordsArray = function(array) {
  var result = [];
  if (type_.isNullOrUndefined(array) || !type_.isArray(array)) {
    return result;
  }

  for (var i = 0; i < array.length; i++) {
    var data = new InternalRecordData(array[i].tnf, array[i].type, array[i].payload, array[i].id);

    if (array[i].recordType == 'Record') {
      result.push(new tizen.NDEFRecord(data.tnf_, data.type_, data.payload_, data.id_));
      continue;
    }

    if (array[i].recordType == 'RecordText') {
      result.push(new tizen.NDEFRecordText(array[i].text, array[i].languageCode,
          array[i].encoding, data));
      continue;
    }

    if (array[i].recordType == 'RecordURI') {
      result.push(new tizen.NDEFRecordURI(array[i].uri, data));
      continue;
    }

    if (array[i].recordType == 'RecordMedia') {
      result.push(new tizen.NDEFRecordMedia(array[i].mimeType, array[i].data, data));
      continue;
    }
  }

  return result;
};

//////////////////NFCTag /////////////////

function NFCTag(tagid) {

  var _my_id = tagid;

  function TypeGetter() {

    var result = native_.callSync('NFCTag_typeGetter', {'id' : _my_id});

    if (native_.isFailure(result)) {
      return;
    }
    return native_.getResultObject(result);
  }

  function IsSupportedNDEFGetter() {

    var result = native_.callSync('NFCTag_isSupportedNDEFGetter', {'id' : _my_id});

    if (native_.isFailure(result)) {
      return;
    }
    return native_.getResultObject(result);
  }

  function NDEFSizeGetter() {

    var result = native_.callSync('NFCTag_NDEFSizeGetter', {'id' : _my_id});

    if (native_.isFailure(result)) {
      return;
    }
    return native_.getResultObject(result);
  }

  function PropertiesGetter() {

    var result = native_.callSync('NFCTag_propertiesGetter', {'id' : _my_id});

    if (native_.isFailure(result)) {
      return;
    }

    console.log('Current result: ' + result);

    var result_array = {};
    for (var i in result.result) {
      var current = result.result[i];
      var keys = Object.keys(current);
      for (var x in keys) {
        result_array[keys[x]] = current[keys[x]];
      }
    }
    return result_array;
  }

  function IsConnectedGetter() {

    var result = native_.callSync('NFCTag_isConnectedGetter', {'id' : _my_id});

    if (native_.isFailure(result)) {
      return;
    }
    return native_.getResultObject(result);
  }

  // Function defined here (not outside Tag "constructor"
  // because access to internal _my_id variable is needed)
  NFCTag.prototype.readNDEF = function() {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_TAG);

    var args = validator_.validateArgs(arguments, [
      {
        name: 'readCallback',
        type: types_.FUNCTION
      },
      {
        name: 'errorCallback',
        type: types_.FUNCTION,
        optional: true,
        nullable: true
      }
    ]);

    native_.call('NFCTag_readNDEF', {'id' : _my_id},
        function(result) {
          if (native_.isFailure(result)) {
            if (!type_.isNullOrUndefined(args.errorCallback)) {
              args.errorCallback(native_.getErrorObject(result));
            }
          } else {
            var message = new tizen.NDEFMessage(toRecordsArray(result.records));
            args.readCallback(message);
          }
        });

  };

  NFCTag.prototype.writeNDEF = function() {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_TAG);

    var args = validator_.validateArgs(arguments, [
      {
        name: 'message',
        type: types_.PLATFORM_OBJECT,
        values: tizen.NDEFMessage
      },
      {
        name: 'successCallback',
        type: types_.FUNCTION,
        optional: true,
        nullable: true
      },
      {
        name: 'errorCallback',
        type: types_.FUNCTION,
        optional: true,
        nullable: true
      }
    ]);

    native_.call('NFCTag_writeNDEF',
        {
          'id' : _my_id,
          'records' : args.message.records,
          'recordsSize' : args.message.recordCount
        },
        function(result) {
          if (native_.isFailure(result)) {
            if (!type_.isNullOrUndefined(args.errorCallback)) {
              args.errorCallback(native_.getErrorObject(result));
            }
          } else {
            if (!type_.isNullOrUndefined(args.successCallback)) {
              args.successCallback();
            }
          }
        });

  };

  NFCTag.prototype.transceive = function() {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_TAG);

    var args = validator_.validateArgs(arguments, [
      {
        name: 'data',
        type: types_.ARRAY
      },
      {
        name: 'dataCallback',
        type: types_.FUNCTION
      },
      {
        name: 'errorCallback',
        type: types_.FUNCTION,
        optional: true,
        nullable: true
      }
    ]);

    native_.call('NFCTag_transceive',
        {
          'id' : _my_id,
          'data' : args.data
        },
        function(result) {
          if (native_.isFailure(result)) {
            if (!type_.isNullOrUndefined(args.errorCallback)) {
              args.errorCallback(result.error);
            }
          } else {
            if (!type_.isNullOrUndefined(args.dataCallback)) {
              args.dataCallback(result.data);
            }
          }
        });

  };

  Object.defineProperties(this, {
    type: {
      set: function() {},
      get: TypeGetter,
      enumerable: true
    },
    isSupportedNDEF: {
      set: function() {},
      get: IsSupportedNDEFGetter,
      enumerable: true
    },
    ndefSize: {
      set: function() {},
      get: NDEFSizeGetter,
      enumerable: true
    },
    properties: {
      set: function() {},
      get: PropertiesGetter,
      enumerable: true
    },
    isConnected: {
      set: function() {},
      get: IsConnectedGetter,
      enumerable: true
    }
  });
}


//////////////////NFCPeer /////////////////

function NFCPeer(peerid) {
  var _my_id = peerid;

  function isConnectedGetter() {
    var ret = native_.callSync('NFCAdapter_PeerIsConnectedGetter', {'id' : _my_id});
    if (native_.isFailure(ret)) {
      return false;
    }
    return native_.getResultObject(ret);
  }

  NFCPeer.prototype.sendNDEF = function() {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_P2P);

    var args = validator_.validateArgs(arguments, [
      {
        name: 'message',
        type: types_.PLATFORM_OBJECT,
        values: tizen.NDEFMessage
      },
      {
        name: 'successCallback',
        type: types_.FUNCTION,
        optional: true,
        nullable: true
      },
      {
        name: 'errorCallback',
        type: types_.FUNCTION,
        optional: true,
        nullable: true
      }
    ]);

    native_.call('NFCPeer_sendNDEF', {
      'id' : _my_id,
      'records' : args.message.records,
      'recordsSize' : args.message.recordCount
    }, function(result) {
      if (native_.isFailure(result)) {
        args.errorCallback(result.error);
      } else {
        args.successCallback();
      }
    });
  };

  NFCPeer.prototype.setReceiveNDEFListener = function() {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_P2P);

    var args = validator_.validateArgs(arguments, [
      {
        name: 'listener',
        type: types_.FUNCTION
      }
    ]);

    var listener = function(msg) {
      var data = undefined;
      if ('onsuccess' === msg.action && _my_id === msg.id) {
        data = new NDEFMessage(msg);
      }
      args.listener[msg.action](data);
    };

    var result = native_.callSync('NFCPeer_setReceiveNDEFListener', {'id' : _my_id});
    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }

    native_.addListener(RECEIVE_NDEF_LISTENER, listener);
    return;
  };

  NFCPeer.prototype.unsetReceiveNDEFListener = function() {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NFC_P2P);

    native_.removeListener(RECEIVE_NDEF_LISTENER);

    var result = native_.callSync('NFCPeer_unsetReceiveNDEFListener', {'id' : _my_id});
    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }

    return;
  };

  Object.defineProperties(this, {
    isConnected: {
      enumerable: true,
      set: function() {},
      get: isConnectedGetter
    }
  });
}

var toByteArray = function(array, max_size, nullable) {
  var resultArray = [];
  if (type_.isNullOrUndefined(array) && nullable === true)
    return resultArray;

  var convertedArray = converter_.toArray(array);
  var len = convertedArray.length;

  if (len > max_size)
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR);
  for (var i = 0; i < len; i++) {
    resultArray.push(converter_.toOctet(convertedArray[i]));
  }
  return resultArray;
};

var isArrayOfType = function(array, type) {
  for (var i = 0; i < array.length; i++) {
    if (!(array[i] instanceof type))
      return false;
  }
  return true;
};

//////////////////NDEFMessage /////////////////
//[Constructor(),
// Constructor(NDEFRecord[] ndefRecords),
// Constructor(byte[] rawData)]
//interface NDEFMessage {
//  readonly attribute long recordCount;
//
//  attribute NDEFRecord[] records;
//
//  byte[] toByte() raises(WebAPIException);
//};

tizen.NDEFMessage = function(data) {
  validator_.isConstructorCall(this, tizen.NDEFMessage);
  var records_ = [];

  try {
    if (arguments.length >= 1) {
      if (type_.isArray(data)) {
        if (isArrayOfType(data, tizen.NDEFRecord)) {
          records_ = data;
        } else {
          var raw_data_ = toByteArray(data);
          var result = native_.callSync(
              'NDEFMessage_constructor', {
                                'rawData': raw_data_,
                                'rawDataSize' : raw_data_.length
              }
              );
          var records_array = result.result.records;
          for (var i = 0; i < records_array.length; i++) {
            records_.push(new tizen.NDEFRecord(records_array[i].tnf,
                                records_array[i].type, records_array[i].payload,
                                records_array[i].id));
          }
        }
      } else {
        throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR);
      }
    }
  } catch (e) {
    //constructor call failed - empty object should be created
    records_ = undefined;
  }

  var recordsSetter = function(data) {
    if (type_.isArray(data)) {
      // Do not check type of array elements - allow all arrays
      //if ( isArrayOfType(data, tizen.NDEFRecord) ) {
      records_ = data;
      //}
    }
  };

  Object.defineProperties(this, {
    recordCount: { enumerable: true,
      set: function() {},
      get: function() { return records_ ? records_.length : undefined;}},
    records: { enumerable: true,
      set: recordsSetter,
      get: function() {return records_;}}
  });
};

tizen.NDEFMessage.prototype.toByte = function() {
  var result = native_.callSync(
      'NDEFMessage_toByte', {
        'records' : this.records,
        'recordsSize' : this.recordCount
      }
      );
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  return toByteArray(result.result.bytes);
};

//helper for inherited object constructors /////////////////////////////////////////////
function InternalData() {
}

//////////////////NDEFRecord /////////////////
tizen.NDEFRecord = function(first, type, payload, id) {
  var tnf_ = undefined;
  var typ_ = undefined;
  var payload_ = undefined;
  var id_ = undefined;
  //if it is inherited call, then ignore validation
  if (!(first instanceof InternalData)) {
    validator_.isConstructorCall(this, tizen.NDEFRecord);
    try {
      if (arguments.length >= 1) {
        if (type_.isArray(first)) {
          var raw_data_ = toByteArray(first);
          var result = native_.callSync(
              'NDEFRecord_constructor', {
                'rawData': raw_data_,
                'rawDataSize' : raw_data_.length
              }
              );
          if (native_.isFailure(result)) {
            throw native_.getErrorObject(result);
          }
          tnf_ = converter_.toLong(result.result.tnf);
          typ_ = toByteArray(result.result.type, 255);
          payload_ = toByteArray(result.result.payload, Math.pow(2, 32) - 1);
          id_ = toByteArray(result.result.id, 255);
        } else if (arguments.length >= 3) {
          tnf_ = converter_.toLong(first);
          typ_ = toByteArray(type, 255);
          payload_ = toByteArray(payload, Math.pow(2, 32) - 1);
          id_ = toByteArray(id, 255, true, []);
        }
      } else {
        throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR);
      }
    } catch (e) {
      //constructor call failed - empty object should be created
      tnf_ = undefined;
      typ_ = undefined;
      payload_ = undefined;
      id_ = undefined;
    }
  }

  Object.defineProperties(this, {
    tnf: {value: tnf_, writable: false, enumerable: true},
    type: {value: typ_, writable: false, enumerable: true},
    id: {value: id_, writable: false, enumerable: true},
    payload: {value: payload_, writable: false, enumerable: true}
  });
};

//////////////////NDEFRecordText /////////////////
tizen.NDEFRecordText = function(text, languageCode, encoding, internal_) {
  var text_ = undefined;
  var languageCode_ = undefined;
  var encoding_ = NDEFRecordTextEncoding[encoding] ?
      NDEFRecordTextEncoding[encoding] : NDEFRecordTextEncoding['UTF8'];
  try {
    if (arguments.length >= 2) {
      text_ = converter_.toString(text);
      languageCode_ = converter_.toString(languageCode);

      if (!type_.isNullOrUndefined(internal_) && (internal_ instanceof InternalRecordData)) {
        tizen.NDEFRecord.call(this, internal_.tnf_, internal_.type_, internal_.payload_, internal_.id_);
      } else {
        var result = native_.callSync(
            'NDEFRecordText_constructor', {
              'text': text_,
              'languageCode' : languageCode_,
              'encoding' : encoding_
            }
            );
        if (native_.isFailure(result)) {
          throw native_.getErrorObject(result);
        }
        tizen.NDEFRecord.call(this, result.result.tnf, result.result.type,
            result.result.payload, result.result.id);
      }
    } else {
      throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR);
    }
  } catch (e) {
    //constructor call failed - empty object should be created
    tizen.NDEFRecord.call(this);
    text_ = undefined;
    languageCode_ = undefined;
    encoding_ = undefined;
  }

  Object.defineProperties(this, {
    text: {value: text_, writable: false, enumerable: true},
    languageCode: {value: languageCode_, writable: false, enumerable: true},
    encoding: {value: encoding_, writable: false, enumerable: true}
  });
};

tizen.NDEFRecordText.prototype = new tizen.NDEFRecord(new InternalData());

tizen.NDEFRecordText.prototype.constructor = tizen.NDEFRecordText;

//////////////////NDEFRecordURI /////////////////
tizen.NDEFRecordURI = function(uri, internal_) {
  var uri_ = undefined;
  try {
    if (arguments.length >= 1) {
      uri_ = converter_.toString(uri);

      if (!type_.isNullOrUndefined(internal_) && (internal_ instanceof InternalRecordData)) {
        tizen.NDEFRecord.call(this, internal_.tnf_, internal_.type_, internal_.payload_, internal_.id_);
      } else {
        var result = native_.callSync(
            'NDEFRecordURI_constructor', {
              'uri': uri_
            }
            );
        if (native_.isFailure(result)) {
          throw native_.getErrorObject(result);
        }
        tizen.NDEFRecord.call(this, result.result.tnf, result.result.type,
            result.result.payload, result.result.id);
      }
    } else {
      throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR);
    }
  } catch (e) {
    //constructor call failed - empty object should be created
    tizen.NDEFRecord.call(this);
    uri_ = undefined;
  }

  Object.defineProperties(this, {
    uri: {value: uri_, writable: false, enumerable: true}
  });
};

tizen.NDEFRecordURI.prototype = new tizen.NDEFRecord(new InternalData());

tizen.NDEFRecordURI.prototype.constructor = tizen.NDEFRecordURI;

//////////////////NDEFRecordMedia /////////////////
tizen.NDEFRecordMedia = function(mimeType, data, internal_) {
  var mimeType_ = undefined;
  var data_ = undefined;
  try {
    if (arguments.length >= 2) {
      mimeType_ = converter_.toString(mimeType);
      data_ = toByteArray(data, Math.pow(2, 32) - 1);

      if (!type_.isNullOrUndefined(internal_) && (internal_ instanceof InternalRecordData)) {
        tizen.NDEFRecord.call(this, internal_.tnf_, internal_.type_, internal_.payload_, internal_.id_);
      } else {
        var result = native_.callSync(
            'NDEFRecordMedia_constructor', {
              'mimeType': mimeType_,
              'data': data_,
              'dataSize': data_.length
            }
            );
        if (native_.isFailure(result)) {
          throw native_.getErrorObject(result);
        }
        tizen.NDEFRecord.call(this, result.result.tnf, result.result.type,
            result.result.payload, result.result.id);
      }
    } else {
      throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR);
    }
  } catch (e) {
    //constructor call failed - empty object should be created
    tizen.NDEFRecord.call(this);
    mimeType_ = undefined;
  }

  Object.defineProperties(this, {
    mimeType: {value: mimeType_, writable: false, enumerable: true}
  });
};

tizen.NDEFRecordMedia.prototype = new tizen.NDEFRecord(new InternalData());

tizen.NDEFRecordMedia.prototype.constructor = tizen.NDEFRecordMedia;
//Exports
exports = new NFCManager();
