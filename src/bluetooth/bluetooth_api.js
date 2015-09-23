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
 
var T = xwalk.utils.type;
var Converter = xwalk.utils.converter;
var AV = xwalk.utils.validator;
var Privilege = xwalk.utils.privilege;

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

//class tizen.BluetoothLEServiceData ////////////////////////////////////////////////////
tizen.BluetoothLEServiceData = function(d) {
  AV.isConstructorCall(this, tizen.BluetoothLEServiceData);
  var uuid_ = '';
  var data_ = '';

  Object.defineProperties(this, {
    uuid: {
      enumerable: true,
      get: function() {
        return uuid_;
      },
      set: function(v) {
        uuid_ = Converter.toString(v);
      }
    },
    data: {
      enumerable: true,
      get: function() {
        return data_;
      },
      set: function(v) {
        data_ = Converter.toString(v);
      }
    }
  });

  if (arguments.length >= 2) {  // public constructor
    this.uuid = arguments[0];
    this.data = arguments[1];
  } else if (d && T.isObject(d)) {  // internal constructor
    this.uuid = d.uuid;
    this.data = d.data;
  } else {
    uuid_ = undefined;
    data_ = undefined;
  }
};

//class BluetoothLEAdvertiseData ////////////////////////////////////////////////////
tizen.BluetoothLEAdvertiseData = function(dict) {
  AV.isConstructorCall(this, tizen.BluetoothLEAdvertiseData);
  var includeName_ = false;
  var uuids_ = null;
  var solicitationuuids_ = null;
  var appearance_ = null;
  var includeTxPowerLevel_ = false;
  var serviceData_ = null;
  var manufacturerData_ = null;

  Object.defineProperties(this, {
    includeName: {
      enumerable: true,
      get: function() {
        return includeName_;
      },
      set: function(v) {
        includeName_ = Converter.toBoolean(v, true);
      }
    },
    uuids: {
      enumerable: true,
      get: function() {
        return uuids_;
      },
      set: function(v) {
        if (T.isNull(v)) {
          uuids_ = v;
        } else if (T.isArray(v)) {
          for (var i = 0; i < v.length; ++i) {
            if (!T.isString(v[i])) {
              v[i] = Converter.toString(v[i]);
            }
          }
          uuids_ = v;
        }
      }
    },
    solicitationuuids: {
      enumerable: true,
      get: function() {
        return solicitationuuids_;
      },
      set: function(v) {
        if (T.isNull(v)) {
          solicitationuuids_ = v;
        } else if (T.isArray(v)) {
          for (var i = 0; i < v.length; ++i) {
            if (!T.isString(v[i])) {
              v[i] = Converter.toString(v[i]);
            }
          }
          solicitationuuids_ = v;
        }
      }
    },
    appearance: {
      enumerable: true,
      get: function() {
        return appearance_;
      },
      set: function(v) {
        appearance_ = Converter.toUnsignedLong(v, true);
      }
    },
    includeTxPowerLevel: {
      enumerable: true,
      get: function() {
        return includeTxPowerLevel_;
      },
      set: function(v) {
        includeTxPowerLevel_ = Converter.toBoolean(v, true);
      }
    },
    serviceData: {
      enumerable: true,
      get: function() {
        return serviceData_;
      },
      set: function(v) {
        if (T.isNull(v) || (v instanceof tizen.BluetoothLEServiceData)) {
          serviceData_ = v;
        }
      }
    },
    manufacturerData: {
      enumerable: true,
      get: function() {
        return manufacturerData_;
      },
      set: function(v) {
        if (T.isNull(v) || (v instanceof tizen.BluetoothLEManufacturerData)) {
          manufacturerData_ = v;
        }
      }
    }
  });

  if (T.isObject(dict)) {
    var o = {};

    // includeName
    if (T.isNull(dict.includeName) || T.isBoolean(dict.includeName)) {
      o.includeName = dict.includeName;
    } else if (!T.isUndefined(dict.includeName)) {
      return;
    }

    // uuids
    if (T.isNull(dict.uuids)) {
      o.uuids = dict.uuids;
    } else if (T.isArray(dict.uuids)) {
      for (var i = 0; i < dict.uuids.length; ++i) {
        if (!T.isString(dict.uuids[i])) {
          return;
        }
      }
      o.uuids = dict.uuids;
    } else if (!T.isUndefined(dict.uuids)) {
      return;
    }

    // solicitationuuids
    if (T.isNull(dict.solicitationuuids)) {
      o.solicitationuuids = dict.solicitationuuids;
    } else if (T.isArray(dict.solicitationuuids)) {
      for (var i = 0; i < dict.solicitationuuids.length; ++i) {
        if (!T.isString(dict.solicitationuuids[i])) {
          return;
        }
      }
      o.solicitationuuids = dict.solicitationuuids;
    } else if (!T.isUndefined(dict.solicitationuuids)) {
      return;
    }

    // appearance
    if (T.isNull(dict.appearance) || T.isNumber(dict.appearance)) {
      o.appearance = dict.appearance;
    } else if (!T.isUndefined(dict.appearance)) {
      return;
    }

    // includeTxPowerLevel
    if (T.isNull(dict.includeTxPowerLevel) || T.isBoolean(dict.includeTxPowerLevel)) {
      o.includeTxPowerLevel = dict.includeTxPowerLevel;
    } else if (!T.isUndefined(dict.includeTxPowerLevel)) {
      return;
    }

    // serviceData
    if (T.isNull(dict.serviceData) || dict.serviceData instanceof tizen.BluetoothLEServiceData) {
      o.serviceData = dict.serviceData;
    } else if (!T.isUndefined(dict.serviceData)) {
      return;
    }

    // manufacturerData
    if (T.isNull(dict.manufacturerData) ||
        (dict.manufacturerData instanceof tizen.BluetoothLEManufacturerData)) {
      o.manufacturerData = dict.manufacturerData;
    } else if (!T.isUndefined(dict.manufacturerData)) {
      return;
    }

    for (var prop in o) {
      if (o.hasOwnProperty(prop) && this.hasOwnProperty(prop)) {
        this[prop] = o[prop];
      }
    }
  }
};

//class tizen.BluetoothLEManufacturerData ////////////////////////////////////////////////////
tizen.BluetoothLEManufacturerData = function(d) {
  AV.isConstructorCall(this, tizen.BluetoothLEManufacturerData);
  var id_ = '';
  var data_ = '';

  Object.defineProperties(this, {
    id: {
      enumerable: true,
      get: function() {
        return id_;
      },
      set: function(v) {
        id_ = Converter.toString(v);
      }
    },
    data: {
      enumerable: true,
      get: function() {
        return data_;
      },
      set: function(v) {
        data_ = Converter.toString(v);
      }
    }
  });

  if (arguments.length >= 2) {  // public constructor
    this.id = arguments[0];
    this.data = arguments[1];
  } else if (d && T.isObject(d)) {  // internal constructor
    this.id = d.id;
    this.data = d.data;
  } else {
    id_ = undefined;
    data_ = undefined;
  }
};

// class BluetoothClass ////////////////////////////////////////////////////
var BluetoothClass = function(data) {
    var services = [];
    if (data) {
        services = data.services;
    }

    Object.defineProperties(this, {
        major : {value: data.major, writable: false, enumerable: true},
        minor : {value: data.minor, writable: false, enumerable: true},
        services : {
            enumerable: true,
            set : function(){},
            get : function(){ return services.slice(); }
        }
    });
};

BluetoothClass.prototype.hasService = function() {
    console.log('Entered BluetoothClass.hasService()');
    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_GAP);

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
    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_SPP);

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
    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_SPP);

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
    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_SPP);

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

//class BluetoothLEDevice ////////////////////////////////////////////////////
var BluetoothLEDevice = function(data) {
    var address = "", name = null, txpowerlevel = null, appearance = null, uuids = null,
        solicitationuuids = null, serviceData = null, manufacturerData = null;

    if (data) {
      address = data.address;
      name = data.name || null;
      txpowerlevel = data.txpowerlevel || null;
      appearance = data.appearance || null;
      uuids = data.uuids || null;
      solicitationuuids = data.solicitationuuids || null;
      if (data.serviceData) {
        data.serviceData.forEach(function(d) {
            serviceData.push(new tizen.BluetoothLEServiceData(d));
        });
      }
      if (data.manufacturerData) {
        manufacturerData = new tizen.BluetoothLEManufacturerData(data.manufacturerData);
      }
    }

    Object.defineProperties(this, {
        address : {value: address, writable: false, enumerable: true},
        name : {value: name, writable: false, enumerable: true},
        txpowerlevel : {value: txpowerlevel, writable: false, enumerable: true},
        appearance : {value: appearance, writable: false, enumerable: true},
        uuids : {
            enumerable: true,
            set : function(){},
            get : function(){
              var service_uuids = uuids ? uuids.slice() : null;
              return service_uuids;
            }
        },
        solicitationuuids : {
            enumerable: true,
            set : function(){},
            get : function(){ return solicitationuuids ? solicitationuuids.slice() : null; }
        },
        serviceData : {
            enumerable: true,
            set : function(){},
            get : function(){ return serviceData ? serviceData.slice() : null; }
        },
        manufacturerData : {
            value: manufacturerData,
            writable: false,
            enumerable: true}
    });
};

BluetoothLEDevice.prototype.connect = function() {
    console.log('Entered BluetoothLEDevice.connect()');

    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_ADMIN);
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

    native.call('BluetoothLEDevice_connect', {address : this.address}, callback);
};

BluetoothLEDevice.prototype.disconnect = function() {
    console.log('Entered BluetoothLEDevice.disconnect()');

    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_ADMIN);
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

    var result = native.call('BluetoothLEDevice_disconnect', {address : this.address}, callback);
    if (native.isFailure(result)) {
        throw native.getErrorObject(result);
    }
};

BluetoothLEDevice.prototype.getService = function() {
    console.log('Entered BluetoothLEDevice.getService()');

    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_ADMIN);

    var args = AV.validateMethod(arguments, [
        {
            name: 'uuid',
            type: AV.Types.STRING
        }
    ]);

    var callArgs = {
        uuid : args.uuid,
        address : this.address
    };

    var result = native.callSync('BluetoothLEDevice_getService', callArgs);

    if (native.isFailure(result)) {
        throw native.getErrorObject(result);
    } else {
        return new BluetoothGATTService(native.getResultObject(result));
    }
};

BluetoothLEDevice.prototype.addConnectStateChangeListener = function() {
    console.log('Entered BluetoothLEDevice.addConnectStateChangeListener()');

    var args = AV.validateMethod(arguments, [
        {
            name: 'listener',
            type: AV.Types.LISTENER,
            values: ['onconnected', 'ondisconnected']
        }
    ]);

    var that = this;

    var func = function(event) {
        if (event.address === that.address && args.listener[event.action]) {
            args.listener[event.action](that);
        }
    }

    var watchId = _bleConnectChangeListener.addListener(func);

    return watchId;
};

BluetoothLEDevice.prototype.removeConnectStateChangeListener = function() {
    console.log('Entered BluetoothLEDevice.removeConnectStateChangeListener()');

    var args = AV.validateMethod(arguments, [
        {
            name: 'watchID',
            type: AV.Types.LONG
        }
    ]);

    _bleConnectChangeListener.removeListener(args.watchID);
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

    var uuids = [];
    if (data) {
        uuids = data.uuids;
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
        uuids : {
            enumerable: true,
            set : function(){},
            get : function(){ return uuids.slice(); }
        }
    });
};

BluetoothDevice.prototype.connectToServiceByUUID = function() {
    console.log('Entered BluetoothDevice.connectToServiceByUUID()');
    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_SPP);

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
    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_SPP);

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
    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_HEALTH);

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
    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_HEALTH);

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
    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_HEALTH);

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
    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_HEALTH);

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
    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_HEALTH);

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

BluetoothHealthChannel.prototype.setListener = function() {
    console.log('Entered BluetoothHealthChannel.setListener()');
    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_HEALTH);

    var args = AV.validateMethod(arguments, [
        {
            name : 'changeCallback',
            type : AV.Types.LISTENER,
            values : ['onmessage', 'onclose']
        }
    ]);

    if (T.isEmptyObject(_healthListeners)) {
        native.addListener('BluetoothHealthChannelChangeCallback',
                _BluetoothHealthChannelChangeCallback);
    }
    _healthListeners[this._id] = args.changeCallback;
};

BluetoothHealthChannel.prototype.unsetListener  = function() {
    console.log('Entered BluetoothHealthChannel.unsetListener ()');
    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_HEALTH);

    delete _healthListeners[this._id];

    if (T.isEmptyObject(_healthListeners)) {
        native.removeListener('BluetoothHealthChannelChangeCallback',
                _BluetoothHealthChannelChangeCallback);
    }
};


/**
 * Creates a manager for specified listener event.
 *
 * @param {string} name - name of the listener this manager handles
 * @param {function} callback - function to be invoked when event specified by the name fires.
 *                              This function should return false if the callback doesn't want
 *                              to handle the event anymore, true otherwise.
 *                              This function should have following signature:
 *                              bool callback(event, successCallback, errorCallback);
 *
 * @return {object} object which allows to add or remove callbacks for specified listener
 */
function _singleListenerBuilder(name, callback) {
  var listenerName = name;
  var successCallback;
  var errorCallback;
  var callbackFunction = callback;
  var listenerRegistered = false;

  function innerCallback(event) {
    if (!callbackFunction(event, successCallback, errorCallback)) {
      removeListener();
    }
  }

  function addListener(s, e) {
    successCallback = s;
    errorCallback = e;

    if (!listenerRegistered) {
      native.addListener(listenerName, innerCallback);
      listenerRegistered = true;
    }
  }

  function removeListener() {
    if (listenerRegistered) {
      native.removeListener(listenerName, innerCallback);
      listenerRegistered = false;
    }

    successCallback = undefined;
    errorCallback = undefined;
  }

  return {
    addListener: addListener,
    removeListener: removeListener
  };
}

var _bleScanListener = _singleListenerBuilder('BluetoothLEScanCallback',
    function(event, successCallback, errorCallback) {

  var d;
  var ret = true;

  switch (event.action) {
    case 'onsuccess':
      d = new BluetoothLEDevice(event.data);
      break;

    case 'onerror':
      if (errorCallback) {
        errorCallback(native.getErrorObject(event));
      }
      return ret;

    default:
      console.log('Unknown mode: ' + event.action);
      return ret;
  }
  if (successCallback) {
    successCallback(d);
  }

  return ret;
});

var _bleAdvertiseListener = _singleListenerBuilder('BluetoothLEAdvertiseCallback',
    function(event, successCallback, errorCallback) {
  var d;
  var ret = true;

  switch (event.action) {
    case 'onstate':
      if (successCallback) {
        successCallback(native.getResultObject(event));
      }
      return ret;

    case 'onerror':
      if (errorCallback) {
        errorCallback(native.getErrorObject(event));
      }
      return ret;

    default:
      console.log('Unknown mode: ' + event.action);
      return ret;
  }
});

//class BluetoothLEAdapter ////////////////////////////////////////////////////
var BluetoothLEAdapter = function() {
};

BluetoothLEAdapter.prototype.startScan = function() {
  console.log('Entered BluetoothLEAdapter.startScan()');

  xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_ADMIN);

  var args = AV.validateMethod(arguments, [{
    name: 'successCallback',
    type: AV.Types.FUNCTION
  }, {
    name: 'errorCallback',
    type: AV.Types.FUNCTION,
    optional: true,
    nullable: true
  }]);

  var result = native.callSync('BluetoothLEAdapter_startScan', {});

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }

  _bleScanListener.addListener(args.successCallback, args.errorCallback);
};

BluetoothLEAdapter.prototype.stopScan = function() {
  console.log('Entered BluetoothLEAdapter.stopScan()');

  xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_ADMIN);

  _bleScanListener.removeListener();

  var result = native.callSync('BluetoothLEAdapter_stopScan', {});

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
};

var _BluetoothAdvertisePacketType = {
  ADVERTISE: 'ADVERTISE',
  SCAN_RESPONSE: 'SCAN_RESPONSE'
};

var _BluetoothAdvertisingMode = {
  BALANCED: 'BALANCED',
  LOW_LATENCY: 'LOW_LATENCY',
  LOW_ENERGY: 'LOW_ENERGY'
};

BluetoothLEAdapter.prototype.startAdvertise = function() {
  console.log('Entered BluetoothLEAdapter.startAdvertise()');

  xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_ADMIN);

  var args = AV.validateMethod(arguments, [{
    name: 'advertiseData',
    type: AV.Types.PLATFORM_OBJECT,
    values: tizen.BluetoothLEAdvertiseData
  }, {
    name: 'packetType',
    type: AV.Types.ENUM,
    values: T.getValues(_BluetoothAdvertisePacketType)
  }, {
    name: 'successCallback',
    type: AV.Types.FUNCTION
  }, {
    name: 'errorCallback',
    type: AV.Types.FUNCTION,
    optional: true,
    nullable: true
  }, {
    name: 'mode',
    type: AV.Types.ENUM,
    values: T.getValues(_BluetoothAdvertisingMode),
    optional: true,
    nullable: true
  }, {
    name: 'connectable',
    type: AV.Types.BOOLEAN,
    optional: true,
    nullable: true
  }]);

  var callArgs = {
    advertiseData: args.advertiseData,
    packetType: args.packetType,
    mode: T.isNullOrUndefined(args.mode) ? _BluetoothAdvertisingMode.BALANCED : args.mode,
    connectable: T.isNullOrUndefined(args.connectable) ? true : args.connectable
  };

  var result = native.callSync('BluetoothLEAdapter_startAdvertise', callArgs);

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }

  _bleAdvertiseListener.addListener(args.successCallback, args.errorCallback);
};

BluetoothLEAdapter.prototype.stopAdvertise = function() {
  console.log('Entered BluetoothLEAdapter.stopAdvertise()');

  xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_ADMIN);

  // TODO: when should we call _bleAdvertiseListener.removeListener()?

  var result = native.callSync('BluetoothLEAdapter_stopAdvertise', {});

  if (native.isFailure(result)) {
    _bleAdvertiseListener.removeListener();
    throw native.getErrorObject(result);
  }
};

//class BluetoothGATTService ////////////////////////////////////////////////////
var BluetoothGATTService = function(data, address) {
    var handle_ = data.handle;
    var uuid_ = data.uuid;
    //address_ is needed to control if device is still connected
    var address_ = address || data.address;
    function servicesGetter() {
        var services = [];
        var result = native.callSync('BluetoothGATTService_getServices',
                {handle: handle_, address : address_});
        if (native.isSuccess(result)) {
            var resultObject = native.getResultObject(result);
            resultObject.forEach(function(s) {
                services.push(new BluetoothGATTService(s, address_));
            });
        }
        return services;
    }
    function characteristicsGetter() {
        var characteristics = [];
        var result = native.callSync('BluetoothGATTService_getCharacteristics',
                {handle: handle_, uuid: uuid_, address : address_});
        if (native.isSuccess(result)) {
            var resultObject = native.getResultObject(result);
            resultObject.forEach(function(c) {
                characteristics.push(new BluetoothGATTCharacteristic(c, address_));
            });
        }
        return characteristics;
    }
    Object.defineProperties(this, {
        uuid : {value: uuid_, writable: false, enumerable: true},
        services : {enumerable: true, set : function() {}, get : servicesGetter},
        characteristics : {enumerable: true, set : function() {}, get : characteristicsGetter}
    });
};

var toByteArray = function(array) {
    var d = [];

    array.forEach(function(b) {
        d.push(Converter.toOctet(b));
    });
    return d;
};

//class BluetoothGATTCharacteristic ////////////////////////////////////////////////////
var BluetoothGATTCharacteristic = function(data, address) {
  var handle_ = data.handle;
  var descriptors_ = [];
  var isBroadcast_ = false;
  var hasExtendedProperties_ = false;
  var isNotify_ = false;
  var isIndication_ = false;
  var isReadable_ = false;
  var isSignedWrite_ = false;
  var isWritable_ = false;
  var isWriteNoResponse_ = false;
  //address_ is needed to control if device is still connected
  var address_ = address;

  if (T.isObject(data)) {
    data.descriptors.forEach(function(dd) {
      descriptors_.push(new BluetoothGATTDescriptor(dd, address_));
    });
    isBroadcast_ = data.isBroadcast;
    hasExtendedProperties_ = data.hasExtendedProperties;
    isNotify_ = data.isNotify;
    isIndication_ = data.isIndication;
    isReadable_ = data.isReadable;
    isSignedWrite_ = data.isSignedWrite;
    isWritable_ = data.isWritable;
    isWriteNoResponse_ = data.isWriteNoResponse;
  }

  Object.defineProperties(this, {
    descriptors: {
      enumerable: true,
      get: function() {
        return descriptors_.slice();
      },
      set: function() {
      }
    },
    isBroadcast: {
      enumerable: true,
      get: function() {
        return isBroadcast_;
      },
      set: function() {
      }
    },
    hasExtendedProperties: {
      enumerable: true,
      get: function() {
        return hasExtendedProperties_;
      },
      set: function() {
      }
    },
    isNotify: {
      enumerable: true,
      get: function() {
        return isNotify_;
      },
      set: function() {
      }
    },
    isIndication: {
      enumerable: true,
      get: function() {
        return isIndication_;
      },
      set: function() {
      }
    },
    isReadable: {
      enumerable: true,
      get: function() {
        return isReadable_;
      },
      set: function() {
      }
    },
    isSignedWrite: {
      enumerable: true,
      get: function() {
        return isSignedWrite_;
      },
      set: function() {
      }
    },
    isWritable: {
      enumerable: true,
      get: function() {
        return isWritable_;
      },
      set: function() {
      }
    },
    isWriteNoResponse: {
      enumerable: true,
      get: function() {
        return isWriteNoResponse_;
      },
      set: function() {
      }
    }
  });

  this.readValue = function() {
      console.log('Entered BluetoothGATTCharacteristic.readValue()');

      xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_ADMIN);

      var args = AV.validateMethod(arguments, [{
        name: 'successCallback',
        type: AV.Types.FUNCTION
      }, {
        name: 'errorCallback',
        type: AV.Types.FUNCTION,
        optional: true,
        nullable: true
      }]);

      var callback = function(result) {
        if (native.isFailure(result)) {
          native.callIfPossible(args.errorCallback, native.getErrorObject(result));
        } else {
          var d = toByteArray(native.getResultObject(result));
          args.successCallback(d);
        }
      };

      var callArgs = {handle : handle_, address : address_};

      var result = native.call('BluetoothGATT_readValue', callArgs, callback);

      if (native.isFailure(result)) {
        throw native.getErrorObject(result);
      }
    };

    this.writeValue = function() {
      console.log('Entered BluetoothGATTCharacteristic.writeValue()');

      xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_ADMIN);

      var args = AV.validateMethod(arguments, [{
        name: 'value',
        type: AV.Types.ARRAY,
        values: AV.Types.BYTE
      }, {
        name: 'successCallback',
        type: AV.Types.FUNCTION,
        optional: true,
        nullable: true
      }, {
        name: 'errorCallback',
        type: AV.Types.FUNCTION,
        optional: true,
        nullable: true
      }]);

      var callback = function(result) {
        if (native.isFailure(result)) {
          native.callIfPossible(args.errorCallback, native.getErrorObject(result));
        } else {
          native.callIfPossible(args.successCallback);
        }
      };

      var callArgs = { handle : handle_, value: toByteArray(args.value), address : address_ };

      var result = native.call('BluetoothGATT_writeValue', callArgs, callback);

      if (native.isFailure(result)) {
        throw native.getErrorObject(result);
      }
    };

  this.addValueChangeListener = function() {
    console.log('Entered BluetoothGATTCharacteristic.addValueChangeListener()');

    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_ADMIN);

    var args = AV.validateMethod(arguments, [{
      name: 'callback',
      type: AV.Types.FUNCTION
    }]);

    var callArgs = { handle: handle_, address : address_ };

    var callback = function(event) {
      if (event.handle === handle_) {
        args.callback(toByteArray(native.getResultObject(event)));
      }
    };

    return _bluetoothGATTCharacteristicListener.addListener(callback, callArgs);
  };

  this.removeValueChangeListener = function() {
    console.log('Entered BluetoothGATTCharacteristic.removeValueChangeListener()');

    var args = AV.validateMethod(arguments, [{
      name: 'watchID',
      type: AV.Types.LONG
    }]);

    var callArgs = { handle: handle_, address : address_ };

    return _bluetoothGATTCharacteristicListener.removeListener(args.watchID, callArgs);
  };
};


/**
 * Creates a manager for specified listener event. Manager handles multiple
 * registered listeners
 *
 * @param {string} name - name of the listener this manager handles
 * @param {function} callback - function to be invoked when event specified by the name fires.
 *                              This function should have following signature:
 *                              void callback(listener, event);
 * @param {string} addListenerId - optional parameter. If specified, this native
 *                                 method will be called synchronously when
 *                                 listener is added.
 * @param {string} removeListenerId - optional parameter. If specified, this native
 *                                 method will be called synchronously when
 *                                 listener is removed.
 * @param {bool} repeatNativeCall - optional parameter. If specified, the addListenerId
 *                                 and removeListenerId methods will be called synchronously
 *                                 each time listener is added/removed. Otherwise they are
 *                                 going to be called just once: when first listener is added
 *                                 and last listener is removed.
 *
 * @return {object} object which allows to add or remove callbacks for specified listener
 */
function _multipleListenerBuilder(name, callback, addListenerId, removeListenerId, repeatNativeCall) {
  var listenerName = name;
  var addId = addListenerId;
  var removeId = removeListenerId;
  var callbackFunction = callback;
  var listeners = {};
  var nextId = 1;
  var jsListenerRegistered = false;
  var nativeListenerRegistered = false;
  var repeatNativeListenerCall = repeatNativeCall;

  function innerCallback(event) {
    for (var watchId in listeners) {
      if (listeners.hasOwnProperty(watchId)) {
        callbackFunction(listeners[watchId], event);
      }
    }
  }

  function addListener(callback, args) {
    var id = ++nextId;

    if (addId && (!nativeListenerRegistered || repeatNativeListenerCall)) {
      var result = native.callSync(addId, args || {});
      if (native.isFailure(result)) {
        throw native.getErrorObject(result);
      }
      nativeListenerRegistered = true;
    }

    if (!jsListenerRegistered) {
      native.addListener(listenerName, innerCallback);
      jsListenerRegistered = true;
    }

    listeners[id] = callback;
    return id;
  }

  function removeListener(watchId, args) {
    if (listeners.hasOwnProperty(watchId)) {
      delete listeners[watchId];
    }

    if (removeId && ((nativeListenerRegistered && T.isEmptyObject(listeners)) || repeatNativeListenerCall)) {
      var result = native.callSync(removeId, args || {});
      if (native.isFailure(result)) {
        throw native.getErrorObject(result);
      }
      nativeListenerRegistered = false;
    }

    if (jsListenerRegistered && T.isEmptyObject(listeners)) {
      native.removeListener(listenerName, innerCallback);
      jsListenerRegistered = false;
    }
  }

  return {
    addListener: addListener,
    removeListener: removeListener
  };
}

var _bluetoothGATTCharacteristicListener = _multipleListenerBuilder(
    'BluetoothGATTCharacteristicValueChangeListener',
    function(listener, event) {
      listener(event);
    },
    'BluetoothGATTCharacteristic_addValueChangeListener',
    'BluetoothGATTCharacteristic_removeValueChangeListener',
    true
);

var _bleConnectChangeListener = _multipleListenerBuilder(
    'BluetoothLEConnectChangeCallback',
    function(listener, event) {
        listener(event);
    },
    'BluetoothLEDevice_addConnectStateChangeListener',
    'BluetoothLEDevice_removeConnectStateChangeListener'
);

//class BluetoothGATTDescriptor ////////////////////////////////////////////////////
var BluetoothGATTDescriptor = function(data, address) {
  var handle_ = data.handle;
  //address_ is needed to control if device is still connected
  var address_ = address;

  this.readValue = function() {
    console.log('Entered BluetoothGATTDescriptor.readValue()');

    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_ADMIN);

    var args = AV.validateMethod(arguments, [{
      name: 'successCallback',
      type: AV.Types.FUNCTION
    }, {
      name: 'errorCallback',
      type: AV.Types.FUNCTION,
      optional: true,
      nullable: true
    }]);

    var callback = function(result) {
      if (native.isFailure(result)) {
        native.callIfPossible(args.errorCallback, native.getErrorObject(result));
      } else {
        var d = toByteArray(native.getResultObject(result));
        args.successCallback(d);
      }
    };

    var callArgs = {handle : handle_, address : address_};

    var result = native.call('BluetoothGATT_readValue', callArgs, callback);

    if (native.isFailure(result)) {
      throw native.getErrorObject(result);
    }
  };

  this.writeValue = function() {
    console.log('Entered BluetoothGATTDescriptor.writeValue()');

    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_ADMIN);

    var args = AV.validateMethod(arguments, [{
      name: 'value',
      type: AV.Types.ARRAY,
      values: AV.Types.BYTE
    }, {
      name: 'successCallback',
      type: AV.Types.FUNCTION,
      optional: true,
      nullable: true
    }, {
      name: 'errorCallback',
      type: AV.Types.FUNCTION,
      optional: true,
      nullable: true
    }]);

    var callback = function(result) {
      if (native.isFailure(result)) {
        native.callIfPossible(args.errorCallback, native.getErrorObject(result));
      } else {
        native.callIfPossible(args.successCallback);
      }
    };

    var callArgs = { handle : handle_, value: toByteArray(args.value), address : address_ };

    var result = native.call('BluetoothGATT_writeValue', callArgs, callback);

    if (native.isFailure(result)) {
      throw native.getErrorObject(result);
    }
  };
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
    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_ADMIN);

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
    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_ADMIN);

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


// This method is deprecated since Tizen 2.3 and will be removed in Tizen 3.0.
BluetoothAdapter.prototype.setVisible = function() {
    console.log('Entered BluetoothAdapter.setVisible()');
    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTHMANAGER);

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
    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_GAP);

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
    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_GAP);

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
    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_GAP);

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
    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_GAP);

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
    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_GAP);

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
    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_GAP);

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
    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_SPP);

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
            throw new WebAPIException('NotSupportedError', 'Profile ' + args.profileType + ' is not supported.');
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

    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_GAP);

    return new BluetoothAdapter();
};

BluetoothManager.prototype.getLEAdapter = function() {
    console.log('Entered BluetoothManager.getLEAdapter()');

    xwalk.utils.checkPrivilegeAccess4Ver("2.4", Privilege.BLUETOOTH, Privilege.BLUETOOTH_ADMIN);

    return new BluetoothLEAdapter();
};
// exports ///////////////////////////////////////////////////////////////////
exports = new BluetoothManager();
