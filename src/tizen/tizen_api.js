// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Tizen API Specification:
// https://developer.tizen.org/dev-guide/2.2.1/org.tizen.web.device.apireference/tizen/tizen.html


// WARNING! This list should be in sync with the equivalent enum
// located at tizen.h. Remember to update tizen.h if you change
// something here.
var errors = {
  '-1': { type: 'NO_ERROR', name: 'NoError', message: '' },

  '0': { type: 'UNKNOWN_ERR', name: 'UnknownError', message: '' },
  '1': { type: 'INDEX_SIZE_ERR', name: 'IndexSizeError', message: '' },
  '2': { type: 'DOMSTRING_SIZE_ERR', name: 'DOMStringSizeError', message: '' },
  '3': { type: 'HIERARCHY_REQUEST_ERR', name: 'HierarchyRequestError', message: '' },
  '4': { type: 'WRONG_DOCUMENT_ERR', name: 'WrongDocumentError', message: '' },
  '5': { type: 'INVALID_CHARACTER_ERR', name: 'InvalidCharacterError', message: '' },
  '6': { type: 'NO_DATA_ALLOWED_ERR', name: 'NoDataAllowedError', message: '' },
  '7': { type: 'NO_MODIFICATION_ALLOWED_ERR', name: 'NoModificationAllowedError', message: '' },
  '8': { type: 'NOT_FOUND_ERR', name: 'NotFoundError', message: '' },
  '9': { type: 'NOT_SUPPORTED_ERR', name: 'Not_supportedError', message: '' },
  '10': { type: 'INUSE_ATTRIBUTE_ERR', name: 'InuseAttributeError', message: '' },
  '11': { type: 'INVALID_STATE_ERR', name: 'InvalidStateError', message: '' },
  '12': { type: 'SYNTAX_ERR', name: 'SyntaxError', message: '' },
  '13': { type: 'INVALID_MODIFICATION_ERR', name: 'InvalidModificationError', message: '' },
  '14': { type: 'NAMESPACE_ERR', name: 'NamespaceError', message: '' },
  '15': { type: 'INVALID_ACCESS_ERR', name: 'InvalidAccessError', message: '' },
  '16': { type: 'VALIDATION_ERR', name: 'ValidationError', message: '' },
  '17': { type: 'TYPE_MISMATCH_ERR', name: 'TypeMismatchError', message: '' },
  '18': { type: 'SECURITY_ERR', name: 'SecurityError', message: '' },
  '19': { type: 'NETWORK_ERR', name: 'NetworkError', message: '' },
  '20': { type: 'ABORT_ERR', name: 'AbortError', message: '' },
  '21': { type: 'URL_MISMATCH_ERR', name: 'UrlMismatchError', message: '' },
  '22': { type: 'QUOTA_EXCEEDED_ERR', name: 'QuotaExceededError', message: '' },
  '23': { type: 'TIMEOUT_ERR', name: 'TimeoutError', message: '' },
  '24': { type: 'INVALID_NODE_TYPE_ERR', name: 'InvalidNodeTypeError', message: '' },
  '25': { type: 'DATA_CLONE_ERR', name: 'DataCloneError', message: '' },

  // Error codes for these errors are not really defined anywhere.
  '100': { type: 'INVALID_VALUES_ERR', name: 'InvalidValuesError', message: '' },
  '101': { type: 'IO_ERR', name: 'IOError', message: 'IOError' },
  '102': { type: 'PERMISSION_DENIED_ERR', name: 'Permission_deniedError', message: '' },
  '103': { type: 'SERVICE_NOT_AVAILABLE_ERR', name: 'ServiceNotAvailableError', message: '' },
  '104': { type: 'DATABASE_ERR', name: 'DATABASE_ERR', message: '' }
};

/**
 * Generic exception interface.
 * @param {number} 16-bit error code.
 * @param {string} An error message that describes the details of an encountered error.
 * @param {string} An error type.
 */
exports.WebAPIException = function(code, message, name) {
  var _code, _message, _name;

  if (typeof code !== 'number') {
    _code = 0;
    _message = message || errors[0].message;
    _name = name || errors[0].name;
  } else {
    _code = code;
    if (typeof message === 'string') {
      _message = message;
    } else {
      _message = errors[_code].message;
    } if (typeof name === 'string') {
      _name = name;
    } else {
      _name = errors[_code].name;
    }
  }

  this.__defineGetter__('code', function() { return _code; });
  this.__defineGetter__('message', function() { return _message; });
  this.__defineGetter__('name', function() { return _name; });
};

for (var value in errors)
  Object.defineProperty(exports.WebAPIException, errors[value].type, { value: parseInt(value) });

/**
 * Generic error interface.
 * @param {number} 16-bit error code.
 * @param {string} An error message that describes the details of an encountered error.
 * @param {string} An error type.
 */
exports.WebAPIError = function(code, message, name) {
  var _code, _message, _name;

  if (typeof code !== 'number') {
    _code = errors[0].value;
    _message = errors[0].message;
    _name = errors[0].name;
  } else {
    _code = code;
    if (typeof message === 'string') {
      _message = message;
    } else {
      _message = errors[_code].message;
    } if (typeof name === 'string') {
      _name = name;
    } else {
      _name = errors[_code].name;
    }
  }

  this.__defineGetter__('code', function() { return _code; });
  this.__defineGetter__('message', function() { return _message; });
  this.__defineGetter__('name', function() { return _name; });
};

for (var value in errors)
  Object.defineProperty(exports.WebAPIError, errors[value].type, { value: value });

/**
 * Filter match flags.
 * @enum {string}
 */
var FilterMatchFlag = {
    EXACTLY : 'EXACTLY',
    FULLSTRING : 'FULLSTRING',
    CONTAINS : 'CONTAINS',
    STARTSWITH : 'STARTSWITH',
    ENDSWITH : 'ENDSWITH',
    EXISTS : 'EXISTS'
};

/**
 * An enumerator that indicates the sorting order.
 * @enum {string}
 */
var SortModeOrder = {
    ASC : 'ASC',
    DESC : 'DESC'
};

/**
 * An enumerator that indicates the type of composite filter.
 * @enum {string}
 */
var CompositeFilterType = {
    UNION : 'UNION',
    INTERSECTION : 'INTERSECTION'
};

// Tizen Filters
// either AttributeFilter, AttributeRangeFilter, or CompositeFilter
function is_tizen_filter(f) {
  return (f instanceof tizen.AttributeFilter) ||
         (f instanceof tizen.AttributeRangeFilter) ||
         (f instanceof tizen.CompositeFilter);
}

/**
 * This is a common interface used by different types of object filters.
 */
exports.AbstractFilter = function() {};

/**
 * Represents a set of filters.
 */
exports.AttributeFilter = function(attrName, matchFlag, matchValue) {
  if (this && this.constructor == exports.AttributeFilter &&
      (typeof(attrName) === 'string' || attrname instanceof String) &&
      matchFlag && matchFlag in FilterMatchFlag) {
    Object.defineProperties(this, {
      'attributeName': { writable: false, enumerable: true, value: attrName },
      'matchFlag': {
        writable: false,
        enumerable: true,
        value: matchValue !== undefined ? (matchFlag ? matchFlag : 'EXACTLY') : 'EXISTS'
      },
      'matchValue': {
        writable: false,
        enumerable: true,
        value: matchValue === undefined ? null : matchValue
      }
    });
  } else {
    throw new exports.WebAPIException(exports.WebAPIException.TYPE_MISMATCH_ERR);
  }
};
exports.AttributeFilter.prototype = new exports.AbstractFilter();
exports.AttributeFilter.prototype.constructor = exports.AttributeFilter;

/**
 * Represents a filter based on an object attribute which has values that are 
 * within a particular range.
 */
exports.AttributeRangeFilter = function(attrName, start, end) {
  if (!this || this.constructor != exports.AttributeRangeFilter ||
      !(typeof(attrName) === 'string' || attrname instanceof String)) {
    throw new exports.WebAPIException(exports.WebAPIException.TYPE_MISMATCH_ERR);
  }

  Object.defineProperties(this, {
    'attributeName': { writable: true, enumerable: true, value: attrName },
    'initialValue': {
      writable: true,
      enumerable: true,
      value: start === undefined ? null : start },
    'endValue': { writable: true, enumerable: true, value: end === undefined ? null : end }
  });
};
exports.AttributeRangeFilter.prototype = new exports.AbstractFilter();
exports.AttributeRangeFilter.prototype.constructor = exports.AttributeRangeFilter;

/**
 * Represents a set of filters.
 */
exports.CompositeFilter = function(type, filters) {
  if (!this || this.constructor != exports.CompositeFilter ||
      !(type in CompositeFilterType) ||
      filters && !(filters instanceof Array)) {
    throw new exports.WebAPIException(exports.WebAPIException.TYPE_MISMATCH_ERR);
  }

  Object.defineProperties(this, {
    'type': { writable: false, enumerable: true, value: type },
    'filters': {
      writable: false,
      enumerable: true,
      value: filters === undefined ? null : filters
    }
  });
};
exports.CompositeFilter.prototype = new exports.AbstractFilter();
exports.CompositeFilter.prototype.constructor = exports.CompositeFilter;

/**
 * SortMode is a common interface used for sorting of queried data.
 */
exports.SortMode = function(attrName, order) {
  if (!(typeof(attrName) === 'string' || attrname instanceof String) ||
      order && (order != 'DESC' && order != 'ASC'))
    throw new exports.WebAPIException(exports.WebAPIException.TYPE_MISMATCH_ERR);

  Object.defineProperties(this, {
    'attributeName': { writable: false, enumerable: true, value: attrName },
    'order': { writable: false, enumerable: true, value: order || 'ASC' }
  });
};
exports.SortMode.prototype.constructor = exports.SortMode;

/**
 * Represents a point (latitude and longitude) in the map coordinate system.
 */
exports.SimpleCoordinates = function(latitude, longitude) {
  if (!(typeof(latitude) === 'number' || typeof(longitude) === 'number'))
    throw new exports.WebAPIException(exports.WebAPIException.TYPE_MISMATCH_ERR);

  Object.defineProperties(this, {
    'latitude': { writable: false, enumerable: true, value: latitude },
    'longitude': { writable: false, enumerable: true, value: longitude }
  });
};
exports.SimpleCoordinates.prototype.constructor = exports.SimpleCoordinates;
