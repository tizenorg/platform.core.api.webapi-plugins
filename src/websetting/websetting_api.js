// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
