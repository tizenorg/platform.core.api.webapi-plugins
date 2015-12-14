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

var validator = xwalk.utils.validator;
var converter = xwalk.utils.converter;
var type = xwalk.utils.type;
var native = new xwalk.utils.NativeManager(extension);

var PermissionType = {
  "NONE" : "NONE",
  "READ": "READ",
  "REMOVE" : "REMOVE",
  "READ_REMOVE": "READ_REMOVE"
};

function KeyManager() {

}

KeyManager.prototype.saveData = function() {
    var args = validator.validateArgs(arguments, [
      {
        name: 'aliasName',
        type: validator.Types.STRING
      },
      {
        name: 'rawData',
        type: validator.Types.STRING
      },
      {
        name: "password",
        type: validator.Types.STRING,
        optional: true,
        nullable: true
      },
      {
        name: 'successCallback',
        type: validator.Types.FUNCTION,
        optional: true,
        nullable: true
      },
      {
        name: 'errorCallback',
        type: validator.Types.FUNCTION,
        optional: true,
        nullable: true
      }
    ]);

    var result = native.call('KeyManager_saveData', {
      aliasName: _trim(args.aliasName),
      rawData: args.rawData,
      password: (args.password ? converter.toString(args.password) : null)
    }, function(msg) {
        if (native.isFailure(msg)) {
          native.callIfPossible(args.errorCallback, native.getErrorObject(msg));
        } else {
          native.callIfPossible(args.successCallback);
        }
      }
    );

    if (native.isFailure(result)) {
      throw native.getErrorObject(result);
    }
  };

KeyManager.prototype.removeData = function() {

  var args = validator.validateArgs(arguments, [
    {
      name : 'dataAlias',
      type : validator.Types.DICTIONARY
    }
  ]);

  var data_alias = _trim(args.dataAlias.name);
  if(args.dataAlias.hasOwnProperty('packageId')) {
    data_alias = args.dataAlias.packageId + ' ' + data_alias;
  }

  var ret = native.callSync('KeyManager_removeAlias', {
    aliasName: data_alias
  });
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
};

KeyManager.prototype.getData = function() {
  var args = validator.validateArgs(arguments, [
    {
      name: "dataAlias",
      type: validator.Types.DICTIONARY
    },
    {
      name: "password",
      type: validator.Types.STRING,
      optional: true,
      nullable: true
    }
  ]);

  var data_alias = _trim(args.dataAlias.name);
  if(args.dataAlias.hasOwnProperty('packageId')) {
    data_alias = args.dataAlias.packageId + ' ' + data_alias;
  }

  var ret = native.callSync('KeyManager_getData', {
    name: data_alias,
    password: args.password
  });
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
  var result = native.getResultObject(ret);
  return result.rawData;
};

KeyManager.prototype.getDataAliasList = function() {
  var ret = native.callSync('KeyManager_getDataAliasList', {});
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
  return native.getResultObject(ret);
};

KeyManager.prototype.setPermission = function() {
  var args = validator.validateArgs(arguments, [
    {
      name: "dataAlias",
      type: validator.Types.DICTIONARY
    },
    {
      name: "packageId",
      type: validator.Types.STRING
    },
    {
      name: 'permissionType',
      type: validator.Types.ENUM,
      values: Object.keys(PermissionType)
    },
    {
      name: 'successCallback',
      type: validator.Types.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'errorCallback',
      type: validator.Types.FUNCTION,
      optional: true,
      nullable: true
    }
  ]);

  var data_alias = _trim(args.dataAlias.name);
  if(args.dataAlias.hasOwnProperty('packageId')) {
    data_alias = args.dataAlias.packageId + ' ' + data_alias;
  }

  var result = native.call('KeyManager_setPermissions', {
    aliasName: data_alias,
    packageId: args.packageId,
    permissionType: args.permissionType
  }, function(msg) {
    if (native.isFailure(msg)) {
      native.callIfPossible(args.errorCallback, native.getErrorObject(msg));
    } else {
      native.callIfPossible(args.successCallback);
    }
  });

  if (native.isFailure(result)) {
    throw native.getErrorObject(result);
  }
};

function _trim(str){
  var val = str;
  if (!type.isString(str)) {
    val = converter.toString(str);
  }

  return val.replace(/\s/gi, '');
}

exports = new KeyManager();
