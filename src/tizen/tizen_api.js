// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Tizen API Specification:
// https://developer.tizen.org/dev-guide/2.3.0/org.tizen.mobile.web.device.apireference/tizen/tizen.html

function __isObject(object) {
  return object instanceof Object;
}

function __isUndefined(object) {
  return object === void 0;
}

function __isNumber(object) {
  return typeof object === 'number';
}

// WARNING! This list should be in sync with the equivalent enum
// located at tizen.h. Remember to update tizen.h if you change
// something here.
var errors = {
  NO_ERROR: 0,
  UNKNOWN_ERR: -1,

  INDEX_SIZE_ERR: 1,
  DOMSTRING_SIZE_ERR: 2,
  HIERARCHY_REQUEST_ERR: 3,
  WRONG_DOCUMENT_ERR: 4,
  INVALID_CHARACTER_ERR: 5,
  NO_DATA_ALLOWED_ERR: 6,
  NO_MODIFICATION_ALLOWED_ERR: 7,
  NOT_FOUND_ERR: 8,
  NOT_SUPPORTED_ERR: 9,
  INUSE_ATTRIBUTE_ERR: 10,
  INVALID_STATE_ERR: 11,
  SYNTAX_ERR: 12,
  INVALID_MODIFICATION_ERR: 13,
  NAMESPACE_ERR: 14,
  INVALID_ACCESS_ERR: 15,
  VALIDATION_ERR: 16,
  TYPE_MISMATCH_ERR: 17,
  SECURITY_ERR: 18,
  NETWORK_ERR: 19,
  ABORT_ERR: 20,
  URL_MISMATCH_ERR: 21,
  QUOTA_EXCEEDED_ERR: 22,
  TIMEOUT_ERR: 23,
  INVALID_NODE_TYPE_ERR: 24,
  DATA_CLONE_ERR: 25,

  // Error codes for these errors are not really defined anywhere.
  INVALID_VALUES_ERR: 100,
  IO_ERR: 101,
  PERMISSION_DENIED_ERR: 102,
  SERVICE_NOT_AVAILABLE_ERR: 103,
  DATABASE_ERR: 104
};

var code_to_name = {};
code_to_name[errors['NO_ERROR']] = 'NoError';
code_to_name[errors['UNKNOWN_ERR']] = 'UnknownError';
code_to_name[errors['INDEX_SIZE_ERR']] = 'IndexSizeError';
code_to_name[errors['DOMSTRING_SIZE_ERR']] = 'DOMStringSizeError';
code_to_name[errors['HIERARCHY_REQUEST_ERR']] = 'HierarchyRequestError';
code_to_name[errors['WRONG_DOCUMENT_ERR']] = 'WrongDocumentError';
code_to_name[errors['INVALID_CHARACTER_ERR']] = 'InvalidCharacterError';
code_to_name[errors['NO_DATA_ALLOWED_ERR']] = 'NoDataAllowedError';
code_to_name[errors['NO_MODIFICATION_ALLOWED_ERR']] = 'NoModificationAllowedError';
code_to_name[errors['NOT_FOUND_ERR']] = 'NotFoundError';
code_to_name[errors['NOT_SUPPORTED_ERR']] = 'NotSupportedError';
code_to_name[errors['INUSE_ATTRIBUTE_ERR']] = 'InuseAttributeError';
code_to_name[errors['INVALID_STATE_ERR']] = 'InvalidStateError';
code_to_name[errors['SYNTAX_ERR']] = 'SyntaxError';
code_to_name[errors['INVALID_MODIFICATION_ERR']] = 'InvalidModificationError';
code_to_name[errors['NAMESPACE_ERR']] = 'NamespaceError';
code_to_name[errors['INVALID_ACCESS_ERR']] = 'InvalidAccessError';
code_to_name[errors['VALIDATION_ERR']] = 'ValidationError';
code_to_name[errors['TYPE_MISMATCH_ERR']] = 'TypeMismatchError';
code_to_name[errors['SECURITY_ERR']] = 'SecurityError';
code_to_name[errors['NETWORK_ERR']] = 'NetworkError';
code_to_name[errors['ABORT_ERR']] = 'AbortError';
code_to_name[errors['URL_MISMATCH_ERR']] = 'URLMismatchError';
code_to_name[errors['QUOTA_EXCEEDED_ERR']] = 'QuotaExceededError';
code_to_name[errors['TIMEOUT_ERR']] = 'TimeoutError';
code_to_name[errors['INVALID_NODE_TYPE_ERR']] = 'InvalidNodeTypeError';
code_to_name[errors['DATA_CLONE_ERR']] = 'DataCloneError';

code_to_name[errors['INVALID_VALUES_ERR']] = 'InvalidValuesError';
code_to_name[errors['IO_ERR']] = 'IOError';
code_to_name[errors['PERMISSION_DENIED_ERR']] = 'PermissionDeniedError';
code_to_name[errors['SERVICE_NOT_AVAILABLE_ERR']] = 'ServiceNotAvailableError';
code_to_name[errors['DATABASE_ERR']] = 'DatabaseError';

var name_to_code = {};
Object.keys(errors).forEach(function(key) {
  name_to_code[code_to_name[errors[key]]] = errors[key];
});


/**
 * Generic exception interface.
 *
 * @param {number} code 16-bit error code.
 * @param {string} message An error message that describes the details of an encountered error.
 * @param {string} name An error type.
 */
var WebAPIException = function(code, message, name) {
  var code_ = 0;
  var name_ = code_to_name[code];
  var message_ = 'Unknown error';

  switch (arguments.length) {
    case 1:
      var error = arguments[0];
      if (__isObject(error)) {
        code_ = error.code;
        name_ = error.name;
        message_ = error.message;
        if (__isUndefined(code_) && !__isUndefined(name_))
          code_ = name_to_code[name_];
        if (__isUndefined(name_) && !__isUndefined(code_))
          name_ = code_to_name[code_];
      } else if (__isNumber(error)) {
        // backward compatibility with crosswalk implementation
        code_ = error;
        name_ = code_to_name[code];
        message_ = name_;
      }
      break;
    case 2:
      if (__isNumber(arguments[0])) {
        code_ = arguments[0];
        if (!__isUndefined(code_to_name[code_])) {
          name_ = code_to_name[code_];
        }
      } else {
        name_ = String(arguments[0]);
        if (!__isUndefined(name_to_code[name_])) {
          code_ = name_to_code[name_];
        }
      }
      message_ = String(arguments[1]);
      break;
    case 3:
      // backward compatibility with crosswalk implementation
      code_ = Number(arguments[0]);
      message_ = String(arguments[1]);
      name_ = String(arguments[2]);
      break;
    default:
      return;
  }

  // attributes
  Object.defineProperties(this, {
    code: {value: code_, writable: false, enumerable: true},
    name: {value: name_, writable: false, enumerable: true},
    message: {value: message_, writable: false, enumerable: true}
  });

  this.constructor.prototype.__proto__ = Error.prototype;
  Error.captureStackTrace(this, this.constructor);
};

WebAPIException.prototype.toString = function() {
  return this.name + ': ' + this.message;
};


var error_constants = {};
for (var prop in errors) {
  error_constants[prop] = {value: errors[prop], writable: false, enumerable: true};
}
Object.defineProperties(WebAPIException, error_constants);
Object.defineProperties(WebAPIException.prototype, error_constants);

exports.WebAPIException = WebAPIException;
exports.WebAPIError = WebAPIException;


/**
 * Filter match flags.
 * @enum {string}
 */
var FilterMatchFlag = {
  EXACTLY: 'EXACTLY',
  FULLSTRING: 'FULLSTRING',
  CONTAINS: 'CONTAINS',
  STARTSWITH: 'STARTSWITH',
  ENDSWITH: 'ENDSWITH',
  EXISTS: 'EXISTS'
};


/**
 * An enumerator that indicates the sorting order.
 * @enum {string}
 */
var SortModeOrder = {
  ASC: 'ASC',
  DESC: 'DESC'
};


/**
 * An enumerator that indicates the type of composite filter.
 * @enum {string}
 */
var CompositeFilterType = {
  UNION: 'UNION',
  INTERSECTION: 'INTERSECTION'
};

// Tizen Filters
// either AttributeFilter, AttributeRangeFilter, or CompositeFilter
function is_tizen_filter(f) {
  return (f instanceof tizen.AttributeFilter) ||
         (f instanceof tizen.AttributeRangeFilter) ||
         (f instanceof tizen.CompositeFilter);
}

//Extract property by string
function _extractProperty(obj, attributeName) {
  var props = attributeName.split('.');
  for (var i = 0; i < props.length; ++i) {
    if (obj instanceof Array) {
      var ret = [];
      for (var j = 0; j < obj.length; ++j)
      {
        ret.push(_extractProperty(obj[j], props.slice(i).join('.')));
      }
      return ret;
    }
    if (!obj.hasOwnProperty(props[i])) {
      if (i === props.length - 1) {
        throw new tizen.WebAPIException('InvalidValuesError', 'Property "' + attributeName +
                '" is not valid');
      }
      return null;
    }
    obj = obj[props[i]];
  }
  return obj;
}


/**
 * This is a common interface used by different types of object filters.
 */
exports.AbstractFilter = function() {};


/**
 * Represents a set of filters.
 */
exports.AttributeFilter = function(attrName, matchFlag, matchValue) {
  if (!(this instanceof exports.AttributeFilter)) {
    throw new WebAPIException('TypeError', 'Constructor cannot be called as function.');
  }
  var name_ = '';
  var flag_ = 'EXACTLY';
  var value_ = null;

  function attributeNameSetter(name) {
    name_ = String(name);
  }

  if (arguments.length > 0)
    attributeNameSetter(attrName);

  function matchFlagSetter(flag) {
    if (Object.keys(FilterMatchFlag).indexOf(flag) >= 0)
      flag_ = flag;
  }

  function matchValueSetter(value) {
    value_ = value;
  }

  if (arguments.length > 2) {
    matchFlagSetter(matchFlag);
    matchValueSetter(matchValue);
  } else if (arguments.length > 1) {
    // if matchValue is not used then matchFlag is set to 'EXISTS'.
    matchFlagSetter('EXISTS');
  }

  Object.defineProperties(this, {
    attributeName: {
      enumerable: true,
      set: attributeNameSetter,
      get: function() {
        return name_;
      }
    },
    matchFlag: {
      enumerable: true,
      set: matchFlagSetter,
      get: function() {
        return flag_;
      }
    },
    matchValue: {
      enumerable: true,
      set: matchValueSetter,
      get: function() {
        return value_;
      }
    }
  });
};
exports.AttributeFilter.prototype = new exports.AbstractFilter();

//TODO: Move filtering to native code
exports.AttributeFilter.prototype._filter = function(element) {
  var elemValue = _extractProperty(element, this.attributeName);

  if (!(elemValue instanceof Array)) {
    elemValue = [elemValue];
  }

  var ret = false;
  for (var i = 0; i < elemValue.length; ++i) {
    var elemValueStr = String(elemValue[i]);
    var elemValueStrU = elemValueStr.toUpperCase();
    var matchValueStr = String(this.matchValue);
    var matchValueStrU = matchValueStr.toUpperCase();

    switch (this.matchFlag) {
      case 'EXACTLY':
        ret = elemValue[i] === this.matchValue;
        break;
      case 'FULLSTRING':
        ret = elemValueStrU === matchValueStrU;
        break;
      case 'CONTAINS':
        ret = elemValueStrU.indexOf(matchValueStrU) > -1;
        break;
      case 'STARTSWITH':
        ret = elemValueStrU.indexOf(matchValueStrU) === 0;
        break;
      case 'ENDSWITH':
        ret = elemValueStrU.lastIndexOf(matchValueStrU) +
                matchValueStrU.length === elemValueStrU.length;
        break;
      case 'EXISTS':
        ret = elemValue[i] !== undefined;
        break;
    }
    if (ret) {
      return ret;
    }
  }
  return ret;
};
exports.AttributeFilter.prototype.constructor = exports.AttributeFilter;


/**
 * Represents a filter based on an object attribute which has values that are
 * within a particular range.
 */
exports.AttributeRangeFilter = function(attrName, start, end) {
  if (!(this instanceof exports.AttributeRangeFilter)) {
    throw new WebAPIException('TypeError', 'Constructor cannot be called as function.');
  }
  var name_ = '';
  var start_ = null;
  var end_ = null;

  function attributeNameSetter(name) {
    name_ = String(name);
  }

  if (arguments.length > 0)
    attributeNameSetter(attrName);

  function initSetter(init) {
    start_ = init;
  }

  if (arguments.length > 1)
    initSetter(start);

  function endSetter(end) {
    end_ = end;
  }

  if (arguments.length > 2)
    endSetter(end);

  Object.defineProperties(this, {
    attributeName: {
      enumerable: true,
      set: attributeNameSetter,
      get: function() {
        return name_;
      }
    },
    initialValue: {
      enumerable: true,
      set: initSetter,
      get: function() {
        return start_;
      }
    },
    endValue: {
      enumerable: true,
      set: endSetter,
      get: function() {
        return end_;
      }
    }
  });
};

exports.AttributeRangeFilter.prototype = new exports.AbstractFilter();

//TODO: Move filtering to native code
exports.AttributeRangeFilter.prototype._filter = function(element) {
  var elemValue = _extractProperty(element, this.attributeName);

  if (!(elemValue instanceof Array)) {
    elemValue = [elemValue];
  }

  for (var i = 0; i < elemValue.length; ++i) {
    var value = elemValue[i];

    if ((this.initialValue !== undefined && this.initialValue !== null) &&
            (this.endValue !== undefined && this.endValue !== null)) {
      if (value instanceof tizen.TZDate) {
        if (this.initialValue.earlierThan(value) && this.endValue.laterThan(value)) {
          return true;
        }
      } else {
        if (this.initialValue <= value && this.endValue > value) {
          return true;
        }
      }
    } else if ((this.initialValue !== undefined && this.initialValue !== null) &&
            (this.endValue === undefined || this.endValue === null)) {
      if (value instanceof tizen.TZDate) {
        if (this.initialValue.earlierThan(value)) {
          return true;
        }
      } else {
        if (this.initialValue <= value) {
          return true;
        }
      }
    } else if ((this.initialValue === undefined || this.initialValue === null) &&
            (this.endValue !== undefined && this.endValue !== null)) {
      if (value instanceof tizen.TZDate) {
        if (this.endValue.laterThan(value)) {
          return true;
        }
      } else {
        if (this.endValue > value) {
          return true;
        }
      }
    }
  }
  return false;
};

exports.AttributeRangeFilter.prototype.constructor = exports.AttributeRangeFilter;


/**
 * Represents a set of filters.
 */
exports.CompositeFilter = function(type, filters) {
  if (!(this instanceof exports.CompositeFilter)) {
    throw new WebAPIException('TypeError', 'Constructor cannot be called as function.');
  }
  var filterTypes = Object.keys(CompositeFilterType);

  var type_ = filterTypes[0];
  var filters_ = [];

  function typeSetter(filterType) {
    if (filterTypes.indexOf(filterType) >= 0)
      type_ = filterType;
  }

  if (arguments.length > 0)
    typeSetter(type);

  function filtersSetter(filterList) {
    if (!(filterList instanceof Array))
      return;

    for (var i in filterList) {
      var valid = (filterList[i] instanceof tizen.AbstractFilter);
      if (!valid)
        return;
    }

    filters_ = filterList.slice(0);
  }

  if (arguments.length > 1)
    filtersSetter(filters);

  Object.defineProperties(this, {
    type: {
      enumerable: true,
      set: typeSetter,
      get: function() {
        return type_;
      }
    },
    filters: {
      enumerable: true,
      set: filtersSetter,
      get: function() {
        return filters_;
      }
    }
  });
};

exports.CompositeFilter.prototype = new exports.AbstractFilter();

//TODO: Move filtering to native code
exports.CompositeFilter.prototype._filter = function(element) {
  var filters = this.filters;
  if (this.type === 'UNION') {
    for (var i = 0; i < filters.length; ++i) {
      if (filters[i]._filter(element)) {
        return true;
      }
    }
    return false;
  } else if (this.type === 'INTERSECTION') {
    if (filters.length === 0)
      return false;
    for (var i = 0; i < filters.length; ++i) {
      if (!filters[i]._filter(element)) {
        return false;
      }
    }
    return true;
  }
};

exports.CompositeFilter.prototype.constructor = exports.CompositeFilter;


/**
 * SortMode is a common interface used for sorting of queried data.
 */
exports.SortMode = function(attrName, order) {
  if (!(this instanceof exports.SortMode)) {
    throw new WebAPIException('TypeError', 'Constructor cannot be called as function.');
  }
  var sortModeOrder = Object.keys(SortModeOrder);

  var attributeName_ = '';
  var order_ = 'ASC';

  function nameSetter(name) {
    attributeName_ = String(name);
  }

  if (arguments.length > 0)
    nameSetter(attrName);

  function orderSetter(sortOrder) {
    if (sortModeOrder.indexOf(sortOrder) >= 0)
      order_ = sortOrder;
  }

  if (arguments.length > 1)
    orderSetter(order);

  Object.defineProperties(this, {
    attributeName: {
      enumerable: true, set: nameSetter, get: function() {
        return attributeName_;
      }
    },
    order: {
      enumerable: true, set: orderSetter, get: function() {
        return order_;
      }
    }
  });
};
exports.SortMode.prototype.constructor = exports.SortMode;


/**
 * Represents a point (latitude and longitude) in the map coordinate system.
 */
exports.SimpleCoordinates = function(lat, lng) {
  if (!(this instanceof exports.SimpleCoordinates)) {
    throw new WebAPIException('TypeError', 'Constructor cannot be called as function.');
  }

  var latitude = 0;
  var longitude = 0;

  function latSetter(lat) {
    var tmp = Number(lat);
    if (!isNaN(tmp)) {
      if (tmp > 90) tmp = 90;
      else if (tmp < -90) tmp = -90;

      latitude = tmp;
    }
  }

  latSetter(lat);

  function lonSetter(lon) {
    var tmp = Number(lon);
    if (!isNaN(tmp)) {
      if (tmp > 180) tmp = 180;
      else if (tmp < -180) tmp = -180;

      longitude = tmp;
    }
  }

  lonSetter(lng);

  Object.defineProperties(this, {
    latitude: {
      enumerable: true, set: latSetter, get: function() {
        return latitude;
      }
    },
    longitude: {
      enumerable: true, set: lonSetter, get: function() {
        return longitude;
      }
    }
  });
};
exports.SimpleCoordinates.prototype.constructor = exports.SimpleCoordinates;
