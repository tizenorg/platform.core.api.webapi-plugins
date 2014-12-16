// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// import ContactClass /////////////////////////////////////////////////////
var _contact = require('./tizen.contact.Contact');
var Contact = _contact.Contact;
_contact = undefined;

// Import auxiliary classes for Contact
var _dataStructures = require('./tizen.contact.ContactDataStructures');
var ContactGroup = _dataStructures.group;
var ContactName = _dataStructures.name;
_dataStructures = undefined;

// import flag /////////////////////////////////////////////////////////////
var _struct = require('./tizen.contact.ContactDataStructures');
var _editGuard = _struct.editGuard;
var _toJsonObject = _struct.toJsonObject;
var _getNextWatchId = _struct.getNextWatchId;
var _promote = _struct.promote;
_struct = undefined;

var _registered = false;
var _listenerId = 'ContactChangeListener';
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
  result = JSON.parse(result);

  var unifiedId = -1;
  var watchId;
  var i;

  // Unified address book case
  if (_contactCallbackMap.hasOwnProperty(unifiedId)) {
    for (watchId in _contactCallbackMap[unifiedId]) {
      if (_contactCallbackMap[unifiedId].hasOwnProperty(watchId)) {
        if (result.added.length) {
          Common.callIfPossible(_contactCallbackMap[unifiedId][watchId]
                                                  .successCallback.oncontactsadded,
              _promote(result.added, Contact));
        }
        if (result.updated.length) {
          Common.callIfPossible(_contactCallbackMap[unifiedId][watchId]
                                                  .successCallback.oncontactsupdated,
              _promote(result.updated, Contact));
        }
        if (result.removed.length) {
          var allRemoved = [];
          for (i = 0; i < result.removed.length; ++i) {
            allRemoved.push(result.removed[i].id);
          }
          Common.callIfPossible(_contactCallbackMap[unifiedId][watchId]
                                                  .successCallback.oncontactsremoved,
              result.allRemoved);
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
          if (filteredAdded.length) {
            Common.callIfPossible(
                _contactCallbackMap[callbackAddressbookId][watchId]
                                            .successCallback.oncontactsadded,
                filteredAdded);
          }
          if (filteredUpdated.length) {
            Common.callIfPossible(
                _contactCallbackMap[callbackAddressbookId][watchId]
                                            .successCallback.oncontactsupdated,
                filteredUpdated);
          }
          if (filteredRemoved.length) {
            Common.callIfPossible(
                _contactCallbackMap[callbackAddressbookId][watchId]
                                            .successCallback.oncontactsremoved,
                filteredRemoved);
          }
        }
      }
    }
  }
};

// class AddressBook ///////////////////////////////////////////////////////

var AddressBook = function(id, name, readOnly) {
  AV.validateConstructorCall(this, AddressBook);

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
  var args = AV.validateMethod(arguments, [
    {
      name: 'id',
      type: AV.Types.STRING,
      optional: false,
      nullable: false
    }
  ]);

  var result = _callSync('AddressBook_get', {
    // TODO move to only sending the address book id (in all functions)
    addressBook: this,
    id: args.id
  });

  if (Common.isFailure(result)) {
    throw Common.getErrorObject(result);
  }

  return _editGuard.run(function() {
    var contact = new Contact(Common.getResultObject(result));

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
  var args = AV.validateMethod(arguments, [
    {
      name: 'contact',
      type: AV.Types.PLATFORM_OBJECT,
      values: Contact,
      optional: false,
      nullable: false
    }
  ]);

  var result = _callSync('AddressBook_add', {
    // TODO move to only sending the address book id (in all functions)
    addressBook: this,
    contact: _toJsonObject(args.contact)
  });

  if (Common.isFailure(result)) {
    throw Common.getErrorObject(result);
  }

  var _updatedContact = Common.getResultObject(result);
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
  var args = AV.validateMethod(arguments, [
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
    if (Common.isFailure(result)) {
      Common.callIfPossible(args.errorCallback, Common.getErrorObject(result));
      return;
    }

    _editGuard.run(function() {
      var _result = Common.getResultObject(result);
      for (var i = 0; i < _result.length; ++i) {
        for (var prop in _result[i]) {
          if (args.contacts[i].hasOwnProperty(prop)) {
            args.contacts[i][prop] = _result[i][prop];
          }
        }
      }
    });

    Common.callIfPossible(args.successCallback, args.contacts);
  };

  var result = _call('AddressBook_addBatch',
      {addressBookId: this.id, batchArgs: _toJsonObject(args.contacts) },
      callback);

  if (Common.isFailure(result)) {
    throw Common.getErrorObject(result);
  }
};

AddressBook.prototype.update = function() {
  var args = AV.validateMethod(arguments, [
    {
      name: 'contact',
      type: AV.Types.PLATFORM_OBJECT,
      values: Contact,
      optional: false,
      nullable: false
    }
  ]);

  var result = _callSync('AddressBook_update', {
    addressBook: this,
    contact: _toJsonObject(args.contact)
  });

  if (Common.isFailure(result)) {
    throw Common.getErrorObject(result);
  }

  var _updatedContact = Common.getResultObject(result);
  _editGuard.run(function() {
    for (var prop in _updatedContact) {
      if (args.contact.hasOwnProperty(prop)) {
        args.contact[prop] = _updatedContact[prop];
      }
    }
  });
};

AddressBook.prototype.updateBatch = function() {
  var args = AV.validateMethod(arguments, [
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
    if (Common.isFailure(result)) {
      Common.callIfPossible(args.errorCallback, Common.getErrorObject(result));
      return;
    }

    _editGuard.run(function() {
      var _result = Common.getResultObject(result);
      for (var i = 0; i < _result.length; ++i) {
        for (var prop in _result[i].result) {
          if (args.contacts[i].hasOwnProperty(prop)) {
            args.contacts[i][prop] = _result[i].result[prop];
          }
        }
      }
    });

    Common.callIfPossible(args.successCallback);
  };

  var result = _call('AddressBook_updateBatch',
      {addressBook: this, batchArgs: _toJsonObject(args.contacts) },
      callback);

  if (Common.isFailure(result)) {
    throw Common.getErrorObject(result);
  }
};

AddressBook.prototype.remove = function() {
  var args = AV.validateMethod(arguments, [
    {
      name: 'id',
      type: AV.Types.STRING,
      optional: false,
      nullable: false
    }
  ]);

  var result = _callSync('AddressBook_remove', {
    addressBook: this,
    id: args.id
  });

  if (Common.isFailure(result)) {
    throw Common.getErrorObject(result);
  }
};

AddressBook.prototype.removeBatch = function(ids, successCallback, errorCallback) {
  var args = AV.validateMethod(arguments, [
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
    if (Common.isFailure(result)) {
      Common.callIfPossible(args.errorCallback, Common.getErrorObject(result));
      return;
    }

    Common.callIfPossible(args.successCallback);
  };

  var result = _call('AddressBook_removeBatch',
      {addressBook: this, batchArgs: args.ids },
      callback);

  if (Common.isFailure(result)) {
    throw Common.getErrorObject(result);
  }
};

AddressBook.prototype.find = function(successCallback, errorCallback, filter, sortMode) {
  var args = AV.validateMethod(arguments, [
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
      values: [tizen.AttributeFilter,
        tizen.AttributeRangeFilter,
        tizen.CompositeFilter],
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
    if (Common.isFailure(result)) {
      Common.callIfPossible(errorCallback, Common.getErrorObject(result));
    }

    var _contacts = [];
    var _result = Common.getResultObject(result);
    _result.forEach(function(data) {
      try {
        _contacts.push(self.get(String(data)));
      } catch (e) {}
    });

    //TODO: Move filtering to native code
    try {
      _contacts = Common.filter(_contacts, args.filter);
    } catch (e) {
      Common.callIfPossible(errorCallback, e);
      return;
    }

    //TODO: Move sorting to native code
    _contacts = Common.sort(_contacts, args.sortMode);

    Common.callIfPossible(successCallback, _contacts);
  };

  var result = _call('AddressBook_find',
      {addressBook: this, filter: filter, sortMode: sortMode},
      callback);

  if (Common.isFailure(result)) {
    throw Common.getErrorObject(result);
  }
};

AddressBook.prototype.addChangeListener = function() {
  var args = AV.validateMethod(arguments, [
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
    var result = _callSync('AddressBook_startListening', {});

    if (Common.isFailure(result)) {
      throw Common.getErrorObject(result);
    }
  }

  if (!_registered) {
    native.addListener(_listenerId, _contactChangeListener);
    _registered = true;
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
  var args = AV.validateMethod(arguments, [
    {
      name: 'watchId',
      type: AV.Types.LONG,
      optional: false,
      nullable: false
    }
  ]);

  if (args.watchId === 0) {
    Common.throwInvalidValues('id is null or undefined');
  }

  if (args.watchId < 0) {
    Common.throwInvalidValues('Negative watch id');
  }

  if (!_contactCallbackMap.hasOwnProperty(this.id) ||
      !_contactCallbackMap[this.id].hasOwnProperty(args.watchId)) {
    Common.throwNotFound('watch id not found for this address book');
  }

  delete _contactCallbackMap[this.id][args.watchId];

  if (Type.isEmptyObject(_contactCallbackMap[this.id])) {
    delete _contactCallbackMap[this.id];
  }

  if (Type.isEmptyObject(_contactCallbackMap)) {
    native.removeListener(_listenerId, _contactChangeListener);
    _registered = false;

    var result = _callSync('AddressBook_stopListening', {});

    if (Common.isFailure(result)) {
      throw Common.getErrorObject(result);
    }
  }
};

AddressBook.prototype.getGroup = function() {
  var args = AV.validateMethod(arguments, [
    {
      name: 'groupId',
      type: AV.Types.STRING,
      optional: false,
      nullable: false
    }
  ]);

  var result = _callSync('AddressBook_getGroup',
      {addressBook: this, id: args.groupId});
  if (Common.isFailure(result)) {
    throw Common.getErrorObject(result);
  }

  return _editGuard.run(function() {
    return new ContactGroup(Common.getResultObject(result));
  });
};

AddressBook.prototype.addGroup = function() {
  var args = AV.validateMethod(arguments, [
    {
      name: 'group',
      type: AV.Types.PLATFORM_OBJECT,
      values: ContactGroup,
      optional: false,
      nullable: false
    }
  ]);

  var result = _callSync('AddressBook_addGroup',
      {addressBookId: this.id, group: args.group});
  if (Common.isFailure(result)) {
    throw Common.getErrorObject(result);
  }

  _editGuard.run(function() {
    result = Common.getResultObject(result);
    args.group.id = result.id;
    args.group.addressBookId = result.addressBookId;
  });
};

AddressBook.prototype.updateGroup = function() {
  var args = AV.validateMethod(arguments, [
    {
      name: 'group',
      type: AV.Types.PLATFORM_OBJECT,
      values: ContactGroup,
      optional: false,
      nullable: false
    }
  ]);

  var result = _callSync('AddressBook_updateGroup',
      {addressBookId: this.id, group: args.group});
  if (Common.isFailure(result)) {
    throw Common.getErrorObject(result);
  }
};

AddressBook.prototype.removeGroup = function() {
  var args = AV.validateMethod(arguments, [
    {
      name: 'groupId',
      type: AV.Types.STRING,
      optional: false,
      nullable: false
    }
  ]);

  var result = _callSync('AddressBook_removeGroup',
      {addressBook: this, id: args.groupId});
  if (Common.isFailure(result)) {
    throw Common.getErrorObject(result);
  }
};

AddressBook.prototype.getGroups = function() {
  var result = _callSync('AddressBook_getGroups', {addressBook: this});
  if (Common.isFailure(result)) {
    throw Common.getErrorObject(result);
  }
  result = Common.getResultObject(result);
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
