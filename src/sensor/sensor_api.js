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

var _supportedSensors = [];
var _startedSensors = {
    LIGHT : false,
    MAGNETIC : false,
    PRESSURE : false,
    PROXIMITY : false,
    ULTRAVIOLET : false
};
var _isChecked = false;

function getAvailableSensors() {
    var result = native_.callSync('SensorService_getAvailableSensors', {});
    if (native_.isFailure(result)) {
        throw native_.getErrorObject(result);
    }
    _supportedSensors = native_.getResultObject(result);
    _isChecked = true;
}

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

    if (!_isChecked) {
        getAvailableSensors();
    }

    var index = _supportedSensors.indexOf(args.type);
    if (index === -1) {
        throw new tizen.WebAPIException(tizen.WebAPIException.NOT_SUPPORTED_ERR, 'Not supported.');
    } else if (_supportedSensors[index] === 'LIGHT') {
        return new LightSensor();
    } else if (_supportedSensors[index] === 'MAGNETIC') {
        return new MagneticSensor();
    } else if (_supportedSensors[index] === 'PRESSURE') {
        return new PressureSensor();
    } else if (_supportedSensors[index] === 'PROXIMITY') {
        return new ProximitySensor();
    } else if (_supportedSensors[index] === 'ULTRAVIOLET') {
        return new UltravioletSensor();
    }
};

SensorService.prototype.getAvailableSensors = function() {
    if (!_isChecked) {
        getAvailableSensors();
    }

    return _supportedSensors.slice();
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

    if (!_startedSensors[this.sensorType]) {
        // sensor not started
        var type = this.sensorType;
        native_.call('Sensor_start', {'sensorType' : type},
            function(result) {
                if (native_.isFailure(result)) {
                    if(!T_.isNullOrUndefined(args.errorCallback)) {
                        args.errorCallback(native_.getErrorObject(result));
                    }
                } else {
                    _startedSensors[type] = true;
                    args.successCallback();
                }
            }
        );
    } else {
        // sensor is already started - just call success callback
        setTimeout(function(){args.successCallback()}, 0);
    }
};

Sensor.prototype.stop = function() {
    var result = native_.callSync('Sensor_stop', {'sensorType' : this.sensorType});
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

    var result = native_.callSync('Sensor_setChangeListener', {'sensorType' : this.sensorType});
    if (native_.isFailure(result)) {
        throw native_.getErrorObject(result);
    }
};

Sensor.prototype.unsetChangeListener = function() {
    var result = native_.callSync('Sensor_unsetChangeListener', {'sensorType' : this.sensorType});
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

    if (!_startedSensors[this.sensorType]) {
        setTimeout(function() {
            if (!T_.isNullOrUndefined(args.errorCallback)) {
                args.errorCallback(new tizen.WebAPIException(
                        tizen.WebAPIException.SERVICE_NOT_AVAILABLE_ERR,
                        'Service is not available.'));
            }
        }, 0);
    } else {
        native_.call('Sensor_getData', { type : this.sensorType },
                function(result) {
            if (native_.isFailure(result)) {
                if(!T_.isNullOrUndefined(args.errorCallback)) {
                    args.errorCallback(native_.getErrorObject(result));
                }
            } else {
                args.successCallback(new SensorLightData(result));
            }
        });
    }
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

    if (!_startedSensors[this.sensorType]) {
        setTimeout(function() {
            if (!T_.isNullOrUndefined(args.errorCallback)) {
                args.errorCallback(new tizen.WebAPIException(
                        tizen.WebAPIException.SERVICE_NOT_AVAILABLE_ERR,
                        'Service is not available.'));
            }
        }, 0);
    } else {
        native_.call('Sensor_getData', { type : this.sensorType },
                function(result) {
            if (native_.isFailure(result)) {
                if(!T_.isNullOrUndefined(args.errorCallback)) {
                    args.errorCallback(native_.getErrorObject(result));
                }
            } else {
                args.successCallback(new SensorMagneticData(result));
            }
        });
    }
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

    if (!_startedSensors[this.sensorType]) {
        setTimeout(function() {
            if (!T_.isNullOrUndefined(args.errorCallback)) {
                args.errorCallback(new tizen.WebAPIException(
                        tizen.WebAPIException.SERVICE_NOT_AVAILABLE_ERR,
                        'Service is not available.'));
            }
        }, 0);
    } else {
        native_.call('Sensor_getData', { type : this.sensorType },
                function(result) {
            if (native_.isFailure(result)) {
                if(!T_.isNullOrUndefined(args.errorCallback)) {
                    args.errorCallback(native_.getErrorObject(result));
                }
            } else {
                args.successCallback(new SensorPressureData(result));
            }
        });
    }
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

    if (!_startedSensors[this.sensorType]) {
        setTimeout(function() {
            if (!T_.isNullOrUndefined(args.errorCallback)) {
                args.errorCallback(new tizen.WebAPIException(
                        tizen.WebAPIException.SERVICE_NOT_AVAILABLE_ERR,
                        'Service is not available.'));
            }
        }, 0);
    } else {
        native_.call('Sensor_getData', { type : this.sensorType },
                function(result) {
            if (native_.isFailure(result)) {
                if(!T_.isNullOrUndefined(args.errorCallback)) {
                    args.errorCallback(native_.getErrorObject(result));
                }
            } else {
                args.successCallback(new SensorProximityData(result));
            }
        });
    }
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

    if (!_startedSensors[this.sensorType]) {
        setTimeout(function() {
            if (!T_.isNullOrUndefined(args.errorCallback)) {
                args.errorCallback(new tizen.WebAPIException(
                        tizen.WebAPIException.SERVICE_NOT_AVAILABLE_ERR,
                        'Service is not available.'));
            }
        }, 0);
    } else {
        native_.call('Sensor_getData', { type : this.sensorType },
                function(result) {
            if (native_.isFailure(result)) {
                if(!T_.isNullOrUndefined(args.errorCallback)) {
                    args.errorCallback(native_.getErrorObject(result));
                }
            } else {
                args.successCallback(new SensorUltravioletData(result));
            }
        });
    }
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
