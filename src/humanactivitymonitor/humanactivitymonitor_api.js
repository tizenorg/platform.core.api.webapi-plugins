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

function convertActivityData(type, data) {
  switch (type) {
    case HumanActivityType.PEDOMETER:
      // TODO(r.galka) Not Supported in current implementation
      return undefined;
    case HumanActivityType.WRIST_UP:
      return null;
    case HumanActivityType.HRM:
      return new HumanActivityHRMData(data);
    case HumanActivityType.GPS:
      var gpsInfo = [];
      for (var i = 0, max = data.length; i < max; i++) {
        gpsInfo.push(new HumanActivityGPSInfo(data[i]));
      }
      return new HumanActivityGPSInfoArray(gpsInfo);
  }
}

function HumanActivityMonitorManager() {
}

HumanActivityMonitorManager.prototype.getHumanActivityData = function(type, successCallback, errorCallback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'type', type: types_.ENUM, values: Object.keys(HumanActivityType)},
    {name: 'successCallback', type: types_.FUNCTION},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  if (args.type === HumanActivityType.WRIST_UP) {
    throw new WebAPIException(WebAPIException.NOT_SUPPORTED_ERR);
  }

  var listenerId ='HumanActivityMonitor_'  + args.type;
  if (!native_.isListenerSet(listenerId)) {
    throw new WebAPIException(WebAPIException.SERVICE_NOT_AVAILABLE_ERR);
  }

  var data = {
    type: args.type
  };

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }

    native_.callIfPossible(args.successCallback,
        convertActivityData(args.type, native_.getResultObject(result)));
  };

  native_.call('HumanActivityMonitorManager_getHumanActivityData', data, callback);
};

HumanActivityMonitorManager.prototype.start = function(type, changedCallback) {
  // TODO(r.galka) check access
  // HRM - http://tizen.org/privilege/healthinfo
  // GPS - http://tizen.org/privilege/location

  var args = validator_.validateArgs(arguments, [
    {name: 'type', type: types_.ENUM, values: Object.keys(HumanActivityType)},
    {name: 'changedCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var listenerId ='HumanActivityMonitor_'  + args.type;

  var data = {
    type: args.type,
    listenerId: listenerId
  };

  if (!native_.isListenerSet(listenerId)) {
    var result = native_.callSync('HumanActivityMonitorManager_start', data);
    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }
  }

  var listener = function(result) {
    native_.callIfPossible(args.changedCallback, convertActivityData(args.type, result));
  };
  native_.addListener(listenerId, listener);
};

HumanActivityMonitorManager.prototype.stop = function(type) {
  var args = validator_.validateArgs(arguments, [
    {name: 'type', type: types_.ENUM, values: Object.keys(HumanActivityType)}
  ]);

  var data = {
    type: args.type
  };

  var listenerId ='HumanActivityMonitor_'  + args.type;

  if (!native_.isListenerSet(listenerId)) {
    return;
  }

  var result = native_.callSync('HumanActivityMonitorManager_stop', data);
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  native_.removeListener(listenerId);
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


function HumanActivityHRMData(data) {
  SetReadOnlyProperty(this, 'heartRate', data.heartRate);
  SetReadOnlyProperty(this, 'rRInterval', data.rRInterval);
}

HumanActivityHRMData.prototype = new HumanActivityData();
HumanActivityHRMData.prototype.constructor = HumanActivityHRMData;


function HumanActivityGPSInfo(data) {
  SetReadOnlyProperty(this, 'latitude', data.latitude);
  SetReadOnlyProperty(this, 'longitude', data.longitude);
  SetReadOnlyProperty(this, 'altitude', data.altitude);
  SetReadOnlyProperty(this, 'speed', data.speed);
  SetReadOnlyProperty(this, 'errorRange', data.errorRange);
  SetReadOnlyProperty(this, 'timestamp', data.timestamp);
}


function HumanActivityGPSInfoArray(data) {
  SetReadOnlyProperty(this, 'gpsInfo', data);
}

HumanActivityGPSInfoArray.prototype = new HumanActivityData();
HumanActivityGPSInfoArray.prototype.constructor = HumanActivityGPSInfoArray;


exports = new HumanActivityMonitorManager();
