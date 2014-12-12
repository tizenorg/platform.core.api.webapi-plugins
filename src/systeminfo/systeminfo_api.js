//Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
//Use of this source code is governed by a BSD-style license that can be
//found in the LICENSE file.

var Common = function(){
    function _getException(type, msg) {
        return new tizen.WebAPIException(type, msg || 'Unexpected exception');
    }

    function _getTypeMismatch(msg) {
        return _getException(tizen.WebAPIException.TYPE_MISMATCH_ERR,
                msg || 'Provided arguments are not valid.');
    }

    function _throwTypeMismatch(msg) {
        throw _getTypeMismatch(msg);
    }

    var _type = xwalk.utils.type;
    var _converter = xwalk.utils.converter;
    var _validator = xwalk.utils.validator;
    function Common() {}

    function _prepareRequest(module, method, args) {
        var request = {
                'cmd'   :   method,
                'args'  :   args
        };
        return JSON.stringify(request);
    }

    Common.prototype.getCallSync = function (module) {
        return function _callSync(method, args) {
            return JSON.parse(extension.internal.sendSyncMessage(_prepareRequest(module, method, args)));
        };
    };

    Common.prototype.getCall = function (module) {
        return function _call(method, args, callback) {
            var callbackId = Callbacks.getInstance().add(callback);
            args['callbackId'] = callbackId;
            extension.postMessage(_prepareRequest(module, method, args));
        };
    };

    Common.prototype.isSuccess = function (result) {
        return (result.status !== 'error');
    };

    Common.prototype.isFailure = function (result) {
        return !this.isSuccess(result);
    };

    Common.prototype.getErrorObject = function (result) {
        return new tizen.WebAPIException(result.error.code ? result.error.code : -1,
                result.error.message, result.error.name);
    };

    Common.prototype.getResultObject = function (result) {
        return result.result;
    };

    Common.prototype.callIfPossible = function(callback) {
        if (!_type.isNullOrUndefined(callback)) {
            callback.apply(callback, [].slice.call(arguments, 1));
        }
    };

    Common.prototype.getTypeMismatch = function(msg) {
        _getTypeMismatch(msg);
    };

    Common.prototype.throwTypeMismatch = function(msg) {
        _throwTypeMismatch(msg);
    };

    Common.prototype.getInvalidValues = function(msg) {
        return _getException('InvalidValuesError',
                msg || 'There\'s a problem with input value.');
    };

    Common.prototype.throwInvalidValues = function(msg) {
        throw this.getInvalidValues(msg);
    };

    Common.prototype.getIOError = function (msg) {
        return _getException('IOError', msg || 'Unexpected IO error.');
    };

    Common.prototype.throwIOError = function (msg) {
        throw this.getIOError(msg);
    };

    Common.prototype.getNotSupported = function (msg) {
        return _getException(tizen.WebAPIException.NOT_SUPPORTED_ERR, msg || 'Not supported.');
    };

    Common.prototype.throwNotSupported = function (msg) {
        throw this.getNotSupported(msg);
    };

    Common.prototype.getNotFound = function (msg) {
        return _getException('NotFoundError', msg || 'Not found.');
    };

    Common.prototype.throwNotFound = function (msg) {
        throw this.getNotFound(msg);
    };

    Common.prototype.getUnknownError = function (msg) {
        return _getException('UnknownError', msg || 'Unknown error.');
    };

    Common.prototype.throwUnknownError = function (msg) {
        throw this.getUnknownError(msg);
    };

    Common.prototype.throwTypeMismatch = function(msg) {
        _throwTypeMismatch(msg);
    };

    Common.prototype.sort = function (arr, sortMode) {
        var _getSortProperty = function (obj, props) {
            for (var i = 0; i < props.length; ++i) {
                if (!obj.hasOwnProperty(props[i])) {
                    return null;
                }
                obj = obj[props[i]];
            }
            return obj;
        };

        if (sortMode instanceof tizen.SortMode) {
            var props = sortMode.attributeName.split('.');
            arr.sort(function (a, b) {
                var aValue = _getSortProperty(a, props);
                var bValue = _getSortProperty(b, props);

                if (sortMode.order === 'DESC') {
                    return aValue < bValue;
                }
                return bValue < aValue;
            });
        }
        return arr;
    };

    Common.prototype.filter = function (arr, filter) {
        if (_type.isNullOrUndefined(arr))
            return arr;
        if (filter instanceof tizen.AttributeFilter ||
                filter instanceof tizen.AttributeRangeFilter ||
                filter instanceof tizen.CompositeFilter) {
            arr = arr.filter(function(element) {
                return filter._filter(element);
            });
        }
        return arr;
    };

    Common.prototype.repackFilter = function (filter) {
        if (filter instanceof tizen.AttributeFilter) {
            return {
                filterType: "AttributeFilter",
                attributeName: filter.attributeName,
                matchFlag: filter.matchFlag,
                matchValue: filter.matchValue,
            };
        }
        if (filter instanceof tizen.AttributeRangeFilter) {
            return {
                filterType: "AttributeRangeFilter",
                attributeName: filter.attributeName,
                initialValue: _type.isNullOrUndefined(filter.initialValue) ? null : filter.initialValue,
                        endValue: _type.isNullOrUndefined(filter.endValue) ? null : filter.endValue
            };
        }
        if (filter instanceof tizen.CompositeFilter) {
            var _f = [];
            var filters = filter.filters;

            for (var i = 0; i < filters.length; ++i) {
                _f.push(this.repackFilter(filters[i]));
            }

            return {
                filterType: "CompositeFilter",
                type: filter.type,
                filters: _f
            };
        }

        return null;
    }

    var _common = new Common();

    return {
        Type : _type,
        Converter : _converter,
        ArgumentValidator : _validator,
        Common : _common
    };
};

var _common = Common();
var T = _common.Type;
var Converter = _common.Converter;
var AV = _common.ArgumentValidator;
var C = _common.Common;
var _callSync = C.getCallSync('SystemInfo');
var _call = C.getCall('SystemInfo');

/**
 * It is singleton object to keep and invoke callbacks.
 * 
 */
var Callbacks = (function () {
    var _collection = {};
    var _id = 0;
    var _next = function () {
        return (_id += 1);
    };

    var CallbackManager = function () {};

    CallbackManager.prototype = {
            add: function (fun) {
                var id = _next();
                _collection[id] = fun;
                return id;
            },
            remove: function (id) {
                if (_collection[id]) delete _collection[id];
            },
            call: function (id, result) {
                _collection[id](result);
            }
    };

    return {
        getInstance: function () {
            return this.instance || (this.instance = new CallbackManager);
        }
    };
})();

extension.setMessageListener(function(json) {
    var msg = JSON.parse(json);
    var callbackId = msg.callbackId;
    if (callbackId) {
        Callbacks.getInstance().call(callbackId, msg);
    }
}
);

//implementation from Node.js//////////////////////////////////////////////////////////

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
        CELLULAR_NETWORK : 'CELLULAR_NETWORK',
        SIM : 'SIM',
        PERIPHERAL : 'PERIPHERAL'
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
            value : Converter.toOctet(data.multiTouchCount),
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
            value : data.platformVersion,
            writable : false,
            enumerable : true
        },
        webApiVersion : {
            value : data.webApiVersion,
            writable : false,
            enumerable : true
        },
        nativeApiVersion : {
            value : data.nativeApiVersion,
            writable : false,
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
            value: Converter.toUnsignedLongLong(data.capacity),
            writable: false,
            enumerable: true
        },
        availableCapacity : {
            value : Converter.toUnsignedLongLong(data.availableCapacity),
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
            value: Converter.toUnsignedLong(data.resolutionWidth),
            writable: false,
            enumerable: true
        },
        resolutionHeight : {
            value: Converter.toUnsignedLong(data.resolutionHeight),
            writable: false,
            enumerable: true
        },
        dotsPerInchWidth : {
            value: Converter.toUnsignedLong(data.dotsPerInchWidth),
            writable: false,
            enumerable: true
        },
        dotsPerInchHeight : {
            value: Converter.toUnsignedLong(data.dotsPerInchHeight),
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
        signalStrength : {value: Number(data.signalStrength), writable: false, enumerable: true}
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
        imei : {value: data.imei, writable: false, enumerable: true}
    });
}

//class SystemInfoSIM ////////////////////////////////////////////////////
function SystemInfoSIM(data) {
    Object.defineProperties(this, {
        state : {value: data.state, writable: false, enumerable: true},
        operatorName : {value: data.operatorName, writable: false, enumerable: true},
        msisdn : {value: data.msisdn, writable: false, enumerable: true},
        iccid : {value: data.iccid, writable: false, enumerable: true},
        mcc : {value: Number(data.mcc), writable: false, enumerable: true},
        mnc : {value: Number(data.mnc), writable: false, enumerable: true},
        msin : {value: data.msin, writable: false, enumerable: true},
        spn : {value: data.spn, writable: false, enumerable: true}
    });
}

//class SystemInfoPeripheral ////////////////////////////////////////////////////
function SystemInfoPeripheral(data) {
    Object.defineProperties(this, {
        isVideoOutputOn : {value: data.isVideoOutputOn, writable: false, enumerable: true}
    });
}

//class SystemInfo ////////////////////////////////////////////////////
var SystemInfo = function() {
};

SystemInfo.prototype.getCapabilities = function() {
    console.log('Entered getCapabilities');
    var result = _callSync('SystemInfo_getCapabilities', {});
    if (C.isFailure(result)) {
        throw C.getErrorObject(result);
    }
    var devCap = new SystemInfoDeviceCapability(C.getResultObject(result));
    return devCap;
};

SystemInfo.prototype.getCapability = function() {
    console.log('Entered getCapability');
    var args = AV.validateMethod(arguments, [
                                             {
                                                 name : 'key',
                                                 type : AV.Types.STRING
                                             }
                                             ]);

    var result = _callSync('SystemInfo_getCapability', {key: args.key});
    if (C.isFailure(result)) {
        throw C.getErrorObject(result);
    }
    var res = C.getResultObject(result);
    if (res.type === 'int') {
        return Number(res.value);
    } else {
        return res.value;
    }
};

SystemInfo.prototype.getPropertyValue = function() {
    console.log('Entered getPropertyValue');
    var args = AV.validateMethod(arguments, [
                                             {
                                                 name : 'property',
                                                 type : AV.Types.ENUM,
                                                 values : T.getValues(SystemInfoPropertyId)
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

    var propObject = _propertyContainer[args.property];
    if (!propObject) {
        C.throwTypeMismatch('Property with id: ' + args.property + ' is not supported.');
    }

    var callback = function(result) {
        if (C.isFailure(result)) {
            setTimeout(function() {
                C.callIfPossible(args.errorCallback, C.getErrorObject(result));
            }, 0);
        } else {
            var resultProp = _createProperty(args.property, C.getResultObject(result));
            args.successCallback(resultProp);
        }
    };

    _call('SystemInfo_getPropertyValue', {property: args.property}, callback);
};

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
var _cellularNetworkStr = SystemInfoPropertyId.CELLULAR_NETWORK;
var _simStr = SystemInfoPropertyId.SIM;
var _peripheralStr = SystemInfoPropertyId.PERIPHERAL;

var _nextId = 0;

var _createProperty = function (property, data) {
    if (_propertyContainer[property]){
        return new _propertyContainer[property].constructor(data);
    } else {
        C.throwTypeMismatch('Property with id: ' + property + ' is not supported.');
    }
};

function _systeminfoBatteryListenerCallback(event) {
    console.log('Entered _systeminfoBatteryListenerCallback');
    var property = _batteryStr;
    var eventObj = JSON.parse(event);
    var propObj = _createProperty(property, eventObj.result);
    var callbacks = _propertyContainer[property].callbacks;

    for (var watchId in callbacks) {
        if (callbacks.hasOwnProperty(watchId)) {
            var listener = callbacks[watchId];
            var executeCall = (T.isUndefined(listener.lowThreshold) ||
                    (propObj.level <= listener.lowThreshold)) ||
                    (T.isUndefined(listener.highThreshold) ||
                            (propObj.level >= listener.highThreshold));
            if (executeCall) {
                listener.callback(propObj);
            }
        }
    }
}

function _systeminfoCpuListenerCallback(event) {
    console.log('Entered _systeminfoCpuListenerCallback');
    var property = _cpuStr;
    var eventObj = JSON.parse(event);
    var propObj = _createProperty(property, eventObj.result);
    var callbacks = _propertyContainer[property].callbacks;

    for (var watchId in callbacks) {
        if (callbacks.hasOwnProperty(watchId)) {
            var listener = callbacks[watchId];
            var executeCall = (T.isUndefined(listener.lowThreshold) ||
                    (propObj.load <= listener.lowThreshold)) ||
                    (T.isUndefined(listener.highThreshold) ||
                            (propObj.load >= listener.highThreshold));
            if (executeCall) {
                listener.callback(propObj);
            }
        }
    }
}

function _systeminfoStorageListenerCallback(event) {
    console.log('Entered _systeminfoStorageListenerCallback');
    var property = _storageStr;
    var eventObj = JSON.parse(event);
    var propObj = _createProperty(property, eventObj.result);
    var callbacks = _propertyContainer[property].callbacks;

    for (var watchId in callbacks) {
        if (callbacks.hasOwnProperty(watchId)) {
            callbacks[watchId].callback(propObj);
        }
    }
}

function _systeminfoDisplayListenerCallback(event) {
    console.log('Entered _systeminfoDisplayListenerCallback');
    var property = _displayStr;
    var eventObj = JSON.parse(event);
    var propObj = _createProperty(property, eventObj.result);
    var callbacks = _propertyContainer[property].callbacks;

    for (var watchId in callbacks) {
        if (callbacks.hasOwnProperty(watchId)) {
            var listener = callbacks[watchId];
            var executeCall = (T.isUndefined(listener.lowThreshold) ||
                    (propObj.brightness <= listener.lowThreshold)) ||
                    (T.isUndefined(listener.highThreshold) ||
                            (propObj.brightness >= listener.highThreshold));
            if (executeCall) {
                listener.callback(propObj);
            }
        }
    }
}

function _systeminfoDeviceOrientationListenerCallback(event) {
    console.log('Entered _systeminfoDeviceOrientationListenerCallback');
    var property = _deviceOrientationStr;
    var eventObj = JSON.parse(event);
    var propObj = _createProperty(property, eventObj.result);
    var callbacks = _propertyContainer[property].callbacks;

    for (var watchId in callbacks) {
        if (callbacks.hasOwnProperty(watchId)) {
            callbacks[watchId].callback(propObj);
        }
    }
}

function _systeminfoLocaleListenerCallback(event) {
    console.log('Entered _systeminfoLocaleListenerCallback');
    var property = _localeStr;
    var eventObj = JSON.parse(event);
    var propObj = _createProperty(property, eventObj.result);
    var callbacks = _propertyContainer[property].callbacks;

    for (var watchId in callbacks) {
        if (callbacks.hasOwnProperty(watchId)) {
            callbacks[watchId].callback(propObj);
        }
    }
}

function _systeminfoNetworkListenerCallback(event) {
    console.log('Entered _systeminfoNetworkListenerCallback');
    var property = _networkStr;
    var eventObj = JSON.parse(event);
    var propObj = _createProperty(property, eventObj.result);
    var callbacks = _propertyContainer[property].callbacks;

    for (var watchId in callbacks) {
        if (callbacks.hasOwnProperty(watchId)) {
            callbacks[watchId].callback(propObj);
        }
    }
}

function _systeminfoWifiNetworkListenerCallback(event) {
    console.log('Entered _systeminfoWifiNetworkListenerCallback');
    var property = _wifiNetworkStr;
    var eventObj = JSON.parse(event);
    var propObj = _createProperty(property, eventObj.result);
    var callbacks = _propertyContainer[property].callbacks;

    for (var watchId in callbacks) {
        if (callbacks.hasOwnProperty(watchId)) {
            callbacks[watchId].callback(propObj);
        }
    }
}

function _systeminfoCellularNetworkListenerCallback(event) {
    console.log('Entered _systeminfoCellularNetworkListenerCallback');
    var property = _cellularNetworkStr;
    var eventObj = JSON.parse(event);
    var propObj = _createProperty(property, eventObj.result);
    var callbacks = _propertyContainer[property].callbacks;

    for (var watchId in callbacks) {
        if (callbacks.hasOwnProperty(watchId)) {
            callbacks[watchId].callback(propObj);
        }
    }
}

function _systeminfoSimListenerCallback(event) {
    console.log('Entered _systeminfoSimListenerCallback');
    var property = _simStr;
    var eventObj = JSON.parse(event);
    var propObj = _createProperty(property, eventObj.result);
    var callbacks = _propertyContainer[property].callbacks;

    for (var watchId in callbacks) {
        if (callbacks.hasOwnProperty(watchId)) {
            callbacks[watchId].callback(propObj);
        }
    }
}

function _systeminfoPeripheralListenerCallback(event) {
    console.log('Entered _systeminfoPeripheralListenerCallback');
    var property = _peripheralStr;
    var eventObj = JSON.parse(event);
    var propObj = _createProperty(property, eventObj.result);
    var callbacks = _propertyContainer[property].callbacks;

    for (var watchId in callbacks) {
        if (callbacks.hasOwnProperty(watchId)) {
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
        }
};

var _registerListener = function (property, listener, errorCallback) {
    console.log('Entered registerListener');
    var watchId;
    var result={};

    var propObject = _propertyContainer[property];
    if (!propObject) {
        C.throwTypeMismatch('Property with id: ' + property + ' is not supported.');
    }
    var callbackBroadcastFunction = propObject.broadcastFunction;
    var signalLabel = propObject.signalLabel;
    var callbacksMap = propObject.callbacks;

    var fail = false;
    if (T.isEmptyObject(callbacksMap)) {
        //registration in C++ layer
        result = _callSync(
                'SystemInfo_addPropertyValueChangeListener',
                {property: Converter.toString(property)});
        fail = C.isFailure(result);
        if (!fail) {
            native.addListener(signalLabel, callbackBroadcastFunction);
        } else {
            setTimeout(function() {
                C.callIfPossible(errorCallback, C.getErrorObject(result));
            }, 0);
        }
    }
    if (!fail){
        watchId = ++_nextId;
        callbacksMap[watchId] = listener;
    }

    return Converter.toUnsignedLong(watchId);
};

var _identifyListener = function (watchId) {
    console.log('Entered _identifyListener');

    for (var p in _propertyContainer) {
        if (_propertyContainer[p].callbacks[watchId]) {
            return p;
        }
    }
    C.throwInvalidValues('Listener with id: ' + watchId + ' does not exist.');
};

var _unregisterListener = function (watchId, isTimeout) {
    console.log('Entered _unregisterListener');
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
    if (T.isEmptyObject(callbacksMap)) {
        native.removeListener(signalLabel, callbackBroadcastFunction);
        //unregistration in C++ layer
        result = _callSync(
                'SystemInfo_removePropertyValueChangeListener',
                {property: Converter.toString(property)});
        if (C.isFailure(result)) {
            throw C.getErrorObject(result);
        }
    }
};

SystemInfo.prototype.addPropertyValueChangeListener = function() {
    console.log('Entered addPropertyVaueChangeListener');
    var args = AV.validateMethod(arguments, [
                                             {
                                                 name : 'property',
                                                 type : AV.Types.ENUM,
                                                 values : T.getValues(SystemInfoPropertyId)
                                             },
                                             {
                                                 name : 'successCallback',
                                                 type : AV.Types.FUNCTION
                                             },
                                             {
                                                 name : 'options',
                                                 type : AV.Types.DICTIONARY,
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

    var listener = {
            callback      : args.successCallback,
            highThreshold : !T.isNullOrUndefined(args.options) ?
                    args.options.highThreshold : undefined,
                    lowThreshold  : !T.isNullOrUndefined(args.options) ?
                            args.options.lowThreshold : undefined
    };
    var watchId = _registerListener(args.property, listener, args.errorCallback);

    var timeout = !T.isNullOrUndefined(args.options) ? args.options.timeout : undefined;
    if (!T.isUndefined(timeout) ){
        setTimeout(function(){_unregisterListener(watchId, true);}, timeout);
    }

    return watchId;
};

SystemInfo.prototype.removePropertyValueChangeListener = function() {
    console.log('Entered removePropertyValueChangeListener');
    var args = AV.validateMethod(arguments, [
                                             {
                                                 name : 'watchId',
                                                 type : AV.Types.UNSIGNED_LONG
                                             }
                                             ]);

    _unregisterListener(args.watchId, false);
};

//Exports
var systeminfoObject = new SystemInfo();
//exports.getTotalMemory = systeminfoObject.getTotalMemory;
//exports.getAvailableMemory = systeminfoObject.getAvailableMemory;
exports.getCapabilities = systeminfoObject.getCapabilities;
exports.getCapability = systeminfoObject.getCapability;
//exports.getCount = systeminfoObject.getCount;
exports.getPropertyValue = systeminfoObject.getPropertyValue;
//exports.getPropertyValueArray = systeminfoObject.getPropertyValueArray;
exports.addPropertyValueChangeListener = systeminfoObject.addPropertyValueChangeListener;
//exports.addPropertyValueArrayChangeListener = systeminfoObject.addPropertyValueArrayChangeListener;
exports.removePropertyValueChangeListener = systeminfoObject.removePropertyValueChangeListener;
