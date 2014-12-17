/* global xwalk, extension */

// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var _common = xwalk.utils;
var Type = _common.type;
var Converter = _common.converter;
var AV = _common.validator;

var native_ = new _common.NativeManager(extension);

var _currentWatchId = 1;

var _getNextWatchId = function() {
  return _currentWatchId++;
};

// Adjusts properties to have the correct format expected by the native side.
// Currently only translates JS Date
var _toJsonObject = function(obj) {
  var ret;
  if (Type.isDate(obj)) {
    var year = ('0000' + obj.getFullYear()).slice(-4);
    var month = ('00' + (obj.getMonth() + 1)).slice(-2);
    var day = ('00' + obj.getDate()).slice(-2);
    return Number(year + month + day);
  }
  if (Type.isArray(obj)) {
    ret = [];
    for (var i = 0; i < obj.length; ++i) {
      ret[i] = _toJsonObject(obj[i]);
    }
    return ret;
  }
  if (obj instanceof Object) {
    ret = {};
    for (var prop in obj) {
      if (obj.hasOwnProperty(prop)) {
        ret[prop] = _toJsonObject(obj[prop]);
      }
    }
    return ret;
  }
  return obj;
};

var _fromJsonDate = function(date) {
  date = date + '';
  var year = date.substr(0, 4);
  var month = date.substr(4, 2);
  var day = date.substr(6, 2);
  return new Date(year, month - 1, day);
};

var _promote = function(val, type) {
  return _editGuard.run(function() {
    if (Type.isArray(val)) {
      var ret = [];
      for (var i = 0; i < val.length; ++i) {
        ret.push(new type(val[i]));
      }
      return ret;
    }
    return new type(val);
  });
};

function _checkError(result) {
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
}

var TypeEnum = ['VCARD_30'];

// Edit Guard //////////////////////////////////////////////////////////////
// This flag is workaround. It is caused by specification
// which tell to edit readonly fields.
var _canEdit = 0;

var EditGuard = function() {
};

EditGuard.prototype.run = function(callback) {
  try {
    this.enable();
    var result = callback();
    this.disable();
    return result;
  } catch (ex) {
    this.disable();
    throw ex;
  }
};

EditGuard.prototype.enable = function() {
  _canEdit++;
};

EditGuard.prototype.disable = function() {
  _canEdit--;
};

EditGuard.prototype.isEditEnabled = function() {
  return _canEdit > 0;
};

var _editGuard = new EditGuard();
