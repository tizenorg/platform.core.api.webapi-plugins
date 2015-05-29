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
 
// An enumerator that indicates the types for the relationships.
var ContactRelationshipType = {
  ASSISTANT: 'ASSISTANT',
  BROTHER: 'BROTHER',
  CHILD: 'CHILD',
  DOMESTIC_PARTNER: 'DOMESTIC_PARTNER',
  FATHER: 'FATHER',
  FRIEND: 'FRIEND',
  MANAGER: 'MANAGER',
  MOTHER: 'MOTHER',
  PARENT: 'PARENT',
  PARTNER: 'PARTNER',
  REFERRED_BY: 'REFERRED_BY',
  RELATIVE: 'RELATIVE',
  SISTER: 'SISTER',
  SPOUSE: 'SPOUSE',
  OTHER: 'OTHER',
  CUSTOM: 'CUSTOM'
};

// An enumerator that indicates the types for instant messenger.
var ContactInstantMessengerType = {
  GOOGLE: 'GOOGLE',
  WLM: 'WLM',
  YAHOO: 'YAHOO',
  FACEBOOK: 'FACEBOOK',
  ICQ: 'ICQ',
  AIM: 'AIM',
  QQ: 'QQ',
  JABBER: 'JABBER',
  SKYPE: 'SKYPE',
  IRC: 'IRC',
  OTHER: 'OTHER',
  CUSTOM: 'CUSTOM'
};

// class ContactRef ////////////////////////////////////////////////////////

var ContactRef = function(data) {
  validator_.isConstructorCall(this, ContactRef);
  var _contactId = '';
  var _addressBookId = '';
  Object.defineProperties(this, {
    addressBookId: {
      get: function() {
        return _addressBookId;
      },
      set: function(v) {
        _addressBookId = converter_.toString(v, false);
      },
      enumerable: true
    },
    contactId: {
      get: function() {
        return _contactId;
      },
      set: function(v) {
        _contactId = converter_.toString(v, false);
      },
      enumerable: true
    }
  });

  if (type_.isObject(data)) {
    this.addressBookId = data.addressBookId;
    this.contactId = data.contactId;
  } else {
    try {
      var args = validator_.validateArgs(arguments, [
        {
          name: 'addressBookId',
          type: types_.STRING,
          optional: false,
          nullable: false
        },
        {
          name: 'contactId',
          type: types_.STRING,
          optional: false,
          nullable: false
        }
      ]);
      _addressBookId = args.addressBookId;
      _contactId = args.contactId;
    } catch (x) {
      // Constructors shouldn't throw
    }
  }
};

// class ContactGroup //////////////////////////////////////////////////

var ContactGroup = function(name, ringtone, photo) {
  validator_.isConstructorCall(this, ContactGroup);

  var _id = null;
  var _address = null;
  var _readOnly = false;
  var _name = '';
  var _ringtoneURI = null;
  var _photoURI = null;

  if (name && type_.isString(name)) {
    _name = name.length ? name : '';
  }

  if (ringtone && type_.isString(ringtone)) {
    _ringtoneURI = ringtone.length ? ringtone : null;
  }

  if (photo && type_.isString(photo)) {
    _photoURI = photo.length ? photo : null;
  }

  Object.defineProperties(this, {
    id: {
      get: function() {
        return _id;
      },
      set: function(v) {
        if (_editGuard.isEditEnabled()) {
          _id = converter_.toString(v, true);
        }
      },
      enumerable: true
    },
    addressBookId: {
      get: function() {
        return _address;
      },
      set: function(v) {
        if (_editGuard.isEditEnabled()) {
          _address = converter_.toString(v, true);
        }
      },
      enumerable: true
    },
    name: {
      get: function() {
        return _name;
      },
      set: function(v) {
        _name = converter_.toString(v, false);
      },
      enumerable: true
    },
    ringtoneURI: {
      get: function() {
        return _ringtoneURI;
      },
      set: function(v) {
        _ringtoneURI = converter_.toString(v, true);
      },
      enumerable: true
    },
    photoURI: {
      get: function() {
        return _photoURI;
      },
      set: function(v) {
        _photoURI = converter_.toString(v, true);
      },
      enumerable: true
    },
    readOnly: {
      get: function() {
        return _readOnly;
      },
      set: function(v) {
        if (_editGuard.isEditEnabled()) {
          _readOnly = converter_.toBoolean(v, false);
        }
      },
      enumerable: true
    }
  });

  if (_editGuard.isEditEnabled()) {
    var data = arguments[0];
    if (type_.isObject(data)) {
      for (var prop in data) {
        if (this.hasOwnProperty(prop)) {
          this[prop] = data[prop];
        }
      }
    }
  }
};

// class ContactEmailAddress ///////////////////////////////////////////

var ContactEmailAddress = function(address, types, isDefault) {
  validator_.isConstructorCall(this, ContactEmailAddress);

  var _email = '';
  var _label = null;
  var _isDefault = false;
  var _types = ['WORK'];

  if (type_.isString(address) && address.indexOf('@') > 0 &&
      address.indexOf('@') !== (address.length - 1)) {
    _email = address;
  }

  if (type_.isBoolean(isDefault)) {
    _isDefault = isDefault;
  }

  if (type_.isArray(types)) {
    _types = [];
    for (var i = 0; i < types.length; ++i) {
      if (type_.isString(types[i])) {
        _types.push(types[i]);
      }
    }
  } else if (type_.isString(types)) {
    _types = [];
    _types.push(types);
  }

  Object.defineProperties(this, {
    email: {
      get: function() {
        return _email;
      },
      set: function(v) {
        if (type_.isString(v) && v.indexOf('@') > 0 &&
            v.indexOf('@') !== (v.length - 1)) {
          _email = v;
        }
      },
      enumerable: true
    },
    isDefault: {
      get: function() {
        return _isDefault;
      },
      set: function(v) {
        _isDefault = converter_.toBoolean(v, false);
      },
      enumerable: true
    },
    types: {
      get: function() {
        return _types;
      },
      set: function(v) {
        if (type_.isArray(v)) {
          _types = [];
          for (var i = 0; i < v.length; ++i) {
            if (type_.isString(v[i])) {
              _types.push(v[i]);
            }
          }
        } else if (type_.isString(v)) {
          _types = [];
          _types.push(v);
        }
      },
      enumerable: true
    },
    label: {
      get: function() {
        return _label;
      },
      set: function(v) {
        _label = converter_.toString(v, true);
      },
      enumerable: true
    }
  });

  if (_editGuard.isEditEnabled()) {
    for (var prop in arguments[0]) {
      if (this.hasOwnProperty(prop)) {
        this[prop] = arguments[0][prop];
      }
    }
  }
};

// class ContactPhoneNumber ////////////////////////////////////////////

var ContactPhoneNumber = function(number, type, isDefault) {
  validator_.isConstructorCall(this, ContactPhoneNumber);

  var _isDefault = false;
  var _number = '';
  var _types = ['VOICE'];

  if (type_.isString(number)) {
    _number = number;
  }

  if (type_.isArray(type)) {
    _types = [];
    for (var i = 0; i < type.length; ++i) {
      _types.push(converter_.toString(type[i], false));
    }
  } else if (type_.isString(type)) {
    _types = [];
    _types.push(type, false);
  }

  if (type_.isBoolean(isDefault)) {
    _isDefault = isDefault;
  }

  Object.defineProperties(this, {
    number: {
      get: function() {
        return _number;
      },
      set: function(v) {
        _number = converter_.toString(v, false);
      },
      enumerable: true
    },
    isDefault: {
      get: function() {
        return _isDefault;
      },
      set: function(v) {
        _isDefault = converter_.toBoolean(v, false);
      },
      enumerable: true
    },
    types: {
      get: function() {
        return _types;
      },
      set: function(v) {
        if (type_.isArray(v)) {
          _types = [];
          for (var i = 0; i < v.length; ++i) {
            _types.push(converter_.toString(v[i], false));
          }
        } else if (type_.isString(v)) {
          _types = [];
          _types.push(v, false);
        }
      },
      enumerable: true
    },
    label: {
      value: null,
      writable: true,
      enumerable: true
    }
  });

  if (_editGuard.isEditEnabled()) {
    for (var prop in arguments[0]) {
      if (this.hasOwnProperty(prop)) {
        this[prop] = arguments[0][prop];
      }
    }
  }
};

// class ContactAddress ////////////////////////////////////////////////

var ContactAddress = function(data) {
  validator_.isConstructorCall(this, ContactAddress);

  var _isDefault = false;
  var _types = ['HOME'];

  Object.defineProperties(this, {
    country: {
      value: null,
      writable: true,
      enumerable: true
    },
    region: {
      value: null,
      writable: true,
      enumerable: true
    },
    city: {
      value: null,
      writable: true,
      enumerable: true
    },
    streetAddress: {
      value: null,
      writable: true,
      enumerable: true
    },
    additionalInformation: {
      value: null,
      writable: true,
      enumerable: true
    },
    postalCode: {
      value: null,
      writable: true,
      enumerable: true
    },
    isDefault: {
      get: function() {
        return _isDefault;
      },
      set: function(v) {
        _isDefault = converter_.toBoolean(v, false);
      },
      enumerable: true
    },
    types: {
      get: function() {
        return _types;
      },
      set: function(v) {
        if (type_.isString(v)) {
          _types = [];
          _types.push(v);
        } else if (type_.isArray(v)) {
          _types = [];
          for (var i = 0; i < v.length; ++i) {
            if (type_.isString(v[i])) {
              _types.push(v[i]);
            }
          }
        }
      },
      enumerable: true
    },
    label: {
      value: null,
      writable: true,
      enumerable: true
    }
  });

  if (type_.isObject(data)) {
    for (var prop in data) {
      if (this.hasOwnProperty(prop)) {
        this[prop] = data[prop];
      }
    }
  }
};

// class ContactAnniversary ////////////////////////////////////////////////

var ContactAnniversary = function(anniversary_date, anniversary_label) {
  validator_.isConstructorCall(this, ContactAnniversary);

  var _anniversary_date = new Date();
  var _anniversary_label = null;

  Object.defineProperties(this, {
    date: {
      get: function() {
        return _anniversary_date;
      },
      set: function(v) {
        _anniversary_date = v instanceof Date ? v : new Date();
      },
      enumerable: true
    },
    label: {
      get: function() {
        return _anniversary_label;
      },
      set: function(v) {
        _anniversary_label = converter_.toString(v, true);
      },
      enumerable: true
    }
  });

  if (_editGuard.isEditEnabled()) {
    _anniversary_date = _fromJsonDate(arguments[0].date);
    _anniversary_label = arguments[0].label;
  } else {
    if (type_.isDate(anniversary_date)) {
      _anniversary_date = anniversary_date;
    }
    if (type_.isString(anniversary_label)) {
      _anniversary_label = anniversary_label;
    }
  }
};

// class ContactWebSite ////////////////////////////////////////////////////

var ContactWebSite = function(contact_url, contact_type) {
  validator_.isConstructorCall(this, ContactWebSite);

  var _url = '';
  var _type = 'HOMEPAGE';

  if (type_.isString(contact_url)) {
    _url = contact_url;
  }
  if (type_.isString(contact_type)) {
    _type = contact_type;
  }

  Object.defineProperties(this, {
    url: {
      get: function() {
        return _url;
      },
      set: function(v) {
        _url = converter_.toString(v, false);
      },
      enumerable: true
    },
    type: {
      get: function() {
        return _type;
      },
      set: function(v) {
        _type = converter_.toString(v, false);
      },
      enumerable: true
    }
  });
};

// class ContactOrganization ///////////////////////////////////////////////

var ContactOrganization = function(data) {
  validator_.isConstructorCall(this, ContactOrganization);
  Object.defineProperties(this, {
    name: {
      value: null,
      writable: true,
      enumerable: true
    },
    department: {
      value: null,
      writable: true,
      enumerable: true
    },
    title: {
      value: null,
      writable: true,
      enumerable: true
    },
    role: {
      value: null,
      writable: true,
      enumerable: true
    },
    logoURI: {
      value: null,
      writable: true,
      enumerable: true
    }
  });

  if (type_.isObject(data)) {
    for (var prop in data) {
      if (this.hasOwnProperty(prop)) {
        this[prop] = data[prop];
      }
    }
  }
};

// class ContactName ///////////////////////////////////////////////////////

var ContactName = function(data) {
  validator_.isConstructorCall(this, ContactName);

  var _displayName = null;
  var _nicknames = [];

  Object.defineProperties(this, {
    prefix: {
      value: null,
      writable: true,
      enumerable: true
    },
    suffix: {
      value: null,
      writable: true,
      enumerable: true
    },
    firstName: {
      value: null,
      writable: true,
      enumerable: true
    },
    middleName: {
      value: null,
      writable: true,
      enumerable: true
    },
    lastName: {
      value: null,
      writable: true,
      enumerable: true
    },
    nicknames: {
      get: function() {
        return _nicknames;
      },
      set: function(nicknames) {
        if (type_.isArray(nicknames)) {
          _nicknames = nicknames;
        }
      },
      enumerable: true
    },
    phoneticFirstName: {
      value: null,
      writable: true,
      enumerable: true
    },
    phoneticMiddleName: {
      value: null,
      writable: true,
      enumerable: true
    },
    phoneticLastName: {
      value: null,
      writable: true,
      enumerable: true
    },
    displayName: {
      get: function() {
        return _displayName;
      },
      set: function(v) {
        if (_editGuard.isEditEnabled()) {
          _displayName = converter_.toString(v, true);
        }
      },
      enumerable: true
    }
  });

  if (type_.isObject(data)) {
    for (var prop in data) {
      if (this.hasOwnProperty(prop)) {
        this[prop] = data[prop];
      }
    }
  }
};

var ContactRelationship = function(relativeName, type) {
  validator_.isConstructorCall(this, ContactRelationship);

  var _relativeName = converter_.toString(relativeName, false);
  var _type = type ? converter_.toEnum(type, Object.keys(ContactRelationshipType), false)
            : ContactRelationshipType.OTHER;
  var _label = null;

  Object.defineProperties(this, {
    relativeName: {
      get: function () {
        return _relativeName;
      },
      set: function (v) {
        _relativeName = converter_.toString(v, false);
      },
      enumerable: true
    },
    type: {
      get: function () {
        return _type;
      },
      set: function (v) {
        _type = v ? converter_.toEnum(v, Object.keys(ContactRelationshipType), false)
                  : _type;
      },
      enumerable: true
    },
    label: {
      get: function () {
        return _label;
      },
      set: function (v) {
        _label = converter_.toString(v, true);
      },
      enumerable: true
    }
  });
};

var ContactInstantMessenger = function(imAddress, type) {
  validator_.isConstructorCall(this, ContactInstantMessenger);

  var _imAddress = '';
  var _type = 'OTHER';

  Object.defineProperties(this, {
    imAddress: {
      get: function() {
        return _imAddress;
      },
      set: function(v) {
        if (type_.isNullOrUndefined(v)) {
          return;
        }
        _imAddress = converter_.toString(v, false);
      },
      enumerable: true
    },
    type: {
      get: function() {
        return _type;
      },
      set: function(v) {
        if (type_.isNullOrUndefined(v)) {
          return;
        }
        _type = converter_.toEnum(v, Object.keys(ContactInstantMessengerType), false);
      },
      enumerable: true
    },
    label: {
      value: null,
      writable: true,
      enumerable: true
    }
  });

  this.imAddress = imAddress;
  this.type = type;
};

// exports /////////////////////////////////////////////////////////////////
tizen.ContactRef = ContactRef;
tizen.ContactName = ContactName;
tizen.ContactOrganization = ContactOrganization;
tizen.ContactWebSite = ContactWebSite;
tizen.ContactAnniversary = ContactAnniversary;
tizen.ContactAddress = ContactAddress;
tizen.ContactPhoneNumber = ContactPhoneNumber;
tizen.ContactEmailAddress = ContactEmailAddress;
tizen.ContactGroup = ContactGroup;
tizen.ContactRelationship = ContactRelationship;
tizen.ContactInstantMessenger = ContactInstantMessenger;
