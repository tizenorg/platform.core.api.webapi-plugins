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

var DEFAULT_ADDRESSBOOK_ID = '0';
var UNIFIED_ADDRESSBOOK_ID = '-1';

var _contactListenerRegistered = false;
var _contactCallbackMap = {};

var _filterById = function(array, id) {
  var ret = [];
  for (var i = 0; i < array.length; ++i) {
    if (array[i].addressBookId === id) {
      ret.push(_promote(array[i], Contact));
    }
  }
  return ret;
};

var _contactChangeListener = function(result) {
  var unifiedId = UNIFIED_ADDRESSBOOK_ID;
  var watchId;
  var callback, i;

  // Unified address book case
  if (_contactCallbackMap.hasOwnProperty(unifiedId)) {
    for (watchId in _contactCallbackMap[unifiedId]) {
      if (_contactCallbackMap[unifiedId].hasOwnProperty(watchId)) {
        callback = _contactCallbackMap[unifiedId][watchId].successCallback;
        if (result.added.length) {
          native_.callIfPossible(callback.oncontactsadded, _promote(result.added, Contact));
        }
        if (result.updated.length) {
          native_.callIfPossible(callback.oncontactsupdated, _promote(result.updated, Contact));
        }
        if (result.removed.length) {
          var allRemoved = [];
          for (i = 0; i < result.removed.length; ++i) {
            allRemoved.push(result.removed[i].id);
          }
          native_.callIfPossible(callback.oncontactsremoved, result.allRemoved);
        }
      }
    }
  }

  for (var callbackAddressbookId in _contactCallbackMap) {
    if (callbackAddressbookId !== UNIFIED_ADDRESSBOOK_ID &&
        _contactCallbackMap.hasOwnProperty(callbackAddressbookId)) {

      var filteredAdded = [];
      var filteredUpdated = [];
      var filteredRemoved = [];

      if (result.added.length) {
        filteredAdded = _filterById(result.added, callbackAddressbookId);
      }
      if (result.updated.length) {
        filteredUpdated = _filterById(result.updated, callbackAddressbookId);
      }
      if (result.removed.length) {
        for (i = 0; i < result.removed.length; ++i) {
          if (result.removed[i].addressBookId === callbackAddressbookId) {
            filteredRemoved.push(result.removed[i].id);
          }
        }
      }

      for (watchId in _contactCallbackMap[callbackAddressbookId]) {
        if (_contactCallbackMap[callbackAddressbookId].hasOwnProperty(watchId)) {
          callback = _contactCallbackMap[callbackAddressbookId][watchId].successCallback;
          if (filteredAdded.length) {
            native_.callIfPossible(callback.oncontactsadded, filteredAdded);
          }
          if (filteredUpdated.length) {
            native_.callIfPossible(callback.oncontactsupdated, filteredUpdated);
          }
          if (filteredRemoved.length) {
            native_.callIfPossible(callback.oncontactsremoved, filteredRemoved);
          }
        }
      }
    }
  }
};

var AddressBook = function(accountId, name) {
  validator_.isConstructorCall(this, AddressBook);

  var id_ = null;
  var name_ = '';
  var readOnly_ = false;
  var accountId_ = null;

  if (type_.isNumber(accountId)) {
    accountId_ = accountId;
  }
  if (type_.isString(name)) {
    name_ = name;
  }

  Object.defineProperties(this, {
    id: {
      get: function() {
        return id_;
      },
      set: function(v) {
        if (_editGuard.isEditEnabled()) {
          id_ = converter_.toString(v, false);
        }
      },
      enumerable: true
    },
    accountId: {
      get: function() {
        return accountId_;
      },
      set: function(v) {
        if (_editGuard.isEditEnabled()) {
          accountId_ = converter_.toLong(v, true);
        }
      },
      enumerable: true
    },
    name: {
      get: function() {
        return name_;
      },
      set: function(v) {
        if (_editGuard.isEditEnabled()) {
          name_ = converter_.toString(v, false);
        }
      },
      enumerable: true
    },
    readOnly: {
      get: function() {
        return readOnly_;
      },
      set: function(v) {
        if (_editGuard.isEditEnabled()) {
          readOnly_ = converter_.toBoolean(v, false);
        }
      },
      enumerable: true
    }
  });
};

function _prepareContact(data) {
  return _editGuard.run(function() {
    var contact = new Contact(data);

    if (contact.name instanceof ContactName) {
      contact.name.displayName = '';
      if (type_.isString(contact.name.firstName)) {
        contact.name.displayName = contact.name.firstName;
        if (type_.isString(contact.name.lastName)) {
          contact.name.displayName += ' ' + contact.name.lastName;
        }
      } else if (type_.isArray(contact.name.nicknames) &&
          type_.isString(contact.name.nicknames[0])) {
        contact.name.displayName = contact.name.nicknames[0];
      } else if (type_.isString(contact.name.nicknames)) {
        contact.name.displayName = contact.name.nicknames;
      }
    }
    return contact;
  });
}

AddressBook.prototype.get = function() {
  var args = validator_.validateArgs(arguments, [{
    name: 'id',
    type: types_.STRING,
    optional: false,
    nullable: false
  }]);

  if (String(converter_.toLong(args.id)) !== args.id) {
    // TCT: AddressBook_get_id_invalid
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR);
  }

  var result = native_.callSync('AddressBook_get', {
    id: args.id
  });

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  return _prepareContact(native_.getResultObject(result));
};

AddressBook.prototype.add = function() {
  var args = validator_.validateArgs(arguments, [
    {
      name: 'contact',
      type: types_.PLATFORM_OBJECT,
      values: Contact,
      optional: false,
      nullable: false
    }
  ]);

  var result = native_.callSync('AddressBook_add', {
    addressBookId: this.id,
    contact: _toJsonObject(args.contact)
  });

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  var _updatedContact = native_.getResultObject(result);
  _editGuard.run(function() {
    for (var prop in _updatedContact) {
      if (args.contact.hasOwnProperty(prop)) {
        args.contact[prop] = _updatedContact[prop];
      }
    }

    if (args.contact.name instanceof ContactName) {
      args.contact.name.displayName = '';
      if (type_.isString(args.contact.name.firstName)) {
        args.contact.name.displayName = args.contact.name.firstName;
        if (type_.isString(args.contact.name.lastName)) {
          args.contact.name.displayName += ' ' + args.contact.name.lastName;
        }
      } else if (type_.isArray(args.contact.name.nicknames) &&
          type_.isString(args.contact.name.nicknames[0])) {
        args.contact.name.displayName = args.contact.name.nicknames[0];
      } else if (type_.isString(args.contact.name.nicknames)) {
        args.contact.name.displayName = args.contact.name.nicknames;
      }
    }
  });
};

AddressBook.prototype.addBatch = function() {
  var args = validator_.validateArgs(arguments, [
    {
      name: 'contacts',
      type: types_.ARRAY,
      value: Contact,
      optional: false,
      nullable: false
    },
    {
      name: 'successCallback',
      type: types_.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'errorCallback',
      type: types_.FUNCTION,
      optional: true,
      nullable: true
    }

  ]);

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }

    _editGuard.run(function() {
      var _result = native_.getResultObject(result);
      for (var i = 0; i < _result.length; ++i) {
        for (var prop in _result[i]) {
          if (args.contacts[i].hasOwnProperty(prop)) {
            args.contacts[i][prop] = _result[i][prop];
          }
        }
      }
    });

    native_.callIfPossible(args.successCallback, args.contacts);
  };

  var result = native_.call('AddressBook_addBatch', {
    addressBookId: this.id,
    batchArgs: _toJsonObject(args.contacts)
  }, callback);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

AddressBook.prototype.update = function() {
  var args = validator_.validateArgs(arguments, [
    {
      name: 'contact',
      type: types_.PLATFORM_OBJECT,
      values: Contact,
      optional: false,
      nullable: false
    }
  ]);

  if (args.contact.addressBookId !== this.id && UNIFIED_ADDRESSBOOK_ID !== this.id) {
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR);
  }

  var result = native_.callSync('AddressBook_update', {
    contact: _toJsonObject(args.contact)
  });

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  var _updatedContact = native_.getResultObject(result);
  _editGuard.run(function() {
    for (var prop in _updatedContact) {
      if (args.contact.hasOwnProperty(prop)) {
        args.contact[prop] = _updatedContact[prop];
      }
    }
  });
};

AddressBook.prototype.updateBatch = function() {
  var args = validator_.validateArgs(arguments, [
    {
      name: 'contacts',
      type: types_.ARRAY,
      values: Contact,
      optional: false,
      nullable: false
    },
    {
      name: 'successCallback',
      type: types_.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'errorCallback',
      type: types_.FUNCTION,
      optional: true,
      nullable: true
    }
  ]);

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }

    _editGuard.run(function() {
      var _result = native_.getResultObject(result);
      for (var i = 0; i < _result.length; ++i) {
        for (var prop in _result[i].result) {
          if (args.contacts[i].hasOwnProperty(prop)) {
            args.contacts[i][prop] = _result[i].result[prop];
          }
        }
      }
    });

    native_.callIfPossible(args.successCallback);
  };

  var thatId = this.id;
  args.contacts.forEach(function(c) {
    if (c.addressBookId !== thatId && UNIFIED_ADDRESSBOOK_ID !== thatId) {
      setTimeout(function() {
        native_.callIfPossible(args.errorCallback, new WebAPIException(
        WebAPIException.INVALID_VALUES_ERR,
        'Contact is not saved in database'));
      }, 0);

      return;
    }
  });

  var result = native_.call('AddressBook_updateBatch', {
    batchArgs: _toJsonObject(args.contacts)
  }, callback);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

AddressBook.prototype.remove = function() {
  var args = validator_.validateArgs(arguments, [{
    name: 'id',
    type: types_.STRING,
    optional: false,
    nullable: false
  }]);

  if (String(converter_.toLong(args.id)) !== args.id) {
    // TCT: AddressBook_remove_id_invalid
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR);
  }

  var result = native_.callSync('AddressBook_remove', {
    id: args.id
  });

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

AddressBook.prototype.removeBatch = function(ids, successCallback, errorCallback) {
  var args = validator_.validateArgs(arguments, [
    {
      name: 'ids',
      type: types_.ARRAY,
      values: types_.STRING,
      optional: false,
      nullable: false
    },
    {
      name: 'successCallback',
      type: types_.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'errorCallback',
      type: types_.FUNCTION,
      optional: true,
      nullable: true
    }
  ]);

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }

    native_.callIfPossible(args.successCallback);
  };

  var result = native_.call('AddressBook_removeBatch', {
    batchArgs: args.ids
  }, callback);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

AddressBook.prototype.find = function(successCallback, errorCallback, filter, sortMode) {
  var args = validator_.validateArgs(arguments, [
    {
      name: 'successCallback',
      type: types_.FUNCTION,
      optional: false,
      nullable: false
    },
    {
      name: 'errorCallback',
      type: types_.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'filter',
      type: types_.PLATFORM_OBJECT,
      values: [
        tizen.AttributeFilter,
        tizen.AttributeRangeFilter,
        tizen.CompositeFilter
      ],
      optional: true,
      nullable: true
    },
    {
      name: 'sortMode',
      type: types_.PLATFORM_OBJECT,
      values: tizen.SortMode,
      optional: true,
      nullable: true
    }
  ]);

  var self = this;
  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(errorCallback, native_.getErrorObject(result));
      return;
    }

    var _contacts = [];
    var _result = native_.getResultObject(result);
    _result.forEach(function(data) {
      var contact = _prepareContact(data);
      _contacts.push(contact);
    });

    native_.callIfPossible(successCallback, _contacts);
  };

  var result = native_.call('AddressBook_find', {
    addressBookId: this.id,
    filter: utils_.repackFilter(filter),
    sortMode: sortMode
  }, callback);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

AddressBook.prototype.addChangeListener = function() {
  var args = validator_.validateArgs(arguments, [{
    name: 'successCallback',
    type: types_.LISTENER,
    values: ['oncontactsadded', 'oncontactsupdated', 'oncontactsremoved'],
    optional: false,
    nullable: false
  }, {
    name: 'errorCallback',
    type: types_.FUNCTION,
    optional: true,
    nullable: true
  }]);

  // always on first registration checking privileges is done
  if (type_.isEmptyObject(_contactCallbackMap)) {
    var result = native_.callSync('AddressBook_startListening', {});

    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }
  }

  if (!_contactListenerRegistered) {
    native_.addListener('ContactChangeListener', _contactChangeListener);
    _contactListenerRegistered = true;
  }

  if (!_contactCallbackMap.hasOwnProperty(this.id)) {
    _contactCallbackMap[this.id] = {};
  }

  var currentWatchId = _getNextWatchId();

  _contactCallbackMap[this.id][currentWatchId] = {
    successCallback: args.successCallback,
    errorCallback: args.errorCallback
  };

  return currentWatchId;
};

var AddressBook_removeChangeListener = function(watchId) {
  if (type_.isEmptyObject(_contactCallbackMap)) {
    utils_.checkPrivilegeAccess(privilege_.CONTACT_READ);
  }
  var args = validator_.validateArgs(arguments, [
    {
      name: 'watchId',
      type: types_.LONG,
      optional: false,
      nullable: false
    }
  ]);

  if (args.watchId <= 0) {
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
        'Wrong watch id');
  }

  if (!_contactCallbackMap.hasOwnProperty(this.id) ||
      !_contactCallbackMap[this.id].hasOwnProperty(args.watchId)) {
    throw new WebAPIException(WebAPIException.NOT_FOUND_ERR,
        'watch id not found for this address book');
  }

  delete _contactCallbackMap[this.id][args.watchId];

  if (type_.isEmptyObject(_contactCallbackMap[this.id])) {
    delete _contactCallbackMap[this.id];
  }

  if (type_.isEmptyObject(_contactCallbackMap)) {
    native_.removeListener('ContactChangeListener', _contactChangeListener);
    _contactListenerRegistered = false;

    var result = native_.callSync('AddressBook_stopListening', {});

    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }
  }
};

AddressBook.prototype.removeChangeListener = function(watchId) {
  AddressBook_removeChangeListener.apply(this, arguments);
};

AddressBook.prototype.getGroup = function() {
  var args = validator_.validateArgs(arguments, [{
    name: 'groupId',
    type: types_.STRING,
    optional: false,
    nullable: false
  }]);

  if (String(converter_.toLong(args.groupId)) !== args.groupId) {
    // TCT: AddressBook_getGroup_groupId_invalid
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR);
  }

  var result = native_.callSync('AddressBook_getGroup', {
    addressBookId: this.id,
    id: args.groupId
  });
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  return _editGuard.run(function() {
    return new ContactGroup(native_.getResultObject(result));
  });
};

AddressBook.prototype.addGroup = function() {
  var args = validator_.validateArgs(arguments, [
    {
      name: 'group',
      type: types_.PLATFORM_OBJECT,
      values: ContactGroup,
      optional: false,
      nullable: false
    }
  ]);

  var result = native_.callSync('AddressBook_addGroup',
      {addressBookId: this.id, group: args.group});
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  _editGuard.run(function() {
    result = native_.getResultObject(result);
    args.group.id = result.id;
    args.group.addressBookId = result.addressBookId;
  });
};

AddressBook.prototype.updateGroup = function() {
  var args = validator_.validateArgs(arguments, [
    {
      name: 'group',
      type: types_.PLATFORM_OBJECT,
      values: ContactGroup,
      optional: false,
      nullable: false
    }
  ]);

  var result = native_.callSync('AddressBook_updateGroup',
      {addressBookId: this.id, group: args.group});
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

AddressBook.prototype.removeGroup = function() {
  var args = validator_.validateArgs(arguments, [{
    name: 'groupId',
    type: types_.STRING,
    optional: false,
    nullable: false
  }]);

  if (String(converter_.toLong(args.groupId)) !== args.groupId) {
    // TCT: AddressBook_removeGroup_groupId_invalid
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR);
  }

  var result = native_.callSync('AddressBook_removeGroup',
      {addressBookId: this.id, id: args.groupId});
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

AddressBook.prototype.getGroups = function() {
  var result = native_.callSync('AddressBook_getGroups', {addressBook: this});
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  result = native_.getResultObject(result);
  var _tab = [];
  _editGuard.run(function() {
    result.forEach(function(data) {
      _tab.push(new ContactGroup(data));
    });
  });
  return _tab;
};

// exports /////////////////////////////////////////////////////////////////
tizen.AddressBook = AddressBook;
