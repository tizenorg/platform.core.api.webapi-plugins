// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var validator_ = xwalk.utils.validator;
var types_ = validator_.Types;
var native_ = new xwalk.utils.NativeManager(extension);

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
    return new NFCAdapter();
};

NFCManager.prototype.setExclusiveMode = function() {

};

//////////////////NFCAdapter /////////////////

function NFCAdapter() {
    Object.defineProperties(this, {
        powered:   {enumerable: true,
            set : function(){},
            get : function(){}},
        cardEmulationMode:   {enumerable: true,
            set : function(){},
            get : function(){}},
        activeSecureElement:   {enumerable: true,
            set : function(){},
            get : function(){}}
    });
};

NFCAdapter.prototype.setPowered = function() {

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

};

NFCAdapter.prototype.removeCardEmulationModeChangeListener = function() {

};

NFCAdapter.prototype.addTransactionEventListener = function() {

};

NFCAdapter.prototype.removeTransactionEventListener = function() {

};

NFCAdapter.prototype.addActiveSecureElementChangeListener = function() {

};

NFCAdapter.prototype.removeActiveSecureElementChangeListener = function() {

};

NFCAdapter.prototype.getCachedMessage = function() {

};

NFCAdapter.prototype.setExclusiveModeForTransaction = function() {

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
var NFCManagerObject = new NFCManager();

exports.getDefaultAdapter = NFCManagerObject.getDefaultAdapter;
exports.setExclusiveMode = NFCManagerObject.setExclusiveMode;
