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


var validator_ = xwalk.utils.validator;
var type_ = xwalk.utils.type;
var types_ = validator_.Types;
var native_ = new xwalk.utils.NativeManager(extension);


function PreferenceManager() {
}


PreferenceManager.prototype.getAll = function() {
  // ...
};

PreferenceManager.prototype.setValue = function() {
  var args = validator_.validateArgs(arguments, [
    { name: 'key', type: types_.STRING },
    { name: 'value', type: types_.SIMPLE_TYPE }
  ]);

  var result = native_.callSync('PreferenceManager_setValue',
                                {
                                   key: args.key,
                                   value: args.value
                                }
  );

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};


PreferenceManager.prototype.getValue = function() {
  var args = validator_.validateArgs(arguments, [
    { name: 'key', type: types_.STRING }
  ]);

  var result = native_.callSync('PreferenceManager_getValue', { key: args.key });

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  } else {
    return native_.getResultObject(result);
  }
};


PreferenceManager.prototype.remove = function() {
  var args = validator_.validateArgs(arguments, [
    { name: 'key', type: types_.STRING }
  ]);

  var result = native_.callSync('PreferenceManager_remove', { key: args.key });

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};


PreferenceManager.prototype.removeAll = function() {
  var result = native_.callSync('PreferenceManager_removeAll', {});

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};


PreferenceManager.prototype.exists = function() {
  var args = validator_.validateArgs(arguments, [
    { name: 'key', type: types_.STRING }
  ]);

  var result = native_.callSync('PreferenceManager_exists', { key: args.key });

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  } else {
    return native_.getResultObject(result);
  }
};


var PREFERENCE_LISTENER = 'PREFERENCE_CHANGED';


function PreferenceListeners() {
  var that = this;
  this.appListener = function (result) {
    var data = native_.getResultObject(result);
    var key = data.key;
    var value = data.value;

    if (that.instances[key]) {
      var listener = that.instances[key];
      if (type_.isFunction(listener)) {
        listener({ key: key, value: value });
      }
    }
  };
}


PreferenceListeners.prototype.instances = {};


PreferenceListeners.prototype.addListener = function(key, listener) {
  if (type_.isEmptyObject(this.instances)) {
    native_.addListener(PREFERENCE_LISTENER, this.appListener);
  }

  this.instances[key] = listener;
};


PreferenceListeners.prototype.removeListener = function(key) {
  if (this.instances[key]) {
    delete this.instances[key];
    if (type_.isEmptyObject(this.instances)) {
      native_.removeListener(PREFERENCE_LISTENER);
    }
  }
};


var _preferenceListeners = new PreferenceListeners();


PreferenceManager.prototype.setChangeListener = function() {
  var args = validator_.validateArgs(arguments, [
    { name: 'key', type: types_.STRING },
    { name: 'listener', type: types_.FUNCTION }
  ]);

  var result = native_.callSync('PreferenceManager_setChangeListener', { key: args.key });
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  _preferenceListeners.addListener(args.key, args.listener);
}


PreferenceManager.prototype.unsetChangeListener = function() {
  var args = validator_.validateArgs(arguments, [
    { name: 'key', type: types_.STRING }
  ]);

  var result = native_.callSync('PreferenceManager_unsetChangeListener', { key: args.key });

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  _preferenceListeners.removeListener(args.key);
};


// Exports
exports = new PreferenceManager();

