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

var ACCUMULATIVE_PEDOMETER_DATA = 'ACCUMULATIVE_PEDOMETER_DATA';
var MIN_OPTION_INTERVAL = 0;
var MIN_OPTION_RETENTION_PERIOD = 1;
var MIN_QUERY_TIME = 0;
var MIN_QUERY_INTERVAL = 0;

var HumanActivityType = {
  PEDOMETER: 'PEDOMETER',
  WRIST_UP: 'WRIST_UP',
  HRM: 'HRM',
  GPS: 'GPS',
  SLEEP_MONITOR: 'SLEEP_MONITOR'
};

var HumanActivityRecorderType = {
  PEDOMETER: 'PEDOMETER',
  HRM: 'HRM',
  SLEEP_MONITOR: 'SLEEP_MONITOR',
  PRESSURE: 'PRESSURE'
};

var PedometerStepStatus = {
  NOT_MOVING: 'NOT_MOVING',
  WALKING: 'WALKING',
  RUNNING: 'RUNNING',
  UNKNOWN: 'UNKNOWN'
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

var SleepStatus = {
  ASLEEP: 'ASLEEP',
  AWAKE: 'AWAKE'
};

function convertActivityData(type, data) {
  switch (type) {
    case HumanActivityType.PEDOMETER:
      return new HumanActivityPedometerData(data);
    case ACCUMULATIVE_PEDOMETER_DATA:
      return new HumanActivityAccumulativePedometerData(data);
    case HumanActivityType.WRIST_UP:
      return null;
    case HumanActivityType.HRM:
      return new HumanActivityHRMData(data);
    case HumanActivityType.GPS:
      var gpsInfo = [];
      for (var i = 0, max = data.gpsInfo.length; i < max; i++) {
        gpsInfo.push(new HumanActivityGPSInfo(data.gpsInfo[i]));
      }
      return new HumanActivityGPSInfoArray(gpsInfo);
    case HumanActivityType.SLEEP_MONITOR:
      return new HumanActivitySleepMonitorData(data);
    default:
      console.error('Uknown human activity type: ' + type);
  }
}

function createRecorderData(func, data) {
  var array = [];

  data.forEach(function (d) {
    array.push(new func(d));
  });

  return array;
}

function convertActivityRecorderData(type, data) {
  var func = undefined;
  switch (type) {
    case HumanActivityRecorderType.PEDOMETER:
      func = HumanActivityRecorderPedometerData;
      break;
    case HumanActivityRecorderType.HRM:
      func = HumanActivityRecorderHRMData;
      break;
    case HumanActivityRecorderType.SLEEP_MONITOR:
      func = HumanActivityRecorderSleepMonitorData;
      break;
    case HumanActivityRecorderType.PRESSURE:
      func = HumanActivityRecorderPressureData;
      break;
    default:
      console.error('Uknown human activity recorder type: ' + type);
      return;
  }

  return createRecorderData(func, data);
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

// Pedometer listener and accumulative pedometer listener are handled by a single
// callback. Native side sends both objects, JS side needs to pass the data to
// appropriate listeners.
var pedometerListener = null;
var accumulativePedometerListener = null;

function pedometerCallback(result) {
  if (pedometerListener) {
    pedometerListener(convertActivityData(HumanActivityType.PEDOMETER, result));
  }

  if (accumulativePedometerListener) {
    accumulativePedometerListener(convertActivityData(ACCUMULATIVE_PEDOMETER_DATA, result));
  }
}

var GPSListener = null;
function GPSCallback(result) {
  if (GPSListener) {
    GPSListener(result);
  }
}

HumanActivityMonitorManager.prototype.start = function(type, changedCallback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'type', type: types_.ENUM, values: Object.keys(HumanActivityType)},
    {name: 'changedCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'options', type : types_.DICTIONARY, optional : true, nullable : true}
  ]);

  var listenerId = 'HumanActivityMonitor_'  + args.type;
  var optionsAttributes = ["callbackInterval", "sampleInterval"], options = args.options || {};

  var callbackInterval = null, sampleInterval = null;

  switch (args.type) {
  case HumanActivityType.GPS:
    callbackInterval = !type_.isNullOrUndefined(options[optionsAttributes[0]]) ?
        options[optionsAttributes[0]] : 150000;
    sampleInterval = !type_.isNullOrUndefined(options[optionsAttributes[1]]) ?
        options[optionsAttributes[1]] : 1000;
    break;
  case HumanActivityType.HRM:
    callbackInterval = !type_.isNullOrUndefined(options[optionsAttributes[0]]) ?
        options[optionsAttributes[0]] : 100;
    if (callbackInterval < 10 || callbackInterval > 1000) {
      throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
                                'callbackInterval is out of range');
    }
    break;
  }

  var listener = null;
  switch (args.type) {
    case HumanActivityType.PEDOMETER:
      listener = pedometerCallback;
      break;
    case HumanActivityType.GPS:
      listener = GPSCallback;
      break;
    default:
      listener = function(result) {
        native_.callIfPossible(args.changedCallback, convertActivityData(args.type, result));
      };
  }

  console.log("callbackInterval = " + callbackInterval + ", sampleInterval = " + sampleInterval);
  startListener(listenerId,
                listener,
                'HumanActivityMonitorManager_start',
                { type: args.type,
                  listenerId: listenerId,
                  callbackInterval: callbackInterval,
                  sampleInterval: sampleInterval
                }
               );

  if (HumanActivityType.PEDOMETER === args.type) {
    pedometerListener = args.changedCallback;
  }

  if (HumanActivityType.GPS === args.type) {
    var callback = function(result) {
      if (native_.isFailure(result)) {
        native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      } else {
        native_.callIfPossible(args.changedCallback, convertActivityData(args.type, result));
      }
    };

    GPSListener = callback;
  }
};

HumanActivityMonitorManager.prototype.stop = function(type) {
  var args = validator_.validateArgs(arguments, [
    {name: 'type', type: types_.ENUM, values: Object.keys(HumanActivityType)}
  ]);

  stopListener('HumanActivityMonitor_'  + args.type,
               'HumanActivityMonitorManager_stop',
               { type: args.type });

  if (HumanActivityType.PEDOMETER === args.type) {
    pedometerListener = null;
  }

  if (HumanActivityType.GPS === args.type) {
    GPSListener = null;
  }
};

HumanActivityMonitorManager.prototype.setAccumulativePedometerListener = function() {
  var args = validator_.validateArgs(arguments, [
    {name: 'changeCallback', type: types_.FUNCTION}
  ]);

  var oldPedometerListener = pedometerListener;

  // calling start() will overwrite pedometerListener, needs to be restored afterwards
  this.start(HumanActivityType.PEDOMETER, args.changeCallback);

  accumulativePedometerListener = args.changeCallback;
  pedometerListener = oldPedometerListener;
};

HumanActivityMonitorManager.prototype.unsetAccumulativePedometerListener = function() {
  var oldPedometerListener = pedometerListener;

  // calling stop() will overwrite pedometerListener, needs to be restored afterwards
  this.stop(HumanActivityType.PEDOMETER);

  accumulativePedometerListener = null;
  pedometerListener = oldPedometerListener;
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
    {name: 'watchId', type: types_.LONG},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var result = native_.call(
                  'HumanActivityMonitorManager_removeActivityRecognitionListener',
                  { watchId: args.watchId });
  if (native_.isFailure(result)) {
    setTimeout(function () { native_.callIfPossible(args.errorCallback, native_.getErrorObject(result)); }, 0);
    return;
  }
  activityRecognitionListener.removeListener(args.watchId);
};

HumanActivityMonitorManager.prototype.startRecorder = function() {
  var args = validator_.validateArgs(arguments, [
    {name: 'type', type: types_.ENUM, values: Object.keys(HumanActivityRecorderType)},
    {name: 'options', type : types_.DICTIONARY, optional: true, nullable: false}
  ]);

  var callArgs = {};

  if (args.options) {
    if (MIN_OPTION_INTERVAL > args.options.interval ||
        MIN_OPTION_RETENTION_PERIOD > args.options.interval) {
      throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR, 'Invalid option value');
    }

    callArgs.options = args.options;
  }

  callArgs.type = args.type;

  var result = native_.callSync('HumanActivityMonitorManager_startRecorder', callArgs);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

HumanActivityMonitorManager.prototype.stopRecorder = function() {
  var args = validator_.validateArgs(arguments, [
    {name: 'type', type: types_.ENUM, values: Object.keys(HumanActivityRecorderType)},
  ]);

  var callArgs = {};
  callArgs.type = args.type;

  var result = native_.callSync('HumanActivityMonitorManager_stopRecorder', callArgs);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

HumanActivityMonitorManager.prototype.readRecorderData = function() {
  var args = validator_.validateArgs(arguments, [
    {name: 'type', type: types_.ENUM, values: Object.keys(HumanActivityRecorderType)},
    {name: 'query', type : types_.DICTIONARY, optional: false, nullable: true},
    {name: 'successCallback', type: types_.FUNCTION},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var callArgs = {};

  if (args.query) {
    if ((args.query.startTime && MIN_QUERY_TIME > args.query.startTime) ||
        (args.query.endTime && MIN_QUERY_TIME > args.query.endTime) ||
        (args.query.anchorTime && MIN_QUERY_TIME > args.query.anchorTime) ||
        (args.query.interval && MIN_QUERY_INTERVAL > args.query.interval) ||
        (args.query.startTime && args.query.endTime && args.query.startTime > args.query.endTime)) {
      throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR, 'Invalid query value');
    }
  }

  callArgs.options = args.options;
  callArgs.type = args.type;
  callArgs.query = args.query;

  var callback = function(result) {
    if (native_.isFailure(result)) {
        native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
    } else {
        var array = convertActivityRecorderData(args.type, native_.getResultObject(result));
        args.successCallback(array);
    }
  };

  var result = native_.call('HumanActivityMonitorManager_readRecorderData', callArgs, callback);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

function StepDifference(data) {
  SetReadOnlyProperty(this, 'stepCountDifference', data.stepCountDifference);
  SetReadOnlyProperty(this, 'timestamp', data.timestamp);
}


function HumanActivityData() {
}


function HumanActivityPedometerData(data) {
  SetReadOnlyProperty(this, 'stepStatus', data.stepStatus);
  SetReadOnlyProperty(this, 'speed', data.speed);
  SetReadOnlyProperty(this, 'walkingFrequency', data.walkingFrequency);
  SetReadOnlyProperty(this, 'cumulativeDistance', data.cumulativeDistance);
  SetReadOnlyProperty(this, 'cumulativeCalorie', data.cumulativeCalorie);
  SetReadOnlyProperty(this, 'cumulativeTotalStepCount', data.cumulativeTotalStepCount);
  SetReadOnlyProperty(this, 'cumulativeWalkStepCount', data.cumulativeWalkStepCount);
  SetReadOnlyProperty(this, 'cumulativeRunStepCount', data.cumulativeRunStepCount);

  var steps = [];
  for (var i = 0; i < data.stepCountDifferences.length; ++i) {
    steps.push(new StepDifference(data.stepCountDifferences[i]));
  }
  SetReadOnlyProperty(this, 'stepCountDifferences', steps);
}

HumanActivityPedometerData.prototype = new HumanActivityData();
HumanActivityPedometerData.prototype.constructor = HumanActivityPedometerData;


function HumanActivityAccumulativePedometerData(data) {
  SetReadOnlyProperty(this, 'stepStatus', data.stepStatus);
  SetReadOnlyProperty(this, 'speed', data.speed);
  SetReadOnlyProperty(this, 'walkingFrequency', data.walkingFrequency);
  SetReadOnlyProperty(this, 'accumulativeDistance', data.accumulativeDistance);
  SetReadOnlyProperty(this, 'accumulativeCalorie', data.accumulativeCalorie);
  SetReadOnlyProperty(this, 'accumulativeTotalStepCount', data.accumulativeTotalStepCount);
  SetReadOnlyProperty(this, 'accumulativeWalkStepCount', data.accumulativeWalkStepCount);
  SetReadOnlyProperty(this, 'accumulativeRunStepCount', data.accumulativeRunStepCount);

  var steps = [];
  for (var i = 0; i < data.stepCountDifferences.length; ++i) {
    steps.push(new StepDifference(data.stepCountDifferences[i]));
  }
  SetReadOnlyProperty(this, 'stepCountDifferences', steps);
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

function HumanActivitySleepMonitorData(data) {
  SetReadOnlyProperty(this, 'status', data.status);
  SetReadOnlyProperty(this, 'timestamp', data.timestamp);
}

HumanActivitySleepMonitorData.prototype = new HumanActivityData();
HumanActivitySleepMonitorData.prototype.constructor = HumanActivitySleepMonitorData;

//Recorded data
function HumanActivityRecorderData(data) {
  if (data) {
    SetReadOnlyProperty(this, 'startTime', data.startTime);
    SetReadOnlyProperty(this, 'endTime', data.endTime);
  }
}

function HumanActivityRecorderPedometerData(data) {
  HumanActivityRecorderData.call(this, data);
  SetReadOnlyProperty(this, 'distance', data.distance);
  SetReadOnlyProperty(this, 'calorie', data.calorie);
  SetReadOnlyProperty(this, 'totalStepCount', data.totalStepCount);
  SetReadOnlyProperty(this, 'walkStepCount', data.walkStepCount);
  SetReadOnlyProperty(this, 'runStepCount', data.runStepCount);
}

HumanActivityRecorderPedometerData.prototype = new HumanActivityRecorderData();
HumanActivityRecorderPedometerData.prototype.constructor = HumanActivityRecorderPedometerData;

function HumanActivityRecorderHRMData(data) {
  HumanActivityRecorderData.call(this, data);
  SetReadOnlyProperty(this, 'heartRate', data.heartRate);
}

HumanActivityRecorderHRMData.prototype = new HumanActivityRecorderData();
HumanActivityRecorderHRMData.prototype.constructor = HumanActivityRecorderHRMData;

function HumanActivityRecorderSleepMonitorData(data) {
  HumanActivityRecorderData.call(this, data);
  SetReadOnlyProperty(this, 'status', data.status);
}

HumanActivityRecorderSleepMonitorData.prototype = new HumanActivityRecorderData();
HumanActivityRecorderSleepMonitorData.prototype.constructor = HumanActivityRecorderSleepMonitorData;

function HumanActivityRecorderPressureData(data) {
  HumanActivityRecorderData.call(this, data);
  SetReadOnlyProperty(this, 'max', data.max);
  SetReadOnlyProperty(this, 'min', data.min);
  SetReadOnlyProperty(this, 'average', data.average);
}

HumanActivityRecorderPressureData.prototype = new HumanActivityRecorderData();
HumanActivityRecorderPressureData.prototype.constructor = HumanActivityRecorderPressureData;

exports = new HumanActivityMonitorManager();
