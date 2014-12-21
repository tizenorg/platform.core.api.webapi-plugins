// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var validator_ = xwalk.utils.validator;
var types_ = validator_.Types;
var T_ = xwalk.utils.type;
var native_ = new xwalk.utils.NativeManager(extension);

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
            if ('CardElement' == msg.type) {
                this.listeners[key](msg.mode);
            } else if ('Transaction' == msg.type) {
                this.listeners[key](msg.aid, msg.data);
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

var CARD_EMULATION_MODE_LISTENER = 'CardEmulationModeChanged';
var ACTIVE_SECURE_ELEMENT_LISTENER = 'ActiveSecureElementChanged';
var TRANSACTION_EVENT_ESE_LISTENER = 'TransactionEventListener_ESE';
var TRANSACTION_EVENT_UICC_LISTENER = 'TransactionEventListener_UICC';
var cardEmulationModeListener = new ListenerManager(native_, CARD_EMULATION_MODE_LISTENER);
var activeSecureElementChangeListener = new ListenerManager(native_, ACTIVE_SECURE_ELEMENT_LISTENER);
var transactionEventListenerEse = new ListenerManager(native_, TRANSACTION_EVENT_ESE_LISTENER);
var transactionEventListenerUicc = new ListenerManager(native_, TRANSACTION_EVENT_UICC_LISTENER);

//enumeration NDEFRecordTextEncoding ////////////////////////////////////////////////////
var NDEFRecordTextEncoding = {
    UTF8 : 'UTF8',
    UTF16 : 'UTF16'
};

//enumeration NFCTagType ////////////////////////////////////////////////////
var NFCTagType = {
    GENERIC_TARGET : 'GENERIC_TARGET',
    ISO14443_A : 'ISO14443_A',
    ISO14443_4A : 'ISO14443_4A',
    ISO14443_3A : 'ISO14443_3A',
    MIFARE_MINI : 'MIFARE_MINI',
    MIFARE_1K : 'MIFARE_1K',
    MIFARE_4K : 'MIFARE_4K',
    MIFARE_ULTRA : 'MIFARE_ULTRA',
    MIFARE_DESFIRE : 'MIFARE_DESFIRE',
    ISO14443_B : 'ISO14443_B',
    ISO14443_4B : 'ISO14443_4B',
    ISO14443_BPRIME : 'ISO14443_BPRIME',
    FELICA : 'FELICA',
    JEWEL : 'JEWEL',
    ISO15693 : 'ISO15693',
    UNKNOWN_TARGET : 'UNKNOWN_TARGET'
};

////enumeration CardEmulationMode ////////////////////////////////////////////////////
var CardEmulationMode = {
    ALWAYS_ON : 'ALWAYS_ON',
    OFF : 'OFF'
};

////enumeration SecureElementType ////////////////////////////////////////////////////
var SecureElementType = {
    ESE : 'ESE',
    UICC : 'UICC'
};

//////////////////NFCManager /////////////////

function NFCManager() {
    Object.defineProperties(this, {
        NFC_RECORD_TNF_EMPTY:   {value: 0, writable: false, enumerable: true},
        NFC_RECORD_TNF_WELL_KNOWN:    {value: 1, writable: false, enumerable: true},
        NFC_RECORD_TNF_MIME_MEDIA:    {value: 2, writable: false, enumerable: true},
        NFC_RECORD_TNF_URI:    {value: 3, writable: false, enumerable: true},
        NFC_RECORD_TNF_EXTERNAL_RTD:    {value: 4, writable: false, enumerable: true},
        NFC_RECORD_TNF_UNKNOWN:    {value: 5, writable: false, enumerable: true},
        NFC_RECORD_TNF_UNCHANGED:    {value: 6, writable: false, enumerable: true}
    });
}

NFCManager.prototype.getDefaultAdapter = function() {
    // First check NFC suppor on C++ level
    var result = native_.callSync(
        'NFCManager_getDefaultAdapter',
        {}
    );
    if(native_.isFailure(result)) {
        throw new tizen.WebAPIException(0, result.error.message,
                result.error.name);
    }

    // If NFC is supported then return new NFCAdapter instance
    return new NFCAdapter();
};

NFCManager.prototype.setExclusiveMode = function() {

    var args = validator_.validateArgs(arguments, [
        {name: 'exclusiveMode', type: types_.BOOLEAN}
    ]);

    var result = native_.callSync(
        'NFCManager_setExclusiveMode',
        { 'exclusiveMode': args.exclusiveMode}
    );

    // If failed then exception should be thrown.
    if(native_.isFailure(result)) {
        throw new tizen.WebAPIException(0, result.error.message,
                result.error.name);
        // Uncoment line below (and remove line above) when problem
        // with error conversion is fixed:
        //
        //throw native_.getErrorObject(result);
    }
    // Otherwise just return
    return;
};

//////////////////NFCAdapter /////////////////

function NFCAdapter() {
    function poweredGetter() {
        var ret = native_.callSync('NFCAdapter_getPowered');

        if (native_.isFailure(ret)) {
            return '';
        }

        return native_.getResultObject(ret);
    }

    function cardEmulationModeGetter() {
        var result = native_.callSync('NFCAdapter_cardEmulationModeGetter');

        if (native_.isFailure(result)) {
            throw new tizen.WebAPIException(0, result.error.message, result.error.name);
        }

        return native_.getResultObject(result);
    }

    function cardEmulationModeSetter(cem) {

        var args = validator_.validateArgs(arguments, [
            {name: 'emulationMode', type: types_.STRING}
        ]);

        var result = native_.callSync(
            'NFCAdapter_cardEmulationModeSetter',
            { 'emulationMode': args.emulationMode}
        );

        if(native_.isFailure(result)) {
            throw new tizen.WebAPIException(0, result.error.message, result.error.name);
        }
        return;
    }

    function activeSecureElementGetter() {

        var result = native_.callSync('NFCAdapter_activeSecureElementGetter');

        if (native_.isFailure(result)) {
            throw new tizen.WebAPIException(0, result.error.message, result.error.name);
        }

        return native_.getResultObject(result);
    }

    function activeSecureElementSetter(ase) {

        var args = validator_.validateArgs(arguments, [
            {name: 'secureElement', type: types_.STRING}
        ]);

        var result = native_.callSync(
            'NFCAdapter_activeSecureElementSetter',
            { 'secureElement': args.secureElement}
        );

        if(native_.isFailure(result)) {
            throw new tizen.WebAPIException(0, result.error.message, result.error.name);
        }
        return;
    }

    Object.defineProperties(this, {
        powered:   {enumerable: true,
            set : function(){},
            get : poweredGetter
        },
        cardEmulationMode:   {enumerable: true,
            set : cardEmulationModeSetter,
            get : cardEmulationModeGetter
        },
        activeSecureElement:   {enumerable: true,
            set : activeSecureElementSetter,
            get : activeSecureElementGetter
        }
    });
};

NFCAdapter.prototype.setPowered = function() {
    var args = validator_.validateArgs(arguments, [
        {
            name : 'powered',
            type : types_.BOOLEAN
        },
        {
            name : 'successCallback',
            type : types_.FUNCTION,
            optional : true,
            nullable : true
        },
        {
            name : 'errorCallback',
            type : types_.FUNCTION,
            optional : true,
            nullable : true
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

NFCAdapter.prototype.setTagListener  = function() {

};

NFCAdapter.prototype.setPeerListener = function() {

};

NFCAdapter.prototype.unsetTagListener = function() {

};

NFCAdapter.prototype.unsetPeerListener = function() {

};

NFCAdapter.prototype.addCardEmulationModeChangeListener = function() {
    var args = validator_.validateArgs(arguments, [
        {
            name: 'callback',
            type: types_.LISTENER,
            values: ['onchanged']
        }
    ]);

    if (T_.isEmptyObject(cardEmulationModeListener.listeners) &&
            T_.isEmptyObject(activeSecureElementChangeListener.listeners)) {
        var result = native_.callSync(
                'NFCAdapter_addCardEmulationModeChangeListener');
        if (native_.isFailure(result)) {
            throw new tizen.WebAPIException(0, result.error.message,
                    result.error.name);
        }
    }

    return cardEmulationModeListener.addListener(args.callback);
};

NFCAdapter.prototype.removeCardEmulationModeChangeListener = function() {
    var args = validator_.validateArgs(arguments, [
        {
            name: 'listenerId',
            type: types_.LONG
        }
    ]);
    cardEmulationModeListener.removeListener(args.listenerId);

    if (T_.isEmptyObject(cardEmulationModeListener.listeners) &&
            T_.isEmptyObject(activeSecureElementChangeListener.listeners)) {
        native_.callSync('NFCAdapter_removeCardEmulationModeChangeListener');
    }
};

NFCAdapter.prototype.addTransactionEventListener = function() {
    var args = validator_.validateArgs(arguments, [
        {
            name: 'type',
            type: types_.ENUM,
            values: T_.getValues(SecureElementType)
        },
        {
            name: 'callback',
            type: types_.LISTENER,
            values: ['ondetected']
        }
    ]);

    var result;

    if (SecureElementType.ESE == args.type) {
        if (T_.isEmptyObject(transactionEventListenerEse.listeners)) {
            result = native_.callSync('NFCAdapter_addTransactionEventListener',{
                type: args.type});
            if (native_.isFailure(result)) {
                throw new tizen.WebAPIException(0, result.error.message,
                        result.error.name);
            }
        }
        return transactionEventListenerEse.addListener(args.callback);
    } else {
        if (T_.isEmptyObject(transactionEventListenerUicc.listeners)) {
            result = native_.callSync('NFCAdapter_addTransactionEventListener',{
                type: args.type});
            if (native_.isFailure(result)) {
                throw new tizen.WebAPIException(0, result.error.message,
                        result.error.name);
            }
        }
        return transactionEventListenerUicc.addListener(args.callback);
    }
};

NFCAdapter.prototype.removeTransactionEventListener = function() {
    var args = validator_.validateArgs(arguments, [
        {
            name: 'watchId',
            type: types_.LONG
        }
    ]);

    var ese_empty = T_.isEmptyObject(transactionEventListenerEse.listeners)
    var uicc_empty = T_.isEmptyObject(transactionEventListenerUicc.listeners)

    transactionEventListenerEse.removeListener(args.watchId);
    transactionEventListenerUicc.removeListener(args.watchId);

    if (T_.isEmptyObject(transactionEventListenerEse.listeners) && !ese_empty) {
        native_.callSync('NFCAdapter_removeTransactionEventListener',{
            type: SecureElementType.ESE});
    }

    if (T_.isEmptyObject(transactionEventListenerUicc.listeners)
            && !uicc_empty) {
        native_.callSync('NFCAdapter_removeTransactionEventListener',{
            type: SecureElementType.UICC});
    }

};

NFCAdapter.prototype.addActiveSecureElementChangeListener = function() {
    var args = validator_.validateArgs(arguments, [
       {
           name: 'callback',
           type: types_.LISTENER,
           values: ['onchanged']
       }
   ]);

    if (T_.isEmptyObject(cardEmulationModeListener.listeners) &&
            T_.isEmptyObject(activeSecureElementChangeListener.listeners)) {
        var result = native_.callSync(
                'NFCAdapter_addActiveSecureElementChangeListener ');
        if (native_.isFailure(result)) {
            throw new tizen.WebAPIException(0, result.error.message,
                    result.error.name);
        }
    }

   return activeSecureElementChangeListener.addListener(args.callback);
};

NFCAdapter.prototype.removeActiveSecureElementChangeListener = function() {
    var args = validator_.validateArgs(arguments, [
        {
            name: 'listenerId',
            type: types_.LONG
        }
    ]);
    activeSecureElementChangeListener.removeListener(args.listenerId);

    if (T_.isEmptyObject(cardEmulationModeListener.listeners) &&
            T_.isEmptyObject(activeSecureElementChangeListener.listeners)) {
        native_.callSync('NFCAdapter_removeCardEmulationModeChangeListener');
    }
};

NFCAdapter.prototype.getCachedMessage = function() {

};

NFCAdapter.prototype.setExclusiveModeForTransaction = function() {

    var args = validator_.validateArgs(arguments, [
        {
            name : 'transactionMode',
            type : types_.BOOLEAN
        }
    ]);

    var result = native_.callSync(
        'NFCAdapter_setExclusiveModeForTransaction',
        { 'transactionMode': args.transactionMode}
    );

    if(native_.isFailure(result)) {
        throw new tizen.WebAPIException(0, result.error.message, result.error.name);
        // throw native_.getErrorObject(result);
    }
    return;
};

//////////////////NFCTag /////////////////

function NFCTag(data) {
    Object.defineProperties(this, {
        type:   {value: data.type, writable: false, enumerable: true},
        isSupportedNDEF:   {value: data.isSupportedNDEF,
            writable: true, enumerable: true},
        ndefSize:   {value: data.ndefSize,
            writable: true, enumerable: true},
        properties:   {value: data.properties, writable: true, enumerable: true},
        isConnected:   {value: data.isConnected,
            writable: true, enumerable: true}
    });
}

NFCTag.prototype.readNDEF = function() {

};

NFCTag.prototype.writeNDEF = function() {

};

NFCTag.prototype.transceive = function() {

};

//////////////////NFCPeer /////////////////

function NFCPeer(data) {
    Object.defineProperties(this, {
        isConnected:   {value: data.isConnected,
            writable: true, enumerable: true}
    });
}

NFCPeer.prototype.setReceiveNDEFListener = function() {

};

NFCPeer.prototype.unsetReceiveNDEFListener = function() {

};

NFCPeer.prototype.sendNDEF = function() {

};

//////////////////NDEFMessage /////////////////

tizen.NDEFMessage = function(data) {

};

tizen.NDEFMessage.prototype.toByte = function() {

};

//////////////////NDEFRecord /////////////////

tizen.NDEFRecord = function(data, type, payload, id) {

};

//////////////////NDEFRecordText /////////////////

tizen.NDEFRecordText = function(text, languageCode, encoding) {

};

//////////////////NDEFRecordURI /////////////////

tizen.NDEFRecordURI = function(uri) {

};

//////////////////NDEFRecordMedia /////////////////

tizen.NDEFRecordMedia = function(mimeType, data) {

};

//Exports
exports = new NFCManager();
