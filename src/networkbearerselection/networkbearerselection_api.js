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


var NetworkType = {
  CELLULAR: 'CELLULAR',
  UNKNOWN: 'UNKNOWN'
};

var callbackId = 0;
var callbacks = {};

function nextCallbackId() {
  return callbackId++;
}

function _networkBearerSelectionCallback(result) {
  var id, callback;

  for (id in callbacks) {
    if (callbacks.hasOwnProperty(result.id)) {
      callback = callbacks[id];
      if (result.state === 'Success') {
        native_.callIfPossible(callback.onsuccess);
      }
      if (result.state === 'Disconnected') {
        native_.callIfPossible(callback.ondisconnected);
        native_.removeListener('NetworkBearerSelectionCallback_' + id);
        delete callbacks[id];
      }
      if (result.state === 'Error') {
        native_.callIfPossible(callback.onerror, native_.getErrorObject(result));
        native_.removeListener('NetworkBearerSelectionCallback_' + id);
        delete callbacks[id];
      }
    }
  }
}

function NetworkBearerSelection() {}

NetworkBearerSelection.prototype.requestRouteToHost = function(networkType, domainName, successCallback, errorCallback) {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NETWORKBEARERSELECTION);
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.INTERNET);

  var args = validator_.validateArgs(arguments, [
    {name: 'networkType', type: types_.ENUM, values: Object.keys(NetworkType)},
    {name: 'domainName', type: types_.STRING},
    {name: 'successCallback', type: types_.LISTENER, values: ['onsuccess', 'ondisconnected']},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var id = nextCallbackId();

  var nativeParam = {
    networkType: args.networkType,
    domainName: args.domainName,
    id: id
  };

  callbacks[id] = {
    onsuccess: args.successCallback.onsuccess,
    ondisconnected: args.successCallback.ondisconnected,
    onerror: args.errorCallback
  };

  native_.addListener('NetworkBearerSelectionCallback_' + id, _networkBearerSelectionCallback);
  var result = native_.callSync('NetworkBearerSelection_requestRouteToHost', nativeParam);

  if (native_.isFailure(result)) {
    native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
  }
};

NetworkBearerSelection.prototype.releaseRouteToHost = function(networkType, domainName, successCallback, errorCallback) {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NETWORKBEARERSELECTION);
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.INTERNET);

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

    native_.callIfPossible(args.successCallback);
  };

  native_.call('NetworkBearerSelection_releaseRouteToHost', args, callback);
};


exports = new NetworkBearerSelection();
