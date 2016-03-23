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

var utils_ = xwalk.utils;
var privilege_ = utils_.privilege;
var validator_ = utils_.validator;
var converter_ = utils_.converter;
var T_ = utils_.type;
var types_ = validator_.Types;
var native_ = new utils_.NativeManager(extension);

// Enums
var SensorType = {
    LIGHT : 'LIGHT',
    MAGNETIC : 'MAGNETIC',
    PRESSURE : 'PRESSURE',
    PROXIMITY : 'PROXIMITY',
    ULTRAVIOLET : 'ULTRAVIOLET',
    HRM_RAW : 'HRM_RAW',
    GRAVITY : 'GRAVITY',
    GYROSCOPE : 'GYROSCOPE',
    GYROSCOPE_ROTATION_VECTOR : 'GYROSCOPE_ROTATION_VECTOR',
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

// helper class for sensor listeners
var SensorListener = function (type, constructor) {
    this.sensorType = type;
    this.isStarted = false;
    this.callback = undefined;
    this.constructor = constructor;
};

SensorListener.prototype.tryCall = function (object) {
    if (this.callback) {
        this.callback(new this.constructor(object));
    }
};

SensorListener.prototype.start = function (successCallback, errorCallback) {
    if (!this.isStarted) {
        // sensor not started
        var thisObject = this;
        native_.call('Sensor_start', {'sensorType' : thisObject.sensorType},
                function(result) {
                    if (native_.isFailure(result)) {
                        if(!T_.isNullOrUndefined(errorCallback)) {
                            errorCallback(native_.getErrorObject(result));
                        }
                    } else {
                        thisObject.isStarted = true;
                        successCallback();
                    }
                }
        );
    } else {
        // sensor is already started - just call success callback
        setTimeout(function(){successCallback()}, 0);
    }
};

SensorListener.prototype.stop = function () {
    if (this.isStarted) {
        var result = native_.callSync('Sensor_stop', {'sensorType' : this.sensorType});
        if (native_.isFailure(result)) {
            throw native_.getErrorObject(result);
        }
        this.isStarted = false;
    }
};

SensorListener.prototype.setListener = function (successCallback, interval) {
    if (!this.callback) {
        //call platform only if there was no listener registered
        var result = native_.callSync('Sensor_setChangeListener', {'sensorType' : this.sensorType, 'interval' : interval});
        if (native_.isFailure(result)) {
            throw native_.getErrorObject(result);
        }
    }
    this.callback = successCallback;
};

SensorListener.prototype.unsetListener = function () {
    if (this.callback) {
        //unregister in platform only if there is callback registered
        this.callback = undefined;
        var result = native_.callSync('Sensor_unsetChangeListener', {'sensorType' : this.sensorType});
        if (native_.isFailure(result)) {
            throw native_.getErrorObject(result);
        }
    }
};

SensorListener.prototype.getData = function (successCallback, errorCallback) {
    var thisObj = this;
    if (!thisObj.isStarted) {
        setTimeout(function() {
            if (!T_.isNullOrUndefined(errorCallback)) {
                errorCallback(new WebAPIException(
                        WebAPIException.SERVICE_NOT_AVAILABLE_ERR,
                        'Service is not available.'));
            }
        }, 0);
    } else {
        native_.call('Sensor_getData', { type : thisObj.sensorType },
                function(result) {
            if (native_.isFailure(result)) {
                if(!T_.isNullOrUndefined(errorCallback)) {
                    errorCallback(native_.getErrorObject(result));
                }
            } else {
                successCallback(new thisObj.constructor(result));
            }
        });
    }
};

var _supportedSensors = [];
var _isChecked = false;
var _sensorListeners = {
    'LIGHT'       : {},
    'MAGNETIC'    : {},
    'PRESSURE'    : {},
    'PROXIMITY'   : {},
    'ULTRAVIOLET' : {},
    'HRM_RAW'     : {},
    'GRAVITY'     : {},
    'GYROSCOPE'   : {},
    'GYROSCOPE_ROTATION_VECTOR' : {},
};

var _listener = function(object) {
    _sensorListeners[object.sensorType].tryCall(object);
};

var SENSOR_CHANGED_LISTENER = 'SensorChangedListener';
native_.addListener(SENSOR_CHANGED_LISTENER, _listener);

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

function getDefaultSensor() {
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
        throw new WebAPIException(WebAPIException.NOT_SUPPORTED_ERR, 'Not supported.');
    } else if (_supportedSensors[index] === SensorType.LIGHT) {
        return new LightSensor();
    } else if (_supportedSensors[index] === SensorType.MAGNETIC) {
        return new MagneticSensor();
    } else if (_supportedSensors[index] === SensorType.PRESSURE) {
        return new PressureSensor();
    } else if (_supportedSensors[index] === SensorType.PROXIMITY) {
        return new ProximitySensor();
    } else if (_supportedSensors[index] === SensorType.ULTRAVIOLET) {
        return new UltravioletSensor();
    } else if (_supportedSensors[index] === SensorType.HRM_RAW) {
        utils_.checkPrivilegeAccess(privilege_.HEALTHINFO);
        return new HRMRawSensor();
    } else if (_supportedSensors[index] === SensorType.GRAVITY) {
        return new GravitySensor();
    } else if(_supportedSensors[index] === SensorType.GYROSCOPE){
        return new GyroscopeSensor();
    } else if(_supportedSensors[index] === SensorType.GYROSCOPE_ROTATION_VECTOR){
        return new GyroscopeRotationVectorSensor();
    }
};

SensorService.prototype.getDefaultSensor = function() {
  return getDefaultSensor.apply(this, arguments);
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

    _sensorListeners[this.sensorType].start(args.successCallback, args.errorCallback);
};

Sensor.prototype.stop = function() {
    _sensorListeners[this.sensorType].stop();
};

Sensor.prototype.setChangeListener = function() {
    var args = validator_.validateArgs(arguments, [
       {
           name : 'successCallback',
           type: types_.FUNCTION
       },
       {
           name : 'interval',
           type: types_.LONG,
           optional: true,
           nullable: true
       }
    ]);

    var interval = args.interval || 100;
    if (interval < 10 || interval > 1000) {
      throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR, 'Interval should be in range [10, 1000] milliseconds.');
    }

    _sensorListeners[this.sensorType].setListener(args.successCallback, interval);
};

Sensor.prototype.unsetChangeListener = function() {
    _sensorListeners[this.sensorType].unsetListener();
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

    _sensorListeners[this.sensorType].getData(args.successCallback, args.errorCallback);
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

    _sensorListeners[this.sensorType].getData(args.successCallback, args.errorCallback);
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

    _sensorListeners[this.sensorType].getData(args.successCallback, args.errorCallback);
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

    _sensorListeners[this.sensorType].getData(args.successCallback, args.errorCallback);
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

    _sensorListeners[this.sensorType].getData(args.successCallback, args.errorCallback);
};

////HRMRawSensor
var HRMRawSensor = function(data) {
    Sensor.call(this, SensorType.HRM_RAW);
};

HRMRawSensor.prototype = new Sensor();

HRMRawSensor.prototype.constructor = Sensor;

function getHRMRawSensorData() {
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

    utils_.checkPrivilegeAccess(privilege_.HEALTHINFO);

    _sensorListeners[this.sensorType].getData(args.successCallback, args.errorCallback);
};

HRMRawSensor.prototype.getHRMRawSensorData = function() {
  getHRMRawSensorData.apply(this, arguments);
};

//// GravitySensor
var GravitySensor = function(data) {
    Sensor.call(this, SensorType.GRAVITY);
};

GravitySensor.prototype = new Sensor();

GravitySensor.prototype.constructor = Sensor;

GravitySensor.prototype.getGravitySensorData = function() {
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

    _sensorListeners[this.sensorType].getData(args.successCallback, args.errorCallback);
};


//// GyroscopeSensor
var GyroscopeSensor = function(data) {
    Sensor.call(this, SensorType.GYROSCOPE);
};

GyroscopeSensor.prototype = new Sensor();

GyroscopeSensor.prototype.constructor = Sensor;

GyroscopeSensor.prototype.getGyroscopeSensorData = function() {
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

    _sensorListeners[this.sensorType].getData(args.successCallback, args.errorCallback);
};

//// GyroscopeRotationVectorSensor
var GyroscopeRotationVectorSensor = function(data) {
    Sensor.call(this, SensorType.GYROSCOPE);
};

GyroscopeRotationVectorSensor.prototype = new Sensor();

GyroscopeRotationVectorSensor.prototype.constructor = Sensor;

GyroscopeRotationVectorSensor.prototype.getGyroscopeRotationVectorSensorData = function() {
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

    _sensorListeners[this.sensorType].getData(args.successCallback, args.errorCallback);
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

_sensorListeners[SensorType.LIGHT] = new SensorListener(SensorType.LIGHT,
        SensorLightData);

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

_sensorListeners[SensorType.MAGNETIC] = new SensorListener(SensorType.MAGNETIC,
        SensorMagneticData);

//// SensorPressureData
var SensorPressureData = function(data) {
    SensorData.call(this);
    Object.defineProperties(this, {
        pressure : {value: data.pressure, writable: false, enumerable: true}
    });
};

SensorPressureData.prototype = new SensorData();

SensorPressureData.prototype.constructor = SensorData;

_sensorListeners[SensorType.PRESSURE] = new SensorListener(SensorType.PRESSURE,
        SensorPressureData);

//// SensorProximityData
var SensorProximityData = function(data) {
    SensorData.call(this);
    Object.defineProperties(this, {
        proximityState : {value: data.proximityState, writable: false, enumerable: true}
    });
};

SensorProximityData.prototype = new SensorData();

SensorProximityData.prototype.constructor = SensorData;

_sensorListeners[SensorType.PROXIMITY] = new SensorListener(SensorType.PROXIMITY,
        SensorProximityData);

//// SensorUltravioletData
var SensorUltravioletData = function(data) {
    SensorData.call(this);
    Object.defineProperties(this, {
        ultravioletLevel : {value: data.ultravioletLevel, writable: false, enumerable: true}
    });
};


SensorUltravioletData.prototype = new SensorData();

SensorUltravioletData.prototype.constructor = SensorData;

_sensorListeners[SensorType.ULTRAVIOLET] = new SensorListener(SensorType.ULTRAVIOLET,
        SensorUltravioletData);

////SensorHRMRawData
var SensorHRMRawData = function(data) {
    SensorData.call(this);
    Object.defineProperties(this, {
        lightType : {value: data.lightType, writable: false, enumerable: true},
        lightIntensity : {value: data.lightIntensity, writable: false, enumerable: true}
    });
};


SensorHRMRawData.prototype = new SensorData();

SensorHRMRawData.prototype.constructor = SensorData;

_sensorListeners[SensorType.HRM_RAW] = new SensorListener(SensorType.HRM_RAW,
        SensorHRMRawData);

//// SensorGravityData
var SensorGravityData = function(data) {
    SensorData.call(this);
    Object.defineProperties(this, {
        x : {value: data.x, writable: false, enumerable: true},
        y : {value: data.y, writable: false, enumerable: true},
        z : {value: data.z, writable: false, enumerable: true},
    });
};

SensorGravityData.prototype = new SensorData();

SensorGravityData.prototype.constructor = SensorData;

_sensorListeners[SensorType.GRAVITY] = new SensorListener(SensorType.GRAVITY,
        SensorGravityData);



//// SensorGyroscopeData
var SensorGyroscopeData = function(data) {
    SensorData.call(this);
    Object.defineProperties(this, {
        x : {value: data.x, writable: false, enumerable: true},
        y : {value: data.y, writable: false, enumerable: true},
        z : {value: data.z, writable: false, enumerable: true},
    });
};

SensorGyroscopeData.prototype = new SensorData();

SensorGyroscopeData.prototype.constructor = SensorData;

_sensorListeners[SensorType.GYROSCOPE] = new SensorListener(SensorType.GYROSCOPE,
        SensorGyroscopeData);

//// SensorGyroscopeRotationVectorData
var SensorGyroscopeRotationVectorData = function(data) {
    SensorData.call(this);
    Object.defineProperties(this, {
        x : {value: data.x, writable: false, enumerable: true},
        y : {value: data.y, writable: false, enumerable: true},
        z : {value: data.z, writable: false, enumerable: true},
        w : {value: data.w, writable: false, enumerable: true},
    });
};

SensorGyroscopeRotationVectorData.prototype = new SensorData();

SensorGyroscopeRotationVectorData.prototype.constructor = SensorData;

_sensorListeners[SensorType.GYROSCOPE_ROTATION_VECTOR] = new SensorListener(SensorType.GYROSCOPE_ROTATION_VECTOR,
        SensorGyroscopeRotationVectorData);

// Exports
exports = new SensorService();
