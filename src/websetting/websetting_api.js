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

var validator_ = xwalk.utils.validator;
var native_ = new xwalk.utils.NativeManager(extension);

exports.setUserAgentString = function() {
  var args = validator_.validateArgs(arguments, [
    {
      name: 'userAgent',
      type: validator_.Types.STRING,
      optional: false,
      nullable: false
    },
    {
      name: 'successCallback',
      type: validator_.Types.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'errorCallback',
      type: validator_.Types.FUNCTION,
      optional: true,
      nullable: true
    }
  ]);

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
    } else {
      args.successCallback();
    }
  };

  native_.sendRuntimeAsyncMessage('tizen://changeUA', args.userAgent, callback);
};

exports.removeAllCookies = function() {
  var args = validator_.validateArgs(arguments, [
    {
      name: 'successCallback',
      type: validator_.Types.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'errorCallback',
      type: validator_.Types.FUNCTION,
      optional: true,
      nullable: true
    }
  ]);

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
    } else {
      args.successCallback();
    }
  };

  native_.sendRuntimeAsyncMessage('tizen://deleteAllCookies', '', callback);
};
