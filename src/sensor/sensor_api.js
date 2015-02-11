// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var validator_ = xwalk.utils.validator;
var converter_ = xwalk.utils.converter;
var types_ = validator_.Types;
var T_ = xwalk.utils.type;
var native_ = new xwalk.utils.NativeManager(extension);

// Enums
var SensorType = {
    LIGHT : 'LIGHT',
    MAGNETIC : 'MAGNETIC',
    PRESSURE : 'PRESSURE',
    PROXIMITY : 'PROXIMITY',
    ULTRAVIOLET : 'ULTRAVIOLET'
};

var ProximityState = {
    FAR : 'FAR',
    NEAR : 'NEAR'
};

var MagneticSensorAccuracy = {
    UNDEFINED : 'ACCURACY_UNDEFINED',
    BAD : 'ACCURACY_BAD',
    NORMAL : 'ACCURACY_NORMAL',
    GOOD : 'ACCURACY_GOOD',
    VERYGOOD : 'ACCURACY_VERYGOOD'
};

function SensorService() {
};

SensorService.prototype.getDefaultSensor = function() {
    var args = validator_.validateArgs(arguments, [
        {
            name : 'type',
            type : types_.ENUM,
            values : T_.getValues(SensorType)
        }
    ]);

    var result = native_.callSync('SensorService_getDefaultSensor', {});
    if (native_.isFailure(result)) {
        throw native_.getErrorObject(result);
    }
};

SensorService.prototype.getAvailableSensors = function() {
    var result = native_.callSync('SensorService_getAvailableSensors', {});
    if (native_.isFailure(result)) {
        throw native_.getErrorObject(result);
    }
    return [];
};

//////////////////////Sensor classes//////////////////////////////////////////////////////////
//// Base Sensor class
var Sensor = function (type) {
    Object.defineProperties(this, {
        sensorType : {value: type, writable: false, enumerable: true}
    });
};

Sensor.prototype.start = function() {
    var args = validator_.validateArgs(arguments, [
       {
           name : 'successCallback',
           type : types_.FUNCTION
       },
       {
           name : 'errorCallback',
           type : types_.FUNCTION,
           optional : true,
           nullable : true
       }
   ]);

    native_.call('Sensor_start', {},
        function(result) {
            if (native_.isFailure(result)) {
                if(!T_.isNullOrUndefined(args.errorCallback)) {
                    args.errorCallback(native_.getErrorObject(result));
                }
            } else {
                args.successCallback();
            }
        }
    );
};

Sensor.prototype.stop = function() {
    var result = native_.callSync('Sensor_stop', {});
    if (native_.isFailure(result)) {
        throw native_.getErrorObject(result);
    }
};

Sensor.prototype.setChangeListener = function() {
    var args = validator_.validateArgs(arguments, [
       {
           name : 'successCallback',
           type: types_.FUNCTION
       }
   ]);

    var result = native_.callSync('Sensor_setChangeListener', {});
    if (native_.isFailure(result)) {
        throw native_.getErrorObject(result);
    }
};

Sensor.prototype.unsetChangeListener = function() {
    var result = native_.callSync('Sensor_unsetChangeListener', {});
    if (native_.isFailure(result)) {
        throw native_.getErrorObject(result);
    }
};

//// LightSensor
var LightSensor = function(data) {
    Sensor.call(this, SensorType.LIGHT);
};

LightSensor.prototype = new Sensor();

LightSensor.prototype.constructor = Sensor;

LightSensor.prototype.getLightSensorData = function() {
    var args = validator_.validateArgs(arguments, [
       {
           name : 'successCallback',
           type : types_.FUNCTION
       },
       {
           name : 'errorCallback',
           type : types_.FUNCTION,
           optional : true,
           nullable : true
       }
    ]);
    native_.call('LightSensor_getData', {},
        function(result) {
            if (native_.isFailure(result)) {
                if(!T_.isNullOrUndefined(args.errorCallback)) {
                    args.errorCallback(native_.getErrorObject(result));
                }
            } else {
                args.successCallback();
            }
        }
    );
};

//// MagneticSensor
var MagneticSensor = function(data) {
    Sensor.call(this, SensorType.MAGNETIC);
};

MagneticSensor.prototype = new Sensor();

MagneticSensor.prototype.constructor = Sensor;

MagneticSensor.prototype.getMagneticSensorData = function() {
    var args = validator_.validateArgs(arguments, [
       {
           name : 'successCallback',
           type : types_.FUNCTION
       },
       {
           name : 'errorCallback',
           type : types_.FUNCTION,
           optional : true,
           nullable : true
       }
    ]);
    native_.call('MagneticSensor_getData', {},
        function(result) {
            if (native_.isFailure(result)) {
                if(!T_.isNullOrUndefined(args.errorCallback)) {
                    args.errorCallback(native_.getErrorObject(result));
                }
            } else {
                args.successCallback();
            }
        }
    );
};

//// PressureSensor
var PressureSensor = function(data) {
    Sensor.call(this, SensorType.PRESSURE);
};

PressureSensor.prototype = new Sensor();

PressureSensor.prototype.constructor = Sensor;

PressureSensor.prototype.getPressureSensorData = function() {
    var args = validator_.validateArgs(arguments, [
       {
           name : 'successCallback',
           type : types_.FUNCTION
       },
       {
           name : 'errorCallback',
           type : types_.FUNCTION,
           optional : true,
           nullable : true
       }
    ]);
    native_.call('PressureSensor_getData', {},
        function(result) {
            if (native_.isFailure(result)) {
                if(!T_.isNullOrUndefined(args.errorCallback)) {
                    args.errorCallback(native_.getErrorObject(result));
                }
            } else {
                args.successCallback();
            }
        }
    );
};

//// ProximitySensor
var ProximitySensor = function(data) {
    Sensor.call(this, SensorType.PROXIMITY);
};

ProximitySensor.prototype = new Sensor();

ProximitySensor.prototype.constructor = Sensor;

ProximitySensor.prototype.getProximitySensorData = function() {
    var args = validator_.validateArgs(arguments, [
       {
           name : 'successCallback',
           type : types_.FUNCTION
       },
       {
           name : 'errorCallback',
           type : types_.FUNCTION,
           optional : true,
           nullable : true
       }
    ]);
    native_.call('ProximitySensor_getData', {},
        function(result) {
            if (native_.isFailure(result)) {
                if(!T_.isNullOrUndefined(args.errorCallback)) {
                    args.errorCallback(native_.getErrorObject(result));
                }
            } else {
                args.successCallback();
            }
        }
    );
};

//// UltravioletSensor
var UltravioletSensor = function(data) {
    Sensor.call(this, SensorType.ULTRAVIOLET);
};

UltravioletSensor.prototype = new Sensor();

UltravioletSensor.prototype.constructor = Sensor;

UltravioletSensor.prototype.getUltravioletSensorData = function() {
    var args = validator_.validateArgs(arguments, [
       {
           name : 'successCallback',
           type : types_.FUNCTION
       },
       {
           name : 'errorCallback',
           type : types_.FUNCTION,
           optional : true,
           nullable : true
       }
    ]);
    native_.call('UltravioletSensor_getData', {},
        function(result) {
            if (native_.isFailure(result)) {
                if(!T_.isNullOrUndefined(args.errorCallback)) {
                    args.errorCallback(native_.getErrorObject(result));
                }
            } else {
                args.successCallback();
            }
        }
    );
};

////////////////////// Sensor Data classes/////////////////////////////////////////////////////
////Base SensorData class
var SensorData = function () {
};

//// SensorLightData
var SensorLightData = function(data) {
    SensorData.call(this);
    Object.defineProperties(this, {
        lightLevel : {value: data.lightLevel, writable: false, enumerable: true}
    });
};

SensorLightData.prototype = new SensorData();

SensorLightData.prototype.constructor = SensorData;

//// SensorMagneticData
var SensorMagneticData = function(data) {
    SensorData.call(this);
    Object.defineProperties(this, {
        x : {value: data.x, writable: false, enumerable: true},
        y : {value: data.y, writable: false, enumerable: true},
        z : {value: data.z, writable: false, enumerable: true},
        accuracy : {value: data.accuracy, writable: false, enumerable: true}
    });
};

SensorMagneticData.prototype = new SensorData();

SensorMagneticData.prototype.constructor = SensorData;

//// SensorPressureData
var SensorPressureData = function(data) {
    SensorData.call(this);
    Object.defineProperties(this, {
        pressure : {value: data.pressure, writable: false, enumerable: true}
    });
};

SensorPressureData.prototype = new SensorData();

SensorPressureData.prototype.constructor = SensorData;

//// SensorProximityData
var SensorProximityData = function(data) {
    SensorData.call(this);
    Object.defineProperties(this, {
        proximityState : {value: data.proximityState, writable: false, enumerable: true}
    });
};

SensorProximityData.prototype = new SensorData();

SensorProximityData.prototype.constructor = SensorData;

//// SensorUltravioletData
var SensorUltravioletData = function(data) {
    SensorData.call(this);
    Object.defineProperties(this, {
        ultravioletLevel : {value: data.ultravioletLevel, writable: false, enumerable: true}
    });
};

SensorUltravioletData.prototype = new SensorData();

SensorUltravioletData.prototype.constructor = SensorData;

// Exports
exports = new SensorService();
