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

var Contact = function(data) {
  validator_.isConstructorCall(this, Contact);

  var _forceEditMode = false;
  if (type_.isString(data)) {
    var result = native_.callSync('ContactManager_importFromVCard', {
      'contact': data
    });
    _checkError(result);

    data = native_.getResultObject(result);
    // These need to be forced to null as a contact created from a vcard is not added
    // to any address book
    data.id = null;
    data.personId = null;
    data.addressBookId = null;
    data.lastUpdate = null;

    // Force edit mode so that anonymous objects can be promoted to their correct types.
    _forceEditMode = true;
  } else if (type_.isObject(data) || type_.isFunction(data)) {
    // It's a dictionary
  } else {
    // null or invalid types.
    data = {};
  }

  var _id = null;
  var _personId = null;
  var _addressBookId = null;
  var _lastUpdate = null;
  var _isFavorite = false;
  var _name = null;
  var _addresses = [];
  var _photoURI = null;
  var _phoneNumbers = [];
  var _emails = [];
  var _messengers = [];
  var _relationships = [];
  var _birthday = null;
  var _anniversaries = [];
  var _organizations = [];
  var _notes = [];
  var _urls = [];
  var _ringtoneURI = null;
  var _messageAlertURI = null;
  var _vibrationURI = null;
  var _groupIds = [];

  var _sanitizeArray = function(arr, type, previousValue) {
    if (!type_.isArray(arr)) {
      return previousValue;
    }
    for (var i = 0; i < arr.length; ++i) {
      if (type_.isString(type)) {
        if (!type_.isString(arr[i])) {
          return previousValue;
        }
      } else if (_editGuard.isEditEnabled()) {
        arr[i] = new type(arr[i]);
      } else if (!(arr[i] instanceof type)) {
        return previousValue;
      }
    }
    return arr;
  };

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
    personId: {
      get: function() {
        return _personId;
      },
      set: function(v) {
        if (_editGuard.isEditEnabled()) {
          _personId = converter_.toString(v, false);
        }
      },
      enumerable: true
    },
    addressBookId: {
      get: function() {
        return _addressBookId;
      },
      set: function(v) {
        if (_editGuard.isEditEnabled()) {
          _addressBookId = converter_.toString(v, false);
        }
      },
      enumerable: true
    },
    lastUpdated: {
      get: function() {
        return _lastUpdate;
      },
      set: function(v) {
        if (_editGuard.isEditEnabled()) {
          if (v instanceof Date || v === null) {
            _lastUpdate = v;
          } else if (type_.isString(v)) {
            _lastUpdate = new Date(v);
          } else {
            _lastUpdate = _fromJsonDate(v);
          }
        }
      },
      enumerable: true
    },
    isFavorite: {
      get: function() {
        return _isFavorite;
      },
      set: function(v) {
        if (_editGuard.isEditEnabled()) {
          _isFavorite = converter_.toBoolean(v, false);
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
          _name = new ContactName(v);
        } else {
          _name = (v instanceof ContactName || v === null) ? v : _name;
        }
      },
      enumerable: true
    },
    addresses: {
      get: function() {
        return _addresses;
      },
      set: function(v) {
        _addresses = _sanitizeArray(v, ContactAddress, _addresses);
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
    phoneNumbers: {
      get: function() {
        return _phoneNumbers;
      },
      set: function(v) {
        _phoneNumbers = _sanitizeArray(v, ContactPhoneNumber, _phoneNumbers);
      },
      enumerable: true
    },
    emails: {
      get: function() {
        return _emails;
      },
      set: function(v) {
        _emails = _sanitizeArray(v, ContactEmailAddress, _emails);
      },
      enumerable: true
    },
    messengers: {
      get: function() {
        return _messengers;
      },
      set: function(v) {
        _messengers = _sanitizeArray(v, ContactInstantMessenger, _messengers);
      },
      enumerable: true
    },
    relationships: {
      get: function() {
        return _relationships;
      },
      set: function(v) {
        _relationships = _sanitizeArray(v, ContactRelationship, _relationships);
      },
      enumerable: true
    },
    birthday: {
      get: function() {
        return _birthday;
      },
      set: function(v) {
        if (v instanceof Date || v === null) {
          _birthday = v;
        } else if (type_.isString(v)) {
          _birthday = new Date(v);
        } else if (_editGuard.isEditEnabled()) {
          _birthday = _fromJsonDate(v);
        }
      },
      enumerable: true
    },
    anniversaries: {
      get: function() {
        return _anniversaries;
      },
      set: function(v) {
        _anniversaries = _sanitizeArray(v, ContactAnniversary, _anniversaries);
      },
      enumerable: true
    },
    organizations: {
      get: function() {
        return _organizations;
      },
      set: function(v) {
        _organizations = _sanitizeArray(v, ContactOrganization, _organizations);
      },
      enumerable: true
    },
    notes: {
      get: function() {
        return _notes;
      },
      set: function(v) {
        _notes = _sanitizeArray(v, '', _notes);
      },
      enumerable: true
    },
    urls: {
      get: function() {
        return _urls;
      },
      set: function(v) {
        _urls = _sanitizeArray(v, ContactWebSite, _urls);
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
    messageAlertURI: {
      get: function() {
        return _messageAlertURI;
      },
      set: function(v) {
        _messageAlertURI = converter_.toString(v, true);
      },
      enumerable: true
    },
    vibrationURI: {
      get: function() {
        return _vibrationURI;
      },
      set: function(v) {
        _vibrationURI = converter_.toString(v, true);
      },
      enumerable: true
    },
    groupIds: {
      get: function() {
        return _groupIds;
      },
      set: function(v) {
        _groupIds = _sanitizeArray(v, '', _groupIds);
      },
      enumerable: true
    }
  });

  var _this = this;
  var _setProperties = function() {
    for (var p in _this) {
      if (data.hasOwnProperty(p)) {
        _this[p] = data[p];
      }
    }
  };

  if (_forceEditMode) {
    _editGuard.run(_setProperties);
  } else {
    _setProperties();
  }

};

// Auxiliary functions /////////////////////////////////////////////////////

// Convert address from Contact object to string
var _contactAddressToString = function(obj) {
  var str = '';
  if (obj.addresses.length === 0) {
    return '';
  }
  // Sorry for so many if statements, but the IDE makes me do it
  for (var it in obj.addresses) {
    if (!it instanceof ContactAddress) {
      continue;
    }
    str += 'ADR;';
    for (var addType in it.types) {
      if (type_.isString(addType)) {
        str += addType + ',';
      }
    }
    if (str.charAt(str.length - 1) === ',') {
      str[str.length - 1] = ':';
    }
    str += ';'; // because of we don't keep Post office addres
    // which is part of vCard 3.0 standard
    str += it.additionalInformation + ';';
    str += it.streetAddress + ';';
    str += it.city + ';';
    str += it.region + ';';
    str += it.postalCode + ';';
    str += it.country + ';';
    str += '\n';
  }
  return str;
};

// Convert email address from Contact object to string
var _contactEmailToString = function(obj) {
  if (!type_.isArray(obj.emails) || obj.emails.length === 0) {
    console.log('Empty email list');
    return '';
  }
  var str = '';
  for (var mail in obj.emails) {
    if (!mail instanceof ContactEmailAddress || !type_.isArray(mail.types) ||
      mail.types.length === 0) {
      console.log('Incorrect email type');
      continue;
    }
    str += 'EMAIL;';
    // set types
    for (var type in mail.types) {
      if (type_.isString(type)) {
        str += type + ',';
      }
    }
    if (str.charAt(str.length - 1) === ',') {
      str[str.length - 1] = ':';
    }
    str += '=' + converter_.toString(mail.email) + '\n';
  }
  return str;
};

// Convert organizations info from Contact object to string
var _contactOrganizationToString = function(obj) {
  if (obj.organizations.length === 0 ||
    !obj.organizations[0] instanceof ContactOrganization) {
    return '';
  }
  var str = '';
  for (var org in obj.organizations) {
    if (!org instanceof ContactOrganization) {
      continue;
    }
    str += 'ORG:';
    str += org.name + ';' + org.department + ';' + org.title + '\n';
  }
  return str;
};

// Convert organizations roles from Contact object to string
var _contactRoleToString = function(obj) {
  if (obj.organizations.length === 0 ||
    !obj.organizations[0] instanceof ContactOrganization) {
    return '';
  }
  var str = '';
  for (var org in obj.organizations) {
    if (!org instanceof ContactOrganization) {
      continue;
    }
    str += 'ROLE:';
    str += org.name + ';' + org.role + '\n';
  }
  return str;
};

// Convert phone numbers from Contact object to string
var _contactPhoneNumbersToString = function(obj) {
  if (obj.phoneNumbers.length === 0 || !obj.phoneNumbers[0] instanceof ContactPhoneNumber) {
    return '';
  }
  var str = '';
  for (var phone in obj.phoneNumbers) {
    if (!phone instanceof ContactPhoneNumber) {
      continue;
    }
    str += 'TEL';
    for (var type in phone.types) {
      if (type_.isString(type)) {
        str += ';' + type;
      }
    }
    str += ':';
    str += phone.number + '\n';
  }
  return str;
};

// Convert urls from Contact object to string
var _contactURLToString = function(obj) {
  if (obj.urls.length === 0 || !obj.urls[0] instanceof ContactWebSite) {
    return '';
  }
  var str = '';
  for (var url in obj.urls) {
    if (url instanceof ContactWebSite) {
      str += 'URL:' + url.url + '\n';
    }
  }
  return str;
};

// Convert anniversaries to string
var _contactAnniversaryToString = function(obj) {
  if (obj.anniversaries.length === 0 || !obj.anniversaries[0] instanceof ContactAnniversary) {
    return '';
  }
  var str = '';
  for (var ann in obj.anniversaries) {
    if (ann instanceof ContactAnniversary) {
      str += 'X-ANNIVERSARY;' + ann.label + ':' + ann.date + ';' + '\n';
    }
  }
  return str;
};

// Convert relationships to string
var _contactRelationshipsToString = function(obj) {
  if (obj.relationships.length === 0 ||
    !obj.relationships[0] instanceof ContactRelationship) {
    return '';
  }
  var str = '';
  for (var rel in obj.relationships) {
    if (rel instanceof ContactRelationship) {
      str += 'X-RELATIONSHIP;' + rel.relativeName + ':' + rel.type +
      ':' + rel.label + ';\n';
    }
  }
  return str;
};

// Convert messengers to string
var _contactInstantMessengeToString = function(obj) {
  if (obj.messengers.length === 0 || !obj.messengers[0] instanceof ContactInstantMessenger) {
    return '';
  }
  var str = '';
  for (var messenger in obj.messengers) {
    if (messenger instanceof ContactInstantMessenger) {
      str += 'X-MESSANGER;' + messenger.imAddress + ':' + messenger.type +
      ':' + messenger.label + ';\n';
    }
  }
  return str;
};

// Auxiliary function, allows to parse JSON to Contact
var _JSONToContactType = function(type, obj) {
  var contact = new type();

  for (var prop in obj) {
    if (contact.hasOwnProperty(prop)) {
      if (contact[prop] instanceof Date && type_.isNumber(obj[prop])) {
        contact[prop] = new Date(1000 * obj[prop]);
      } else {
        contact[prop] = obj[prop];
      }
    }
  }

  return contact;
};

// Converts the Contact item to a string format.
Contact.prototype.convertToString = function(format) {
  format = format || TypeEnum[0];

  if (!type_.isString(format)) {
    throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR, 'Invalid format');
  }

  if (TypeEnum.indexOf(format) < 0) {
    throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR, 'Invalid format');
  }

  if (this.id === '') {
    throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR,
      'Contact ID is empty.');
  }

  var str = 'BEGIN:VCARD\nVERSION:3.0\n';

  // set contact name
  str += 'N:' + this.name.lastName + ';' + this.name.firstName + ';' +
  this.name.middleName + ';' + this.name.prefix + ';' + this.name.suffix + '\n';
  str += 'FN' + this.name.displayName + '\n';

  // set phonetic names
  str += 'X-PHONETIC-FIRST-NAME' + this.name.phoneticFirstName + '\n' +
  'X-PHONETIC-LAST-NAME' + this.name.phoneticLastName + '\n';

  // set contact address
  str += _contactAddressToString(this);

  // set Birthday
  if (this.birthday) {
    str += 'BDAY:' + this.birthday.getYear() + '-' + this.birthday.getMonth() +
    '-' + this.birthday.getDay() + '\n';
  }

  // set anniversary
  str += _contactAnniversaryToString(this);

  // set relationship
  str += _contactRelationshipsToString(this);

  // set emails
  str += _contactEmailToString(this);

  // set organization data
  str += _contactOrganizationToString(this);

  // set role
  str += _contactRoleToString(this);

  // set phone numbers
  str += _contactPhoneNumbersToString(this);

  // set user ID
  str += 'UID:' + this.id + '\n';

  // set isFavorite
  str += 'X-IS-FAVORITE' + this.isFavorite + '\n';

  // set URLs
  str += _contactURLToString(this);

  // set messengers
  str += _contactInstantMessengeToString(this);

  // set last revision
  str += 'REV:' + this.lastUpdated.getYear() + '-' + this.lastUpdated.getMonth() +
  '-' + this.lastUpdated.getDay() + 'T' + this.lastUpdated.getHours() + ':' +
  this.lastUpdated.getMinutes() + ':' + this.lastUpdated.getSeconds() + 'Z\n';

  str += 'END:VCARD\n';

  return str;
};

// Creates a clone of the Contact object, detached from any address book
Contact.prototype.clone = function() {
  return new Contact(this);
};

// exports /////////////////////////////////////////////////////////////////
tizen.Contact = Contact;
