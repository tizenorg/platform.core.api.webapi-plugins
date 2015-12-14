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
var type_ = xwalk.utils.type;
var types_ = validator_.Types;
var native_ = new xwalk.utils.NativeManager(extension);

var SystemSettingTypeValues = ['HOME_SCREEN', 'LOCK_SCREEN', 'INCOMING_CALL', 'NOTIFICATION_EMAIL'];

function SystemSettingManager() {
}

SystemSettingManager.prototype.getProperty = function() {
    var args = validator_.validateArgs(arguments, [
        { name: 'type', type: types_.ENUM, values: SystemSettingTypeValues },
        { name: 'successCallback', type: types_.FUNCTION },
        { name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true }
    ]);

    var callback = function(result) {
        if (native_.isFailure(result)) {
            native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
        }
        else {
            args.successCallback(result.result.value);
        }
    }

    var callArgs = {
        type: args.type
    };

    var result = native_.call('SystemSettingManager_getProperty', callArgs, callback);

    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }
};

SystemSettingManager.prototype.setProperty = function() {
    var args = validator_.validateArgs(arguments, [
        { name: 'type', type: types_.ENUM, values: SystemSettingTypeValues },
        { name: 'value', type: types_.STRING },
        { name: 'successCallback', type: types_.FUNCTION },
        { name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true }
    ]);

    var callback = function(result) {
        if (native_.isFailure(result)) {
            native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
        }
        else {
            args.successCallback();
        }
    }

    var callArgs = {
        type: args.type,
        value: args.value
    };

    var result = native_.call('SystemSettingManager_setProperty', callArgs, callback);

    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }
};

// Exports
exports = new SystemSettingManager();

