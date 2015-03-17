/* global xwalk, extension, tizen */

// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


var validator_ = xwalk.utils.validator;
var type_ = xwalk.utils.type;
var types_ = validator_.Types;
var native_ = new xwalk.utils.NativeManager(extension);

function throwException_(err) {
    throw new WebAPIException(err.code, err.name, err.message);
}

function callSync_(msg) {
    var ret = extension.internal.sendSyncMessage(JSON.stringify(msg));
    var obj = JSON.parse(ret);
    if (obj.error)
        throwException_(obj.error);
    return obj.result;
}

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

    native_.call('SystemSettingManager_getProperty', callArgs, callback);
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

    native_.call('SystemSettingManager_setProperty', callArgs, callback);
};

// Exports
exports = new SystemSettingManager();

