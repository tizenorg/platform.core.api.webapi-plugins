/* global tizen, xwalk, extension */

// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


var utils_ = xwalk.utils;
var type_ = utils_.type;
var converter_ = utils_.converter;
var validator_ = utils_.validator;
var types_ = validator_.Types;
var native_ = new xwalk.utils.NativeManager(extension);

var callbackId = 0;
var callbacks = {};

function nextCallbackId() {
  return callbackId++;
}

function SetReadOnlyProperty(obj, n, v) {
  Object.defineProperty(obj, n, {value: v, writable: false});
}

var HumanActivityType = {
  PEDOMETER: 'PEDOMETER',
  WRIST_UP: 'WRIST_UP',
  HRM: 'HRM',
  GPS: 'GPS'
};
var PedometerStepStatus = {
  NOT_MOVING: 'NOT_MOVING',
  WALKING: 'WALKING',
  RUNNING: 'RUNNING'
};


function HumanActivityMonitorManager() {
}

HumanActivityMonitorManager.prototype.getHumanActivityData = function(type, successCallback, errorCallback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'type', type: types_.ENUM, values: ['PEDOMETER', 'WRIST_UP', 'HRM', 'GPS']},
    {name: 'successCallback', type: types_.FUNCTION},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var data = {
    type: args.type
  };

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.successCallback);
  };

  native_.call('HumanActivityMonitorManager_getHumanActivityData', data, callback);
};

HumanActivityMonitorManager.prototype.start = function(type, changedCallback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'type', type: types_.ENUM, values: Object.keys(HumanActivityType)},
    {name: 'changedCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var data = {
    type: args.type
  };

  var callback = function(result) {
    native_.callIfPossible(args.changedCallback);
  };

  native_.call('HumanActivityMonitorManager_start', data, callback);
};

HumanActivityMonitorManager.prototype.stop = function(type) {
  var args = validator_.validateArgs(arguments, [
    {name: 'type', type: types_.ENUM, values: ['PEDOMETER', 'WRIST_UP', 'HRM', 'GPS']}
  ]);

  var data = {
    type: args.type
  };

  var result = native_.callSync('HumanActivityMonitorManager_stop', data);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

HumanActivityMonitorManager.prototype.setAccumulativePedometerListener = function(changeCallback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'changeCallback', type: types_.FUNCTION}
  ]);

  var data = {
  };

  var callback = function(result) {
    native_.callIfPossible(args.changeCallback);
  };

  native_.call('HumanActivityMonitorManager_setAccumulativePedometerListener', data, callback);
};

HumanActivityMonitorManager.prototype.unsetAccumulativePedometerListener = function() {

  var data = {
  };

  var result = native_.callSync(
      'HumanActivityMonitorManager_unsetAccumulativePedometerListener', data);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

function StepDifference() {
  SetReadOnlyProperty(this, 'stepCountDifference', null);
  SetReadOnlyProperty(this, 'timestamp', null);
}


function HumanActivityData() {
}


function HumanActivityPedometerData() {
  SetReadOnlyProperty(this, 'stepStatus', null);
  SetReadOnlyProperty(this, 'speed', null);
  SetReadOnlyProperty(this, 'walkingFrequency', null);
  SetReadOnlyProperty(this, 'cumulativeDistance', null);
  SetReadOnlyProperty(this, 'cumulativeCalorie', null);
  SetReadOnlyProperty(this, 'cumulativeTotalStepCount', null);
  SetReadOnlyProperty(this, 'cumulativeWalkStepCount', null);
  SetReadOnlyProperty(this, 'cumulativeRunStepCount', null);
  SetReadOnlyProperty(this, 'stepCountDifferences', null);
}

HumanActivityPedometerData.prototype = new HumanActivityData();
HumanActivityPedometerData.prototype.constructor = HumanActivityPedometerData;


function HumanActivityAccumulativePedometerData() {
  SetReadOnlyProperty(this, 'stepStatus', null);
  SetReadOnlyProperty(this, 'speed', null);
  SetReadOnlyProperty(this, 'walkingFrequency', null);
  SetReadOnlyProperty(this, 'accumulativeDistance', null);
  SetReadOnlyProperty(this, 'accumulativeCalorie', null);
  SetReadOnlyProperty(this, 'accumulativeTotalStepCount', null);
  SetReadOnlyProperty(this, 'accumulativeWalkStepCount', null);
  SetReadOnlyProperty(this, 'accumulativeRunStepCount', null);
  SetReadOnlyProperty(this, 'stepCountDifferences', null);
}

HumanActivityAccumulativePedometerData.prototype = new HumanActivityData();
HumanActivityAccumulativePedometerData.prototype.constructor = HumanActivityAccumulativePedometerData;


function HumanActivityHRMData() {
  SetReadOnlyProperty(this, 'heartRate', null);
  SetReadOnlyProperty(this, 'rRInterval', null);
}

HumanActivityHRMData.prototype = new HumanActivityData();
HumanActivityHRMData.prototype.constructor = HumanActivityHRMData;


function HumanActivityGPSInfo() {
  SetReadOnlyProperty(this, 'latitude', null);
  SetReadOnlyProperty(this, 'longitude', null);
  SetReadOnlyProperty(this, 'altitude', null);
  SetReadOnlyProperty(this, 'speed', null);
  SetReadOnlyProperty(this, 'errorRange', null);
  SetReadOnlyProperty(this, 'timestamp', null);
}


function HumanActivityGPSInfoArray() {
  SetReadOnlyProperty(this, 'gpsInfo', null);
}

HumanActivityGPSInfoArray.prototype = new HumanActivityData();
HumanActivityGPSInfoArray.prototype.constructor = HumanActivityGPSInfoArray;


exports = new HumanActivityMonitorManager();
