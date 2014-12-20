/* global xwalk, extension, tizen */

// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
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
            value : 'TEST',
            writable : false,
            enumerable : true
        }
    });

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

    var ret = native_.call('FMRadio_SeekUp', {}, function(result) {
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
            throw new tizen.WebAPIException(0,
                    tizen.WebAPIException.INVALID_VALUES_ERR,
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

    var ret = native_.call('FMRadio_SeekDown', {}, function(result) {
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

};

FMRadioManager.prototype.stop = function() {
    var ret = native_.callSync('FMRadio_Stop');
};

FMRadioManager.prototype.scanStop = function() {

};

function FMRadioInterruptManager() {

    this.oninterrupted;
    this.oninterruptfinished;
};

FMRadioInterruptManager.prototype.FMRadioInterruptedCBSwitch = function(args) {
    if (args.action == 'oninterrupted') {
        if (this.oninterrupted) {
            this.oninterrupted();
        }
    } else {

        if (this.oninterruptfinished) {
            this.oninterruptfinished();
        }
    }
};

FMRadioInterruptManager.prototype.FMRadioInterruptedSet = function(oi, oif) {
    this.oninterrupted = oi;
    this.oninterruptfinished = oif;
    native_.addListener('FMRadio_Interrupted', this.FMRadioInterruptedCBSwitch
            .bind(this));
};

var intmgr = new FMRadioInterruptManager();

FMRadioManager.prototype.setFMRadioInterruptedListener = function() {

    var args = validator_.validateArgs(arguments, [ {
        name : 'eventCallback',
        type : types_.LISTENER,
        values : [ 'oninterrupted', 'oninterruptfinished' ]
    } ]);
    intmgr.FMRadioInterruptedSet(args.eventCallback.oninterrupted,
            args.eventCallback.oninterruptfinished)

    var ret = native_.callSync('FMRadio_SetFMRadioInterruptedListener');

};

FMRadioManager.prototype.unsetFMRadioInterruptedListener = function() {

    // TODO intmgr unset
    var ret = native_.callSync('FMRadio_UnsetFMRadioInterruptedListener');
};

FMRadioManager.prototype.setAntennaChangeListener = function() {
    var args = validator_.validateArgs(arguments, [ {
        name : 'eventCallback',
        type : types_.LISTENER,
        values : [ 'onchange' ]
    } ]);

    // TODO listener manager
    var ret = native_.callSync('FMRadio_SetAntennaChangeListener');

};

FMRadioManager.prototype.unsetAntennaChangeListener = function() {

    // TODO listener manager
    var ret = native_.callSync('FMRadio_UnsetAntennaChangeListener');

};

exports = new FMRadioManager();
