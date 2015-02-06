/* global tizen, xwalk, extension */

// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


var validator_ = xwalk.utils.validator;
var types_ = validator_.Types;

var callbackId = 0;
var callbacks = {};

var lastAccountChangeListenerId = -1;
var accountChangeListener = {};

function invokeListener(result) {
  if (result.listener === 'accountChange') {
    for (var listenerId in accountChangeListener) {
      console.log("[Account][invokeListener] AccountChangeListenerId: " + listenerId);
      var listener = callbacks[accountChangeListener[listenerId]];
      listener(result);
    }
  }
}

extension.setMessageListener(function(json) {
  var result = JSON.parse(json);

  if (result.hasOwnProperty('listener')) {
    console.log("[Account][setMessageListener] Call listner");
    invokeListener(result);
  } else {
    console.log("[Account][setMessageListener] Call callback");
    var callback = callbacks[result.callbackId];
    callback(result);
  }
});

function nextCallbackId() {
  return callbackId++;
}

function nextAccountChangeListenerId() {
  return ++lastAccountChangeListenerId;
}

function callNative(cmd, args) {
  var json = {'cmd': cmd, 'args': args};
  var argjson = JSON.stringify(json);
  var resultString = extension.internal.sendSyncMessage(argjson);
  var result = JSON.parse(resultString);

  if (typeof result !== 'object') {
    throw new tizen.WebAPIException(tizen.WebAPIException.UNKNOWN_ERR);
  }

  if (result.status === 'success') {
    if (result.result) {
      return result.result;
    }
    return true;
  } else if (result.status === 'error') {
    var err = result.error;
    if (err) {
      throw new tizen.WebAPIException(err.name, err.message);
    }
    return false;
  }
}


function callNativeWithCallback(cmd, args, callback) {
  if (callback) {
    var id = nextCallbackId();
    args.callbackId = id;
    callbacks[id] = callback;
  }

  return callNative(cmd, args);
}

function SetReadOnlyProperty(obj, n, v){
    if (v)
        Object.defineProperty(obj, n, {value:v, writable: false, enumerable: true, configurable: true});
    else
        Object.defineProperty(obj, n, {writable: false, enumerable: true, configurable: true});
}

function AccountProvider(providerInfo) {
  SetReadOnlyProperty(this, 'applicationId', providerInfo.applicationId); // read only property
  SetReadOnlyProperty(this, 'displayName', providerInfo.displayName); // read only property
  SetReadOnlyProperty(this, 'iconUri', providerInfo.iconUri); // read only property
  SetReadOnlyProperty(this, 'smallIconUri', providerInfo.smallIconUri); // read only property
  SetReadOnlyProperty(this, 'capabilities', providerInfo.capabilities); // read only property
  SetReadOnlyProperty(this, 'isMultipleAccountSupported', providerInfo.isMultipleAccountSupported); // read only property
}

function SetAccountId(account, accountId) {

  Object.defineProperty(account, 'id', {writable: true});
  account.id = accountId;
  Object.defineProperty(account, 'id', {writable: false});

  return account;
}

function Account(provider, accountInitDict) {
  console.log("[Constructor of Account] Enter");

  var args = validator_.validateArgs(arguments, [
    {'name' : 'provider', 'type' : types_.PLATFORM_OBJECT, 'values' : AccountProvider},
    {'name' : 'accountInitDict',
      'type' : types_.DICTIONARY,
      'optional' : true,
      'nullable' : true
    }
  ]);

  SetReadOnlyProperty(this, 'id', null); // read only property
  SetReadOnlyProperty(this, 'provider', provider); // read only property
  if(arguments.length > 1) {
      this.userName = accountInitDict.userName;
      this.iconUri = accountInitDict.iconUri;
  } else {
      this.userName = null;
      this.iconUri = null;
  }
}

function MakeAccountObject(obj) {
  console.log("[Account][MakeAccountObject] Enter");
  var account = new Account(new AccountProvider(obj.provider), obj.accountInitDict);
  return SetAccountId(account, obj.id);
}

Account.prototype.setExtendedData = function(key, value) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'key', 'type': types_.STRING},
    {'name': 'value', 'type': types_.STRING}
  ]);

  var nativeParam = {
    'key': args.key,
    'value': args.value
  };


  try {
    var syncResult = callNative('Account_setExtendedData', nativeParam);
    // if you need synchronous result from native function using 'syncResult'.
  } catch (e) {
    throw e;
  }

};

Account.prototype.getExtendedData = function(key) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'key', 'type': types_.STRING}
  ]);

  var nativeParam = {
    'key': args.key
  };


  try {
    var syncResult = callNative('Account_getExtendedData', nativeParam);
    // if you need synchronous result from native function using 'syncResult'.
  } catch (e) {
    throw e;
  }

};

Account.prototype.getExtendedData = function(successCallback, errorCallback) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'successCallback', 'type': types_.LISTENER, 'values': ['onsuccess']},
    {'name': 'errorCallback', 'type': types_.FUNCTION}
  ]);

  var nativeParam = {
  };


  try {
    var syncResult = callNative('Account_getExtendedData', nativeParam);
    // if you need synchronous result from native function using 'syncResult'.
  } catch (e) {
    throw e;
  }

};



function AccountManager() {
  // constructor of AccountManager

}


AccountManager.prototype.add = function(account) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'account', 'type': types_.PLATFORM_OBJECT, 'values': Account}
  ]);

  var nativeParam = {
  };


  try {
    var syncResult = callNative('AccountManager_add', nativeParam);
    // if you need synchronous result from native function using 'syncResult'.
  } catch (e) {
    throw e;
  }

};

AccountManager.prototype.remove = function(accountId) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'accountId', 'type': types_.LONG}
  ]);

  var nativeParam = {
    'accountId': args.accountId
  };


  try {
    var syncResult = callNative('AccountManager_remove', nativeParam);
    // if you need synchronous result from native function using 'syncResult'.
  } catch (e) {
    throw e;
  }

};

AccountManager.prototype.update = function(account) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'account', 'type': types_.PLATFORM_OBJECT, 'values': Account}
  ]);

  var nativeParam = {
  };


  try {
    var syncResult = callNative('AccountManager_update', nativeParam);
    // if you need synchronous result from native function using 'syncResult'.
  } catch (e) {
    throw e;
  }

};

AccountManager.prototype.getAccount = function(accountId) {
  console.log("[AccountManager][getAccount] Enter");
  var args = validator_.validateArgs(arguments, [
    {'name': 'accountId', 'type': types_.UNSIGNED_LONG}
  ]);

  var nativeParam = {
    'accountId': args.accountId
  };

  try {
    var syncResult = callNative('AccountManager_getAccount', nativeParam);
    // if you need synchronous result from native function using 'syncResult'.
  } catch (e) {
    throw e;
  }

  var returnObject = new Account();
  return returnObject;
};

AccountManager.prototype.getAccounts = function(successCallback, errorCallback, applicationId) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'successCallback', 'type': types_.LISTENER, 'values': ['onsuccess']},
    {'name': 'errorCallback', 'type': types_.FUNCTION},
    {'name': 'applicationId', 'type': types_.STRING}
  ]);

  var nativeParam = {
  };

  if (args.applicationId) {
    nativeParam.applicationId = args.applicationId;
  }

  try {
    var syncResult = callNative('AccountManager_getAccounts', nativeParam);
    // if you need synchronous result from native function using 'syncResult'.
  } catch (e) {
    throw e;
  }

};

AccountManager.prototype.getProvider = function(applicationId) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'applicationId', 'type': types_.STRING}
  ]);

  var nativeParam = {
    'applicationId': args.applicationId
  };

  try {
    var syncResult = callNative('AccountManager_getProvider', nativeParam);
    return new AccountProvider(syncResult);
  } catch (e) {
    throw e;
  }
};

AccountManager.prototype.getProviders = function(successCallback, errorCallback, capability) {
  var args = validator_.validateArgs(arguments, [
    {'name' : 'successCallback', 'type' : types_.FUNCTION},
    {'name' : 'errorCallback', 'type' : types_.FUNCTION, 'optional' : true, 'nullable' : true},
    {'name' : 'capability', 'type' : types_.STRING, 'optional' : true, 'nullable' : true}
  ]);

  var nativeParam = {
  };

  if (args.capability) {
    nativeParam.capability = args.capability;
  }

  try {
    var syncResult = callNativeWithCallback(
        'AccountManager_getProviders',
        nativeParam,
        function(param) {
          if (param.status == 'success') {
            for (var i = 0; i < param.result.length; i++) {
              param.result[i] = new AccountProvider(param.result[i]);
            }
            args.successCallback(param.result);
          } else if (param.status == 'error') {
            var err = param.error;
            if (err) {
              args.errorCallback(new tizen.WebAPIError(err.name, err.message));
              return;
            }
          }

          delete callbacks[param.callbackId];
        });
  } catch (e) {
    throw e;
  }
};

AccountManager.prototype.addAccountListener = function(callback) {
  var args = validator_.validateArgs(
      arguments,
      [
          {'name' : 'callback',
            'type' : types_.LISTENER,
            'values' : ['onadded', 'onremoved', 'onupdated']}
      ]);

  var nativeParam = {
  };

  try {
    var syncResult = callNativeWithCallback(
        'AccountManager_addAccountListener',
        nativeParam,
        function(param) {
          if (param.status === 'added') {            
            args.callback.onadded(MakeAccountObject(param.result));
          } else if (param.status === 'removed') {
            args.callback.onremoved(param.result);
          } else if (param.status === 'updated') {
            args.callback.onupdated(MakeAccountObject(param.result));
          }
        });

    console.log("[Account][addAccountListener] callbackId: " + nativeParam.callbackId);
    var listenerId = nextAccountChangeListenerId();
    accountChangeListener[listenerId] = nativeParam.callbackId;
    console.log("[Account][addAccountListener] listenerId: " + listenerId);
    return listenerId;
  } catch (e) {
    throw e;
  }
};

AccountManager.prototype.removeAccountListener = function(accountListenerId) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'accountListenerId', 'type': types_.LONG}
  ]);

  var nativeParam = {
  };

  try {
    if (args.accountListenerId in accountChangeListener) {
      delete callbacks[accountChangeListener[args.accountListenerId]];
      delete accountChangeListener[args.accountListenerId];
    }

    if (Object.keys(accountChangeListener).length === 0) {
      console.log("[Account][removeAccountListener] Unscribe native notification");
      var syncResult = callNative('AccountManager_removeAccountListener', nativeParam);
    }
  } catch (e) {
    throw e;
  }
};

tizen.Account = Account;

exports = new AccountManager();

