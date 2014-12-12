/* global xwalk, extension, tizen */

// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var validator_ = xwalk.utils.validator;
var types_ = validator_.Types;
var native_ = new xwalk.utils.NativeManager(extension);

var RadioState = {
        PLAYING : 'PLAYING',
        SCANNING : 'SCANNING',
        READY : 'READY'
};

function FMRadioManager() {
     Object.defineProperties(this, {
            'frequency': { value: 'TEST', writable: false,enumerable: true },
            'frequencyUpperBound': { value: 'TEST',writable: false,enumerable: true },
            'frequencyLowerBound': { value: 'TEST',writable: false,enumerable: true },
            'signalStrength': {  value: 'TEST',writable: false,enumerable: true },
            'state': { value: 'TEST',writable: false,enumerable: true },
            'isAntennaConnected': {  value: 'TEST',writable: false,enumerable: true },
            'mute': {value: 'TEST', writable: false,enumerable: true }
          });
}

FMRadioManager.prototype.seekUp = function() {
    var ret = native_.call('FMRadio_SeekUp');
    return 'SeekUp';
};

FMRadioManager.prototype.seekDown = function() {
    var ret = native_.call('FMRadio_SeekDown');
    return 'SeekDown'
};

FMRadioManager.prototype.scanStart = function() {
    var ret = native_.call('FMRadio_ScanStart');
    return 'scanStart'
};

FMRadioManager.prototype.scanStop = function() {
    var ret = native_.call('FMRadio_ScanStop');
    return 'scanStop'
};

FMRadioManager.prototype.setFMRadioInterruptedListener = function() {
    var ret = native_.callSync('FMRadio_SetFMRadioInterruptedListener');
    return 'setFMRadioInterruptedListener'
};

FMRadioManager.prototype.unsetFMRadioInterruptedListener = function() {
    var ret = native_.callSync('FMRadio_UnsetFMRadioInterruptedListener');
    return 'unsetFMRadioInterruptedListener'
};

FMRadioManager.prototype.setAntennaChangeListener = function() {
    var ret = native_.callSync('FMRadio_SetAntennaChangeListener');
    return 'setAntennaChangeListener'
};

FMRadioManager.prototype.unsetAntennaChangeListener = function() {
    var ret = native_.callSync('FMRadio_UnsetAntennaChangeListener');
    return 'unsetAntennaChangeListener'
};

exports = new FMRadioManager();

