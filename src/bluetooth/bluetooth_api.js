//@ sourceURL=bluetooth_api.js

// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var T = xwalk.utils.type;
var Converter = xwalk.utils.converter;
var AV = xwalk.utils.validator;

var native = new xwalk.utils.NativeManager(extension);

// class BluetoothClassDeviceMajor /////////////////////////////////////////
var BluetoothClassDeviceMajor = function() {
    Object.defineProperties(this, {
        MISC:          {value: 0x00, writable: false, enumerable: true},
        COMPUTER:      {value: 0x01, writable: false, enumerable: true},
        PHONE:         {value: 0x02, writable: false, enumerable: true},
        NETWORK:       {value: 0x03, writable: false, enumerable: true},
        AUDIO_VIDEO:   {value: 0x04, writable: false, enumerable: true},
        PERIPHERAL:    {value: 0x05, writable: false, enumerable: true},
        IMAGING:       {value: 0x06, writable: false, enumerable: true},
        WEARABLE:      {value: 0x07, writable: false, enumerable: true},
        TOY:           {value: 0x08, writable: false, enumerable: true},
        HEALTH:        {value: 0x09, writable: false, enumerable: true},
        UNCATEGORIZED: {value: 0x1F, writable: false, enumerable: true}
    });
};

// class BluetoothClassDeviceMinor /////////////////////////////////////////
var BluetoothClassDeviceMinor = function() {
    Object.defineProperties(this, {
        COMPUTER_UNCATEGORIZED:           {value: 0x00, writable: false, enumerable: true},
        COMPUTER_DESKTOP:                 {value: 0x01, writable: false, enumerable: true},
        COMPUTER_SERVER:                  {value: 0x02, writable: false, enumerable: true},
        COMPUTER_LAPTOP:                  {value: 0x03, writable: false, enumerable: true},
        COMPUTER_HANDHELD_PC_OR_PDA:      {value: 0x04, writable: false, enumerable: true},
        COMPUTER_PALM_PC_OR_PDA:          {value: 0x05, writable: false, enumerable: true},
        COMPUTER_WEARABLE:                {value: 0x06, writable: false, enumerable: true},

        PHONE_UNCATEGORIZED:              {value: 0x00, writable: false, enumerable: true},
        PHONE_CELLULAR:                   {value: 0x01, writable: false, enumerable: true},
        PHONE_CORDLESS:                   {value: 0x02, writable: false, enumerable: true},
        PHONE_SMARTPHONE:                 {value: 0x03, writable: false, enumerable: true},
        PHONE_MODEM_OR_GATEWAY:           {value: 0x04, writable: false, enumerable: true},
        PHONE_ISDN:                       {value: 0x05, writable: false, enumerable: true},

        AV_UNRECOGNIZED:                  {value: 0x00, writable: false, enumerable: true},
        AV_WEARABLE_HEADSET:              {value: 0x01, writable: false, enumerable: true},
        AV_HANDSFREE:                     {value: 0x02, writable: false, enumerable: true},
        AV_MICROPHONE:                    {value: 0x04, writable: false, enumerable: true},
        AV_LOUDSPEAKER:                   {value: 0x05, writable: false, enumerable: true},
        AV_HEADPHONES:                    {value: 0x06, writable: false, enumerable: true},
        AV_PORTABLE_AUDIO:                {value: 0x07, writable: false, enumerable: true},
        AV_CAR_AUDIO:                     {value: 0x08, writable: false, enumerable: true},
        AV_SETTOP_BOX:                    {value: 0x09, writable: false, enumerable: true},
        AV_HIFI:                          {value: 0x0A, writable: false, enumerable: true},
        AV_VCR:                           {value: 0x0B, writable: false, enumerable: true},
        AV_VIDEO_CAMERA:                  {value: 0x0C, writable: false, enumerable: true},
        AV_CAMCORDER:                     {value: 0x0D, writable: false, enumerable: true},
        AV_MONITOR:                       {value: 0x0E, writable: false, enumerable: true},
        AV_DISPLAY_AND_LOUDSPEAKER:       {value: 0x0F, writable: false, enumerable: true},
        AV_VIDEO_CONFERENCING:            {value: 0x10, writable: false, enumerable: true},
        AV_GAMING_TOY:                    {value: 0x12, writable: false, enumerable: true},

        PERIPHERAL_UNCATEGORIZED:         {value: 0x00, writable: false, enumerable: true},
        PERIPHERAL_KEYBOARD:              {value: 0x10, writable: false, enumerable: true},
        PERIPHERAL_POINTING_DEVICE:       {value: 0x20, writable: false, enumerable: true},
        PERIPHERAL_KEYBOARD_AND_POINTING_DEVICE: {
            value: 0x30,
            writable: false,
            enumerable: true
        },
        PERIPHERAL_JOYSTICK:              {value: 0x01, writable: false, enumerable: true},
        PERIPHERAL_GAMEPAD:               {value: 0x02, writable: false, enumerable: true},
        PERIPHERAL_REMOTE_CONTROL:        {value: 0x03, writable: false, enumerable: true},
        PERIPHERAL_SENSING_DEVICE:        {value: 0x04, writable: false, enumerable: true},
        PERIPHERAL_DEGITIZER_TABLET:      {value: 0x05, writable: false, enumerable: true},
        PERIPHERAL_CARD_READER:           {value: 0x06, writable: false, enumerable: true},
        PERIPHERAL_DIGITAL_PEN:           {value: 0x07, writable: false, enumerable: true},
        PERIPHERAL_HANDHELD_SCANNER:      {value: 0x08, writable: false, enumerable: true},
        PERIPHERAL_HANDHELD_INPUT_DEVICE: {value: 0x09, writable: false, enumerable: true},

        IMAGING_UNCATEGORIZED:            {value: 0x00, writable: false, enumerable: true},
        IMAGING_DISPLAY:                  {value: 0x04, writable: false, enumerable: true},
        IMAGING_CAMERA:                   {value: 0x08, writable: false, enumerable: true},
        IMAGING_SCANNER:                  {value: 0x10, writable: false, enumerable: true},
        IMAGING_PRINTER:                  {value: 0x20, writable: false, enumerable: true},

        WEARABLE_WRITST_WATCH:            {value: 0x01, writable: false, enumerable: true},
        WEARABLE_PAGER:                   {value: 0x02, writable: false, enumerable: true},
        WEARABLE_JACKET:                  {value: 0x03, writable: false, enumerable: true},
        WEARABLE_HELMET:                  {value: 0x04, writable: false, enumerable: true},
        WEARABLE_GLASSES:                 {value: 0x05, writable: false, enumerable: true},

        TOY_ROBOT:                        {value: 0x01, writable: false, enumerable: true},
        TOY_VEHICLE:                      {value: 0x02, writable: false, enumerable: true},
        TOY_DOLL:                         {value: 0x03, writable: false, enumerable: true},
        TOY_CONTROLLER:                   {value: 0x04, writable: false, enumerable: true},
        TOY_GAME:                         {value: 0x05, writable: false, enumerable: true},

        HEALTH_UNDEFINED:                 {value: 0x00, writable: false, enumerable: true},
        HEALTH_BLOOD_PRESSURE_MONITOR:    {value: 0x01, writable: false, enumerable: true},
        HEALTH_THERMOMETER:               {value: 0x02, writable: false, enumerable: true},
        HEALTH_WEIGHING_SCALE:            {value: 0x03, writable: false, enumerable: true},
        HEALTH_GLUCOSE_METER:             {value: 0x04, writable: false, enumerable: true},
        HEALTH_PULSE_OXIMETER:            {value: 0x05, writable: false, enumerable: true},
        HEALTH_PULSE_RATE_MONITOR:        {value: 0x06, writable: false, enumerable: true},
        HEALTH_DATA_DISPLAY:              {value: 0x07, writable: false, enumerable: true},
        HEALTH_STEP_COUNTER:              {value: 0x08, writable: false, enumerable: true},
        HEALTH_BODY_COMPOSITION_ANALYZER: {value: 0x09, writable: false, enumerable: true},
        HEALTH_PEAK_FLOW_MONITOR:         {value: 0x0A, writable: false, enumerable: true},
        HEALTH_MEDICATION_MONITOR:        {value: 0x0B, writable: false, enumerable: true},
        HEALTH_KNEE_PROSTHESIS:           {value: 0x0C, writable: false, enumerable: true},
        HEALTH_ANKLE_PROSTHESIS:          {value: 0x0D, writable: false, enumerable: true}
    });
};

// class BluetoothClassDeviceService ///////////////////////////////////////
var BluetoothClassDeviceService = function() {
    Object.defineProperties(this, {
        LIMITED_DISCOVERABILITY: {value: 0x0001, writable: false, enumerable: true},
        POSITIONING:             {value: 0x0008, writable: false, enumerable: true},
        NETWORKING:              {value: 0x0010, writable: false, enumerable: true},
        RENDERING:               {value: 0x0020, writable: false, enumerable: true},
        CAPTURING:               {value: 0x0040, writable: false, enumerable: true},
        OBJECT_TRANSFER:         {value: 0x0080, writable: false, enumerable: true},
        AUDIO:                   {value: 0x0100, writable: false, enumerable: true},
        TELEPHONY:               {value: 0x0200, writable: false, enumerable: true},
        INFORMATION:             {value: 0x0400, writable: false, enumerable: true}
    });
};

// class BluetoothClass ////////////////////////////////////////////////////
var BluetoothClass = function(data) {
    Object.defineProperties(this, {
        major : {value: data.major, writable: false, enumerable: true},
        minor : {value: data.minor, writable: false, enumerable: true},
        services : {value: data.services, writable: false, enumerable: true}
    });
};

var _PRIVILEGE_BLUETOOTH_GAP = 'http://tizen.org/privilege/bluetooth.gap';

BluetoothClass.prototype.hasService = function() {
    console.log('Entered BluetoothClass.hasService()');

    var result = native.callSync('Bluetooth_checkPrivilege', {privilege : _PRIVILEGE_BLUETOOTH_GAP});

    if (native.isFailure(result)) {
        throw native.getErrorObject(result);
    } else {
        var args = AV.validateMethod(arguments, [
            {
                name : 'service',
                type : AV.Types.UNSIGNED_LONG
            }
        ]);

        var size = this.services.length;
        for (var i = 0; i < size; i++) {
            if (this.services[i] === args.service) {
                return true;
            }
        }
        return false;
    }
};

// class BluetoothSocket ////////////////////////////////////////////////////
var _BLUETOOTH_SOCKET_STATE_CLOSED = 'CLOSED';

function BluetoothSocketListeners() {
    var that = this;
    this.socketCallback = function (data) {
        var event = data;
        var socket = that.sockets[event.id];

        if (socket) {
            if ('onclose' === event.event) {
                // no more events
                that.removeListener(event.id);
                // change state
                Object.defineProperty(socket, 'state', {value : _BLUETOOTH_SOCKET_STATE_CLOSED});
            }

            var callback = socket[event.event];
            if (T.isFunction(callback)) {
                callback();
            }
        } else {
            console.log('Received event for an unknown socket: ' + event.id);
        }
    };
}

BluetoothSocketListeners.prototype.sockets = {};

BluetoothSocketListeners.prototype.addListener = function(socket) {
    if (T.isEmptyObject(this.sockets)) {
        native.addListener('BLUETOOTH_SOCKET_STATE_CHANGED', this.socketCallback);
    }

    this.sockets[socket._id] = socket;
};

BluetoothSocketListeners.prototype.removeListener = function(id) {
    delete this.sockets[id];

    if (T.isEmptyObject(this.sockets)) {
        native.removeListener('BLUETOOTH_SOCKET_STATE_CHANGED', this.socketCallback);
    }
};

var _bluetoothSocketListeners = new BluetoothSocketListeners();

var BluetoothSocket = function(data) {
    Object.defineProperties(this, {
        uuid : {value: data.uuid, writable: false, enumerable: true},
        state : {value: data.state, writable: false, enumerable: true, configurable: true},
        peer : {value: new BluetoothDevice(data.peer), writable: false, enumerable: true},
        onmessage : {value: null, writable: true, enumerable: true},
        onclose : {value: null, writable: true, enumerable: true},
        _id : {value: data.id, writable: false, enumerable: false}
    });

    _bluetoothSocketListeners.addListener(this);
};

BluetoothSocket.prototype.writeData = function() {
    console.log('Entered BluetoothSocket.writeData()');

    var args = AV.validateMethod(arguments, [
        {
            name : 'data',
            type : AV.Types.ARRAY,
            values : AV.Types.BYTE
        }
    ]);

    var callArgs = {
        id : this._id,
        data : args.data
    };

    var result = native.callSync('BluetoothSocket_writeData', callArgs);

    if (native.isFailure(result)) {
        throw native.getErrorObject(result);
    } else {
        return native.getResultObject(result);
    }
};

BluetoothSocket.prototype.readData = function() {
    console.log('Entered BluetoothSocket.readData()');

    var callArgs = {
        id : this._id
    };

    var result = native.callSync('BluetoothSocket_readData', callArgs);

    if (native.isFailure(result)) {
        throw native.getErrorObject(result);
    } else {
        return native.getResultObject(result);
    }
};

BluetoothSocket.prototype.close = function() {
    console.log('Entered BluetoothSocket.close()');

    if (_BLUETOOTH_SOCKET_STATE_CLOSED !== this.state) {
        var callArgs = {
            id : this._id
        };

        var result = native.callSync('BluetoothSocket_close', callArgs);

        if (native.isFailure(result)) {
            throw native.getErrorObject(result);
        }

        // change state
        Object.defineProperty(this, 'state', { value : _BLUETOOTH_SOCKET_STATE_CLOSED });
    }
};

// class BluetoothDevice ////////////////////////////////////////////////////
var BluetoothDevice = function(data) {
    var self = this;
    function _getter(field) {
        var callArgs = {};

        callArgs.address = self.address;
        callArgs.field = field;

        var result = native.callSync('BluetoothDevice_getBoolValue', callArgs);

        if (native.isFailure(result)) {
            return false;
        } else {
            return native.getResultObject(result);
        }
    }

    function isBondedGetter() {
        return _getter('isBonded');
    }

    function isTrustedGetter() {
        return _getter('isTrusted');
    }

    function isConnectedGetter() {
        return _getter('isConnected');
    }

    Object.defineProperties(this, {
        name : {value: data.name, writable: false, enumerable: true},
        address : {value: data.address, writable: false, enumerable: true},
        deviceClass : {value: new BluetoothClass(data.deviceClass),
            writable: false,
            enumerable: true},
        isBonded : {
            enumerable: true,
            set : function(){},
            get : isBondedGetter
        },
        isTrusted : {
            enumerable: true,
            set : function(){},
            get : isTrustedGetter
        },
        isConnected : {
            enumerable: true,
            set : function(){},
            get : isConnectedGetter
        },
        uuids : {value: data.uuids, writable: false, enumerable: true}
    });
};

BluetoothDevice.prototype.connectToServiceByUUID = function() {
    console.log('Entered BluetoothDevice.connectToServiceByUUID()');

    var args = AV.validateMethod(arguments, [
        {
            name : 'uuid',
            type : AV.Types.STRING
        },
        {
            name : 'successCallback',
            type : AV.Types.FUNCTION
        },
        {
            name : 'errorCallback',
            type : AV.Types.FUNCTION,
            optional : true,
            nullable : true
        }
    ]);

    var callArgs = {
        address : this.address,
        uuid : args.uuid
    };
    var callback = function(result) {
        if (native.isFailure(result)) {
            native.callIfPossible(args.errorCallback, native.getErrorObject(result));
        } else {
            args.successCallback(new BluetoothSocket(native.getResultObject(result)));
        }
    };

    // native.call does not inform if call results in failure
    // TODO: what to do in this case?
    native.call('BluetoothDevice_connectToServiceByUUID', callArgs, callback);
};

// class BluetoothServiceHandler ////////////////////////////////////////////////////
function BluetoothServiceListeners() {
    var that = this;
    this.serviceCallback = function (data) {
        var e = data;
        var service = that.services[e.uuid];
        var result = new BluetoothSocket(e);

        if (service) {
            console.log(service);
            service.onconnect(result);
        }
    };
}

BluetoothServiceListeners.prototype.services = {};

BluetoothServiceListeners.prototype.addListener = function(service) {
    if (T.isEmptyObject(this.services)) {
        native.addListener('BLUETOOTH_SERVICE_ONCONNECT', this.serviceCallback);
    }

    this.services[service.uuid] = service;
};

BluetoothServiceListeners.prototype.removeListener = function(uuid) {
    delete this.services[uuid];

    if (T.isEmptyObject(this.services)) {
        native.removeListener('BLUETOOTH_SERVICE_ONCONNECT', this.serviceCallback);
    }
};

var _bluetoothServiceListeners = new BluetoothServiceListeners();

var BluetoothServiceHandler = function(data) {
    function isConnectedGetter() {
        var callArgs = {
            uuid : this.uuid
        };

        var result = native.callSync('BluetoothAdapter_isServiceConnected', { uuid : this.uuid });

        if (native.isFailure(result)) {
            return false;
        } else {
            return native.getResultObject(result);
        }
    }

    Object.defineProperties(this, {
        uuid : {value: data.uuid, writable: false, enumerable: true},
        name : {value: data.name, writable: false, enumerable: true},
        isConnected : {
            enumerable: true,
            set : function(){},
            get : isConnectedGetter
        },
        onconnect : {value: null, writable: true, enumerable: true}
    });

    _bluetoothServiceListeners.addListener(this);
};

BluetoothServiceHandler.prototype.unregister = function() {
    console.log('Entered BluetoothServiceHandler.unregister()');
    var args = AV.validateMethod(arguments, [
        {
            name : 'successCallback',
            type : AV.Types.FUNCTION,
            optional : true,
            nullable : true
        },
        {
            name : 'errorCallback',
            type : AV.Types.FUNCTION,
            optional : true,
            nullable : true
        }
    ]);

    var callArgs = {
        uuid : this.uuid
    };

    var callback = function(result) {
        if (native.isFailure(result)) {
            native.callIfPossible(args.errorCallback, native.getErrorObject(result));
        } else {
            native.callIfPossible(args.successCallback);
        }
    };

    // native.call does not inform if call results in failure
    // TODO: what to do in this case?
    native.call('BluetoothServiceHandler_unregister', callArgs, callback);

    _bluetoothServiceListeners.removeListener(this.uuid);
};

// class BluetoothHealthApplication ////////////////////////////////////////////////////
function BluetoothHealthApplicationListeners() {
    var that = this;
    this.appCallback = function (data) {
        var event = data;
        var app = that.apps[event.id];

        if (app) {
            var callback = app[event.event];
            if (T.isFunction(callback)) {
                var param;
                switch (event.event) {
                case 'onconnect':
                    param = new BluetoothHealthChannel(native.getResultObject(event));
                    break;

                default:
                    console.log('Unknown event: ' + event.event);
                    break;
                }
                callback(param);
            }
        } else {
            console.log('Received event for an unknown application: ' + event.id);
        }
    };
}

BluetoothHealthApplicationListeners.prototype.apps = {};

BluetoothHealthApplicationListeners.prototype.addListener = function(app) {
    if (T.isEmptyObject(this.apps)) {
        native.addListener('BLUETOOTH_HEALTH_APPLICATION_CHANGED', this.appCallback);
    }

    this.apps[app._id] = app;
};

BluetoothHealthApplicationListeners.prototype.removeListener = function(id) {
    delete this.apps[id];

    if (T.isEmptyObject(this.apps)) {
        native.removeListener('BLUETOOTH_HEALTH_APPLICATION_CHANGED', this.appCallback);
    }
};

var _bluetoothHealthApplicationListeners = new BluetoothHealthApplicationListeners();

var BluetoothHealthApplication = function(data) {
    Object.defineProperties(this, {
        dataType : {value: data.dataType, writable: false, enumerable: true},
        name : {value: data.name, writable: false, enumerable: true},
        onconnect : {value: null, writable: true, enumerable: true},
        _id : {value: data._id, writable: false, enumerable: false}
    });

    _bluetoothHealthApplicationListeners.addListener(this);
};

BluetoothHealthApplication.prototype.unregister = function() {
    console.log('Entered BluetoothHealthApplication.unregister()');
    var args = AV.validateMethod(arguments, [
        {
            name : 'successCallback',
            type : AV.Types.FUNCTION,
            optional : true,
            nullable : true
        },
        {
            name : 'errorCallback',
            type : AV.Types.FUNCTION,
            optional : true,
            nullable : true
        }
    ]);

    var callArgs = {id : this._id};

    var callback = function(result) {
        if (native.isFailure(result)) {
            native.callIfPossible(args.errorCallback, native.getErrorObject(result));
        } else {
            native.callIfPossible(args.successCallback);
        }
    };

    // native.call does not inform if call results in failure
    // TODO: what to do in this case?
    native.call('BluetoothHealthApplication_unregister', callArgs, callback);

    _bluetoothHealthApplicationListeners.removeListener(this._id);
};

// class BluetoothProfileHandler ////////////////////////////////////////////////////
var _BluetoothProfileType = {
    HEALTH : 'HEALTH'
};

var BluetoothProfileHandler = function(data) {
    if (data) {
        Object.defineProperties(this, {
            profileType : {value: data.profileType, writable: false, enumerable: true}
        });
    }
};

// class BluetoothHealthProfileHandler ////////////////////////////////////////////////////
var BluetoothHealthProfileHandler = function(data) {
    BluetoothProfileHandler.call(this, data);
};

BluetoothHealthProfileHandler.prototype = new BluetoothProfileHandler();

BluetoothHealthProfileHandler.prototype.constructor = BluetoothProfileHandler;

BluetoothHealthProfileHandler.prototype.registerSinkApplication = function() {
    console.log('Entered BluetoothHealthProfileHandler.registerSinkApplication()');

    var args = AV.validateMethod(arguments, [
        {
            name : 'dataType',
            type : AV.Types.LONG // there's no short type
        },
        {
            name : 'name',
            type : AV.Types.STRING
        },
        {
            name : 'successCallback',
            type : AV.Types.FUNCTION
        },
        {
            name : 'errorCallback',
            type : AV.Types.FUNCTION,
            optional : true,
            nullable : true
        }
    ]);

    var callArgs = {
        dataType : args.dataType,
        name : args.name
    };

    var callback = function(result) {
        if (native.isFailure(result)) {
            native.callIfPossible(args.errorCallback, native.getErrorObject(result));
        } else {
            args.successCallback(new BluetoothHealthApplication(native.getResultObject(result)));
        }
    };

    // native.call does not inform if call results in failure
    // TODO: what to do in this case?
    native.call('BluetoothHealthProfileHandler_registerSinkApp', callArgs, callback);
};

BluetoothHealthProfileHandler.prototype.connectToSource = function() {
    console.log('Entered BluetoothHealthProfileHandler.connectToSource()');

    var args = AV.validateMethod(arguments, [
        {
            name : 'peer',
            type : AV.Types.PLATFORM_OBJECT,
            values : BluetoothDevice
        },
        {
            name : 'application',
            type : AV.Types.PLATFORM_OBJECT,
            values : BluetoothHealthApplication
        },
        {
            name : 'successCallback',
            type : AV.Types.FUNCTION
        },
        {
            name : 'errorCallback',
            type : AV.Types.FUNCTION,
            optional : true,
            nullable : true
        }
    ]);

    var callArgs = {
        address : args.peer.address,
        appId : args.application._id
    };

    var callback = function(result) {
        if (native.isFailure(result)) {
            native.callIfPossible(args.errorCallback, native.getErrorObject(result));
        } else {
            var channel = native.getResultObject(result);
            channel.peer = args.peer;
            channel.appId = args.application._id;
            args.successCallback(new BluetoothHealthChannel(channel));
        }
    };

    // native.call does not inform if call results in failure
    // TODO: what to do in this case?
    native.call('BluetoothHealthProfileHandler_connectToSource', callArgs, callback);
};

// class BluetoothHealthChannel ////////////////////////////////////////////////////
var BluetoothHealthChannel = function(data) {
    Object.defineProperties(this, {
        peer : {value: data.peer, writable: false, enumerable: true},
        channelType : {value: data.channelType, writable: false, enumerable: true},
        application : {
            value: _bluetoothHealthApplicationListeners.apps[data.appId],
            writable: false,
            enumerable: true
        },
        isConnected : {
            value: data.isConnected,
            writable: false,
            enumerable: true,
            configurable: true},
        _id : {value: data._id, writable: false, enumerable: false}
    });
};

BluetoothHealthChannel.prototype.close = function() {
    console.log('Entered BluetoothHealthChannel.close()');

    if (this.isConnected) {
        var callArgs = {
            channel : this._id,
            address : this.peer.address
        };

        var result = native.callSync('BluetoothHealthChannel_close', callArgs);

        if (native.isFailure(result)) {
            throw native.getErrorObject(result);
        }

        Object.defineProperty(this, 'isConnected', { value : false });
    }
};

BluetoothHealthChannel.prototype.sendData = function() {
    console.log('Entered BluetoothHealthChannel.sendData()');

    var args = AV.validateMethod(arguments, [
        {
            name : 'data',
            type : AV.Types.ARRAY,
            values : AV.Types.BYTE
        }
    ]);

    var callArgs = {
        channel : this._id,
        data : args.data
    };

    var result = native.callSync('BluetoothHealthChannel_sendData', callArgs);

    if (native.isFailure(result)) {
        throw native.getErrorObject(result);
    } else {
        return native.getResultObject(result);
    }
};

var _healthListeners = {};

function _BluetoothHealthChannelChangeCallback(event) {
    var e = event;
    var callback = _healthListeners[e.id];
    var d;

    switch (e.event) {
    case 'onmessage':
        d = e.data;
        break;

    case 'onclose':
        break;

    default:
        console.log('Unknown mode: ' + e.event);
        return;
    }

    if (callback[e.event]) {
        callback[e.event](d);
    }
}

var _PRIVILEGE_BLUETOOTH_HEALTH = 'http://tizen.org/privilege/bluetooth.health';

BluetoothHealthChannel.prototype.setListener = function() {
    console.log('Entered BluetoothHealthChannel.setListener()');
    var args = AV.validateMethod(arguments, [
        {
            name : 'changeCallback',
            type : AV.Types.LISTENER,
            values : ['onmessage', 'onclose']
        }
    ]);

    var result = native.callSync('Bluetooth_checkPrivilege', {privilege : _PRIVILEGE_BLUETOOTH_HEALTH});

    if (native.isFailure(result)) {
        throw native.getErrorObject(result);
    } else {
        if (T.isEmptyObject(_healthListeners)) {
            native.addListener('BluetoothHealthChannelChangeCallback',
                    _BluetoothHealthChannelChangeCallback);
        }
        _healthListeners[this._id] = args.changeCallback;
    }
};

BluetoothHealthChannel.prototype.unsetListener  = function() {
    console.log('Entered BluetoothHealthChannel.unsetListener ()');

    var result = native.callSync('Bluetooth_checkPrivilege', {privilege : _PRIVILEGE_BLUETOOTH_HEALTH});

    if (native.isFailure(result)) {
        throw native.getErrorObject(result);
    } else {
        delete _healthListeners[this._id];

        if (T.isEmptyObject(_healthListeners)) {
            native.removeListener('BluetoothHealthChannelChangeCallback',
                    _BluetoothHealthChannelChangeCallback);
        }
    }
};

// class BluetoothAdapter ////////////////////////////////////////////////////
var BluetoothAdapter = function() {
    function nameGetter() {
        var result = native.callSync('BluetoothAdapter_getName', {});

        if (native.isFailure(result)) {
            return '';
        } else {
            return native.getResultObject(result);
        }
    }

    function addressGetter() {
        var result = native.callSync('BluetoothAdapter_getAddress', {});

        if (native.isFailure(result)) {
            return '';
        } else {
            return native.getResultObject(result);
        }
    }

    function poweredGetter() {
        var result = native.callSync('BluetoothAdapter_getPowered', {});

        if (native.isFailure(result)) {
            return false;
        } else {
            return native.getResultObject(result);
        }
    }

    function visibleGetter() {
        var result = native.callSync('BluetoothAdapter_getVisible', {});

        if (native.isFailure(result)) {
            return false;
        } else {
            return native.getResultObject(result);
        }
    }

    Object.defineProperties(this, {
        name : {
            enumerable: true,
            set : function(){},
            get : nameGetter
        },
        address : {
            enumerable: true,
            set : function(){},
            get : addressGetter
        },
        powered : {
            enumerable: true,
            set : function(){},
            get : poweredGetter
        },
        visible : {
            enumerable: true,
            set : function(){},
            get : visibleGetter
        }
    });
};

BluetoothAdapter.prototype.setName = function() {
    console.log('Entered BluetoothAdapter.setName()');
    var args = AV.validateMethod(arguments, [
        {
            name : 'name',
            type : AV.Types.STRING
        },
        {
            name : 'successCallback',
            type : AV.Types.FUNCTION,
            optional : true,
            nullable : true
        },
        {
            name : 'errorCallback',
            type : AV.Types.FUNCTION,
            optional : true,
            nullable : true
        }
    ]);

    var callArgs = {
        name : args.name
    };

    var callback = function(result) {
        if (native.isFailure(result)) {
            native.callIfPossible(args.errorCallback, native.getErrorObject(result));
        } else {
            native.callIfPossible(args.successCallback);
        }
    };

    // native.call does not inform if call results in failure
    // TODO: what to do in this case?
    native.call('BluetoothAdapter_setName', callArgs, callback);
};

BluetoothAdapter.prototype.setPowered = function() {
    console.log('Entered BluetoothAdapter.setPowered()');
    var args = AV.validateMethod(arguments, [
        {
            name : 'powered',
            type : AV.Types.BOOLEAN
        },
        {
            name : 'successCallback',
            type : AV.Types.FUNCTION,
            optional : true,
            nullable : true
        },
        {
            name : 'errorCallback',
            type : AV.Types.FUNCTION,
            optional : true,
            nullable : true
        }
    ]);

    var callArgs = {
        powered : args.powered
    };

    var callback = function(result) {
        if (native.isFailure(result)) {
            native.callIfPossible(args.errorCallback, native.getErrorObject(result));
        } else {
            native.callIfPossible(args.successCallback);
        }
    };

    // native.call does not inform if call results in failure
    // TODO: what to do in this case?
    native.call('BluetoothAdapter_setPowered', callArgs, callback);
};

BluetoothAdapter.prototype.setVisible = function() {
    console.log('Entered BluetoothAdapter.setVisible()');
    var args = AV.validateMethod(arguments, [
        {
            name : 'visible',
            type : AV.Types.BOOLEAN
        },
        {
            name : 'successCallback',
            type : AV.Types.FUNCTION,
            optional : true,
            nullable : true
        },
        {
            name : 'errorCallback',
            type : AV.Types.FUNCTION,
            optional : true,
            nullable : true
        },
        {
            name : 'timeout',
            type : AV.Types.UNSIGNED_LONG,
            optional : true,
            nullable : true
        }
    ]);

    var callArgs = {
        visible : args.visible,
    };

    if (args.visible === true) {
        if (T.isNullOrUndefined(args.timeout)) {
            callArgs.timeout = 0;
        } else {
            callArgs.timeout = args.timeout > 65535 ? 180 : args.timeout;
        }
    }

    var callback = function(result) {
        if (native.isFailure(result)) {
            native.callIfPossible(args.errorCallback, native.getErrorObject(result));
        } else {
            native.callIfPossible(args.successCallback);
        }
    };

    // native.call does not inform if call results in failure
    // TODO: what to do in this case?
    native.call('BluetoothAdapter_setVisible', callArgs, callback);
};

var _listener;

function _BluetoothAdapterChangeCallback(event) {
    console.log('_BluetoothAdapterChangeCallback');

    var e = event;
    var d;

    switch (e.action) {
    case 'onstatechanged':
        d = e.powered;
        break;

    case 'onnamechanged':
        d = e.name;
        break;

    case 'onvisibilitychanged':
        d = e.visible;
        break;

    default:
        console.log('Unknown mode: ' + e.action);
        return;
    }

    if (_listener[e.action]) {
        _listener[e.action](d);
    }
}

BluetoothAdapter.prototype.setChangeListener = function() {
    console.log('Entered BluetoothAdapter.setChangeListener()');
    var args = AV.validateMethod(arguments, [
        {
            name : 'changeCallback',
            type : AV.Types.LISTENER,
            values : ['onstatechanged', 'onnamechanged', 'onvisibilitychanged']
        }
    ]);

    if (T.isNullOrUndefined(_listener)) {
        native.addListener('BluetoothAdapterChangeCallback', _BluetoothAdapterChangeCallback);
    }
    _listener = args.changeCallback;
};

BluetoothAdapter.prototype.unsetChangeListener = function() {
    console.log('Entered BluetoothAdapter.unsetChangeListener()');
    if (!T.isNullOrUndefined(_listener)) {
        native.removeListener('BluetoothAdapterChangeCallback', _BluetoothAdapterChangeCallback);
        _listener = undefined;
    }
};

var _discoverDevicesSuccessCallback;
var _discoverDevicesErrorCallback;

function _BluetoothDiscoverDevicesSuccessCallback(event) {
    var e = event;
    var d = null;

    switch (e.action) {
    case 'onstarted':
        break;

    case 'ondevicefound':
        d = new BluetoothDevice(e.data);
        break;

    case 'ondevicedisappeared':
        d = e.data;
        break;

    case 'onfinished':
        var result = e.data;
        d = [];
        result.forEach(function (data) {
            d.push(new BluetoothDevice(data));
        });

        //remove listeners after discovering
        native.removeListener('BluetoothDiscoverDevicesSuccessCallback',
                _BluetoothDiscoverDevicesSuccessCallback);
        native.removeListener('BluetoothDiscoverDevicesErrorCallback',
                _BluetoothDiscoverDevicesErrorCallback);
        break;

    default:
        console.log('Unknown mode: ' + e.action);
        return;
    }

    if (_discoverDevicesSuccessCallback[e.action]) {
        _discoverDevicesSuccessCallback[e.action](d);
    }
}

function _BluetoothDiscoverDevicesErrorCallback(event) {
    var e = event;
    setTimeout(function() {
        native.callIfPossible(_discoverDevicesErrorCallback, native.getErrorObject(e));
    }, 0);
}

BluetoothAdapter.prototype.discoverDevices = function() {
    console.log('Entered BluetoothAdapter.discoverDevices()');
    var args = AV.validateMethod(arguments, [
        {
            name : 'successCallback',
            type : AV.Types.LISTENER,
            values : ['onstarted', 'ondevicefound', 'ondevicedisappeared', 'onfinished']
        },
        {
            name : 'errorCallback',
            type : AV.Types.FUNCTION,
            optional : true,
            nullable : true
        }
    ]);

    _discoverDevicesSuccessCallback = args.successCallback;
    _discoverDevicesErrorCallback = args.errorCallback;
    native.addListener('BluetoothDiscoverDevicesSuccessCallback',
            _BluetoothDiscoverDevicesSuccessCallback);
    native.addListener('BluetoothDiscoverDevicesErrorCallback',
            _BluetoothDiscoverDevicesErrorCallback);

    var result = native.callSync('BluetoothAdapter_discoverDevices', {});

    if (native.isFailure(result)) {
        native.removeListener('BluetoothDiscoverDevicesSuccessCallback',
                _BluetoothDiscoverDevicesSuccessCallback);
        native.removeListener('BluetoothDiscoverDevicesErrorCallback',
                _BluetoothDiscoverDevicesErrorCallback);
        throw native.getErrorObject(result);
    }
};

BluetoothAdapter.prototype.stopDiscovery = function() {
    console.log('Entered BluetoothAdapter.stopDiscovery()');
    var args = AV.validateMethod(arguments, [
        {
            name : 'successCallback',
            type : AV.Types.FUNCTION,
            optional : true,
            nullable : true
        },
        {
            name : 'errorCallback',
            type : AV.Types.FUNCTION,
            optional : true,
            nullable : true
        }
    ]);

    var callback = function(result) {
        if (native.isFailure(result)) {
            native.callIfPossible(args.errorCallback, native.getErrorObject(result));
        } else {
            native.callIfPossible(args.successCallback);
        }
    };

    // native.call does not inform if call results in failure
    // TODO: what to do in this case?
    native.call('BluetoothAdapter_stopDiscovery', {}, callback);
};

BluetoothAdapter.prototype.getKnownDevices = function() {
    console.log('Entered BluetoothAdapter.getKnownDevices()');
    var args = AV.validateMethod(arguments, [
        {
            name : 'successCallback',
            type : AV.Types.FUNCTION
        },
        {
            name : 'errorCallback',
            type : AV.Types.FUNCTION,
            optional : true,
            nullable : true
        }
    ]);

    var callback = function(result) {
        if (native.isFailure(result)) {
            native.callIfPossible(args.errorCallback, native.getErrorObject(result));
        } else {
            var r = native.getResultObject(result).devices;
            var devices = [];
            r.forEach(function (data) {
                devices.push(new BluetoothDevice(data));
            });
            args.successCallback(devices);
        }
    };

    // native.call does not inform if call results in failure
    // TODO: what to do in this case?
    native.call('BluetoothAdapter_getKnownDevices', {}, callback);
};

BluetoothAdapter.prototype.getDevice = function() {
    console.log('Entered BluetoothAdapter.getDevice()');
    var args = AV.validateMethod(arguments, [
        {
            name : 'address',
            type : AV.Types.STRING
        },
        {
            name : 'successCallback',
            type : AV.Types.FUNCTION
        },
        {
            name : 'errorCallback',
            type : AV.Types.FUNCTION,
            optional : true,
            nullable : true
        }
    ]);

    var callback = function(result) {
        if (native.isFailure(result)) {
            native.callIfPossible(args.errorCallback, native.getErrorObject(result));
        } else {
            args.successCallback(new BluetoothDevice(native.getResultObject(result)));
        }
    };

    // native.call does not inform if call results in failure
    // TODO: what to do in this case?
    native.call('BluetoothAdapter_getDevice', {address : args.address}, callback);
};

BluetoothAdapter.prototype.createBonding = function() {
    console.log('Entered BluetoothAdapter.createBonding()');
    var args = AV.validateMethod(arguments, [
        {
            name : 'address',
            type : AV.Types.STRING
        },
        {
            name : 'successCallback',
            type : AV.Types.FUNCTION,
            optional : false,
            nullable : false
        },
        {
            name : 'errorCallback',
            type : AV.Types.FUNCTION,
            optional : true,
            nullable : true
        }
    ]);

    var callArgs = {
        address : args.address
    };

    var callback = function(result) {
        if (native.isFailure(result)) {
            native.callIfPossible(args.errorCallback, native.getErrorObject(result));
        } else {
            args.successCallback(new BluetoothDevice(native.getResultObject(result)));
        }
    };

    // native.call does not inform if call results in failure
    // TODO: what to do in this case?
    native.call('BluetoothAdapter_createBonding', callArgs, callback);
};

BluetoothAdapter.prototype.destroyBonding = function() {
    console.log('Entered BluetoothAdapter.destroyBonding()');
    var args = AV.validateMethod(arguments, [
        {
            name : 'address',
            type : AV.Types.STRING
        },
        {
            name : 'successCallback',
            type : AV.Types.FUNCTION,
            optional : true,
            nullable : true
        },
        {
            name : 'errorCallback',
            type : AV.Types.FUNCTION,
            optional : true,
            nullable : true
        }
    ]);

    var callArgs = {
        address : args.address
    };

    var callback = function(result) {
        if (native.isFailure(result)) {
            native.callIfPossible(args.errorCallback, native.getErrorObject(result));
        } else {
            native.callIfPossible(args.successCallback);
        }
    };

    // native.call does not inform if call results in failure
    // TODO: what to do in this case?
    native.call('BluetoothAdapter_destroyBonding', callArgs, callback);
};

BluetoothAdapter.prototype.registerRFCOMMServiceByUUID = function() {
    console.log('Entered BluetoothAdapter.registerRFCOMMServiceByUUID()');
    var args = AV.validateMethod(arguments, [
        {
            name : 'uuid',
            type : AV.Types.STRING
        },
        {
            name : 'name',
            type : AV.Types.STRING
        },
        {
            name : 'successCallback',
            type : AV.Types.FUNCTION,
        },
        {
            name : 'errorCallback',
            type : AV.Types.FUNCTION,
            optional : true,
            nullable : true
        }
    ]);

    var callArgs = {
        uuid : args.uuid,
        name : args.name
    };

    var callback = function(result) {
        if (native.isFailure(result)) {
            native.callIfPossible(args.errorCallback, native.getErrorObject(result));
        } else {
            // if registration was finished with success create BluetoothServiceHandler
            // with parameters passed to this function (uuid and name).
            args.successCallback(new BluetoothServiceHandler(callArgs));
        }
    };

    // native.call does not inform if call results in failure
    // TODO: what to do in this case?
    native.call('BluetoothAdapter_registerRFCOMMServiceByUUID', callArgs, callback);
};

BluetoothAdapter.prototype.getBluetoothProfileHandler = function() {
    console.log('Entered BluetoothAdapter.getBluetoothProfileHandler()');

    var args = AV.validateMethod(arguments, [
        {
            name : 'profileType',
            type : AV.Types.ENUM,
            values : T.getValues(_BluetoothProfileType)
        }
    ]);

    var callArgs = {profileType : args.profileType};

    var result = native.callSync('BluetoothAdapter_getBluetoothProfileHandler', callArgs);

    if (native.isFailure(result)) {
        throw native.getErrorObject(result);
    } else {
        switch (args.profileType) {
        case _BluetoothProfileType.HEALTH:
            return new BluetoothHealthProfileHandler(callArgs);

        default:
            throw new tizen.WebAPIException('NotSupportedError', 'Profile ' + args.profileType + ' is not supported.');
        }
    }
};

// class BluetoothManager ////////////////////////////////////////////////////
var BluetoothManager = function() {
    Object.defineProperties(this, {
        deviceMajor : {
            value: new BluetoothClassDeviceMajor(),
            writable: false,
            enumerable: true
        },
        deviceMinor : {
            value: new BluetoothClassDeviceMinor(),
            writable: false,
            enumerable: true
        },
        deviceService : {
            value: new BluetoothClassDeviceService(),
            writable: false,
            enumerable: true
        }
    });
};

BluetoothManager.prototype.getDefaultAdapter = function() {
    console.log('Entered BluetoothManager.getDefaultAdapter()');

    var result = native.callSync('Bluetooth_checkPrivilege', {privilege : _PRIVILEGE_BLUETOOTH_GAP});

    if (native.isFailure(result)) {
        throw native.getErrorObject(result);
    } else {
        return new BluetoothAdapter();
    }
};

// exports ///////////////////////////////////////////////////////////////////
exports = new BluetoothManager();
