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
var WindowType = {
  MAIN: 'MAIN'
};

//TVWindowManager interface
function TVWindowManager() {
  if (!(this instanceof TVWindowManager)) {
    throw new TypeError;
  }
  var ret = native.callSync('TVWindow_GetScreenDimension', {});
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
  var result = native.getResultObject(ret);
  this.max_width = result.width;
  this.max_height = result.height;
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
  // todo privilege check
//  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.TV_WINDOW);
  var args = validator.validateArgs(arguments, [
    {
      name: 'successCallback',
      type: validator.Types.FUNCTION
    },
    {
      name: 'errorCallback',
      type: validator.Types.FUNCTION,
      optional: true,
      nullable: true
    }//,
    //todo rectangle
//    {
//      name: 'type',
//      optional: true,
//      nullable: true,
//      type: validator.Types.ENUM,
//      values: Object.keys(WindowType)
//    }
  ]);

  native.sendRuntimeAsyncMessage('tizen://tvwindow/show', null, function(result) {
    if (native.isFailure(result)) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(result));
    } else {
      native.sendRuntimeAsyncMessage('tizen://tvwindow/reposition', '400,400,120,80', function(result) {
        if (native.isFailure(result)) {
          native.callIfPossible(args.errorCallback, native.getErrorObject(result));
        } else {
          //todo wait for tv-window ready
          setTimeout(function() {
              args.successCallback([400,400,120,80], "MAIN");
          }, 1000);
        }
      });
    }
  });
};

TVWindowManager.prototype.hide = function(successCallback, errorCallback, type) {
  // todo privilege check
//  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.TV_WINDOW);
  var args = validator.validateArgs(arguments, [
    {
      name: 'successCallback',
      type: validator.Types.FUNCTION
    },
    {
      name: 'errorCallback',
      type: validator.Types.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'type',
      optional: true,
      nullable: true,
      type: validator.Types.ENUM,
      values: Object.keys(WindowType)
    }
  ]);

  native.sendRuntimeAsyncMessage('tizen://tvwindow/hide', null, function(result) {
    if (native.isFailure(result)) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(result));
    } else {
      native.callIfPossible(args.successCallback);
    }
  });
};

TVWindowManager.prototype.getRect = function(successCallback, errorCallback, unit, type) {
  return undefined;
};

exports = new TVWindowManager();
