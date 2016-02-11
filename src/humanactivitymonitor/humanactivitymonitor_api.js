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

var ActivityRecognitionType = {
  STATIONARY: 'STATIONARY',
  WALKING: 'WALKING',
  RUNNING: 'RUNNING',
  IN_VEHICLE: 'IN_VEHICLE'
};

var ActivityAccuracy = {
  LOW: 'LOW',
  MEDIUM: 'MEDIUM',
  HIGH: 'HIGH'
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

function ActivityRecognitionListenerManager() {
  this.listeners = {};
  this.nextId = 1;
  this.nativeSet = false;
  this.native = native_;
  this.listenerName = 'ActivityRecognitionListener';
};

ActivityRecognitionListenerManager.prototype.onListener = function(data) {
  var watchId = data.watchId;

  if (this.listeners[watchId]) {
    if (native_.isFailure(data)) {
      native_.callIfPossible(this.listeners[watchId].errorCallback, native_.getErrorObject(data));
      return;
    }

    native_.callIfPossible(
        this.listeners[watchId].listener,
        new HumanActivityRecognitionData(native_.getResultObject(data)));
  }
};

ActivityRecognitionListenerManager.prototype.addListener = function(watchId, listener, errorCallback) {
  this.listeners[watchId] = {
    listener: listener,
    errorCallback: errorCallback
  };

  if (!this.nativeSet) {
    this.native.addListener(this.listenerName, this.onListener.bind(this));
    this.nativeSet = true;
  }
};

ActivityRecognitionListenerManager.prototype.removeListener = function(watchId) {
  if (this.listeners[watchId] === null || this.listeners[watchId] === undefined) {
    throw new WebAPIException(0, 'Listener id not found.', 'InvalidValuesError');
  }

  if (this.listeners.hasOwnProperty(watchId)) {
    delete this.listeners[watchId];
    if (type_.isEmptyObject(this.listeners)) {
      this.native.removeListener(this.listenerName);
      this.nativeSet = false;
    }
  }
};

var activityRecognitionListener = new ActivityRecognitionListenerManager();

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

  var listenerId = 'HumanActivityMonitor_'  + args.type;
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

  var result = native_.call('HumanActivityMonitorManager_getHumanActivityData', data, callback);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

function startListener(listenerId, listener, method, data) {
  if (!native_.isListenerSet(listenerId)) {
    var result = native_.callSync(method, data);
    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }
  }

  // always set the listener, if it's another call to startListener() overwrite the old one
  native_.addListener(listenerId, listener);
}

function checkPrivilegesForMethod(method, type) {
  utils_.checkPrivilegeAccess(utils_.privilege.HEALTHINFO);
  if ('HumanActivityMonitorManager_stop' === method && 'GPS' === type) {
    utils_.checkPrivilegeAccess(utils_.privilege.LOCATION);
  }
}

function stopListener(listenerId, method, data) {
  if (!native_.isListenerSet(listenerId)) {
    checkPrivilegesForMethod(method, data.type);
    return;
  }

  var result = native_.callSync(method, data);
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  native_.removeListener(listenerId);
}

HumanActivityMonitorManager.prototype.start = function(type, changedCallback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'type', type: types_.ENUM, values: Object.keys(HumanActivityType)},
    {name: 'changedCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name : 'option', type : types_.DICTIONARY, optional : true, nullable : true}
  ]);

  var listenerId = 'HumanActivityMonitor_'  + args.type;
  var callbackInterval = null, sampleInterval = null;

  switch (args.type) {
  case HumanActivityType.GPS:
    callbackInterval = !type_.isNullOrUndefined(args.option) ?
        args.option.callbackInterval : 120000;
    sampleInterval = !type_.isNullOrUndefined(args.option) ?
        args.option.sampleInterval : 1000;
    if (callbackInterval < 120000 || callbackInterval > 600000) {
      throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
                                'callbackInterval is out of range');
    }
    if (sampleInterval < 1000 || sampleInterval > 120000) {
      throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
                                'sampleInterval is out of range');
    }
    break;
  case HumanActivityType.HRM:
    callbackInterval = !type_.isNullOrUndefined(args.option) ?
        args.option.callbackInterval : 100;
    if (callbackInterval < 10 || callbackInterval > 1000) {
      throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
                                'callbackInterval is out of range');
    }
    break;
  }

  console.log("callbackInterval = " + callbackInterval + ", sampleInterval = " + sampleInterval);
  startListener(listenerId,
                function(result) {
                  native_.callIfPossible(args.changedCallback, convertActivityData(args.type, result));
                },
                'HumanActivityMonitorManager_start',
                { type: args.type,
                  listenerId: listenerId,
                  callbackInterval: callbackInterval,
                  sampleInterval: sampleInterval
                }
               );
};

HumanActivityMonitorManager.prototype.stop = function(type) {
  var args = validator_.validateArgs(arguments, [
    {name: 'type', type: types_.ENUM, values: Object.keys(HumanActivityType)}
  ]);

  stopListener('HumanActivityMonitor_'  + args.type,
               'HumanActivityMonitorManager_stop',
               { type: args.type });
};

var accumulativePedometerListenerId = 'HumanActivityMonitor_AccumulativePedometerListener';

HumanActivityMonitorManager.prototype.setAccumulativePedometerListener = function(changeCallback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'changeCallback', type: types_.FUNCTION}
  ]);

  var listenerId = accumulativePedometerListenerId;

  startListener(listenerId,
                function(result) {
                  args.changeCallback(convertActivityData(HumanActivityType.PEDOMETER, result));
                },
                'HumanActivityMonitorManager_setAccumulativePedometerListener',
                { listenerId: listenerId });
};

HumanActivityMonitorManager.prototype.unsetAccumulativePedometerListener = function() {
  stopListener(accumulativePedometerListenerId,
               'HumanActivityMonitorManager_unsetAccumulativePedometerListener',
               {});
};


HumanActivityMonitorManager.prototype.addActivityRecognitionListener = function() {
  var args = validator_.validateArgs(arguments, [
    {name: 'type', type: types_.ENUM, values: Object.keys(ActivityRecognitionType)},
    {name: 'listener', type: types_.FUNCTION},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);


  var result = native_.call(
                  'HumanActivityMonitorManager_addActivityRecognitionListener',
                  { type: args.type,
                    listenerId: activityRecognitionListener.listenerName });
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  var watchId = result.watchId;
  activityRecognitionListener.addListener(watchId, args.listener, args.errorCallback);

  return watchId;
};

HumanActivityMonitorManager.prototype.removeActivityRecognitionListener = function() {
  var args = validator_.validateArgs(arguments, [
    {name: 'watchId', type: types_.ENUM},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var result = native_.call(
                  'HumanActivityMonitorManager_removeActivityRecognitionListener',
                  { watchId: args.watchId });
  if (native_.isFailure(result)) {
    setTimeout(function () { native_.callIfPossible(args.errorCallback, native_.getErrorObject(result)); }, 0);
    return;
  }
  activityRecognitionListener.removeListener(watchId);
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

function HumanActivityRecognitionData(data) {
  SetReadOnlyProperty(this, 'type', data.type);
  SetReadOnlyProperty(this, 'timestamp', data.timestamp);
  SetReadOnlyProperty(this, 'accuracy', data.accuracy);
}

HumanActivityRecognitionData.prototype = new HumanActivityData();
HumanActivityRecognitionData.prototype.constructor = HumanActivityRecognitionData;

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
