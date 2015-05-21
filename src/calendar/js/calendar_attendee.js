// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var AttendeeType = {
  INDIVIDUAL: 'INDIVIDUAL',
  GROUP: 'GROUP',
  RESOURCE: 'RESOURCE',
  ROOM: 'ROOM',
  UNKNOWN: 'UNKNOWN'
};

var AttendeeStatus = {
  PENDING: 'PENDING',
  ACCEPTED: 'ACCEPTED',
  DECLINED: 'DECLINED',
  TENTATIVE: 'TENTATIVE',
  DELEGATED: 'DELEGATED',
  COMPLETED: 'COMPLETED',
  IN_PROCESS: 'IN_PROCESS'
};

var AttendeeRole = {
  REQ_PARTICIPANT: 'REQ_PARTICIPANT',
  OPT_PARTICIPANT: 'OPT_PARTICIPANT',
  NON_PARTICIPANT: 'NON_PARTICIPANT',
  CHAIR: 'CHAIR'
};

var CalendarAttendeeInit = function(data) {
  var _name = null;
  var _role = 'REQ_PARTICIPANT';
  var _status = 'PENDING';
  var _RSVP = false;
  var _type = 'INDIVIDUAL';
  var _group = null;
  var _delegatorURI = null;
  var _delegateURI = null;
  var _contactRef = null;

  Object.defineProperties(this, {
    name: {
      get: function() {
        return _name;
      },
      set: function(v) {
        _name = converter_.toString(v, true);
      },
      enumerable: true
    },
    role: {
      get: function() {
        return _role;
      },
      set: function(v) {
        if (v === null) {
          return;
        }
        _role = converter_.toEnum(v, Object.keys(AttendeeRole), false);
      },
      enumerable: true
    },
    status: {
      get: function() {
        return _status;
      },
      set: function(v) {
        if (v === null) {
          return;
        }
        _status = converter_.toEnum(v, Object.keys(AttendeeStatus), false);
      },
      enumerable: true
    },
    RSVP: {
      get: function() {
        return _RSVP;
      },
      set: function(v) {
        _RSVP = converter_.toBoolean(v);
      },
      enumerable: true
    },
    type: {
      get: function() {
        return _type;
      },
      set: function(v) {
        if (v === null) {
          return;
        }
        _type = converter_.toEnum(v, Object.keys(AttendeeType), false);
      },
      enumerable: true
    },
    group: {
      get: function() {
        return _group;
      },
      set: function(v) {
        _group = converter_.toString(v, true);
      },
      enumerable: true
    },
    delegatorURI: {
      get: function() {
        return _delegatorURI;
      },
      set: function(v) {
        _delegatorURI = converter_.toString(v, true);
      },
      enumerable: true
    },
    delegateURI: {
      get: function() {
        return _delegateURI;
      },
      set: function(v) {
        _delegateURI = converter_.toString(v, true);
      },
      enumerable: true
    },
    contactRef: {
      get: function() {
        return _contactRef;
      },
      set: function(v) {
        _contactRef = v instanceof tizen.ContactRef ? v : _contactRef;
      },
      enumerable: true
    }
  });

  if (data instanceof Object) {
    for (var prop in data) {
      if (this.hasOwnProperty(prop)) {
        this[prop] = data[prop];
      }
    }
  }
};

var CalendarAttendee = function(uri, attendeeInitDict) {
  validator_.isConstructorCall(this, CalendarAttendee);

  CalendarAttendeeInit.call(this, attendeeInitDict);

  var _uri = null;

  Object.defineProperties(this, {
    uri: {
      get: function() {
        return _uri;
      },
      set: function(v) {
        if (v === null) {
          return;
        }
        _uri = converter_.toString(v, true);
      },
      enumerable: true
    }
  });

  this.uri = uri;
};

CalendarAttendee.prototype = new CalendarAttendeeInit();
CalendarAttendee.prototype.constructor = CalendarAttendee;

tizen.CalendarAttendee = CalendarAttendee;
