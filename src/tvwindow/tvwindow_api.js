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
 
var native = new xwalk.utils.NativeManager(extension);
var validator = xwalk.utils.validator;
var validatorTypes = xwalk.utils.validator.Types;

//TVWindowManager interface
function TVWindowManager() {
  if (!(this instanceof TVWindowManager)) {
    throw new TypeError;
  }
}


TVWindowManager.prototype.getAvailableWindows = function(successCallback, errorCallback) {
  return undefined;
};

TVWindowManager.prototype.setSource = function(videosource, successCallback, errorCallback, type) {
  return undefined;
};

TVWindowManager.prototype.getSource = function(type) {
  return undefined;
};

TVWindowManager.prototype.show = function(successCallback, errorCallback, rectangle, type) {
  return undefined;
};

TVWindowManager.prototype.hide = function(successCallback, errorCallback, type) {
  return undefined;
};

TVWindowManager.prototype.getRect = function(successCallback, errorCallback, unit, type) {
  return undefined;
};

exports = new TVWindowManager();
