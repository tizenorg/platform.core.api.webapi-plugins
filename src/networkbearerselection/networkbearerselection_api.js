// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var utils_ = xwalk.utils;
var type_ = utils_.type;
var converter_ = utils_.converter;
var validator_ = utils_.validator;
var types_ = validator_.Types;
var native_ = new xwalk.utils.NativeManager(extension);


var NetworkType = {
  CELLULAR: 'CELLULAR',
  UNKNOWN: 'UNKNOWN'
};


function NetworkBearerSelection() {}

NetworkBearerSelection.prototype.requestRouteToHost = function(networkType, domainName, successCallback, errorCallback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'networkType', type: types_.ENUM, values: Object.keys(NetworkType)},
    {name: 'domainName', type: types_.STRING},
    {name: 'successCallback', type: types_.LISTENER, values: ['onsuccess', 'ondisconnected']},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }

    var _result = native_.getResultObject(result);

    native_.callIfPossible(args.successCallback);
  };

  native_.call('NetworkBearerSelection_requestRouteToHost', args, callback);
};

NetworkBearerSelection.prototype.releaseRouteToHost = function(networkType, domainName, successCallback, errorCallback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'networkType', type: types_.ENUM, values: Object.keys(NetworkType)},
    {name: 'domainName', type: types_.STRING},
    {name: 'successCallback', type: types_.FUNCTION},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }

    var _result = native_.getResultObject(result);

    native_.callIfPossible(args.successCallback);
  };

  native_.call('NetworkBearerSelection_releaseRouteToHost', args, callback);
};


exports = new NetworkBearerSelection();
