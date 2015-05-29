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

var Person = function(data) {
  validator_.isConstructorCall(this, Person);

  var _id = '';
  var _displayName = '';
  var _contactCount = 0;
  var _hasPhoneNumber = false;
  var _hasEmail = false;
  var _isFavorite = false;
  var _displayContactId = '';

  if (data.hasOwnProperty('id') && type_.isString(data.id)) {
    _id = data.id;
  }
  if (data.hasOwnProperty('displayName') && type_.isString(data.displayName)) {
    _displayName = data.displayName;
  }
  if (data.hasOwnProperty('contactCount') && type_.isNumber(data.contactCount)) {
    _contactCount = data.contactCount;
  }
  if (data.hasOwnProperty('hasPhoneNumber') && type_.isBoolean(data.hasPhoneNumber)) {
    _hasPhoneNumber = data.hasPhoneNumber;
  }
  if (data.hasOwnProperty('hasEmail') && type_.isBoolean(data.hasEmail)) {
    _hasEmail = data.hasEmail;
  }
  if (data.hasOwnProperty('displayContactId') && type_.isString(data.displayContactId)) {
    _displayContactId = data.displayContactId;
  }
  if (data.hasOwnProperty('isFavorite') && type_.isBoolean(data.isFavorite)) {
    _isFavorite = data.isFavorite;
  }

  Object.defineProperties(this, {
    id: {
      get: function() {
        return _id;
      },
      set: function(v) {
        if (_editGuard.isEditEnabled()) {
          _id = converter_.toString(v, false);
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
          _displayName = converter_.toString(v, false);
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
          _contactCount = converter_.toLong(v, false);
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
          _hasPhoneNumber = converter_.toBoolean(v, false);
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
          _hasEmail = converter_.toBoolean(v, false);
        }
      },
      enumerable: true
    },

    isFavorite: {
      get: function() {
        return _isFavorite;
      },
      set: function(v) {
        _isFavorite = converter_.toBoolean(v, false);
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
        _displayContactId = converter_.toString(v, false);
      },
      enumerable: true
    }
  });
};

// Aggregates another person to this person.
Person.prototype.link = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.CONTACT_WRITE);

  var args = validator_.validateArgs(arguments, [{
    name: 'personId',
    type: types_.STRING,
    optional: false,
    nullable: false
  }]);

  if (String(converter_.toLong(args.personId)) !== args.personId) {
    // TCT: Person_link_personId_invalid
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR);
  }

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
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.CONTACT_WRITE);

  var args = validator_.validateArgs(arguments, [{
    name: 'contactId',
    type: types_.STRING,
    optional: false,
    nullable: false
  }]);

  if (String(converter_.toLong(args.contactId)) !== args.contactId) {
    // TCT: Person_unlink_contactId_invalid
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR);
  }

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
