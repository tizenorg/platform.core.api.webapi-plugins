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

tizen.debug = extension;

var validator_ = xwalk.utils.validator;
var types_ = validator_.Types;


var callbackId = 0;
var callbacks = {};

extension.setMessageListener(function(json) {
  var result = JSON.parse(json);
  var callback = callbacks[result['callbackId']];
  callback(result);
  delete callbacks[result['callbackId']];
});

function nextCallbackId() {
  return callbackId++;
}

function callNative(cmd, args) {
  var json = {'cmd': cmd, 'args': args};
  var argjson = JSON.stringify(json);
  var resultString = extension.internal.sendSyncMessage(argjson);
  var result = JSON.parse(resultString);

  if (typeof result !== 'object') {
    throw new WebAPIException(WebAPIException.UNKNOWN_ERR);
  }

  if (result['status'] == 'success') {
    if (result['result']) {
      return result['result'];
    }
    return true;
  } else if (result['status'] == 'error') {
    var err = result['error'];
    if (err) {
      throw new WebAPIException(err.name, err.message);
    }
    return false;
  }
}


function callNativeWithCallback(cmd, args, callback) {
  if (callback) {
    var id = nextCallbackId();
    args['callbackId'] = id;
    callbacks[id] = callback;
  }

  return callNative(cmd, args);
}

function SetReadOnlyProperty(obj, n, v) {
  Object.defineProperty(obj, n, {value: v, writable: false});
}

var DataType = {
  'MAP': 'MAP',
  'SQL': 'SQL'
};

function DataControlManager() {
  // constructor of DataControlManager
}


DataControlManager.prototype.getDataControlConsumer = function(providerId, dataId, type) {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.DATACONTROL_CONSUMER);

  var args = validator_.validateArgs(arguments, [
    {'name': 'providerId', 'type': types_.STRING},
    {'name': 'dataId', 'type': types_.STRING},
    {'name': 'type', 'type': types_.ENUM, 'values': ['MAP', 'SQL']}
  ]);

  var returnObject = null;
  if (type === 'SQL') {
    returnObject = new SQLDataControlConsumer();
  } else if (type === 'MAP') {
    returnObject = new MappedDataControlConsumer();
  }
  SetReadOnlyProperty(returnObject, 'type', type); // read only property
  SetReadOnlyProperty(returnObject, 'providerId', providerId); // read only property
  SetReadOnlyProperty(returnObject, 'dataId', dataId); // read only property

  return returnObject;
};


function DataControlConsumerObject() {
  // constructor of DataControlConsumerObject
}



function SQLDataControlConsumer() {
  // constructor of SQLDataControlConsumer
}

SQLDataControlConsumer.prototype = new DataControlConsumerObject();
SQLDataControlConsumer.prototype.constructor = SQLDataControlConsumer;

SQLDataControlConsumer.prototype.insert = function(reqId, insertionData) {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.DATACONTROL_CONSUMER);

  var args = validator_.validateArgs(arguments, [
    {'name': 'reqId', 'type': types_.LONG},
    {'name': 'insertionData', 'type': types_.DICTIONARY},
    {'name': 'successCallback', 'type': types_.FUNCTION, optional: true, nullable: true},
    {'name': 'errorCallback', 'type': types_.FUNCTION, optional: true, nullable: true}
  ]);

  var nativeParam = {
    'providerId': this.providerId,
    'dataId': this.dataId,
    'reqId': args.reqId,
    'insertionData': insertionData
  };
  try {
    var syncResult =
        callNativeWithCallback('SQLDataControlConsumer_insert', nativeParam, function(result) {
      if (result.status == 'success') {
        if (args.successCallback) {
          args.successCallback(result['requestId'], result['result']);
        }
      }
      if (result.status == 'error') {
        if (args.errorCallback) {
          var err = result['result'];
          var e = new WebAPIException(err.name, err.message);
          args.errorCallback(result['requestId'], e);
        }
      }
    });
  } catch (e) {
    throw e;
  }
};

SQLDataControlConsumer.prototype.update = function(reqId, updateData, where) {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.DATACONTROL_CONSUMER);

  var args = validator_.validateArgs(arguments, [
    {'name': 'reqId', 'type': types_.LONG},
    {'name': 'updateData', 'type': types_.DICTIONARY},
    {'name': 'where', 'type': types_.STRING},
    {'name': 'successCallback', 'type': types_.FUNCTION, optional: true, nullable: true},
    {'name': 'errorCallback', 'type': types_.FUNCTION, optional: true, nullable: true}
  ]);

  var nativeParam = {
    'providerId': this.providerId,
    'dataId': this.dataId,
    'reqId': args.reqId,
    'where': args.where,
    'updateData': args.updateData
  };
  try {
    var syncResult =
        callNativeWithCallback('SQLDataControlConsumer_update', nativeParam, function(result) {
      if (result.status == 'success') {
        if (args.successCallback) {
          args.successCallback(result['requestId']);
        }
      }
      if (result.status == 'error') {
        if (args.errorCallback) {
          var err = result['result'];
          var e = new WebAPIException(err.name, err.message);
          args.errorCallback(result['requestId'], e);
        }
      }
    });
  } catch (e) {
    throw e;
  }

};

SQLDataControlConsumer.prototype.remove = function(reqId, where) {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.DATACONTROL_CONSUMER);

  var args = validator_.validateArgs(arguments, [
    {'name': 'reqId', 'type': types_.LONG},
    {'name': 'where', 'type': types_.STRING},
    {'name': 'successCallback', 'type': types_.FUNCTION, optional: true, nullable: true},
    {'name': 'errorCallback', 'type': types_.FUNCTION, optional: true, nullable: true}
  ]);

  var nativeParam = {
    'providerId': this.providerId,
    'dataId': this.dataId,
    'reqId': args.reqId,
    'where': args.where
  };
  try {
    var syncResult =
        callNativeWithCallback('SQLDataControlConsumer_remove', nativeParam, function(result) {
      if (result.status == 'success') {
        if (args.successCallback) {
          args.successCallback(result['requestId']);
        }
      }
      if (result.status == 'error') {
        if (args.errorCallback) {
          var err = result['result'];
          var e = new WebAPIException(err.name, err.message);
          args.errorCallback(result['requestId'], e);
        }
      }
    });
  } catch (e) {
    throw e;
  }

};

SQLDataControlConsumer.prototype.select = function(reqId, columns, where, successCallback) {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.DATACONTROL_CONSUMER);

  var args = validator_.validateArgs(arguments, [
    {'name': 'reqId', 'type': types_.LONG},
    {'name': 'columns', 'type': types_.ARRAY},
    {'name': 'where', 'type': types_.STRING},
    {'name': 'successCallback', 'type': types_.FUNCTION},
    {'name': 'errorCallback', 'type': types_.FUNCTION, optional: true, nullable: true},
    {'name': 'page', 'type': types_.LONG, optional: true},
    {'name': 'maxNumberPerPage', 'type': types_.LONG, optional: true}
  ]);

  var nativeParam = {
    'providerId': this.providerId,
    'dataId': this.dataId,
    'reqId': args.reqId,
    'columns': args.columns,
    'where': args.where
  };
  if (args['page']) {
    nativeParam['page'] = args.page;
  }
  if (args['maxNumberPerPage']) {
    nativeParam['maxNumberPerPage'] = args.maxNumberPerPage;
  }
  try {
    var syncResult =
        callNativeWithCallback('SQLDataControlConsumer_select', nativeParam, function(result) {
      if (result.status == 'success') {
        args.successCallback(result['result'], result['requestId']);
      }
      if (result.status == 'error') {
        if (args.errorCallback) {
          var err = result['result'];
          var e = new WebAPIException(err.name, err.message);
          args.errorCallback(result['requestId'], e);
        }
      }
    });
    // if you need synchronous result from native function using 'syncResult'.
  } catch (e) {
    throw e;
  }

};


function MappedDataControlConsumer() {
  // constructor of MappedDataControlConsumer
}

MappedDataControlConsumer.prototype = new DataControlConsumerObject();
MappedDataControlConsumer.prototype.constructor = MappedDataControlConsumer;

MappedDataControlConsumer.prototype.addValue = function(reqId, key, value) {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.DATACONTROL_CONSUMER);

  var args = validator_.validateArgs(arguments, [
    {'name': 'reqId', 'type': types_.LONG},
    {'name': 'key', 'type': types_.STRING},
    {'name': 'value', 'type': types_.STRING},
    {'name': 'successCallback', 'type': types_.FUNCTION, optional: true, nullable: true},
    {'name': 'errorCallback', 'type': types_.FUNCTION, optional: true, nullable: true}
  ]);

  var nativeParam = {
    'providerId': this.providerId,
    'dataId': this.dataId,
    'reqId': args.reqId,
    'key': args.key,
    'value': args.value
  };
  try {
    var syncResult =
        callNativeWithCallback('MappedDataControlConsumer_addValue', nativeParam, function(result) {
      if (result.status == 'success') {
        if (args.successCallback) {
          args.successCallback(result['requestId']);
        }
      }
      if (result.status == 'error') {
        if (args.errorCallback) {
          var err = result['result'];
          var e = new WebAPIException(err.name, err.message);
          args.errorCallback(result['requestId'], e);
        }
      }
    });
  } catch (e) {
    throw e;
  }

};

MappedDataControlConsumer.prototype.removeValue = function(reqId, key, value, successCallback) {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.DATACONTROL_CONSUMER);

  var args = validator_.validateArgs(arguments, [
    {'name': 'reqId', 'type': types_.LONG},
    {'name': 'key', 'type': types_.STRING},
    {'name': 'value', 'type': types_.STRING},
    {'name': 'successCallback', 'type': types_.FUNCTION},
    {'name': 'errorCallback', 'type': types_.FUNCTION, optional: true, nullable: true}
  ]);

  var nativeParam = {
    'providerId': this.providerId,
    'dataId': this.dataId,
    'reqId': args.reqId,
    'key': args.key,
    'value': args.value
  };
  try {
    var syncResult =
        callNativeWithCallback('MappedDataControlConsumer_removeValue', nativeParam, function(result) {
      if (result.status == 'success') {
        args.successCallback(result['requestId']);
      }
      if (result.status == 'error') {
        if (args.errorCallback) {
          var err = result['result'];
          var e = new WebAPIException(err.name, err.message);
          args.errorCallback(result['requestId'], e);
        }
      }
    });
  } catch (e) {
    throw e;
  }

};

MappedDataControlConsumer.prototype.getValue = function(reqId, key, successCallback) {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.DATACONTROL_CONSUMER);

  var args = validator_.validateArgs(arguments, [
    {'name': 'reqId', 'type': types_.LONG},
    {'name': 'key', 'type': types_.STRING},
    {'name': 'successCallback', 'type': types_.FUNCTION},
    {'name': 'errorCallback', 'type': types_.FUNCTION, optional: true, nullable: true}
  ]);

  var nativeParam = {
    'providerId': this.providerId,
    'dataId': this.dataId,
    'reqId': args.reqId,
    'key': args.key
  };
  try {
    var syncResult = callNativeWithCallback('MappedDataControlConsumer_getValue', nativeParam, function(result) {
      if (result.status == 'success') {
        args.successCallback(result['result'], result['requestId']);
      }
      if (result.status == 'error') {
        if (args.errorCallback) {
          var err = result['result'];
          var e = new WebAPIException(err.name, err.message);
          args.errorCallback(result['requestId'], e);
        }
      }
    });
  } catch (e) {
    throw e;
  }

};

MappedDataControlConsumer.prototype.updateValue = function(
        reqId, key, oldValue, newValue, successCallback) {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.DATACONTROL_CONSUMER);

  var args = validator_.validateArgs(arguments, [
    {'name': 'reqId', 'type': types_.LONG},
    {'name': 'key', 'type': types_.STRING},
    {'name': 'oldValue', 'type': types_.STRING},
    {'name': 'newValue', 'type': types_.STRING},
    {'name': 'successCallback', 'type': types_.FUNCTION},
    {'name': 'errorCallback', 'type': types_.FUNCTION, optional: true, nullable: true}
  ]);

  var nativeParam = {
    'providerId': this.providerId,
    'dataId': this.dataId,
    'reqId': args.reqId,
    'key': args.key,
    'oldValue': args.oldValue,
    'newValue': args.newValue
  };
  try {
    var syncResult =
        callNativeWithCallback('MappedDataControlConsumer_updateValue', nativeParam, function(result) {
      if (result.status == 'success') {
        args.successCallback(result['requestId']);
      }
      if (result.status == 'error') {
        if (args.errorCallback) {
          var err = result['result'];
          var e = new WebAPIException(err.name, err.message);
          args.errorCallback(result['requestId'], e);
        }
      }
    });
  } catch (e) {
    throw e;
  }

};



exports = new DataControlManager();

