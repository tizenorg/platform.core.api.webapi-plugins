// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


var _personListenerRegistered = false;
var _personCallbackMap = {};
var _personChangeListener = function(result) {
  for (var key in _personCallbackMap) {
    if (_personCallbackMap.hasOwnProperty(key)) {
      if (result.added.length) {
        native_.callIfPossible(_personCallbackMap[key].onpersonsadded,
            _promote(result.added, Person));
      }
      if (result.updated.length) {
        native_.callIfPossible(_personCallbackMap[key].onpersonsupdated,
            _promote(result.updated, Person));
      }
      if (result.removed.length) {
        native_.callIfPossible(_personCallbackMap[key].onpersonsremoved,
            result.removed);
      }
    }
  }
};


var ContactManager = function() {};

// Gets the available address books
ContactManager.prototype.getAddressBooks = function() {
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
    }
  ]);

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
    } else {
      var books = native_.getResultObject(result);
      var tmp = [];

      books.forEach(function(data) {
        return _editGuard.run(function() {
          var addressBook = new AddressBook(result.accountId, result.name);
          addressBook.id = data.id;
          addressBook.readOnly = result.readOnly;

          tmp.push(addressBook);
        });
      });

      native_.callIfPossible(args.successCallback, tmp);
    }
  };

  native_.call('ContactManager_getAddressBooks', {}, callback);
};

// Gets the aggregation of all address books.
ContactManager.prototype.getUnifiedAddressBook = function() {
  // TODO check privileges
  //var result = native_.callSync('CheckReadPrivileges', {});
  //if (native_.isFailure(result)) {
  //  throw new tizen.WebAPIException(WebAPIException.SECURITY_ERR,
  //      'You do not have privileges for this operation');
  //}

  return _editGuard.run(function() {
    var addressBook = new AddressBook(0, 'Unified address book');
    addressBook.id = UNIFIED_ADDRESSBOOK_ID;
    addressBook.readOnly = false;

    return addressBook;
  });
};

// Gets the default address book.
ContactManager.prototype.getDefaultAddressBook = function() {
  //privileges are checked in getAddressBook function
  return this.getAddressBook(DEFAULT_ADDRESSBOOK_ID);
};

// Gets the address book with the specified identifier.
ContactManager.prototype.getAddressBook = function() {
  var args = AV.validateArgs(arguments, [{
    name: 'addressBookId',
    type: AV.Types.STRING,
    optional: false,
    nullable: false
  }]);

  if (String(Converter.toLong(args.addressBookId)) !== args.addressBookId) {
    // TCT: ContactManager_getAddressBook_addressBookId_invalid
    throw new tizen.WebAPIException(tizen.WebAPIException.NOT_FOUND_ERR);
  }

  var result = native_.callSync('ContactManager_getAddressBook', {
    addressBookId: args.addressBookId
  });

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  result = native_.getResultObject(result);

  return _editGuard.run(function() {
    var addressBook = new AddressBook(result.accountId, result.name);
    addressBook.id = args.addressBookId;
    addressBook.readOnly = result.readOnly;

    return addressBook;
  });
};

ContactManager.prototype.addAddressBook = function() {
  var args = AV.validateArgs(arguments, [{
    name: 'addressBook',
    type: AV.Types.PLATFORM_OBJECT,
    values: tizen.AddressBook,
    optional: false,
    nullable: false
  }]);

  var result = native_.callSync('ContactManager_addAddressBook', {
    addressBook: args.addressBook
  });

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  var data = native_.getResultObject(result);
  _editGuard.run(function() {
    for (var prop in data) {
      if (args.addressBook.hasOwnProperty(prop)) {
        args.addressBook[prop] = data[prop];
      }
    }
  });
};

ContactManager.prototype.removeAddressBook = function() {
  // TCT: ContactManager_removeAddressBook_misarg
  if (Type.isNullOrUndefined(arguments[0])) {
    throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR);
  }

  var args = AV.validateArgs(arguments, [{
    name: 'addressBookId',
    type: AV.Types.STRING,
    optional: false,
    nullable: false
  }]);

  if (args.addressBookId === UNIFIED_ADDRESSBOOK_ID) {
    throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR,
        'Unified address book can not be deleted');
  }

  if (args.addressBookId === DEFAULT_ADDRESSBOOK_ID) {
    throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR,
        'Default address book can not be deleted');
  }

  var result = native_.callSync('ContactManager_removeAddressBook', {
    addressBookId: args.addressBookId
  });

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

// Gets the person with the specified identifier.
ContactManager.prototype.get = function() {
  // validation
  var args = AV.validateArgs(arguments, [
    {
      name: 'personId',
      type: AV.Types.STRING,
      optional: false,
      nullable: false
    }
  ]);

  if (String(Converter.toLong(args.personId)) !== args.personId) {
    // TCT: ContactManager_get_personId_invalid
    throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR);
  }

  var result = native_.callSync('ContactManager_get', {
    personId: args.personId
  });
  _checkError(result);

  return _editGuard.run(function() {
    return new Person(native_.getResultObject(result));
  });
};

// Updates a person in the address book synchronously.
ContactManager.prototype.update = function() {
  // validation
  var args = AV.validateArgs(arguments, [{
    name: 'person',
    type: AV.Types.PLATFORM_OBJECT,
    values: Person,
    optional: false,
    nullable: false
  }]);
  var result = native_.callSync('ContactManager_update', { person: args.person });
  _checkError(result);

  result = native_.getResultObject(result);
  for (var prop in result) {
    if (args.person.hasOwnProperty(prop)) {
      args.person[prop] = result[prop];
    }
  }
};

// Updates several existing persons in the contact DB asynchronously.
ContactManager.prototype.updateBatch = function() {
  var args = AV.validateArgs(arguments, [
    {
      name: 'persons',
      type: AV.Types.ARRAY,
      values: Person,
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

  native_.call('ContactManager_updateBatch', {
    addressBook: {},
    batchArgs: _toJsonObject(args.persons)
  }, callback);
};

// Removes a person from the contact DB synchronously.
ContactManager.prototype.remove = function() {
  // validation
  var args = AV.validateArgs(arguments, [{
    name: 'personId',
    type: AV.Types.STRING,
    optional: false,
    nullable: false
  }]);

  if (String(Converter.toLong(args.personId)) !== args.personId) {
    // TCT: ContactManager_remove_personId_invalid
    throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR);
  }

  var result = native_.callSync('ContactManager_remove', {personId: args.personId});
  _checkError(result);
};

// Removes persons from contact DB asynchronously.
ContactManager.prototype.removeBatch = function() {
  var args = AV.validateArgs(arguments, [
    {
      name: 'personIds',
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

  native_.call('ContactManager_removeBatch', {
    addressBook: {},
    batchArgs: _toJsonObject(args.personIds)
  }, callback);
};

// Gets an array of all Person objects from the contact DB or the ones that match the
// optionally supplied filter.
ContactManager.prototype.find = function() {
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

  // TODO implement contact filtering/sorting.
  var data = {
    filter: C.repackFilter(args.filter),
    sortMode: args.sortMode
  };

  var self = this;

  var callback = function(result) {
    if (native_.isSuccess(result)) {
      var _result = native_.getResultObject(result);
      var retval = [];
      for (var i = 0; i < _result.length; ++i) {
        retval.push(self.get(String(_result[i])));
      }
      //TODO: Move sorting to native code
      retval = C.sort(retval, args.sortMode);
      args.successCallback(retval);
    } else {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
    }
  };

  native_.call('ContactManager_find', data, callback);
};

// Subscribes to receive notifications about persons' changes.
ContactManager.prototype.addChangeListener = function() {
  var args = AV.validateArgs(arguments, [
    {
      name: 'successCallback',
      type: AV.Types.LISTENER,
      values: ['onpersonsadded', 'onpersonsupdated', 'onpersonsremoved'],
      optional: false,
      nullable: false
    }
  ]);

  if (Type.isEmptyObject(_personCallbackMap)) {
    var result = native_.callSync('ContactManager_startListening', {});

    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }
  }

  if (!_personListenerRegistered) {
    native_.addListener('ContactPersonChangeListener', _personChangeListener);
    _personListenerRegistered = true;
  }

  var currentWatchId = _getNextWatchId();

  _personCallbackMap[currentWatchId] = args.successCallback;

  return currentWatchId;
};

// Unsubscribes a persons' changes watch operation.
ContactManager.prototype.removeChangeListener = function() {
  var args = AV.validateArgs(arguments, [
    {
      name: 'watchId',
      type: AV.Types.LONG,
      optional: false,
      nullable: false
    }
  ]);

  // This makes UTC_contact_removeChangeListenerPerson_N_001 pass.
  // watch id's start at 1
  if (args.watchId === 0) {
    throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR,
      'id is null or undefined');
  }

  if (args.watchId < 0) {
    throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR,
        'Negative watch id');
  }

  if (!_personCallbackMap.hasOwnProperty(args.watchId)) {
    throw new tizen.WebAPIException(tizen.WebAPIException.NOT_FOUND_ERR,
        'watch id not found');
  }

  delete _personCallbackMap[args.watchId];

  if (Type.isEmptyObject(_personCallbackMap)) {
    native_.removeListener('ContactPersonChangeListener', _personChangeListener);
    _personListenerRegistered = false;

    var result = native_.callSync('ContactManager_stopListening', {});

    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }
  }
};

// exports /////////////////////////////////////////////////////////////////
exports = new ContactManager();
