// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var validator_ = xwalk.utils.validator;
var type_ = xwalk.utils.type;
var native_ = new xwalk.utils.NativeManager(extension);
var converter_ = xwalk.utils.converter;

exports.setUserAgentString = function() {
  var args = validator_.validateArgs(arguments, [
    {
      name: 'uri',
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
      native_.callIfPossible(args.errorCallback,
          native_.getErrorObject(result));
    } else {
      var exifInfo = native_.getResultObject(result);
      args.successCallback(exifInfo);
    }
  };

  native_.call('WebSettingManager_setUserAgentString', {'userAgent': args.userAgent},
      callback);
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
      native_.callIfPossible(args.errorCallback,
          native_.getErrorObject(result));
    } else {
      var exifInfo = native_.getResultObject(result);
      args.successCallback(exifInfo);
    }
  };

  native_.call('WebSettingManager_removeAllCookies', {},
      callback);
};
