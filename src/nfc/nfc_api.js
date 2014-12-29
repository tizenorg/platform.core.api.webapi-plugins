// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var validator_ = xwalk.utils.validator;
var types_ = validator_.Types;
var T_ = xwalk.utils.type;
var Converter_ = xwalk.utils.converter;
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

var PEER_LISTENER = 'PeerListener';
var RECEIVE_NDEF_LISTENER = 'ReceiveNDEFListener';
var CARD_EMULATION_MODE_LISTENER = 'CardEmulationModeChanged';
var ACTIVE_SECURE_ELEMENT_LISTENER = 'ActiveSecureElementChanged';
var TRANSACTION_EVENT_ESE_LISTENER = 'TransactionEventListener_ESE';
var TRANSACTION_EVENT_UICC_LISTENER = 'TransactionEventListener_UICC';
var TAG_LISTENER = 'TagListener';
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
            return false;
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

    var args = validator_.validateArgs(arguments, [
        {
            name: 'listener',
            type: types_.LISTENER,
            values: ['onattach', 'ondetach']
        },
        {
            name : 'tagType',
            type : types_.STRING,
            optional : true,
            nullable : true
        }
    ]);

    // TODO: NFCTag type value validation needed here

    // Listener object creation
    var listenerCallback = function(message) {
        var tagObject = undefined;

        if('onattach' === message.action) {
            tagObject = new NFCTag(message.id);

            if(!types_.isNullOrUndefined(args.tagType)) {
                // If filter set for listener then check tag type
                if(tagObject.type !== args.tagType) {
                    return;
                }
            }
        }
        args.listener[message.action](tagObject);
    }

    // Register (acivate) core listener if not done yet
    if(!native_.isListenerSet(TAG_LISTENER)) {
        var result = native_.callSync('NFCAdapter_setTagListener');
        if (native_.isFailure(result)) {
            throw new tizen.WebAPIException(0, result.error.message, result.error.name);
        }
    }

    native_.addListener(TAG_LISTENER, listenerCallback);
    return;
};

NFCAdapter.prototype.setPeerListener = function() {
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
    }

    if (!native_.isListenerSet(PEER_LISTENER)) {
        var result = native_.callSync('NFCAdapter_setPeerListener');
        if (native_.isFailure(result)) {
            throw new tizen.WebAPIException(0, result.error.message, result.error.name);
        }
    }

    native_.addListener(PEER_LISTENER, listener);
    return;
};

NFCAdapter.prototype.unsetTagListener = function() {

    native_.removeListener(TAG_LISTENER);

    var result = native_.callSync('NFCAdapter_unsetTagListener');
    if (native_.isFailure(result)) {
        throw new tizen.WebAPIException(0, result.error.message, result.error.name);
    }

    return;
};

NFCAdapter.prototype.unsetPeerListener = function() {
    native_.removeListener(PEER_LISTENER);

    var result = native_.callSync('NFCAdapter_unsetPeerListener');
    if (native_.isFailure(result)) {
        throw new tizen.WebAPIException(0, result.error.message, result.error.name);
    }

    return;
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
                'NFCAdapter_addActiveSecureElementChangeListener');
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

        console.log("Current result: " + result);

        var result_array = new Object();
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

        var args = validator_.validateArgs(arguments, [
            {
                name : 'readCallback',
                type : types_.FUNCTION
            },
            {
                name : 'errorCallback',
                type : types_.FUNCTION,
                optional : true,
                nullable : true
            }
        ]);

        native_.call('NFCTag_readNDEF', {'id' : _my_id},
        function(result) {
            if (native_.isFailure(result)) {
                if(!T_.isNullOrUndefined(args.errorCallback)) {
                    args.errorCallback(native_.getErrorObject(result));
                }
            } else {
                var message = new tizen.NDEFMessage(result.records);
                args.readCallback(message);
            }
        });

    };

    NFCTag.prototype.writeNDEF = function() {
        var args = validator_.validateArgs(arguments, [
            {
                name: 'message',
                type : types_.PLATFORM_OBJECT,
                values : tizen.NDEFMessage
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

        native_.call('NFCTag_writeNDEF',
        {
            'id' : _my_id,
            'records' : args.message.records,
            'recordsSize' : args.message.recordCount
        },
        function(result) {
            if (native_.isFailure(result)) {
                if(!T_.isNullOrUndefined(args.errorCallback)) {
                    args.errorCallback(native_.getErrorObject(result));
                }
            } else {
                if(!T_.isNullOrUndefined(args.successCallback)) {
                    args.successCallback();
                }
            }
        });

    };

    Object.defineProperties(this, {
        type:   {
            set: function() {},
            get: TypeGetter,
            enumerable: true
        },
        isSupportedNDEF:   {
            set: function() {},
            get: IsSupportedNDEFGetter,
            enumerable: true
        },
        ndefSize:   {
            set: function() {},
            get: NDEFSizeGetter,
            enumerable: true
        },
        properties:   {
            set: function() {},
            get: PropertiesGetter,
            enumerable: true
        },
        isConnected:   {
            set: function() {},
            get: IsConnectedGetter,
            enumerable: true
        }
    });
}

NFCTag.prototype.transceive = function() {

};

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

    Object.defineProperties(this, {
        isConnected: {
            enumerable: true,
            set : function(){},
            get : isConnectedGetter
        }
    });
}

NFCPeer.prototype.setReceiveNDEFListener = function() {
    var args = validator_.validateArgs(arguments, [
        {
            name: 'listener',
            type: types_.LISTENER,
            values: ['onsuccess']
        }
    ]);

    var listener = function(msg) {
        var data = undefined;
        if ('onsuccess' === msg.action && this._my_id === msg.id) {
            data = new NDEFMessage(msg);
        }
        args.listener[msg.action](data);
    }

    var result = native_.callSync('NFCPeer_setReceiveNDEFListener', {'id' : this._my_id});
    if (native_.isFailure(result)) {
        throw new tizen.WebAPIException(0, result.error.message, result.error.name);
    }

    native_.addListener(RECEIVE_NDEF_LISTENER, listener);
    return;
};

NFCPeer.prototype.unsetReceiveNDEFListener = function() {
    native_.removeListener(RECEIVE_NDEF_LISTENER);

    var result = native_.callSync('NFCPeer_unsetReceiveNDEFListener', {'id' : this._my_id});
    if (native_.isFailure(result)) {
        throw new tizen.WebAPIException(0, result.error.message, result.error.name);
    }

    return;
};

NFCPeer.prototype.sendNDEF = function() {

};


var toByteArray = function(array, max_size, nullable) {
    var resultArray = [];
    if (T_.isNullOrUndefined(array) && nullable === true)
        return resultArray;

    var convertedArray = Converter_.toArray(array);
    var len = convertedArray.length;

    if (len > max_size)
        throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR);
    for (var i = 0; i < len; i++){
        resultArray.push(Converter_.toOctet(convertedArray[i]));
    }
    return resultArray;
}

var isArrayOfType = function(array, type) {
    for (var i = 0; i < array.length; i++) {
        if (!(array[i] instanceof type))
            return false;
    }
    return true;
}

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
    var records_ = [];
    var recordCount_ = 0;

    try {
        if (arguments.length >= 1) {
            if (T_.isArray(data)) {
                if ( isArrayOfType(data, tizen.NDEFRecord) ) {
                    records_ = data;
                    recordCount_ = data.length;
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
                throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR);
            }
        }
    } catch (e) {
        //constructor call failed - empty object should be created
        records_ = undefined;
        recordCount_ = undefined;
    }

    var recordsSetter = function(data){
        if (T_.isArray(data)) {
            if ( isArrayOfType(data, tizen.NDEFRecord) ) {
                recordCount_ = data.length;
                records_ = data;
            }
        }
    }

    Object.defineProperties(this, {
        recordCount:   { enumerable: true,
            set : function(){},
            get : function(){return recordCount_;}},
        records:   { enumerable: true,
            set : recordsSetter,
            get : function(){return records_;}}
    });
};

tizen.NDEFMessage.prototype.toByte = function() {
    var result = native_.callSync(
            'NDEFMessage_toByte', {
                'records' : this.records,
                'recordsSize' : this.recordCount
            }
    );
    if(native_.isFailure(result)) {
        throw new tizen.WebAPIException(0, result.error.message,
                result.error.name);
    }

    return toByteArray(result.result.bytes);
};

//helper for inherited object constructors /////////////////////////////////////////////
function InternalData() {
}

//////////////////NDEFRecord /////////////////
tizen.NDEFRecord = function(first, type, payload, id) {
    var tnf_ = undefined;
    var type_ = undefined;
    var payload_ = undefined;
    var id_ = undefined;
    //if it is inherited call, then ignore validation
    if ( !(first instanceof InternalData) ){
        try {
            if (arguments.length >= 1) {
                if (T_.isArray(first)) {
                    var raw_data_ = toByteArray(first);
                    var result = native_.callSync(
                            'NDEFRecord_constructor', {
                                'rawData': raw_data_,
                                'rawDataSize' : raw_data_.length
                            }
                    );
                    if(native_.isFailure(result)) {
                        throw new tizen.WebAPIException(0, result.error.message,
                                result.error.name);
                        // throw native_.getErrorObject(result);
                    }
                    tnf_ = Converter_.toLong(result.result.tnf);
                    type_ = toByteArray(result.result.type, 255);
                    payload_ = toByteArray(result.result.payload, Math.pow(2, 32)-1);
                    id_ = toByteArray(result.result.id, 255);
                } else if (arguments.length >= 3){
                    tnf_ = Converter_.toLong(first);
                    type_ = toByteArray(type, 255);
                    payload_ = toByteArray(payload, Math.pow(2, 32)-1);
                    id_ = toByteArray(id, 255, true, []);
                }
            } else {
                throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR);
            }
        } catch (e) {
            //constructor call failed - empty object should be created
            tnf_ = undefined;
            type_ = undefined;
            payload_ = undefined;
            id_ = undefined;
        }
    }

    Object.defineProperties(this, {
        tnf:   {value: tnf_, writable: false, enumerable: true},
        type:   {value: type_, writable: false, enumerable: true},
        id:   {value: id_, writable: false, enumerable: true},
        payload:   {value: payload_, writable: false, enumerable: true},
    });
};

//////////////////NDEFRecordText /////////////////
var ENCODING = ["UTF8", "UTF16"];

tizen.NDEFRecordText = function(text, languageCode, encoding) {
    var text_ = undefined;
    var languageCode_ = undefined;
    var encoding_ = ENCODING[0];
    try {
        if (arguments.length >= 2) {
            text_ = Converter_.toString(text);
            languageCode_ = Converter_.toString(languageCode);

            if (!T_.isNullOrUndefined(encoding)) {
                encoding_ = Converter_.toEnum(encoding, ENCODING, true);
            }

            var result = native_.callSync(
                    'NDEFRecordText_constructor', {
                        'text': text_,
                        'languageCode' : languageCode_,
                        'encoding' : encoding_
                    }
            );
            if(native_.isFailure(result)) {
                throw new tizen.WebAPIException(0, result.error.message,
                        result.error.name);
                // throw native_.getErrorObject(result);
            }
            tizen.NDEFRecord.call(this, result.result.tnf, result.result.type,
                    result.result.payload, result.result.id);
        } else {
            throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR);
        }
    } catch (e) {
        //constructor call failed - empty object should be created
        tizen.NDEFRecord.call(this);
        text_ = undefined;
        languageCode_ = undefined;
        encoding_ = undefined;
    }

    Object.defineProperties(this, {
        text:   {value: text_, writable: false, enumerable: true},
        languageCode:   {value: languageCode_, writable: false, enumerable: true},
        encoding:   {value: encoding_, writable: false, enumerable: true},
    });
};

tizen.NDEFRecordText.prototype = new tizen.NDEFRecord(new InternalData());

tizen.NDEFRecordText.prototype.constructor = tizen.NDEFRecordText;

//////////////////NDEFRecordURI /////////////////
tizen.NDEFRecordURI = function(uri) {
    var uri_ = undefined;
    try {
        if (arguments.length >= 1) {
            uri_ = Converter_.toString(uri);

            var result = native_.callSync(
                    'NDEFRecordURI_constructor', {
                        'uri': uri_
                    }
            );
            if(native_.isFailure(result)) {
                throw new tizen.WebAPIException(0, result.error.message,
                        result.error.name);
                // throw native_.getErrorObject(result);
            }
            tizen.NDEFRecord.call(this, result.result.tnf, result.result.type,
                    result.result.payload, result.result.id);
        } else {
            throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR);
        }
    } catch (e) {
        //constructor call failed - empty object should be created
        tizen.NDEFRecord.call(this);
        uri_ = undefined;
    }

    Object.defineProperties(this, {
        uri:   {value: uri_, writable: false, enumerable: true}
    });
};

tizen.NDEFRecordURI.prototype = new tizen.NDEFRecord(new InternalData());

tizen.NDEFRecordURI.prototype.constructor = tizen.NDEFRecordURI;

//////////////////NDEFRecordMedia /////////////////
tizen.NDEFRecordMedia = function(mimeType, data) {
    var mimeType_ = undefined;
    var data_ = undefined;
    try {
        if (arguments.length >= 2) {
            mimeType_ = Converter_.toString(mimeType);
            data_ = toByteArray(data, Math.pow(2, 32)-1);

            var result = native_.callSync(
                    'NDEFRecordMedia_constructor', {
                        'mimeType': mimeType_,
                        'data': data_,
                        'dataSize': data_.length
                    }
            );
            if(native_.isFailure(result)) {
                throw new tizen.WebAPIException(0, result.error.message,
                        result.error.name);
                // throw native_.getErrorObject(result);
            }
            tizen.NDEFRecord.call(this, result.result.tnf, result.result.type,
                    result.result.payload, result.result.id);
        } else {
            throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR);
        }
    } catch (e) {
        //constructor call failed - empty object should be created
        tizen.NDEFRecord.call(this);
        mimeType_ = undefined;
    }

    Object.defineProperties(this, {
        mimeType:   {value: mimeType_, writable: false, enumerable: true}
    });
};

tizen.NDEFRecordMedia.prototype = new tizen.NDEFRecord(new InternalData());

tizen.NDEFRecordMedia.prototype.constructor = tizen.NDEFRecordMedia;
//Exports
exports = new NFCManager();
