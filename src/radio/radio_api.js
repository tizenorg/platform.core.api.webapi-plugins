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
var types_ = validator_.Types;
var native_ = new xwalk.utils.NativeManager(extension);

var _listeners_FMRadioInterrupted;
var _listeners_AntennaChange;

var RadioState = {
    PLAYING : 'PLAYING',
    SCANNING : 'SCANNING',
    READY : 'READY'
};

function FMRadioInterruptCallbackManager() {

    this.oninterrupted;
    this.oninterruptfinished;
};

FMRadioInterruptCallbackManager.prototype.FMRadioInterruptedCBSwitch = function(args) {
    if (args.action == 'oninterrupted')
        this.oninterrupted(args.reason);
    else
        this.oninterruptfinished();
};

FMRadioInterruptCallbackManager.prototype.FMRadioInterruptedSet = function(oi, oif) {
    this.oninterrupted = oi;
    this.oninterruptfinished = oif;
    native_.addListener('FMRadio_Interrupted', this.FMRadioInterruptedCBSwitch
            .bind(this));
};

FMRadioInterruptCallbackManager.prototype.FMRadioInterruptedUnset = function() {
    native_.removeListener('FMRadio_Interrupted');
};

function FMRadioScanCallbackManager() {

    this.radioScanCallback;

};

FMRadioScanCallbackManager.prototype.FMRadioScanCBSwitch = function(args) {
    this.radioScanCallback(args.frequency);
};

FMRadioScanCallbackManager.prototype.FMRadioScanSet = function(cb) {
    this.radioScanCallback = cb;
    native_.addListener('FMRadio_Onfrequencyfound',
            this.FMRadioScanCBSwitch.bind(this));
};

FMRadioScanCallbackManager.prototype.FMRadioScanUnset = function() {
    native_.removeListener('FMRadio_Onfrequencyfound');
};

function FMRadioAntennaChangeCallbackManager() {

    this.onchanged;

};

FMRadioAntennaChangeCallbackManager.prototype.FMRadioAntennaCBSwitch = function(args) {
    this.onchanged(args.connected);
};

FMRadioAntennaChangeCallbackManager.prototype.FMRadioAntennaChangeSet = function(cb) {
    this.onchanged = cb;
    native_.addListener('FMRadio_Antenna', this.FMRadioAntennaCBSwitch
            .bind(this));
};

FMRadioAntennaChangeCallbackManager.prototype.FMRadioAntennaUnset = function() {
    native_.removeListener('FMRadio_Antenna');
};

var antennaCBmanager = new FMRadioAntennaChangeCallbackManager();
var interruptedCBmanager = new FMRadioInterruptCallbackManager();
var scanCBmanager = new FMRadioScanCallbackManager();

function FMRadioManager() {
    Object.defineProperties(this, {
        'frequency' : {
            enumerable : true,
            get : frequencyGetter,
            set : function() {
            }
        },
        'frequencyUpperBound' : {
            enumerable : true,
            get : frequencyUpperGetter,
            set : function() {
            }
        },
        'frequencyLowerBound' : {
            enumerable : true,
            get : frequencyLowerGetter,
            set : function() {
            }
        },
        'signalStrength' : {
            enumerable : true,
            get : signalStrengthGetter,
            set : function() {
            }
        },
        'state' : {
            enumerable : true,
            get : radioStateGetter,
            set : function() {
            }
        },
        'isAntennaConnected' : {
            enumerable : true,
            get : isAntennaConnectedGetter,
            set : function() {
            }
        },
        'mute' : {
            enumerable : true,
            get : muteGetter,
            set : muteSetter
        }
    });

    function muteGetter() {
        var ret = native_.callSync('FMRadio_MuteGetter');
        return native_.getResultObject(ret);
    }

    function muteSetter() {

        var args = validator_.validateArgs(arguments, [ {
            name : 'mute',
            type : types_.BOOLEAN
        } ]);
        native_.callSync('FMRadio_MuteSetter', args);
    }

    function radioStateGetter() {
        var ret = native_.callSync('FMRadio_RadioStateGetter');

        return native_.getResultObject(ret);
    }

    function isAntennaConnectedGetter() {
        var ret = native_.callSync('FMRadio_IsAntennaConnectedGetter');

        return native_.getResultObject(ret);
    }

    function signalStrengthGetter() {
        var ret = native_.callSync('FMRadio_SignalStrengthGetter');

        return native_.getResultObject(ret);
    }

    function frequencyGetter() {

        var ret = native_.callSync('FMRadio_FrequencyGetter');

        return native_.getResultObject(ret);
    }

    function frequencyUpperGetter() {
        return 108;
    }

    function frequencyLowerGetter() {
        return 87.5;
    }
}

FMRadioManager.prototype.seekUp = function() {
    var args = validator_.validateArgs(arguments, [ {
        name : 'successCallback',
        type : types_.FUNCTION,
        optional : true,
        nullable : true
    }, {
        name : 'errorCallback',
        type : types_.FUNCTION,
        optional : true,
        nullable : true
    } ]);

    native_.call('FMRadio_SeekUp', {}, function(result) {
        if (native_.isFailure(result)) {
            if (args.errorCallback)
                args.errorCallback(native_.getErrorObject(result));
        } else {
            if (args.successCallback)
                args.successCallback();
        }
    });

};

FMRadioManager.prototype.start = function() {
    var args = validator_.validateArgs(arguments, [ {
        name : 'frequency',
        type : types_.DOUBLE,
        optional : true,
        nullable : true
    } ]);

    if (args.frequency) {
        if (args.frequency < this.frequencyLowerBound
                || args.frequency > this.frequencyUpperBound)
            throw new WebAPIException(
                    WebAPIException.INVALID_VALUES_ERR,
                    'Frequency out of bounds');
    }
    var result = native_.callSync('FMRadio_Start', {
        'frequency' : args.frequency ? args.frequency
                : this.frequencyLowerBound
    });
    if (native_.isFailure(result)) {
        throw native_.getErrorObject(result);
    }
};

FMRadioManager.prototype.seekDown = function() {
    var args = validator_.validateArgs(arguments, [ {
        name : 'successCallback',
        type : types_.FUNCTION,
        optional : true,
        nullable : true
    }, {
        name : 'errorCallback',
        type : types_.FUNCTION,
        optional : true,
        nullable : true
    } ]);

    native_.call('FMRadio_SeekDown', {}, function(result) {
        if (native_.isFailure(result)) {
            if (args.errorCallback)
                args.errorCallback(native_.getErrorObject(result));
        } else {
            if (args.successCallback)
                args.successCallback();
        }
    });
};

FMRadioManager.prototype.scanStart = function() {
    var args = validator_.validateArgs(arguments, [ {
        name : 'radioScanCallback',
        type : types_.LISTENER,
        values : [ 'onfrequencyfound', 'onfinished' ]
    }, {
        name : 'errorCallback',
        type : types_.FUNCTION,
        optional : true,
        nullable : true
    } ]);

    scanCBmanager.FMRadioScanSet(args.radioScanCallback.onfrequencyfound);

    native_.call('FMRadio_ScanStart', {}, function(result) {
        if (native_.isFailure(result)) {
            if (args.errorCallback)
                args.errorCallback(native_.getErrorObject(result));

        } else {
            scanCBmanager.FMRadioScanUnset();
            args.radioScanCallback.onfinished(result.frequencies);
        }
    });
};

FMRadioManager.prototype.stop = function() {
    var ret = native_.callSync('FMRadio_Stop');
    if (native_.isFailure(ret)) {
        throw native_.getErrorObject(ret);
    }
};

FMRadioManager.prototype.scanStop = function() {
    var args = validator_.validateArgs(arguments, [ {
        name : 'successCallback',
        type : types_.FUNCTION,
        optional : true,
        nullable : true
    }, {
        name : 'errorCallback',
        type : types_.FUNCTION,
        optional : true,
        nullable : true
    } ]);

    native_.call('FMRadio_ScanStop', {}, function(result) {
        if (native_.isFailure(result)) {
            if (args.errorCallback)
                args.errorCallback(native_.getErrorObject(result));
        } else {

            args.successCallback();

        }
    });
};

FMRadioManager.prototype.setFMRadioInterruptedListener = function() {

    var args = validator_.validateArgs(arguments, [ {
        name : 'interruptCallback',
        type : types_.LISTENER,
        values : [ 'oninterrupted', 'oninterruptfinished' ]
    } ]);
    interruptedCBmanager.FMRadioInterruptedSet(args.interruptCallback.oninterrupted,
            args.interruptCallback.oninterruptfinished);

    var ret = native_.callSync('FMRadio_SetFMRadioInterruptedListener');
    if (native_.isFailure(ret)) {
        throw native_.getErrorObject(ret);
    }
};

FMRadioManager.prototype.unsetFMRadioInterruptedListener = function() {

    interruptedCBmanager.FMRadioInterruptedUnset();
    var ret = native_.callSync('FMRadio_UnsetFMRadioInterruptedListener');
    if (native_.isFailure(ret)) {
        throw native_.getErrorObject(ret);
    }
};

FMRadioManager.prototype.setAntennaChangeListener = function() {
    var args = validator_.validateArgs(arguments, [ {
        name : 'changeCallback',
        type : types_.FUNCTION
    } ]);

    antennaCBmanager.FMRadioAntennaChangeSet(args.changeCallback);
    var ret = native_.callSync('FMRadio_SetAntennaChangeListener');
    if (native_.isFailure(ret)) {
        throw native_.getErrorObject(ret);
    }
};

FMRadioManager.prototype.unsetAntennaChangeListener = function() {

    antennaCBmanager.FMRadioAntennaUnset();
    var ret = native_.callSync('FMRadio_UnsetAntennaChangeListener');
    if (native_.isFailure(ret)) {
        throw native_.getErrorObject(ret);
    }
};

exports = new FMRadioManager();
