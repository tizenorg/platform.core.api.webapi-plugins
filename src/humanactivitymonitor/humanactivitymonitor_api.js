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
    {name: 'changedCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var listenerId = 'HumanActivityMonitor_'  + args.type;

  startListener(listenerId,
                function(result) {
                  native_.callIfPossible(args.changedCallback, convertActivityData(args.type, result));
                },
                'HumanActivityMonitorManager_start',
                { type: args.type, listenerId: listenerId });
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
