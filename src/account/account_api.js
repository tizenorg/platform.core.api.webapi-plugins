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
var types_ = validator_.Types;
var T_ = xwalk.utils.type;
var native_ = new xwalk.utils.NativeManager(extension);


function InternalValues_(data) {
    if (!(this instanceof InternalValues_)) {
        return new InternalValues_(data);
    }
    for(var key in data) {
        if (data.hasOwnProperty(key)) {
            this[key] = data[key];
        }
    }
}


function AccountProvider(data) {
    var internal_ = [];
    if (data) {
        internal_ = data.capabilities;
    }
    Object.freeze(internal_);

    Object.defineProperties(this, {
        applicationId:              { enumerable: true, writable: false, value: data.applicationId },
        displayName:                { enumerable: true, writable: false, value: data.displayName },
        iconUri:                    { enumerable: true, writable: false, value: data.iconUri },
        smallIconUri:               { enumerable: true, writable: false, value: data.smallIconUri },
        capabilities:               { enumerable: true,
                                      set: function() {},
                                      get: function() { return internal_; }
                                    },
        isMultipleAccountSupported: { enumerable: true, writable: false, value: data.isMultipleAccountSupported },
    });
}


function Account() {
    validator_.isConstructorCall(this, tizen.Account);
    var args = validator_.validateArgs(arguments, [
        { name: 'provider', type: types_.PLATFORM_OBJECT, values: AccountProvider },
        { name: 'accountInitDict', type: types_.DICTIONARY, optional: true, nullable: true }
    ]);

    var _internal = { id: null };

    Object.defineProperties(this, {
        id:         { enumerable: true,
                      set: function (value) { if (value instanceof InternalValues_) _internal.id = value.id; },
                      get: function () { return _internal.id; }
        },
        userName:   { enumerable: true, writable: true,
                      value: (args.accountInitDict ? args.accountInitDict.userName : null) },
        iconUri:    { enumerable: true, writable: true,
                      value: (args.accountInitDict ? args.accountInitDict.iconUri : null) },
        provider:   { enumerable: true, writable: false, value: args.provider }
    });
}


Account.prototype.setExtendedData = function() {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.ACCOUNT_WRITE);

    var args = validator_.validateArgs(arguments, [
        { name: 'key', type: types_.STRING },
        { name: 'value', type: types_.STRING }
    ]);

    var result = native_.callSync('Account_setExtendedData',
                                  {
                                      accountId: this.id,
                                      key: args.key,
                                      value: args.value
                                  }
    );

    if (native_.isFailure(result)) {
        throw native_.getErrorObject(result);
    }
};


Account.prototype.getExtendedData = function() {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.ACCOUNT_READ);

    if (T_.isFunction(arguments[0]) || arguments.length > 1) {
        var args = validator_.validateArgs(arguments, [
            {
                name : 'successCallback',
                type : types_.FUNCTION
            },
            {
                name : 'errorCallback',
                type : types_.FUNCTION,
                optional : true,
                nullable : true
            }
        ]);

        // TODO handling exceptions

        native_.call('Account_getExtendedData', { accountId: this.id },
            function(result) {
                if (native_.isFailure(result)) {
                    if(!T_.isNullOrUndefined(args.errorCallback)) {
                        args.errorCallback(native_.getErrorObject(result));
                    }
                } else {
                    var data = native_.getResultObject(result);
                    for (var i = 0; i < data.length; ++i) {
                        Object.freeze(data[i]);
                    }
                    args.successCallback(native_.getResultObject(result));
                }
            }
        );
    } else {
        var args = validator_.validateArgs(arguments, [
            { name: 'key', type: types_.STRING }
        ]);

        var result = native_.callSync('Account_getExtendedDataSync',
                                      {
                                           accountId: this.id,
                                           key: args.key
                                      }
        );
        if (native_.isFailure(result)) {
            throw native_.getErrorObject(result);
        }
        return native_.getResultObject(result);
    }
};


function AccountFromResult(result) {
    var provider = new AccountProvider(result.provider);
    var account = new Account(provider, result.accountInitDict);
    account.id = new InternalValues_({ id: result.id });
    return account;
}


function AccountManager() {}


AccountManager.prototype.add = function() {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.ACCOUNT_WRITE);

    var args = validator_.validateArgs(arguments, [
        { name: 'account', type: types_.PLATFORM_OBJECT, values: Account }
    ]);

    var result = native_.callSync('AccountManager_add',
                                  {
                                      userName: args.account.userName,
                                      iconUri: args.account.iconUri,
                                      applicationId: args.account.provider.applicationId
                                  }
    );

    if (native_.isFailure(result)) {
        throw native_.getErrorObject(result);
    } else {
        args.account.id = new InternalValues_({ id: native_.getResultObject(result) });
    }
}


AccountManager.prototype.remove = function() {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.ACCOUNT_WRITE);

    var args = validator_.validateArgs(arguments, [
        { name: 'accountId', type: types_.UNSIGNED_LONG}
    ]);

    var result = native_.callSync('AccountManager_remove', { accountId: args.accountId });

    if (native_.isFailure(result)) {
        throw native_.getErrorObject(result);
    }
}


AccountManager.prototype.update = function() {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.ACCOUNT_WRITE);

    var args = validator_.validateArgs(arguments, [
        { name: 'account', type: types_.PLATFORM_OBJECT, values: Account }
    ]);

    var result = native_.callSync('AccountManager_update',
                                  {
                                      accountId:  args.account.id,
                                      userName:   args.account.userName,
                                      iconUri:    args.account.iconUri
                                  }
    );

    if (native_.isFailure(result)) {
        throw native_.getErrorObject(result);
    }
}


AccountManager.prototype.getAccount = function() {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.ACCOUNT_READ);

    var args = validator_.validateArgs(arguments, [
        { name: 'accountId', type: types_.UNSIGNED_LONG }
    ]);

    var result = native_.callSync(
                        'AccountManager_getAccount',
                        { accountId: args.accountId }
    );

    if (native_.isFailure(result)) {
        throw native_.getErrorObject(result);
    }

    var account_result = native_.getResultObject(result);

    if (!T_.isNull(account_result)) {
        return AccountFromResult(account_result);
    } else {
        return null;
    }
}


AccountManager.prototype.getAccounts = function() {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.ACCOUNT_READ);

    var args = validator_.validateArgs(arguments, [
        { name: 'successCallback', type: types_.FUNCTION, optional: false, nullable: false },
        { name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true },
        { name: 'applicationId', type: types_.STRING, optional: true, nullable: true }
    ]);

    // TODO handling exceptions

    native_.call('AccountManager_getAccounts',
        {
            applicationId: args.applicationId
        },
        function(result) {
            if (native_.isFailure(result)) {
                if(!T_.isNullOrUndefined(args.errorCallback)) {
                    args.errorCallback(native_.getErrorObject(result));
                }
            } else {
                var accounts_result = native_.getResultObject(result);
                var accounts_table = [];
                for (var i = 0; i < accounts_result.length; i++) {
                    accounts_table[i] = AccountFromResult(accounts_result[i]);
                }
                args.successCallback(accounts_table);
            }
        }
    );
}


AccountManager.prototype.getProvider = function() {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.ACCOUNT_READ);

    var args = validator_.validateArgs(arguments, [
        { name: 'applicationId', type: types_.STRING }
    ]);

    var result = native_.callSync(
                        'AccountManager_getProvider',
                        { applicationId: args.applicationId }
    );

    if (native_.isFailure(result)) {
        throw native_.getErrorObject(result);
    }

    var provider_result = native_.getResultObject(result);

    if (!T_.isNull(provider_result)) {
        return new AccountProvider(provider_result);
    } else {
        return null;
    }
};


AccountManager.prototype.getProviders = function() {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.ACCOUNT_READ);

    var args = validator_.validateArgs(arguments, [
        { name: 'successCallback', type: types_.FUNCTION, optional: false, nullable: false },
        { name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true },
        { name: 'capability', type: types_.STRING, optional: true, nullable: true }
    ]);

    // TODO handling exceptions

    native_.call( 'AccountManager_getProviders',
        {
            capability: args.capability
        },
        function(result) {
            if (native_.isFailure(result)) {
                if(!T_.isNullOrUndefined(args.errorCallback)) {
                    args.errorCallback(native_.getErrorObject(result));
                }
            } else {
                var providers_result = native_.getResultObject(result);
                var providers_table = [];
                for (var i = 0; i < providers_result.length; i++) {
                    providers_table[i] = new AccountProvider(providers_result[i]);
                }
                args.successCallback(providers_table);
            }
        }
    );
}


var ACCOUNT_LISTENER = 'ACCOUNT_CHANGED';


function AccountListeners() {
    var that = this;
    this.appCallback = function (event) {
        if (!T_.isEmptyObject(that.instances)) {
            var param;
            switch (event.action) {
                case 'onadded':
                    param = AccountFromResult(native_.getResultObject(event));
                    break;

                case 'onremoved':
                    param = native_.getResultObject(event);
                    break;

                case 'onupdated':
                    param = AccountFromResult(native_.getResultObject(event));
                    break;

                default:
                    console.log('Unknown event: ' + event.action);
                    break;
            }

            var callback;
            for (var key in that.instances) {
                if (that.instances.hasOwnProperty(key)) {
                    callback = that.instances[key];
                    if (T_.isFunction(callback[event.action])) {
                        callback[event.action](param);
                    }
                }
            }
        }
    };
}


AccountListeners.prototype.instances = {};
AccountListeners.prototype.nextID = 0;


AccountListeners.prototype.addListener = function(callback) {
    var id = ++this.nextID;

    if (T_.isEmptyObject(this.instances)) {
        var result = native_.callSync('AccountManager_addAccountListener');
        if (native_.isFailure(result)) {
            throw native_.getErrorObject(result);
        }

        native_.addListener(ACCOUNT_LISTENER, this.appCallback);
    }
    this.instances[id] = callback;

    return id;
};


AccountListeners.prototype.removeListener = function(accountListenerId) {
    delete this.instances[accountListenerId];
    if (T_.isEmptyObject(this.instances)) {
        native_.removeListener(ACCOUNT_LISTENER, this.appCallback);

        var result = native_.callSync('AccountManager_removeAccountListener');

        if (native_.isFailure(result)) {
            throw native_.getErrorObject(result);
        }
    }
};


var _accountListeners = new AccountListeners();


AccountManager.prototype.addAccountListener = function() {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.ACCOUNT_READ);

    var args = validator_.validateArgs(arguments, [
        { name: 'callback', type: types_.LISTENER, values: ['onadded', 'onremoved', 'onupdated'] }
    ]);

    return _accountListeners.addListener(args.callback);
}


AccountManager.prototype.removeAccountListener = function() {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.ACCOUNT_READ);

    var args = validator_.validateArgs(arguments, [
        { name: 'accountListenerId', type: types_.UNSIGNED_LONG }
    ]);

    _accountListeners.removeListener(args.accountListenerId);
}

tizen.Account = Account;

exports = new AccountManager();
