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

var WidgetLifecycleEventType = {
  CREATE : 'CREATE',
  DESTROY : 'DESTROY',
  PAUSE : 'PAUSE',
  RESUME : 'RESUME',
};

function createObjects(data, func, widget) {
  var array = [];
  var objects = native.getResultObject(data);

  objects.forEach(function (d) {
    array.push(new func(d, widget));
  });

  return array;
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

function WidgetVariant(data) {
  Object.defineProperties(this, {
    sizeType: {
      value: data.sizeType,
      writable: false,
      enumerable: true
    },
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
    previewImagePath: {
      value: data.previewImagePath,
      writable: false,
      enumerable: true
    },
    needsMouseEvents: {
      value: data.needsMouseEvents,
      writable: false,
      enumerable: true
    },
    needsTouchEffect: {
      value: data.needsTouchEffect,
      writable: false,
      enumerable: true
    },
    needsFrame: {
      value: data.needsFrame,
      writable: false,
      enumerable: true
    },
  });
};

function WidgetInstance(data, widget) {
  Object.defineProperties(this, {
    widget: {
      value: widget,
      writable: false,
      enumerable: true
    },
    id: {
      value: data.id,
      writable: false,
      enumerable: true
    },
  });
};

WidgetInstance.prototype.changeUpdatePeriod = function() {
  var args = validator.validateMethod(arguments, [{
    name : 'period',
    type : types.DOUBLE,
  }]);

  var callArgs = {};
  callArgs.widgetId = this.widget.id;
  callArgs.instanceId = this.id;
  callArgs.period = args.period;

  var ret = native.callSync('WidgetInstance_changeUpdatePeriod', callArgs);

  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
};

WidgetInstance.prototype.sendContent = function() {
  var args = validator.validateMethod(arguments, [{
    name : 'data',
    type : types.ARRAY,
    values: types.BYTE
  }, {
    name : 'force',
    type : types.BOOLEAN,
  }]);

  var callArgs = {};
  callArgs.widgetId = this.widget.id;
  callArgs.instanceId = this.id;
  callArgs.data = args.data;
  callArgs.force = args.force;

  var ret = native.callSync('WidgetInstance_sendContent', callArgs);

  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
};

WidgetInstance.prototype.getContent = function() {
  var args = validator.validateMethod(arguments, [{
    name : 'successCallback',
    type : types.FUNCTION,
  }, {
    name : 'errorCallback',
    type : types.FUNCTION,
  }]);

  var callArgs = {};
  callArgs.widgetId = this.widget.id;
  callArgs.instanceId = this.id;

  var callback = function(result) {
    if (native.isFailure(result)) {
      args.errorCallback(native.getErrorObject(result));
    } else {
      //TODO what is type of returned data
      args.successCallback(native.getResultObject(result));
    }
  };

  var result = native.call('WidgetInstance_getContent', callArgs, callback);
  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
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

Widget.prototype.getName = function() {
  var args = validator.validateMethod(arguments, [{
    name : 'lang',
    type : types.STRING,
    optional : true,
    nullable : true
  }]);

  var callArgs = {};
  callArgs.widgetId = this.id;

  if (args.lang) {
    callArgs.lang = args.lang;
  }

  var ret = native.callSync('Widget_getName', callArgs);

  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  } else {
    return native.getResultObject(ret);
  }
};

Widget.prototype.getInstances = function() {
  var args = validator.validateMethod(arguments, [{
    name : 'successCallback',
    type : types.FUNCTION,
  }, {
    name : 'errorCallback',
    type : types.FUNCTION,
    optional : true,
    nullable : true
  }]);

  var callback = function(result) {
    if (native.isFailure(result)) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(result));
    } else {
      var instances = createObjects(result, WidgetInstance, this);
      args.successCallback(instances);
    }
  };

  var callArgs = {};
  callArgs.widgetId = this.id;

  var result = native.call('Widget_getInstances', callArgs, callback);
  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
};

Widget.prototype.getVariant = function() {
  var args = validator.validateMethod(arguments, [{
    name : 'sizeType',
    type: types.ENUM,
    values: T.getValues(WidgetSizeType)
  }]);

  var callArgs = {};
  callArgs.widgetId = this.id;
  callArgs.sizeType = args.sizeType;

  var ret = native.callSync('Widget_getVariant', callArgs);

  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  } else {
    return new WidgetVariant(native.getResultObject(ret));
  }
};

Widget.prototype.getVariants = function() {
  var args = validator.validateMethod(arguments, [{
    name : 'successCallback',
    type : types.FUNCTION,
  }, {
    name : 'errorCallback',
    type : types.FUNCTION,
    optional : true,
    nullable : true
  }]);

  var callback = function(result) {
    if (native.isFailure(result)) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(result));
    } else {
      var variants = createObjects(result, WidgetVariant);
      args.successCallback(variants);
    }
  };

  var callArgs = {};
  callArgs.widgetId = this.id;

  var result = native.call('Widget_getVariants', callArgs, callback);
  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
};

function ListenerManager(native, listenerName) {
  this.listeners = {};
  this.nextId = 1;
  this.nativeSet = false;
  this.native = native;
  this.listenerName = listenerName;
};

ListenerManager.prototype.onListenerCalled = function(msg) {
  for (var watchId in this.listeners) {
    if (this.listeners.hasOwnProperty(watchId) && this.listeners[watchId][msg.action]) {
      this.listeners[watchId][msg.action](msg.id, msg.event);
    }
  }
};

ListenerManager.prototype.addListener = function(callback) {
  var id = this.nextId;
  if (!this.nativeSet) {
    this.native.addListener(this.listenerName, this.onListenerCalled.bind(this));
    this.nativeSet = true;
  }
  this.listeners[id] = callback;
  ++this.nextId;
  return id;
};

ListenerManager.prototype.removeListener = function(watchId) {
  if (this.listeners[watchId] === null || this.listeners[watchId] === undefined) {
    throw new WebAPIException(0, 'Watch id not found.', 'NotFoundError');
  }

  if (this.listeners.hasOwnProperty(watchId)) {
    delete this.listeners[watchId];
  }
};

var WIDGET_CHANGE_LISTENER = 'WidgetChangeCallback';
var widgetChangeListener = new ListenerManager(native, WIDGET_CHANGE_LISTENER);

Widget.prototype.addChangeListener = function() {
  var args = validator.validateMethod(arguments, [{
    name : 'eventCallback',
    type : types.FUNCTION,
  }]);

  if (T.isEmptyObject(widgetChangeListener.listeners)) {
    var result = native.callSync('Widget_addChangeListener', {widgetId : this.id});
    if (native.isFailure(result)) {
      throw native.getErrorObject(result);
    }
  }

  return widgetChangeListener.addListener(args.eventCallback);
};

Widget.prototype.removeChangeListener = function() {
  var args = validator.validateMethod(arguments, [{
    name : 'watchId',
    type : types.LONG,
  }]);

  widgetChangeListener.removeListener(args.watchId);

  if (T.isEmptyObject(widgetChangeListener.listeners)) {
    var result = native.callSync('Widget_removeChangeListener', {widgetId : this.id});
    if (native.isFailure(result)) {
      throw native.getErrorObject(result);
    }
  }
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
      var widgets = createObjects(result, Widget);
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
