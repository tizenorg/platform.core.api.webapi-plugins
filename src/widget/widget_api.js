/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd All Rights Reserved
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

var validator = xwalk.utils.validator;
var converter = xwalk.utils.converter;
var types = validator.Types;
var T = xwalk.utils.type;
var native = new xwalk.utils.NativeManager(extension);

var WidgetSizeType = {
  S_1x1 : '1x1',
  S_2x1 : '2x1',
  S_2x2 : '2x2',
  S_4x1 : '4x1',
  S_4x2 : '4x2',
  S_4x3 : '4x3',
  S_4x4 : '4x4',
  S_4x5 : '4x5',
  S_4x6 : '4x6',
  EASY_1x1 : 'EASY_1x1',
  EASY_3x1 : 'EASY_3x1',
  EASY_3x3 : 'EASY_3x3',
  FULL : 'FULL',
};

function WidgetSize(data) {
  Object.defineProperties(this, {
    width: {
      value: data.width,
      writable: false,
      enumerable: true
    },
    height: {
      value: data.height,
      writable: false,
      enumerable: true
    },
  });
};

function createWidgets(e) {
  var widgets_array = [];
  var widgets = native.getResultObject(e);

  widgets.forEach(function (data) {
    widgets_array.push(new Widget(data));
  });

  return widgets_array;
};

function Widget(data) {
  Object.defineProperties(this, {
    id: {
      value: data.id,
      writable: false,
      enumerable: true
    },
    applicationId: {
      value: data.applicationId,
      writable: false,
      enumerable: true
    },
    setupApplicationId: {
      value: data.setupApplicationId ? data.setupApplicationId : null,
      writable: false,
      enumerable: true
    },
    packageId: {
      value: data.packageId,
      writable: false,
      enumerable: true
    },
    noDisplay: {
      value: data.noDisplay,
      writable: false,
      enumerable: true
    },
  });
};

function WidgetManager() {
};

WidgetManager.prototype.getWidget = function() {
  var args = validator.validateMethod(arguments, [{
    name : 'widgetId',
    type : types.STRING,
  }]);

  var callArgs = {};
  callArgs.widgetId = args.widgetId;

  var ret = native.callSync('WidgetManager_getWidget', callArgs);

  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  } else {
    return new Widget(native.getResultObject(ret));
  }
};

WidgetManager.prototype.getWidgets = function() {
  var args = validator.validateMethod(arguments, [{
    name : 'successCallback',
    type : types.FUNCTION,
  }, {
    name : 'errorCallback',
    type : types.FUNCTION,
    optional : true,
    nullable : true
  }, {
    name : 'packageId',
    type : types.STRING,
    optional : true,
    nullable : true
  }]);

  var callback = function(result) {
    if (native.isFailure(result)) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(result));
    } else {
      var widgets = createWidgets(result);
      args.successCallback(widgets);
    }
  };

  var callArgs = {};
  if (args.packageId) {
    callArgs.packageId = args.packageId;
  }

  var result = native.call('WidgetManager_getWidgets', callArgs, callback);
  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
};

WidgetManager.prototype.getPrimaryWidgetId = function() {
  var args = validator.validateMethod(arguments, [{
    name : 'id',
    type : types.STRING,
  }]);

  var callArgs = {};
  callArgs.id = args.id;

  var ret = native.callSync('WidgetManager_getPrimaryWidgetId', callArgs);

  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  } else {
    return native.getResultObject(ret);
  }
};

WidgetManager.prototype.getSize = function() {
  var args = validator.validateMethod(arguments, [{
    name : 'sizeType',
    type: types.ENUM,
    values: T.getValues(WidgetSizeType)
  }]);

  var callArgs = {};
  callArgs.sizeType = args.sizeType;

  var ret = native.callSync('WidgetManager_getSize', callArgs);

  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  } else {
    return new WidgetSize(native.getResultObject(ret));
  }
};

//Exports
exports = new WidgetManager();
