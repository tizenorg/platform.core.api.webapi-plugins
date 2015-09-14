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
var privilege_ = xwalk.utils.privilege;
var types_ = validator_.Types;
var T_ = xwalk.utils.type;
var Converter_ = xwalk.utils.converter;
var native_ = new xwalk.utils.NativeManager(extension);

// index of default property for sim-related callbacks
var defaultListenerIndex = 0;

//enumeration SystemInfoPropertyId ////////////////////////////////////////////////////
var SystemInfoPropertyId = {
        BATTERY : 'BATTERY',
        CPU : 'CPU',
        STORAGE : 'STORAGE',
        DISPLAY : 'DISPLAY',
        DEVICE_ORIENTATION : 'DEVICE_ORIENTATION',
        BUILD : 'BUILD',
        LOCALE : 'LOCALE',
        NETWORK : 'NETWORK',
        WIFI_NETWORK : 'WIFI_NETWORK',
        ETHERNET_NETWORK : 'ETHERNET_NETWORK',
        CELLULAR_NETWORK : 'CELLULAR_NETWORK',
        SIM : 'SIM',
        PERIPHERAL : 'PERIPHERAL',
        MEMORY : 'MEMORY',
        CAMERA_FLASH : 'CAMERA_FLASH'
};

//class SystemInfoDeviceCapability ////////////////////////////////////////////////////
function SystemInfoDeviceCapability(data) {
    Object.defineProperties(this, {
        bluetooth : {
            value : data.bluetooth,
            writable : false,
            enumerable : true
        },
        nfc : {
            value : data.nfc,
            writable : false,
            enumerable : true
        },
        nfcReservedPush : {
            value : data.nfcReservedPush,
            writable : false,
            enumerable : true
        },
        multiTouchCount : {
            value : Converter_.toOctet(data.multiTouchCount),
            writable : false,
            enumerable : true
        },
        inputKeyboard : {
            value : data.inputKeyboard,
            writable : false,
            enumerable : true
        },
        inputKeyboardLayout : {
            value : data.inputKeyboardLayout,
            writable : false,
            enumerable : true
        },
        wifi : {
            value : data.wifi,
            writable : false,
            enumerable : true
        },
        wifiDirect : {
            value : data.wifiDirect,
            writable : false,
            enumerable : true
        },
        opengles : {
            value : data.opengles,
            writable : false,
            enumerable : true
        },
        openglestextureFormat : {
            value : data.openglestextureFormat,
            writable : false,
            enumerable : true
        },
        openglesVersion1_1 : {
            value : data.openglesVersion1_1,
            writable : false,
            enumerable : true
        },
        openglesVersion2_0 : {
            value : data.openglesVersion2_0,
            writable : false,
            enumerable : true
        },
        fmRadio : {
            value : data.fmRadio,
            writable : false,
            enumerable : true
        },
        platformVersion : {
            get : function() {
                xwalk.utils.checkPrivilegeAccess(privilege_.SYSTEM);
                return data.platformVersion;
            },
            set : function() {},
            enumerable : true
        },
        webApiVersion : {
            get : function() {
                xwalk.utils.checkPrivilegeAccess(privilege_.SYSTEM);
                return data.webApiVersion;
            },
            set : function() {},
            enumerable : true
        },
        nativeApiVersion : {
            get : function() {
                xwalk.utils.checkPrivilegeAccess(privilege_.SYSTEM);
                return data.nativeApiVersion;
            },
            set : function() {},
            enumerable : true
        },
        platformName : {
            value : data.platformName,
            writable : false,
            enumerable : true
        },
        camera : {
            value : data.camera,
            writable : false,
            enumerable : true
        },
        cameraFront : {
            value : data.cameraFront,
            writable : false,
            enumerable : true
        },
        cameraFrontFlash : {
            value : data.cameraFrontFlash,
            writable : false,
            enumerable : true
        },
        cameraBack : {
            value : data.cameraBack,
            writable : false,
            enumerable : true
        },
        cameraBackFlash : {
            value : data.cameraBackFlash,
            writable : false,
            enumerable : true
        },
        location : {
            value : data.location,
            writable : false,
            enumerable : true
        },
        locationGps : {
            value : data.locationGps,
            writable : false,
            enumerable : true
        },
        locationWps : {
            value : data.locationWps,
            writable : false,
            enumerable : true
        },
        microphone : {
            value : data.microphone,
            writable : false,
            enumerable : true
        },
        usbHost : {
            value : data.usbHost,
            writable : false,
            enumerable : true
        },
        usbAccessory : {
            value : data.usbAccessory,
            writable : false,
            enumerable : true
        },
        screenOutputRca : {
            value : data.screenOutputRca,
            writable : false,
            enumerable : true
        },
        screenOutputHdmi : {
            value : data.screenOutputHdmi,
            writable : false,
            enumerable : true
        },
        platformCoreCpuArch : {
            value : data.platformCoreCpuArch,
            writable : false,
            enumerable : true
        },
        platformCoreFpuArch : {
            value : data.platformCoreFpuArch,
            writable : false,
            enumerable : true
        },
        sipVoip : {
            value : data.sipVoip,
            writable : false,
            enumerable : true
        },
        duid : {
            value : data.duid,
            writable : false,
            enumerable : true
        },
        speechRecognition : {
            value : data.speechRecognition,
            writable : false,
            enumerable : true
        },
        speechSynthesis : {
            value : data.speechSynthesis,
            writable : false,
            enumerable : true
        },
        accelerometer : {
            value : data.accelerometer,
            writable : false,
            enumerable : true
        },
        accelerometerWakeup : {
            value : data.accelerometerWakeup,
            writable : false,
            enumerable : true
        },
        barometer : {
            value : data.barometer,
            writable : false,
            enumerable : true
        },
        barometerWakeup : {
            value : data.barometerWakeup,
            writable : false,
            enumerable : true
        },
        gyroscope : {
            value : data.gyroscope,
            writable : false,
            enumerable : true
        },
        gyroscopeWakeup : {
            value : data.gyroscopeWakeup,
            writable : false,
            enumerable : true
        },
        magnetometer : {
            value : data.magnetometer,
            writable : false,
            enumerable : true
        },
        magnetometerWakeup : {
            value : data.magnetometerWakeup,
            writable : false,
            enumerable : true
        },
        photometer : {
            value : data.photometer,
            writable : false,
            enumerable : true
        },
        photometerWakeup : {
            value : data.photometerWakeup,
            writable : false,
            enumerable : true
        },
        proximity : {
            value : data.proximity,
            writable : false,
            enumerable : true
        },
        proximityWakeup : {
            value : data.proximityWakeup,
            writable : false,
            enumerable : true
        },
        tiltmeter : {
            value : data.tiltmeter,
            writable : false,
            enumerable : true
        },
        tiltmeterWakeup : {
            value : data.tiltmeterWakeup,
            writable : false,
            enumerable : true
        },
        dataEncryption : {
            value : data.dataEncryption,
            writable : false,
            enumerable : true
        },
        graphicsAcceleration : {
            value : data.graphicsAcceleration,
            writable : false,
            enumerable : true
        },
        push : {
            value : data.push,
            writable : false,
            enumerable : true
        },
        telephony : {
            value : data.telephony,
            writable : false,
            enumerable : true
        },
        telephonyMms : {
            value : data.telephonyMms,
            writable : false,
            enumerable : true
        },
        telephonySms : {
            value : data.telephonySms,
            writable : false,
            enumerable : true
        },
        screenSizeNormal : {
            value : data.screenSizeNormal,
            writable : false,
            enumerable : true
        },
        screenSize480_800 : {
            value : data.screenSize480_800,
            writable : false,
            enumerable : true
        },
        screenSize720_1280 : {
            value : data.screenSize720_1280,
            writable : false,
            enumerable : true
        },
        autoRotation : {
            value : data.autoRotation,
            writable : false,
            enumerable : true
        },
        shellAppWidget : {
            value : data.shellAppWidget,
            writable : false,
            enumerable : true
        },
        visionImageRecognition : {
            value : data.visionImageRecognition,
            writable : false,
            enumerable : true
        },
        visionQrcodeGeneration : {
            value : data.visionQrcodeGeneration,
            writable : false,
            enumerable : true
        },
        visionQrcodeRecognition : {
            value : data.visionQrcodeRecognition,
            writable : false,
            enumerable : true
        },
        visionFaceRecognition : {
            value : data.visionFaceRecognition,
            writable : false,
            enumerable : true
        },
        secureElement : {
            value : data.secureElement,
            writable : false,
            enumerable : true
        },
        nativeOspCompatible : {
            value : data.nativeOspCompatible,
            writable : false,
            enumerable : true
        },
        profile : {
            value : data.profile,
            writable : false,
            enumerable : true
        }
    });
}

//class SystemInfoBattery ////////////////////////////////////////////////////
function SystemInfoBattery(data) {
    Object.defineProperties(this, {
        level : {
            value : data.level,
            writable : false,
            enumerable : true
        },
        isCharging : {
            value : data.isCharging,
            writable : false,
            enumerable : true
        }
    });
}

//class SystemInfoCpu ////////////////////////////////////////////////////
function SystemInfoCpu(data) {
    Object.defineProperties(this, {
        load : {
            value: data.load,
            writable: false,
            enumerable: true
        }
    });
}

//class SystemInfoStorageUnit ////////////////////////////////////////////////////
function SystemInfoStorageUnit(data) {
    Object.defineProperties(this, {
        type : {
            value: data.type,
            writable: false,
            enumerable: true
        },
        capacity : {
            value: Converter_.toUnsignedLongLong(data.capacity),
            writable: false,
            enumerable: true
        },
        availableCapacity : {
            value : Converter_.toUnsignedLongLong(data.availableCapacity),
            writable : false,
            enumerable : true
        },
        isRemovable : {
            value : data.isRemovable,
            writable : false,
            enumerable : true
        },
        isRemoveable : {
            value : data.isRemovable,
            writable : false,
            enumerable : true
        }
    });
}

//class SystemInfoStorage ////////////////////////////////////////////////////
function SystemInfoStorage(data) {
    var len = data.storages.length;
    var storageArray = new Array(len);
    for (var i = 0; i < len; ++i) {
        storageArray[i] = new SystemInfoStorageUnit(data.storages[i]);
    }
    Object.defineProperties(this, {
        units : {
            value: storageArray,
            writable: false,
            enumerable: true
        }
    });
}

//class SystemInfoDisplay ////////////////////////////////////////////////////
function SystemInfoDisplay(data) {
    Object.defineProperties(this, {
        resolutionWidth : {
            value: Converter_.toUnsignedLong(data.resolutionWidth),
            writable: false,
            enumerable: true
        },
        resolutionHeight : {
            value: Converter_.toUnsignedLong(data.resolutionHeight),
            writable: false,
            enumerable: true
        },
        dotsPerInchWidth : {
            value: Converter_.toUnsignedLong(data.dotsPerInchWidth),
            writable: false,
            enumerable: true
        },
        dotsPerInchHeight : {
            value: Converter_.toUnsignedLong(data.dotsPerInchHeight),
            writable: false,
            enumerable: true
        },
        physicalWidth : {
            value: Number(data.physicalWidth),
            writable: false,
            enumerable: true
        },
        physicalHeight : {
            value: Number(data.physicalHeight),
            writable: false,
            enumerable: true
        },
        brightness : {
            value: Number(data.brightness),
            writable: false,
            enumerable: true
        }
    });
}

//class SystemInfoDeviceOrientation ////////////////////////////////////////////////////
function SystemInfoDeviceOrientation(data) {
    Object.defineProperties(this, {
        status : {value: data.status, writable: false, enumerable: true},
        isAutoRotation : {value: data.isAutoRotation, writable: false, enumerable: true}
    });
}

//class SystemInfoBuild ////////////////////////////////////////////////////
function SystemInfoBuild(data) {
    Object.defineProperties(this, {
        model : {value: data.model, writable: false, enumerable: true},
        manufacturer : {value: data.manufacturer, writable: false, enumerable: true},
        buildVersion : {value: data.buildVersion, writable: false, enumerable: true}
    });
}

//class SystemInfoLocale ////////////////////////////////////////////////////
function SystemInfoLocale(data) {
    Object.defineProperties(this, {
        language : {value: data.language, writable: false, enumerable: true},
        country : {value: data.country, writable: false, enumerable: true}
    });
}
//class SystemInfoNetwork ////////////////////////////////////////////////////
function SystemInfoNetwork(data) {
    Object.defineProperties(this, {
        networkType : {value: data.networkType, writable: false, enumerable: true}
    });
}

//class SystemInfoWifiNetwork ////////////////////////////////////////////////////
function SystemInfoWifiNetwork(data) {
    Object.defineProperties(this, {
        status : {value: data.status, writable: false, enumerable: true},
        ssid : {value: data.ssid, writable: false, enumerable: true},
        ipAddress : {value: data.ipAddress, writable: false, enumerable: true},
        ipv6Address : {value: data.ipv6Address, writable: false, enumerable: true},
        macAddress : {value: data.macAddress, writable: false, enumerable: true},
        signalStrength : {value: Number(data.signalStrength), writable: false, enumerable: true}
    });
}

//class SystemInfoEthernetNetwork ////////////////////////////////////////////////////
function SystemInfoEthernetNetwork(data) {
    Object.defineProperties(this, {
        cable : {value: data.cable, writable: false, enumerable: true},
        status : {value: data.status, writable: false, enumerable: true},
        ipAddress : {value: data.ipAddress, writable: false, enumerable: true},
        ipv6Address : {value: data.ipv6Address, writable: false, enumerable: true},
        macAddress : {value: data.macAddress, writable: false, enumerable: true},
    });
}

//class SystemInfoCellularNetwork ////////////////////////////////////////////////////
function SystemInfoCellularNetwork(data) {
    Object.defineProperties(this, {
        status : {value: data.status, writable: false, enumerable: true},
        apn : {value: data.apn, writable: false, enumerable: true},
        ipAddress : {value: data.ipAddress, writable: false, enumerable: true},
        ipv6Address : {value: data.ipv6Address, writable: false, enumerable: true},
        mcc : {value: Number(data.mcc), writable: false, enumerable: true},
        mnc : {value: Number(data.mnc), writable: false, enumerable: true},
        cellId : {value: Number(data.cellId), writable: false, enumerable: true},
        lac : {value: Number(data.lac), writable: false, enumerable: true},
        isRoaming : {value: data.isRoaming, writable: false, enumerable: true},
        isFlightMode : {value: data.isFligthMode, writable: false, enumerable: true},
        imei : {
            get: function() {
                xwalk.utils.checkPrivilegeAccess4Ver("2.3.1", privilege_.TELEPHONY, privilege_.SYSTEMMANAGER);
                return data.imei;
            },
            set: function() {},
            enumerable: true
        }
    });
}

//class SystemInfoSIM ////////////////////////////////////////////////////
function SystemInfoSIM(data) {
    Object.defineProperties(this, {
        state : {
            get: function() {
                xwalk.utils.checkPrivilegeAccess(privilege_.SYSTEM);
                return data.state;
            },
            set: function() {},
            enumerable: true
        },
        operatorName : {
            get: function() {
                xwalk.utils.checkPrivilegeAccess(privilege_.SYSTEM);
                return data.operatorName;
            },
            set: function() {},
            enumerable: true
        },
        msisdn : {
            get: function() {
                xwalk.utils.checkPrivilegeAccess4Ver("2.3.1", privilege_.TELEPHONY, privilege_.SYSTEMMANAGER);
                return data.msisdn;
            },
            set: function() {},
            enumerable: true
        },
        iccid : {
            get: function() {
                xwalk.utils.checkPrivilegeAccess(privilege_.SYSTEM);
                return data.iccid;
            },
            set: function() {},
            enumerable: true
        },
        mcc : {
            get: function() {
                xwalk.utils.checkPrivilegeAccess(privilege_.SYSTEM);
                return Number(data.mcc);
            },
            set: function() {},
            enumerable: true
        },
        mnc : {
            get: function() {
                xwalk.utils.checkPrivilegeAccess(privilege_.SYSTEM);
                return Number(data.mnc);
            },
            set: function() {},
            enumerable: true
        },
        msin : {
            get: function() {
                xwalk.utils.checkPrivilegeAccess4Ver("2.3.1", privilege_.TELEPHONY, privilege_.SYSTEMMANAGER);
                return data.msin;
            },
            set: function() {},
            enumerable: true
        },
        spn : {
            get: function() {
                xwalk.utils.checkPrivilegeAccess(privilege_.SYSTEM);
                return data.spn;
            },
            set: function() {},
            enumerable: true
        },
    });
}

//class SystemInfoPeripheral ////////////////////////////////////////////////////
function SystemInfoPeripheral(data) {
    Object.defineProperties(this, {
        isVideoOutputOn : {value: data.isVideoOutputOn, writable: false, enumerable: true}
    });
}

//class SystemInfoMemory ////////////////////////////////////////////////////
function SystemInfoMemory(data) {
    Object.defineProperties(this, {
        status : {value: data.state, writable: false, enumerable: true}
    });
}

function SystemInfoCameraFlash(data) {
  var getBrightness = function() {
      xwalk.utils.checkPrivilegeAccess(privilege_.LED);

      var result = native_.callSync('SystemInfo_getBrightness', {});
      if (native_.isSuccess(result)) {
        return Converter_.toLong(native_.getResultObject(result)) / this.levels;
      }
      return null;
    };

  var getLevels = function() {
      xwalk.utils.checkPrivilegeAccess(privilege_.LED);

      var result = native_.callSync('SystemInfo_getMaxBrightness', {});
      if (native_.isSuccess(result)) {
        return Converter_.toLong(native_.getResultObject(result));
      }
      return null;
    };

  Object.defineProperties(this, {
        brightness : {set : function(){}, get: getBrightness, enumerable: true},
        camera : {value: data.camera, enumerable: true},
        levels : {get: getLevels, enumerable: true},
    });
}

SystemInfoCameraFlash.prototype.setBrightness = function(brightness) {
  xwalk.utils.checkPrivilegeAccess(privilege_.LED);

  var args = validator_.validateArgs(arguments, [
    {name: 'brightness', type: types_.DOUBLE}
  ]);
  if (args.brightness < 0 || args.brightness > 1)
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
                              'Value is not between 0 and 1');
  args.brightness = args.brightness * this.levels;

  var result = native_.callSync('SystemInfo_setBrightness', args);
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  this.brightness = result;
};

//class SystemInfo ////////////////////////////////////////////////////
var SystemInfo = function() {
};

SystemInfo.prototype.getCapabilities = function() {
    var result = native_.callSync('SystemInfo_getCapabilities', {});
    if (native_.isFailure(result)) {
        throw native_.getErrorObject(result);
    }
    var devCap = new SystemInfoDeviceCapability(native_.getResultObject(result));
    return devCap;
};

SystemInfo.prototype.getCapability = function() {
    var args = validator_.validateArgs(arguments, [
             {
                 name : 'key',
                 type : types_.STRING
             }
             ]);

    var result = native_.callSync('SystemInfo_getCapability', {key: args.key});
    if (native_.isFailure(result)) {
        throw native_.getErrorObject(result);
    }
    var res = native_.getResultObject(result);
    if (res.type === 'int') {
        return Number(res.value);
    } else {
        return res.value;
    }
};


var _createProperty = function (property, data) {
    if (_propertyContainer[property]){
        return new _propertyContainer[property].constructor(data);
    } else {
        throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR, 'Property with id: ' + property + ' is not supported.');
    }
};

var _createPropertyArray = function (property, data) {
    var jsonArray = data.array;
    var propertyArray = [];
    if (_propertyContainer[property]){
        var arrayLength = jsonArray.length;
        for (var i = 0; i < arrayLength; i++) {
            propertyArray.push(new _propertyContainer[property].constructor(jsonArray[i]));
        }
    } else {
        throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR, 'Property with id: ' + property + ' is not supported.');
    }
    return propertyArray;
};


var getPropertyFunction = function(cppLabel, objectCreateFunction) {
    return function() {
        if (arguments[0] === "CELLULAR_NETWORK") {
            xwalk.utils.checkPrivilegeAccess4Ver("2.4", privilege_.TELEPHONY);
        }

        var args = validator_.validateArgs(arguments, [
                 {
                     name : 'property',
                     type : types_.ENUM,
                     values : T_.getValues(SystemInfoPropertyId)
                 },
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
        var propObject = _propertyContainer[args.property];
        if (!propObject) {
            throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR, 'Property with id: ' + args.property + ' is not supported.');
        }
        var callback = function(result) {
            if (native_.isFailure(result)) {
                setTimeout(function() {
                    native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
                }, 0);
            } else {
                var resultProp = objectCreateFunction(args.property, native_.getResultObject(result));
                args.successCallback(resultProp);
            }
        };
        native_.call(cppLabel, {property: args.property}, callback);
    };
}

SystemInfo.prototype.getPropertyValue =
    getPropertyFunction('SystemInfo_getPropertyValue', _createProperty);

SystemInfo.prototype.getPropertyValueArray =
    getPropertyFunction('SystemInfo_getPropertyValueArray', _createPropertyArray);


//SystemInfo helpers ///////////////////////////////////////////////////
var _batteryStr = SystemInfoPropertyId.BATTERY;
var _cpuStr = SystemInfoPropertyId.CPU;
var _storageStr = SystemInfoPropertyId.STORAGE;
var _displayStr = SystemInfoPropertyId.DISPLAY;
var _deviceOrientationStr = SystemInfoPropertyId.DEVICE_ORIENTATION;
//var _buildStr = SystemInfoPropertyId.BUILD;
var _localeStr = SystemInfoPropertyId.LOCALE;
var _networkStr = SystemInfoPropertyId.NETWORK;
var _wifiNetworkStr = SystemInfoPropertyId.WIFI_NETWORK;
var _ethernetNetworkStr = SystemInfoPropertyId.ETHERNET_NETWORK;
var _cellularNetworkStr = SystemInfoPropertyId.CELLULAR_NETWORK;
var _simStr = SystemInfoPropertyId.SIM;
var _peripheralStr = SystemInfoPropertyId.PERIPHERAL;
var _memoryStr = SystemInfoPropertyId.MEMORY;
var _cameraFlashStr = SystemInfoPropertyId.CAMERA_FLASH;

var _nextId = 0;


function _systeminfoBatteryListenerCallback(eventObj) {
    var property = _batteryStr;
    var callbacks = _propertyContainer[property].callbacks;

    for (var watchId in callbacks) {
        if (callbacks.hasOwnProperty(watchId)) {
            var listener = callbacks[watchId];
            var executeCall = (T_.isUndefined(listener.lowThreshold) ||
                    (propObj.level <= listener.lowThreshold)) ||
                    (T_.isUndefined(listener.highThreshold) ||
                            (propObj.level >= listener.highThreshold));
            var propObj = !listener.isArrayType ?
                    _createProperty(property, eventObj.result.array[0]) :
                        _createPropertyArray(property, eventObj.result);
            if (executeCall) {
                listener.callback(propObj);
            }
        }
    }
}

function _systeminfoCpuListenerCallback(eventObj) {
    var property = _cpuStr;
    var callbacks = _propertyContainer[property].callbacks;

    for (var watchId in callbacks) {
        if (callbacks.hasOwnProperty(watchId)) {
            var listener = callbacks[watchId];
            var propObj = !listener.isArrayType ?
                    _createProperty(property, eventObj.result.array[0]) :
                        _createPropertyArray(property, eventObj.result);
            var executeCall = (T_.isUndefined(listener.lowThreshold) ||
                    (propObj.load <= listener.lowThreshold)) ||
                    (T_.isUndefined(listener.highThreshold) ||
                            (propObj.load >= listener.highThreshold));
            if (executeCall) {
                listener.callback(propObj);
            }
        }
    }
}

function _systeminfoStorageListenerCallback(eventObj) {
    var property = _storageStr;
    var callbacks = _propertyContainer[property].callbacks;

    for (var watchId in callbacks) {
        if (callbacks.hasOwnProperty(watchId)) {
            var listener = callbacks[watchId];
            var propObj = !listener.isArrayType ?
                    _createProperty(property, eventObj.result.array[0]) :
                        _createPropertyArray(property, eventObj.result);
            callbacks[watchId].callback(propObj);
        }
    }
}

function _systeminfoDisplayListenerCallback(eventObj) {
    var property = _displayStr;
    var callbacks = _propertyContainer[property].callbacks;

    for (var watchId in callbacks) {
        if (callbacks.hasOwnProperty(watchId)) {
            var listener = callbacks[watchId];
            var propObj = !listener.isArrayType ?
                    _createProperty(property, eventObj.result.array[0]) :
                        _createPropertyArray(property, eventObj.result);
            var executeCall = (T_.isUndefined(listener.lowThreshold) ||
                    (propObj.brightness <= listener.lowThreshold)) ||
                    (T_.isUndefined(listener.highThreshold) ||
                            (propObj.brightness >= listener.highThreshold));
            if (executeCall) {
                listener.callback(propObj);
            }
        }
    }
}

function _systeminfoDeviceOrientationListenerCallback(eventObj) {
    var property = _deviceOrientationStr;
    var callbacks = _propertyContainer[property].callbacks;

    for (var watchId in callbacks) {
        if (callbacks.hasOwnProperty(watchId)) {
            var listener = callbacks[watchId];
            var propObj = !listener.isArrayType ?
                    _createProperty(property, eventObj.result.array[0]) :
                        _createPropertyArray(property, eventObj.result);
            callbacks[watchId].callback(propObj);
        }
    }
}

function _systeminfoLocaleListenerCallback(eventObj) {
    var property = _localeStr;
    var callbacks = _propertyContainer[property].callbacks;

    for (var watchId in callbacks) {
        if (callbacks.hasOwnProperty(watchId)) {
            var listener = callbacks[watchId];
            var propObj = !listener.isArrayType ?
                    _createProperty(property, eventObj.result.array[0]) :
                        _createPropertyArray(property, eventObj.result);
            callbacks[watchId].callback(propObj);
        }
    }
}

function _systeminfoNetworkListenerCallback(eventObj) {
    var property = _networkStr;
    var callbacks = _propertyContainer[property].callbacks;

    for (var watchId in callbacks) {
        if (callbacks.hasOwnProperty(watchId)) {
            var listener = callbacks[watchId];
            var propObj = !listener.isArrayType ?
                    _createProperty(property, eventObj.result.array[0]) :
                        _createPropertyArray(property, eventObj.result);
            callbacks[watchId].callback(propObj);
        }
    }
}

function _systeminfoWifiNetworkListenerCallback(eventObj) {
    var property = _wifiNetworkStr;
    var callbacks = _propertyContainer[property].callbacks;

    for (var watchId in callbacks) {
        if (callbacks.hasOwnProperty(watchId)) {
            var listener = callbacks[watchId];
            var propObj = !listener.isArrayType ?
                    _createProperty(property, eventObj.result.array[0]) :
                        _createPropertyArray(property, eventObj.result);
            callbacks[watchId].callback(propObj);
        }
    }
}

function _systeminfoEthernetNetworkListenerCallback(eventObj) {
  var property = _ethernetNetworkStr;
  var callbacks = _propertyContainer[property].callbacks;

  for (var watchId in callbacks) {
      if (callbacks.hasOwnProperty(watchId)) {
          var listener = callbacks[watchId];
          var propObj = !listener.isArrayType ?
                  _createProperty(property, eventObj.result.array[0]) :
                      _createPropertyArray(property, eventObj.result);
          callbacks[watchId].callback(propObj);
      }
  }
}

function _systeminfoCellularNetworkListenerCallback(eventObj) {
    var property = _cellularNetworkStr;
    var callbacks = _propertyContainer[property].callbacks;

    for (var watchId in callbacks) {
        if (callbacks.hasOwnProperty(watchId)) {
            var listener = callbacks[watchId];
            if (!listener.isArrayType && eventObj.changedPropertyIndex != defaultListenerIndex) {
                // if this is not arrayListener, ignore events of non-default SIM
                return;
            }

            var propObj = !listener.isArrayType ?
                    _createProperty(property, eventObj.result.array[0]) :
                        _createPropertyArray(property, eventObj.result);
            callbacks[watchId].callback(propObj);
        }
    }
}

function _systeminfoSimListenerCallback(eventObj) {
    var property = _simStr;
    var callbacks = _propertyContainer[property].callbacks;

    for (var watchId in callbacks) {
        if (callbacks.hasOwnProperty(watchId)) {
            var listener = callbacks[watchId];
            var propObj = !listener.isArrayType ?
                    _createProperty(property, eventObj.result.array[0]) :
                        _createPropertyArray(property, eventObj.result);
            callbacks[watchId].callback(propObj);
        }
    }
}

function _systeminfoPeripheralListenerCallback(eventObj) {
    var property = _peripheralStr;
    var callbacks = _propertyContainer[property].callbacks;

    for (var watchId in callbacks) {
        if (callbacks.hasOwnProperty(watchId)) {
            var listener = callbacks[watchId];
            var propObj = !listener.isArrayType ?
                    _createProperty(property, eventObj.result.array[0]) :
                        _createPropertyArray(property, eventObj.result);
            callbacks[watchId].callback(propObj);
        }
    }
}

function _systeminfoMemoryListenerCallback(eventObj) {
    var property = _memoryStr;
    var callbacks = _propertyContainer[property].callbacks;

    for (var watchId in callbacks) {
        if (callbacks.hasOwnProperty(watchId)) {
            var listener = callbacks[watchId];
            var propObj = !listener.isArrayType ?
                    _createProperty(property, eventObj.result.array[0]) :
                        _createPropertyArray(property, eventObj.result);
            callbacks[watchId].callback(propObj);
        }
    }
}

function  _systeminfoCameraFlashListenerCallback(eventObj) {
    var property = _cameraFlashStr;
    var callbacks = _propertyContainer[property].callbacks;

    for (var watchId in callbacks) {
        if (callbacks.hasOwnProperty(watchId)) {
            var listener = callbacks[watchId];
            var propObj = !listener.isArrayType ?
                    _createProperty(property, eventObj.result.array[0]) :
                        _createPropertyArray(property, eventObj.result);
            callbacks[watchId].callback(propObj);
        }
    }
}

var _propertyContainer = {
        'BATTERY' : {
            callbacks : {},
            constructor : SystemInfoBattery,
            broadcastFunction : _systeminfoBatteryListenerCallback,
            signalLabel : 'SystemInfoBatteryChangeBroadcast'
        },
        'CPU' : {
            callbacks : {},
            constructor : SystemInfoCpu,
            broadcastFunction : _systeminfoCpuListenerCallback,
            signalLabel : 'SystemInfoCpuChangeBroadcast'
        },
        'STORAGE' : {
            callbacks : {},
            constructor : SystemInfoStorage,
            broadcastFunction : _systeminfoStorageListenerCallback,
            signalLabel : 'SystemInfoStorageChangeBroadcast'
        },
        'DISPLAY' : {
            callbacks : {},
            constructor : SystemInfoDisplay,
            broadcastFunction : _systeminfoDisplayListenerCallback,
            signalLabel : 'SystemInfoDisplayChangeBroadcast'
        },
        'DEVICE_ORIENTATION' : {
            callbacks : {},
            constructor : SystemInfoDeviceOrientation,
            broadcastFunction : _systeminfoDeviceOrientationListenerCallback,
            signalLabel : 'SystemInfoDeviceOrientationChangeBroadcast'
        },
        'BUILD' : {
            callbacks : {}, //adding callbacks for build is not possible
            constructor : SystemInfoBuild,
            broadcastFunction : function(){},
            signalLabel : ''
        },
        'LOCALE' : {
            callbacks : {},
            constructor : SystemInfoLocale,
            broadcastFunction : _systeminfoLocaleListenerCallback,
            signalLabel : 'SystemInfoLocaleChangeBroadcast'
        },
        'NETWORK' : {
            callbacks : {},
            constructor : SystemInfoNetwork,
            broadcastFunction : _systeminfoNetworkListenerCallback,
            signalLabel : 'SystemInfoNetworkChangeBroadcast'
        },
        'WIFI_NETWORK' : {
            callbacks : {},
            constructor : SystemInfoWifiNetwork,
            broadcastFunction : _systeminfoWifiNetworkListenerCallback,
            signalLabel : 'SystemInfoWifiNetworkChangeBroadcast'
        },
        'ETHERNET_NETWORK' : {
            callbacks : {},
            constructor : SystemInfoEthernetNetwork,
            broadcastFunction : _systeminfoEthernetNetworkListenerCallback,
            signalLabel : 'SystemInfoEthernetNetworkChangeBroadcast'
        },
        'CELLULAR_NETWORK' : {
            callbacks : {},
            constructor : SystemInfoCellularNetwork,
            broadcastFunction : _systeminfoCellularNetworkListenerCallback,
            signalLabel : 'SystemInfoCellularNetworkChangeBroadcast'
        },
        'SIM' : {
            callbacks : {},
            constructor : SystemInfoSIM,
            broadcastFunction : _systeminfoSimListenerCallback,
            signalLabel : 'SystemInfoSimChangeBroadcast'
        },
        'PERIPHERAL' : {
            callbacks : {},
            constructor : SystemInfoPeripheral,
            broadcastFunction : _systeminfoPeripheralListenerCallback,
            signalLabel : 'SystemInfoPeripheralChangeBroadcast'
        },
        'MEMORY' : {
            callbacks : {},
            constructor : SystemInfoMemory,
            broadcastFunction : _systeminfoMemoryListenerCallback,
            signalLabel : 'SystemInfoMemoryChangeBroadcast'
        },
        'CAMERA_FLASH' : {
            callbacks : {},
            constructor : SystemInfoCameraFlash,
            broadcastFunction : _systeminfoCameraFlashListenerCallback,
            signalLabel : 'SystemInfoCameraFlashChangeBroadcast'
    }
};

/// It common function to be called when listener would be triggered

var _listenerFunction = function(msg) {
    var propertyId = msg.propertyId;
    if (propertyId) {
        _propertyContainer[propertyId].broadcastFunction(msg);
    } else {
        console.log("No propertyId provided - ignoring");
    }
}
native_.addListener("SysteminfoCommonListenerLabel", _listenerFunction);

var _registerListener = function (property, listener, errorCallback) {
    var watchId;
    var result={};

    var propObject = _propertyContainer[property];
    if (!propObject) {
        throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR, 'Property with id: ' + property + ' is not supported.');
    }
    var callbackBroadcastFunction = propObject.broadcastFunction;
    var signalLabel = propObject.signalLabel;
    var callbacksMap = propObject.callbacks;

    var fail = false;
    if (T_.isEmptyObject(callbacksMap)) {
        //registration in C++ layer
        result = native_.callSync(
                'SystemInfo_addPropertyValueChangeListener',
                {property: Converter_.toString(property)});
        fail = native_.isFailure(result);
        if (native_.isFailure(result)) {
            setTimeout(function() {
                native_.callIfPossible(errorCallback, native_.getErrorObject(result));
            }, 0);
        }
    }
    if (!fail){
        watchId = ++_nextId;
        callbacksMap[watchId] = listener;
    }

    return Converter_.toUnsignedLong(watchId);
};

var _identifyListener = function (watchId) {
    for (var p in _propertyContainer) {
        if (_propertyContainer[p].callbacks[watchId]) {
            return p;
        }
    }
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR, 'Listener with id: ' + watchId + ' does not exist.');
};

var _unregisterListener = function (watchId, isTimeout) {
    var property = {};
    try {
        property = _identifyListener(watchId);
    } catch (e) {
        // if it is "auto" timeout call it should not throw an exception
        if (isTimeout) {
            console.log('Listener was already deleted');
            return;
        }
        throw e;
    }

    var propObject = _propertyContainer[property];
    var result={};

    var callbackBroadcastFunction = propObject.broadcastFunction;
    var signalLabel = propObject.signalLabel;
    var callbacksMap = propObject.callbacks;

    delete callbacksMap[Number(watchId)];
    if (T_.isEmptyObject(callbacksMap)) {
        //unregistration in C++ layer
        result = native_.callSync(
                'SystemInfo_removePropertyValueChangeListener',
                {property: Converter_.toString(property)});
        if (native_.isFailure(result)) {
            throw native_.getErrorObject(result);
        }
    }
};

var getListenerFunction = function (isArray) {
    return function() {
        if (arguments[0] === "CELLULAR_NETWORK") {
            xwalk.utils.checkPrivilegeAccess4Ver("2.4", privilege_.TELEPHONY);
        }
        var args = validator_.validateArgs(arguments, [
                 {
                     name : 'property',
                     type : types_.ENUM,
                     values : T_.getValues(SystemInfoPropertyId)
                 },
                 {
                     name : 'successCallback',
                     type : types_.FUNCTION
                 },
                 {
                     name : 'options',
                     type : types_.DICTIONARY,
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

        var listener = {
                callback      : args.successCallback,
                isArrayType     : isArray,
                highThreshold : !T_.isNullOrUndefined(args.options) ?
                        args.options.highThreshold : undefined,
                lowThreshold  : !T_.isNullOrUndefined(args.options) ?
                        args.options.lowThreshold : undefined
        };
        var watchId = _registerListener(args.property, listener, args.errorCallback);

        var timeout = !T_.isNullOrUndefined(args.options) ? args.options.timeout : undefined;
        if (!T_.isUndefined(timeout) ){
            setTimeout(function(){_unregisterListener(watchId, true);}, timeout);
        }

        return watchId;
    };
};

SystemInfo.prototype.addPropertyValueChangeListener = getListenerFunction(false);

SystemInfo.prototype.addPropertyValueArrayChangeListener = getListenerFunction(true);

SystemInfo.prototype.removePropertyValueChangeListener = function() {
    var args = validator_.validateArgs(arguments, [
             {
                 name : 'watchId',
                 type : types_.UNSIGNED_LONG
             }
             ]);

    _unregisterListener(args.watchId, false);
};

SystemInfo.prototype.getTotalMemory = function() {
    var result = native_.callSync('SystemInfo_getTotalMemory', {});
    if (native_.isFailure(result)) {
        throw native_.getErrorObject(result);
    }
    return native_.getResultObject(result).totalMemory;
};

SystemInfo.prototype.getAvailableMemory = function() {
    var result = native_.callSync('SystemInfo_getAvailableMemory', {});
    if (native_.isFailure(result)) {
        throw native_.getErrorObject(result);
    }
    return native_.getResultObject(result).availableMemory;
};

SystemInfo.prototype.getCount = function() {
    var args = validator_.validateArgs(arguments, [
             {
                 name : 'property',
                 type : types_.ENUM,
                 values : T_.getValues(SystemInfoPropertyId)
             }
             ]);

    var result = native_.callSync('SystemInfo_getCount', {property: args.property});
    if (native_.isFailure(result)) {
        throw native_.getErrorObject(result);
    }
    var res = native_.getResultObject(result);
    return Number(res.count);
};

//Exports
exports = new SystemInfo();
