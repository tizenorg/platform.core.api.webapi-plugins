// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


var Person = function(data) {
  AV.isConstructorCall(this, Person);

  var _id = '';
  var _displayName = '';
  var _contactCount = 0;
  var _hasPhoneNumber = false;
  var _hasEmail = false;
  var _isFavorite = false;
  var _displayContactId = '';

  if (data.hasOwnProperty('id') && Type.isString(data.id)) {
    _id = data.id;
  }
  if (data.hasOwnProperty('displayName') && Type.isString(data.displayName)) {
    _displayName = data.displayName;
  }
  if (data.hasOwnProperty('contactCount') && Type.isNumber(data.contactCount)) {
    _contactCount = data.contactCount;
  }
  if (data.hasOwnProperty('hasPhoneNumber') && Type.isBoolean(data.hasPhoneNumber)) {
    _hasPhoneNumber = data.hasPhoneNumber;
  }
  if (data.hasOwnProperty('hasEmail') && Type.isBoolean(data.hasEmail)) {
    _hasEmail = data.hasEmail;
  }
  if (data.hasOwnProperty('displayContactId') && Type.isString(data.displayContactId)) {
    _displayContactId = data.displayContactId;
  }
  if (data.hasOwnProperty('isFavorite') && Type.isBoolean(data.isFavorite)) {
    _isFavorite = data.isFavorite;
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
    displayName: {
      get: function() {
        return _displayName;
      },
      set: function(v) {
        if (_editGuard.isEditEnabled()) {
          _displayName = Converter.toString(v, false);
        }
      },
      enumerable: true
    },
    contactCount: {
      get: function() {
        return _contactCount;
      },
      set: function(v) {
        if (_editGuard.isEditEnabled()) {
          _contactCount = Converter.toLong(v, false);
        }
      },
      enumerable: true
    },
    hasPhoneNumber: {
      get: function() {
        return _hasPhoneNumber;
      },
      set: function(v) {
        if (_editGuard.isEditEnabled()) {
          _hasPhoneNumber = Converter.toBoolean(v, false);
        }
      },
      enumerable: true
    },
    hasEmail: {
      get: function() {
        return _hasEmail;
      },
      set: function(v) {
        if (_editGuard.isEditEnabled()) {
          _hasEmail = Converter.toBoolean(v, false);
        }
      },
      enumerable: true
    },

    isFavorite: {
      get: function() {
        return _isFavorite;
      },
      set: function(v) {
        _isFavorite = Converter.toBoolean(v, false);
      },
      enumerable: true
    },
    photoURI: {
      value: data.hasOwnProperty('photoURI') ? data.photoURI : null,
      writable: true,
      enumerable: true
    },
    ringtoneURI: {
      value: data.hasOwnProperty('ringtoneURI') ? data.ringtoneURI : null,
      writable: true,
      enumerable: true
    },
    displayContactId: {
      get: function() {
        return _displayContactId;
      },
      set: function(v) {
        _displayContactId = Converter.toString(v, false);
      },
      enumerable: true
    }
  });
};

// Aggregates another person to this person.
Person.prototype.link = function() {
  var args = AV.validateArgs(arguments, [
    {
      name: 'personId',
      type: AV.Types.STRING,
      optional: false,
      nullable: false
    }
  ]);

  var result = native_.callSync('Person_link', {
    // TODO move to only sending the person id (in all functions)
    person: this,
    id: args.personId
  });
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  var _this = this;
  _editGuard.run(function() {
    _this.contactCount = _this.contactCount + 1;
  });
};

// Separates a contact from this person.
Person.prototype.unlink = function(contactId) {
  var args = AV.validateArgs(arguments, [
    {
      name: 'contactId',
      type: AV.Types.STRING,
      optional: false,
      nullable: false
    }
  ]);

  var result = native_.callSync('Person_unlink', {
    // TODO move to only sending the person id (in all functions)
    person: this,
    id: args.contactId
  });
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  var _this = this;
  return _editGuard.run(function() {
    _this.contactCount = _this.contactCount - 1;
    return new Person(native_.getResultObject(result));
  });
};
