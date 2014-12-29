// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
  var unifiedId = -1;
  var watchId;
  var i;

  // Unified address book case
  if (_contactCallbackMap.hasOwnProperty(unifiedId)) {
    for (watchId in _contactCallbackMap[unifiedId]) {
      if (_contactCallbackMap[unifiedId].hasOwnProperty(watchId)) {
        var callback = _contactCallbackMap[unifiedId][watchId].successCallback;
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
    if (callbackAddressbookId !== -1 &&
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
          var callback = _contactCallbackMap[callbackAddressbookId][watchId].successCallback;
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


var AddressBook = function(id, name, readOnly) {
  AV.isConstructorCall(this, AddressBook);

  var _id = '';
  var _name = '';
  var _readOnly = false;

  if (Type.isString(id)) {
    _id = id;
  }
  if (Type.isString(name)) {
    _name = name;
  }
  if (Type.isBoolean(readOnly)) {
    _readOnly = readOnly;
  }

  Object.defineProperties(this, {
    id: {
      get: function() {
        return _id;
      },
      set: function(v) {
        if (_editGuard.isEditEnabled()) {
          _id = Converter.toString(v, false);
        }
      },
      enumerable: true
    },
    name: {
      get: function() {
        return _name;
      },
      set: function(v) {
        if (_editGuard.isEditEnabled()) {
          _name = Converter.toString(v, false);
        }
      },
      enumerable: true
    },
    readOnly: {
      get: function() {
        return _readOnly;
      },
      set: function(v) {
        if (_editGuard.isEditEnabled()) {
          _readOnly = Converter.toBoolean(v, false);
        }
      },
      enumerable: true
    }
  });
};

AddressBook.prototype.get = function() {
  var args = AV.validateArgs(arguments, [{
    name: 'id',
    type: AV.Types.STRING,
    optional: false,
    nullable: false
  }]);

  if (String(Converter.toLong(args.id)) !== args.id) {
    // TCT: AddressBook_get_id_invalid
    throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR);
  }

  var result = native_.callSync('AddressBook_get', {
    // TODO move to only sending the address book id (in all functions)
    addressBook: this,
    id: args.id
  });

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  return _editGuard.run(function() {
    var contact = new Contact(native_.getResultObject(result));

    if (contact.name instanceof ContactName) {
      contact.name.displayName = '';
      if (Type.isString(contact.name.firstName)) {
        contact.name.displayName = contact.name.firstName;
        if (Type.isString(contact.name.lastName)) {
          contact.name.displayName += ' ' + contact.name.lastName;
        }
      } else if (Type.isArray(contact.name.nicknames) &&
          Type.isString(contact.name.nicknames[0])) {
        contact.name.displayName = contact.name.nicknames[0];
      } else if (Type.isString(contact.name.nicknames)) {
        contact.name.displayName = contact.name.nicknames;
      }
    }



    return contact;
  });
};

AddressBook.prototype.add = function() {
  var args = AV.validateArgs(arguments, [
    {
      name: 'contact',
      type: AV.Types.PLATFORM_OBJECT,
      values: Contact,
      optional: false,
      nullable: false
    }
  ]);

  var result = native_.callSync('AddressBook_add', {
    // TODO move to only sending the address book id (in all functions)
    addressBook: this,
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
      if (Type.isString(args.contact.name.firstName)) {
        args.contact.name.displayName = args.contact.name.firstName;
        if (Type.isString(args.contact.name.lastName)) {
          args.contact.name.displayName += ' ' + args.contact.name.lastName;
        }
      } else if (Type.isArray(args.contact.name.nicknames) &&
          Type.isString(args.contact.name.nicknames[0])) {
        args.contact.name.displayName = args.contact.name.nicknames[0];
      } else if (Type.isString(args.contact.name.nicknames)) {
        args.contact.name.displayName = args.contact.name.nicknames;
      }
    }
  });
};

AddressBook.prototype.addBatch = function() {
  var args = AV.validateArgs(arguments, [
    {
      name: 'contacts',
      type: AV.Types.ARRAY,
      value: Contact,
      optional: false,
      nullable: false
    },
    {
      name: 'successCallback',
      type: AV.Types.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'errorCallback',
      type: AV.Types.FUNCTION,
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

  native_.call('AddressBook_addBatch', {
    addressBookId: this.id,
    batchArgs: _toJsonObject(args.contacts)
  }, callback);
};

AddressBook.prototype.update = function() {
  var args = AV.validateArgs(arguments, [
    {
      name: 'contact',
      type: AV.Types.PLATFORM_OBJECT,
      values: Contact,
      optional: false,
      nullable: false
    }
  ]);

  var result = native_.callSync('AddressBook_update', {
    addressBook: this,
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
  var args = AV.validateArgs(arguments, [
    {
      name: 'contacts',
      type: AV.Types.ARRAY,
      values: Contact,
      optional: false,
      nullable: false
    },
    {
      name: 'successCallback',
      type: AV.Types.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'errorCallback',
      type: AV.Types.FUNCTION,
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

  native_.call('AddressBook_updateBatch', {
    addressBook: this,
    batchArgs: _toJsonObject(args.contacts)
  }, callback);
};

AddressBook.prototype.remove = function() {
  var args = AV.validateArgs(arguments, [{
    name: 'id',
    type: AV.Types.STRING,
    optional: false,
    nullable: false
  }]);

  if (String(Converter.toLong(args.id)) !== args.id) {
    // TCT: AddressBook_remove_id_invalid
    throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR);
  }

  var result = native_.callSync('AddressBook_remove', {
    addressBook: this,
    id: args.id
  });

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

AddressBook.prototype.removeBatch = function(ids, successCallback, errorCallback) {
  var args = AV.validateArgs(arguments, [
    {
      name: 'ids',
      type: AV.Types.ARRAY,
      values: AV.Types.STRING,
      optional: false,
      nullable: false
    },
    {
      name: 'successCallback',
      type: AV.Types.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'errorCallback',
      type: AV.Types.FUNCTION,
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

  native_.call('AddressBook_removeBatch', {
    addressBook: this,
    batchArgs: args.ids
  }, callback);
};

AddressBook.prototype.find = function(successCallback, errorCallback, filter, sortMode) {
  var args = AV.validateArgs(arguments, [
    {
      name: 'successCallback',
      type: AV.Types.FUNCTION,
      optional: false,
      nullable: false
    },
    {
      name: 'errorCallback',
      type: AV.Types.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'filter',
      type: AV.Types.PLATFORM_OBJECT,
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
      type: AV.Types.PLATFORM_OBJECT,
      values: tizen.SortMode,
      optional: true,
      nullable: true
    }
  ]);

  var self = this;
  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(errorCallback, native_.getErrorObject(result));
    }

    var _contacts = [];
    var _result = native_.getResultObject(result);
    _result.forEach(function(data) {
      try {
        _contacts.push(self.get(String(data)));
      } catch (e) {}
    });

    //TODO: Move filtering to native code
    try {
      _contacts = C.filter(_contacts, args.filter);
    } catch (e) {
      native_.callIfPossible(errorCallback, e);
      return;
    }

    //TODO: Move sorting to native code
    _contacts = C.sort(_contacts, args.sortMode);

    native_.callIfPossible(successCallback, _contacts);
  };

  native_.call('AddressBook_find', {
    addressBook: this,
    filter: filter,
    sortMode: sortMode
  }, callback);
};

AddressBook.prototype.addChangeListener = function() {
  var args = AV.validateArgs(arguments, [
    {
      name: 'successCallback',
      type: AV.Types.LISTENER,
      values: ['oncontactsadded', 'oncontactsupdated', 'oncontactsremoved'],
      optional: false,
      nullable: false
    },
    {
      name: 'errorCallback',
      type: AV.Types.FUNCTION,
      optional: true,
      nullable: true
    }
  ]);

  if (Type.isEmptyObject(_contactCallbackMap)) {
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

AddressBook.prototype.removeChangeListener = function(watchId) {
  var args = AV.validateArgs(arguments, [
    {
      name: 'watchId',
      type: AV.Types.LONG,
      optional: false,
      nullable: false
    }
  ]);

  if (args.watchId === 0) {
    throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR,
        'id is null or undefined');
  }

  if (args.watchId < 0) {
    throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR,
        'Negative watch id');
  }

  if (!_contactCallbackMap.hasOwnProperty(this.id) ||
      !_contactCallbackMap[this.id].hasOwnProperty(args.watchId)) {
    throw new tizen.WebAPIException(tizen.WebAPIException.NOT_FOUND_ERR,
        'watch id not found for this address book');
  }

  delete _contactCallbackMap[this.id][args.watchId];

  if (Type.isEmptyObject(_contactCallbackMap[this.id])) {
    delete _contactCallbackMap[this.id];
  }

  if (Type.isEmptyObject(_contactCallbackMap)) {
    native_.removeListener('ContactChangeListener', _contactChangeListener);
    _contactListenerRegistered = false;

    var result = native_.callSync('AddressBook_stopListening', {});

    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }
  }
};

AddressBook.prototype.getGroup = function() {
  var args = AV.validateArgs(arguments, [{
    name: 'groupId',
    type: AV.Types.STRING,
    optional: false,
    nullable: false
  }]);

  if (String(Converter.toLong(args.groupId)) !== args.groupId) {
    // TCT: AddressBook_getGroup_groupId_invalid
    throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR);
  }

  var result = native_.callSync('AddressBook_getGroup', {
    addressBook: this,
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
  var args = AV.validateArgs(arguments, [
    {
      name: 'group',
      type: AV.Types.PLATFORM_OBJECT,
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
  var args = AV.validateArgs(arguments, [
    {
      name: 'group',
      type: AV.Types.PLATFORM_OBJECT,
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
  var args = AV.validateArgs(arguments, [{
    name: 'groupId',
    type: AV.Types.STRING,
    optional: false,
    nullable: false
  }]);

  if (String(Converter.toLong(args.groupId)) !== args.groupId) {
    // TCT: AddressBook_removeGroup_groupId_invalid
    throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR);
  }

  var result = native_.callSync('AddressBook_removeGroup',
      {addressBook: this, id: args.groupId});
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
