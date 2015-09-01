// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var _global = {};
if (typeof window != 'undefined') {
  _global = window;
}
else if (typeof global != 'undefiend') {
  _global = global;
}

/**
 * @deprecated Used only by validateArguments()
 */
var signature_to_type = {
  'n': 'number',
  'f': 'function',
  'b': 'boolean',
  's': 'string',
  'o': 'object'
};

var DateConverter = function() {};

DateConverter.prototype.toTZDate = function(v, isAllDay) {
  if (typeof v === 'number') {
    v = {
      UTCTimestamp: v
        };
    isAllDay = false;
  }

  if (!(v instanceof _global.Object)) {
    return v;
  }

  if (isAllDay) {
    return new tizen.TZDate(v.year, v.month - 1, v.day,
        null, null, null, null, v.timezone || null);
  } else {
    return new tizen.TZDate(new Date(v.UTCTimestamp * 1000), 'UTC').toLocalTimezone();
  }
};

DateConverter.prototype.fromTZDate = function(v) {
  if (!tizen.TZDate || !(v instanceof tizen.TZDate)) {
    return v;
  }

  var timestamp = Date.UTC(v.date_.getUTCFullYear(),
                           v.date_.getUTCMonth(),
                           v.date_.getUTCDate(),
                           v.date_.getUTCHours(),
                           v.date_.getUTCMinutes(),
                           v.date_.getUTCSeconds(),
                           v.date_.getUTCMilliseconds()) / 1000;

  return {
    year: v.getFullYear(),
    month: v.getMonth(),
    day: v.getDate(),
    timezone: v.getTimezone(),
    UTCTimestamp: timestamp
  };

};

var _dateConverter = new DateConverter();

/** @constructor */
function Utils() {
  var privilege = {
    ACCOUNT_READ: 'http://tizen.org/privilege/account.read',
    ACCOUNT_WRITE: 'http://tizen.org/privilege/account.write',
    ALARM: 'http://tizen.org/privilege/alarm',
    APPLICATION_INFO: 'http://tizen.org/privilege/application.info',
    APPLICATION_LAUNCH: 'http://tizen.org/privilege/application.launch',
    APPMANAGER_CERTIFICATE: 'http://tizen.org/privilege/appmanager.certificate',
    APPMANAGER_KILL: 'http://tizen.org/privilege/appmanager.kill',
    BLUETOOTH_ADMIN: 'http://tizen.org/privilege/bluetooth.admin',
    BLUETOOTH_GAP: 'http://tizen.org/privilege/bluetooth.gap',
    BLUETOOTH_HEALTH: 'http://tizen.org/privilege/bluetooth.health',
    BLUETOOTH_SPP: 'http://tizen.org/privilege/bluetooth.spp',
    BLUETOOTHMANAGER: 'http://tizen.org/privilege/bluetoothmanager',
    BLUETOOTH: 'http://tizen.org/privilege/bluetooth',
    BOOKMARK_READ: 'http://tizen.org/privilege/bookmark.read',
    BOOKMARK_WRITE: 'http://tizen.org/privilege/bookmark.write',
    CALENDAR_READ: 'http://tizen.org/privilege/calendar.read',
    CALENDAR_WRITE: 'http://tizen.org/privilege/calendar.write',
    CALLHISTORY_READ: 'http://tizen.org/privilege/callhistory.read',
    CALLHISTORY_WRITE: 'http://tizen.org/privilege/callhistory.write',
    CONTACT_READ: 'http://tizen.org/privilege/contact.read',
    CONTACT_WRITE: 'http://tizen.org/privilege/contact.write',
    CONTENT_READ: 'http://tizen.org/privilege/content.read',
    CONTENT_WRITE: 'http://tizen.org/privilege/content.write',
    DATACONTROL_CONSUMER: 'http://tizen.org/privilege/datacontrol.consumer',
    DATASYNC: 'http://tizen.org/privilege/datasync',
    DOWNLOAD: 'http://tizen.org/privilege/download',
    FILESYSTEM_READ: 'http://tizen.org/privilege/filesystem.read',
    FILESYSTEM_WRITE: 'http://tizen.org/privilege/filesystem.write',
    HEALTHINFO: 'http://tizen.org/privilege/healthinfo',
    INTERNET: 'http://tizen.org/privilege/internet',
    KEYMANAGER: 'http://tizen.org/privilege/keymanager',
    LED: 'http://tizen.org/privilege/led',
    LOCATION: 'http://tizen.org/privilege/location',
    MEDIACONTROLLER_SERVER: 'http://tizen.org/privilege/mediacontroller.server',
    MEDIACONTROLLER_CLIENT: 'http://tizen.org/privilege/mediacontroller.client',
    MESSAGING_READ: 'http://tizen.org/privilege/messaging.read',
    MESSAGING_WRITE: 'http://tizen.org/privilege/messaging.write',
    NETWORKBEARERSELECTION: 'http://tizen.org/privilege/networkbearerselection',
    NFC_ADMIN: 'http://tizen.org/privilege/nfc.admin',
    NFC_CARDEMULATION: 'http://tizen.org/privilege/nfc.cardemulation',
    NFC_COMMON: 'http://tizen.org/privilege/nfc.common',
    NFC_P2P: 'http://tizen.org/privilege/nfc.p2p',
    NFC_TAG: 'http://tizen.org/privilege/nfc.tag',
    NOTIFICATION: 'http://tizen.org/privilege/notification',
    PACKAGE_INFO: 'http://tizen.org/privilege/package.info',
    PACKAGEMANAGER_INSTALL: 'http://tizen.org/privilege/packagemanager.install',
    POWER: 'http://tizen.org/privilege/power',
    PUSH: 'http://tizen.org/privilege/push',
    SECUREELEMENT: 'http://tizen.org/privilege/secureelement',
    SETTING: 'http://tizen.org/privilege/setting',
    SYSTEM: 'http://tizen.org/privilege/system',
    SYSTEMMANAGER: 'http://tizen.org/privilege/systemmanager',
    TELEPHONY: 'http://tizen.org/privilege/telephony',
    TV_WINDOW: 'http://tizen.org/privilege/tv.window ',
    VOLUME_SET: 'http://tizen.org/privilege/volume.set'
  };
  Object.freeze(privilege);

  Object.defineProperty(this, 'privilege', {
    value: privilege,
    writable: false,
    enumerable: true,
    configurable: false
  });
}

Utils.prototype.repackFilter = function(filter) {
  if (filter instanceof tizen.AttributeFilter) {
    return {
      filterType: 'AttributeFilter',
      attributeName: filter.attributeName,
      matchFlag: filter.matchFlag,
      matchValue: _dateConverter.fromTZDate(filter.matchValue)
    };
  }
  if (filter instanceof tizen.AttributeRangeFilter) {
    return {
      filterType: 'AttributeRangeFilter',
      attributeName: filter.attributeName,
      initialValue: _dateConverter.fromTZDate(filter.initialValue),
      endValue: _dateConverter.fromTZDate(filter.endValue)
    };
  }
  if (filter instanceof tizen.CompositeFilter) {
    var _f = [];
    var filters = filter.filters;

    for (var i = 0; i < filters.length; ++i) {
      _f.push(this.repackFilter(filters[i]));
    }

    return {
      filterType: 'CompositeFilter',
      type: filter.type,
      filters: _f
    };
  }

  return null;
};

/**
 * @deprecated You should use xwalk.utils.validator.validateMethod() instead.
 */
Utils.prototype.validateArguments = function(signature, args) {
  var full_args = Array.prototype.slice.call(args);

  // After '?' everything is optional.
  var mandatory_len = signature.indexOf('?') === -1 ? signature.length : signature.indexOf('?');

  if (full_args.length < mandatory_len)
    return false;

  // Mandatory arguments.
  for (var i = 0; i < mandatory_len; i++) {
    if (typeof full_args[i] !== signature_to_type[signature[i]] || full_args[i] === null)
      return false;
  }

  // Optional args may be null.
  for (var i = mandatory_len; i < full_args.length && i < signature.length - 1; i++) {
    if (full_args[i] !== null && typeof full_args[i] !== signature_to_type[signature[i + 1]])
      return false;
  }

  return true;
};

Utils.prototype.validateObject = function(object, signature, attributes) {
  for (var i = 0; i < signature.length; i++) {
    if (object.hasOwnProperty(attributes[i]) &&
        typeof object[attributes[i]] !== signature_to_type[signature[i]]) {
      return false;
    }
  }

  return true;
};

Utils.prototype.getPkgApiVersion = function() {
  var result = native_.callSync('Utils_getPkgApiVersion');
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  return native_.getResultObject(result);
};

Utils.prototype.checkPrivilegeAccess = function(privilege) {
  var result = native_.callSync('Utils_checkPrivilegeAccess', {
    privilege : _toString(privilege),
  });

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

Utils.prototype.checkPrivilegeAccess4Ver = function(new_ver, new_priv, old_priv) {
  var app_ver = this.getPkgApiVersion();

  var arr_new_ver = new_ver.split(".");
  var arr_app_ver = app_ver.split(".");
  var num_new;
  var num_app;
  var sel = 0;

  var i;
  var length = Math.min(arr_new_ver.length, arr_app_ver.length);
  for (i = 0; i < length; i++) {
    num_new = parseInt(arr_new_ver[i]);
    num_app = parseInt(arr_app_ver[i]);
    if (num_app < num_new) {
      sel = 1;
      break;
    } else if (num_app > num_new) {
      sel = -1;
      break;
    }
  }

  if (sel == 0 && arr_new_ver.length > arr_app_ver.length) {
    sel = 1;
  }

  if (sel != 1) {
    this.checkPrivilegeAccess(new_priv);
  } else if (old_priv != undefined) {
    this.checkPrivilegeAccess(old_priv);
  }
}

Utils.prototype.checkBackwardCompabilityPrivilegeAccess = function(current_privilege, previous_privilege) {
  var result = native_.callSync('Utils_checkBackwardCompabilityPrivilegeAccess', {
    current_privilege : _toString(current_privilege),
    previous_privilege : _toString(previous_privilege),
  });

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

/////////////////////////////////////////////////////////////////////////////
/** @constructor */
var Type = function() {};

Type.prototype.isBoolean = function(obj) {
  return typeof obj === 'boolean';
};

Type.prototype.isObject = function(obj) {
  return (null !== obj && typeof obj === 'object' && !this.isArray(obj));
};

Type.prototype.isArray = function(obj) {
  return Array.isArray(obj);
};

Type.prototype.isFunction = function(obj) {
  return typeof obj === 'function';
};

Type.prototype.isNumber = function(obj) {
  return typeof obj === 'number';
};

Type.prototype.isString = function(obj) {
  return typeof obj === 'string';
};

Type.prototype.isDate = function(obj) {
  return obj instanceof Date;
};

Type.prototype.isNull = function(obj) {
  return obj === null;
};

Type.prototype.isNullOrUndefined = function(obj) {
  return (obj === null || obj === undefined);
};

Type.prototype.isUndefined = function(obj) {
  return obj === void 0;
};

Type.prototype.isA = function(obj, type) {
  var clas = Object.prototype.toString.call(obj).slice(8, -1);
  return (obj !== undefined) && (obj !== null) && (clas === type);
};

Type.prototype.isEmptyObject = function(obj) {
  for (var property in obj) {
    if (obj.hasOwnProperty(property)) {
      return false;
    }
  }
  return true;
};

Type.prototype.hasProperty = function(obj, prop) {
  return prop in obj;
};

Type.prototype.arrayContains = function(arr, value) {
  return (arr.indexOf(value) > -1);
};

Type.prototype.getValues = function(obj) {
  var ret = [];
  for (var key in obj) {
    if (obj.hasOwnProperty(key)) {
      ret.push(obj[key]);
    }
  }
  return ret;
};

var _type = new Type();



/////////////////////////////////////////////////////////////////////////////
/** @constructor */
var Converter = function() {};

function _nullableGeneric(func, nullable, val) {
  if (_type.isNull(val) && nullable === true) {
    return val;
  } else {
    return func.apply(null, [].slice.call(arguments, 2));
  }
}

function _toBoolean(val) {
  return Boolean(val);
}

Converter.prototype.toBoolean = function(val, nullable) {
  return _nullableGeneric(_toBoolean, nullable, val);
};

function _toLong(val) {
  var ret = parseInt(val, 10);
  return isNaN(ret) ? (val === true ? 1 : 0) : ret;
}

Converter.prototype.toLong = function(val, nullable) {
  return _nullableGeneric(_toLong, nullable, val);
};

function _toLongLong(val) {
  // TODO: how to implement this?
  return _toLong(val);
}

Converter.prototype.toLongLong = function(val, nullable) {
  return _nullableGeneric(_toLongLong, nullable, val);
};

function _toUnsignedLong(val) {
  return _toLong(val) >>> 0;
}

Converter.prototype.toUnsignedLong = function(val, nullable) {
  return _nullableGeneric(_toUnsignedLong, nullable, val);
};

function _toUnsignedLongLong(val) {
  // TODO: how to implement this?
  return _toUnsignedLong(val);
}

Converter.prototype.toUnsignedLongLong = function(val, nullable) {
  return _nullableGeneric(_toUnsignedLongLong, nullable, val);
};

function _toByte(val) {
  return ((_toLong(val) + 128) & 0xFF) - 128;
}

Converter.prototype.toByte = function(val, nullable) {
  return _nullableGeneric(_toByte, nullable, val);
};

function _toOctet(val) {
  return _toLong(val) & 0xFF;
}

Converter.prototype.toOctet = function(val, nullable) {
  return _nullableGeneric(_toOctet, nullable, val);
};

function _toDouble(val) {
  var ret = Number(val);
  if (isNaN(ret) || !isFinite(ret)) {
    throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR,
        'Cannot convert ' + String(val) + ' to double.');
  }
  return ret;
}

Converter.prototype.toDouble = function(val, nullable) {
  return _nullableGeneric(_toDouble, nullable, val);
};

function _toString(val) {
  return String(val);
}

Converter.prototype.toString = function(val, nullable) {
  return _nullableGeneric(_toString, nullable, val);
};

function _toPlatformObject(val, types) {
  var v;
  var t;
  if (_type.isArray(val)) {
    v = val;
  } else {
    v = [val];
  }

  if (_type.isArray(types)) {
    t = types;
  } else {
    t = [types];
  }
  var match = false;
  for (var i = 0; i < t.length; ++i) {
    for (var j = 0; j < v.length; ++j) {
      match = match || (v[j] instanceof t[i]);
    }
  }
  if (match) {
    return val;
  }

  throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR,
      'Cannot convert ' + String(val) + ' to ' + String(t[0].name) + '.');
}

Converter.prototype.toPlatformObject = function(val, types, nullable) {
  return _nullableGeneric(_toPlatformObject, nullable, val, types);
};

function _toFunction(val) {
  if (_type.isFunction(val)) {
    return val;
  }

  throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR,
      'Cannot convert ' + String(val) + ' to function.');
}

Converter.prototype.toFunction = function(val, nullable) {
  return _nullableGeneric(_toFunction, nullable, val);
};

function _toArray(val) {
  if (_type.isArray(val)) {
    return val;
  }

  throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR,
      'Cannot convert ' + String(val) + ' to array.');
}

Converter.prototype.toArray = function(val, nullable) {
  return _nullableGeneric(_toArray, nullable, val);
};

function _toDictionary(val) {
  if (_type.isObject(val) || _type.isFunction(val)) {
    return val;
  }

  throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR,
      'Cannot convert ' + String(val) + ' to dictionary.');
}

Converter.prototype.toDictionary = function(val, nullable) {
  return _nullableGeneric(_toDictionary, nullable, val);
};

function _toEnum(val, e) {
  var v = _toString(val);
  if (_type.arrayContains(e, v)) {
    return v;
  }

  throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR,
      'Cannot convert ' + v + ' to enum.');
}

Converter.prototype.toEnum = function(val, e, nullable) {
  return _nullableGeneric(_toEnum, nullable, val, e);
};

var _converter = new Converter();

/////////////////////////////////////////////////////////////////////////////
/** @constructor */
var Validator = function() {
  this.Types = {
    BOOLEAN: 'BOOLEAN',
    LONG: 'LONG',
    LONG_LONG: 'LONG_LONG',
    UNSIGNED_LONG: 'UNSIGNED_LONG',
    UNSIGNED_LONG_LONG: 'UNSIGNED_LONG_LONG',
    BYTE: 'BYTE',
    OCTET: 'OCTET',
    DOUBLE: 'DOUBLE',
    STRING: 'STRING',
    FUNCTION: 'FUNCTION',
    DICTIONARY: 'DICTIONARY',
    PLATFORM_OBJECT: 'PLATFORM_OBJECT',
    LISTENER: 'LISTENER',
    ARRAY: 'ARRAY',
    ENUM: 'ENUM'
  };
};


/**
 * Verifies if arguments passed to function are valid.
 *
 * Description of expected arguments.
 * This is an array of objects, each object represents one argument.
 * First object in this array describes first argument, second object describes second
 * argument, and so on.
 * Object describing an argument needs to have two properties:
 *   - name - name of the argument,
 *   - type - type of the argument, only values specified in Validator.Types are allowed.
 * Other properties, which may appear:
 *   - optional - if set to value which evaluates to true, argument is optional
 *   - nullable - if set to to true, argument may be set to null
 *   - values - required in case of some objects, value depends on type
 *   - validator - function which accepts a single parameter and returns true or false;
 *                 if this property is present, this function will be executed,
 *                 argument converted to expected type is going to be passed to this function
 *
 * @param {Array} a - arguments of a method
 * @param {Array} d - description of expected arguments
 * @return {Object} which holds all available arguments.
 * @throws TypeMismatchError if arguments are not valid
 *
 * @code
 * [
 *   {
 *     name: 'first',
 *     type: 'aType'
 *   }
 * ]
 * @code
 * [
 *   {
 *     name: 'first',
 *     type: 'aType',
 *     optional: true
 *   }
 * ]
 * @code
 * [
 *   {
 *     name: 'first',
 *     type: 'aType',
 *     nullable: true
 *   }
 * ]
 * @code
 * [
 *   {
 *     name: 'first',
 *     type: 'aType',
 *     optional: true,
 *     nullable: true
 *   }
 * ]
 * @code
 * [
 *   {
 *     name: 'first',
 *     type: Validator.Types.PLATFORM_OBJECT,
 *     values: ApplicationControl // type of platform object
 *   }
 * ]
 * @code
 * [
 *   {
 *     name: 'first',
 *     type: Validator.Types.PLATFORM_OBJECT,
 *     values: [Alarm, AlarmRelative, AlarmAbsolute] // accepted types
 *   }
 * ]
 * @code
 * [
 *   {
 *     name: 'first',
 *     type: Validator.Types.LISTENER,
 *     values: ['onsuccess', 'onfailure'] // array of callbacks' names
 *   }
 * ]
 * @code
 * [
 *   {
 *     name: 'first',
 *     type: Validator.Types.ARRAY,
 *     values: ApplicationControlData // type of each element in array,
 *                                    // tested with instanceof
 *   }
 * ]
 * @code
 * [
 *   {
 *     name: 'first',
 *     type: Validator.Types.ARRAY,
 *     values: Validator.Types.DOUBLE // converts elements, only primitive types are supported
 *   }
 * ]
 * @code
 * [
 *   {
 *     name: 'first',
 *     type: Validator.Types.ENUM,
 *     values: ['SCREEN_DIM', 'SCREEN_NORMAL', 'CPU_AWAKE'] // array of allowed values
 *   }
 * ]
 */
Validator.prototype.validateArgs = function(a, d) {
  var args = {has: {}};

  for (var i = 0; i < d.length; ++i) {
    var name = d[i].name;
    args.has[name] = (i < a.length);

    var optional = d[i].optional;
    var nullable = d[i].nullable;
    var val = a[i];

    if (args.has[name] || !optional) {
      var type = d[i].type;
      var values = d[i].values;

      switch (type) {
        case this.Types.BOOLEAN:
          val = _converter.toBoolean(val, nullable);
          break;

        case this.Types.LONG:
          val = _converter.toLong(val, nullable);
          break;

        case this.Types.LONG_LONG:
          val = _converter.toLongLong(val, nullable);
          break;

        case this.Types.UNSIGNED_LONG:
          val = _converter.toUnsignedLong(val, nullable);
          break;

        case this.Types.UNSIGNED_LONG_LONG:
          val = _converter.toUnsignedLongLong(val, nullable);
          break;

        case this.Types.BYTE:
          val = _converter.toByte(val, nullable);
          break;

        case this.Types.OCTET:
          val = _converter.toOctet(val, nullable);
          break;

        case this.Types.DOUBLE:
          val = _converter.toDouble(val, nullable);
          break;

        case this.Types.STRING:
          val = _converter.toString(val, nullable);
          break;

        case this.Types.FUNCTION:
          val = _converter.toFunction(val, nullable);
          break;

        case this.Types.DICTIONARY:
          val = _converter.toDictionary(val, nullable);
          break;

        case this.Types.PLATFORM_OBJECT:
          val = _converter.toPlatformObject(val, values, nullable);
          break;

        case this.Types.LISTENER:
          if (_type.isNull(val)) {
            if (!nullable) {
              throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR,
                  'Argument "' + name + '" cannot be null.');
            }
          } else {
            if (!_type.isObject(val)) {
              throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR,
                  'Argument "' + name + '" should be an object.');
            }
            for (var ii = 0; ii < values.length; ++ii) {
              if (_type.hasProperty(val, values[ii])) {
                val[values[ii]] = _converter.toFunction(val[values[ii]], false);
              }
            }
          }
          break;

        case this.Types.ARRAY:
          val = _converter.toArray(val, nullable);
          if (!_type.isNull(val) && values) {
            var func;

            switch (values) {
              case this.Types.BOOLEAN:
                func = _converter.toBoolean;
                break;

              case this.Types.LONG:
                func = _converter.toLong;
                break;

              case this.Types.LONG_LONG:
                func = _converter.toLongLong;
                break;

              case this.Types.UNSIGNED_LONG:
                func = _converter.toUnsignedLong;
                break;

              case this.Types.UNSIGNED_LONG_LONG:
                func = _converter.toUnsignedLongLong;
                break;

              case this.Types.BYTE:
                func = _converter.toByte;
                break;

              case this.Types.OCTET:
                func = _converter.toOctet;
                break;

              case this.Types.DOUBLE:
                func = _converter.toDouble;
                break;

              case this.Types.STRING:
                func = _converter.toString;
                break;

              default:
                func = function(val) {
                  if (!(val instanceof values)) {
                    throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR,
                        'Items of array "' + name + '" should be of type: ' + values + '.');
                  }
                  return val;
                };
            }

            for (var j = 0; j < val.length; ++j) {
              val[j] = func(val[j]);
            }
          }
          break;

        case this.Types.ENUM:
          val = _converter.toEnum(val, values, nullable);
          break;

        default:
          throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR,
              'Unknown type: "' + type + '".');
      }

      var _validator = d[i].validator;

      if (_type.isFunction(_validator) && !_validator(val)) {
        throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR,
            'Argument "' + name + '" did not pass additional validation.');
      }

      args[name] = val;
    }
  }

  return args;
};


/**
 * @deprecated Use validateArgs() instead.
 */
Validator.prototype.validateMethod = function(a, d) {
  return this.validateArgs(a, d);
};


/**
 * Use this helper to ensure that constructor is invoked by "new" operator.
 *
 * @param {Object} obj
 * @param {Function} instance
 */
Validator.prototype.isConstructorCall = function(obj, instance) {
  if (!(obj instanceof instance) || obj._previouslyConstructed) {
    // There is no TypeError exception in Tizen 2.3.0 API spec but it's required by current TCTs.
    // For Tizen compliance it's wrapped into WebAPIException.
    throw new WebAPIException('TypeError', 'Constructor cannot be called as function.');
  }

  Object.defineProperty(obj, '_previouslyConstructed', {
    value: true,
    writable: false,
    enumerable: false
  });
};


/**
 * @deprecated Use isConstructorCall() instead.
 */
Validator.prototype.validateConstructorCall = function(obj, instance) {
  this.isConstructorCall(obj, instance);
};

var _validator = new Validator();



/////////////////////////////////////////////////////////////////////////////
/** @constructor */
var NativeManager = function(extension) {

  /**
   * @type {string}
   * @const
   */
  this.CALLBACK_ID_KEY = 'callbackId';

  /**
   * @type {string}
   * @const
   */
  this.LISTENER_ID_KEY = 'listenerId';

  /**
   * @type {Object}
   * @private
   */
  var extension_ = extension;

  /**
   * @type {number}
   * @private
   */
  var replyId_ = 0;

  /**
   * Map of async reply callbacks.
   *
   * @type {Object.<number, function>}
   * @protected
   */
  this.callbacks_ = {};

  /**
   * Map of registered listeners.
   *
   * @type {Object.<string, function>}
   * @protected
   */
  this.listeners_ = {};

  _validator.isConstructorCall(this, NativeManager);

  // TODO: Remove mockup if WRT implements sendRuntimeMessage
  // This is temporary mockup!
  extension.sendRuntimeMessage = extension.sendRuntimeMessage || function() {
    console.error('Runtime did not implement extension.sendRuntimeMessage!');
    throw new WebAPIException(WebAPIException.UNKNOWN_ERR,
        'Runtime did not implement extension.sendRuntimeMessage!');
  };

  extension.sendRuntimeAsyncMessage = extension.sendRuntimeAsyncMessage || function() {
    console.error('Runtime did not implement extension.sendRuntimeAsyncMessage!');
    throw new WebAPIException(WebAPIException.UNKNOWN_ERR,
        'Runtime did not implement extension.sendRuntimeAsyncMessage!');
  };

  extension.sendRuntimeSyncMessage = extension.sendRuntimeSyncMessage || function() {
    console.error('Runtime did not implement extension.sendRuntimeSyncMessage!');
    throw new WebAPIException(WebAPIException.UNKNOWN_ERR,
        'Runtime did not implement extension.sendRuntimeSyncMessage!');
  };

  // check extension prototype
  if (!extension || !extension.internal ||
      !_type.isFunction(extension.postMessage) ||
      !_type.isFunction(extension.internal.sendSyncMessage) ||
      !_type.isFunction(extension.sendSyncData) ||
      !_type.isFunction(extension.sendRuntimeMessage) ||
      !_type.isFunction(extension.sendRuntimeAsyncMessage) ||
      !_type.isFunction(extension.sendRuntimeSyncMessage) ||
      !_type.isFunction(extension.setMessageListener)) {
    throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR,
                              'Wrong extension object passed');
  }

  Object.defineProperties(this, {
    nextReplyId: {
      get: function() {
        return ++replyId_;
      },
      enumerable: false
    },
    extension: {
      get: function() {
        return extension_;
      },
      enumerable: true
    }
  });

  extension_.setMessageListener(function(json) {
    var msg = JSON.parse(json);
    var id;

    if (msg.hasOwnProperty(this.CALLBACK_ID_KEY)) {
      id = msg[this.CALLBACK_ID_KEY];
      delete msg[this.CALLBACK_ID_KEY];

      if (!_type.isFunction(this.callbacks_[id])) {
        console.error('Wrong callback identifier. Ignoring message.');
        return;
      }

      var f = this.callbacks_[id];
      setTimeout(function() {
        try {
          f(msg);
        } catch (e) {
          console.error('########## exception');
          console.error(e);
        }
      }, 0);
      delete this.callbacks_[id];

      return;
    }

    if (msg.hasOwnProperty(this.LISTENER_ID_KEY)) {
      id = msg[this.LISTENER_ID_KEY];
      delete msg[this.LISTENER_ID_KEY];

      if (!_type.isFunction(this.listeners_[id])) {
        console.error('Wrong listener identifier. Ignoring message.');
        return;
      }

      var f = this.listeners_[id];
      setTimeout(function() {
        try {
          f(msg);
        } catch (e) {
          console.error('########## exception');
          console.error(e);
        }
      }, 0);

      return;
    }

    console.error('Missing callback or listener identifier. Ignoring message.');

  }.bind(this));
};

NativeManager.prototype.call = function(cmd, args, callback) {
  args = args || {};

  var replyId = this.nextReplyId;
  args[this.CALLBACK_ID_KEY] = replyId;
  this.callbacks_[replyId] = callback;

  return this.callSync(cmd, args);
};

NativeManager.prototype.callData = function(cmd, args, chunk, callback) {
  args = args || {};

  var replyId = this.nextReplyId;
  args[this.CALLBACK_ID_KEY] = replyId;
  this.callbacks_[replyId] = callback;

  return this.callSyncData(cmd, args, chunk);
};

NativeManager.prototype.callSync = function(cmd, args) {
  var request = JSON.stringify({
    cmd: cmd,
    args: args || {}
  });

  return JSON.parse(this.extension.internal.sendSyncMessage(request));
};

NativeManager.prototype.callSyncData = function(cmd, args, type, chunk) {
  if (!type) type = "string";
  var request = JSON.stringify({
    cmd: cmd,
    args: args || {}
  });
  var response = this.extension.sendSyncData(request, chunk);
  response.reply = JSON.parse(response.reply);
  response.output = this.extension.receiveChunkData(response.chunk_id, type);
  return response;
};

NativeManager.prototype.sendRuntimeMessage = function(msg, body) {
  return this.extension.sendRuntimeMessage(msg, body || '');
};

NativeManager.prototype.sendRuntimeAsyncMessage = function(msg, body, callback) {
  var handler = function(response) {
    if (_type.isFunction(callback)) {
      var result = {};
      if ('success' === response.toLowerCase()) {
        result.status = 'success';
      } else {
        result.status = 'error';
        result.error = new WebAPIException(WebAPIException.UNKNOWN_ERR,
                                           'Runtime message failure');
      }
      callback(result);
    }
  };
  return this.extension.sendRuntimeAsyncMessage(msg, body || '', handler);
};

NativeManager.prototype.sendRuntimeSyncMessage = function(msg, body) {
  return this.extension.sendRuntimeSyncMessage(msg, body || '');
};

NativeManager.prototype.addListener = function(name, callback) {
  if (!_type.isString(name) || !name.length) {
    throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR);
  }

  this.listeners_[name] = callback;
};

NativeManager.prototype.removeListener = function(name) {
  if (this.listeners_.hasOwnProperty(name)) {
    delete this.listeners_[name];
  }
};

NativeManager.prototype.isListenerSet = function(name) {
  return this.listeners_.hasOwnProperty(name);
};

NativeManager.prototype.isSuccess = function(result) {
  return (result.status !== 'error');
};

NativeManager.prototype.isFailure = function(result) {
  return !this.isSuccess(result);
};

NativeManager.prototype.getResultObject = function(result) {
  return result.result;
};

NativeManager.prototype.getErrorObject = function(result) {
  return new WebAPIException(result.error);
};

NativeManager.prototype.callIfPossible = function(callback) {
  if (!_type.isNullOrUndefined(callback)) {
    callback.apply(callback, [].slice.call(arguments, 1));
  }
};

/*
 *bridge is a two way communication interface
 *Example usage:
 *var bridge = new NativeBridge(extension);
 *    To send sync method:
 *    var result = bridge.sync({
 *        cmd: 'my_cpp_function_symbol',
 *        args: {
 *            name: 'My name',
 *            age: 28
 *        }
 *    });
 *    console.log(result);
 *
 *    To send async method and handle response:
 *    bridge.async({
 *        cmd: 'my_cpp_function_symbol',
 *        args: {
 *            name: 'My name'
 *        }
 *    }).then({
 *        success: function (data) {
 *            var age = data.age;
 *            args.successCallback(age);
 *        },
 *        error: function (e) {...},
 *        someCallback: function (data) {...}
 *    });
 *bridge.async will add special param to passed data called cid
 *that param need to be kept and returned with respons
 *To determine which callback should be invoked, response should
 *contain "action" param. Value of "action" param indicates name of
 *triggered callback.
 *Callbask are removed from listenr by defoult to prevent that behaviour
 *param "keep" should be assigned to value true
 *Example of c++ async response:
 *    Simple succes with data:
 *    {
 *        cid: 23,
 *        action: 'success',
 *        args: {
 *            age: 23
 *        }
 *    }
 *    More complicated example:
 *    {
 *        cid: 23,
 *        action: 'progress',
 *        keep: true,
 *        args: {
 *            age: 23
 *        }
 *    }
 */
var NativeBridge = (function (extension, debug) {
    debug = !!debug;
    var Callbacks = (function () {
        var _collection = {};
        var _cid = 0;
        var _next = function () {
            return (_cid += 1);
        };

        var CallbackManager = function () {};

        CallbackManager.prototype = {
            add: function (/*callbacks, cid?*/) {
                if (debug) console.log('bridge.CallbackManager.add');
                var args = Array.prototype.slice.call(arguments);
                var c = args.shift();
                var cid = args.pop();
                if (cid) {
                    if (c !== null && typeof c === 'object') {
                        for (var key in c) {
                            if (c.hasOwnProperty(key)) _collection[cid][key] = c[key];
                        }
                    }
                } else {
                    cid = _next();
                    _collection[cid] = c;
                }
                return cid;
            },
            remove: function (cid) {
                if (debug)  console.log('bridge.CallbackManager.remove, cid: ' + cid);
                if (_collection[cid]) delete _collection[cid];
            },
            call: function (cid, key, args, keep) {
                if (debug) console.log('bridge.CallbackManager.call, cid: '+ cid + ', key: ' + key);
                var callbacks = _collection[cid];
                keep = !!keep;
                if (callbacks) {
                    var fn = callbacks[key];
                    if (fn) {
                        fn.apply(null, args);
                        if (!keep) this.remove(cid)
                    }
                }
            }
        };

        return {
            getInstance: function () {
                return this.instance || (this.instance = new CallbackManager);
            }
        };
    })();


    var Listeners = (function () {
        var _listeners = {};
        var _id = 0;
        var _next = function () {
            return (_id += 1);
        };

        var ListenerManager = function () {};

        ListenerManager.prototype = {
            add: function (l) {
                if (debug) console.log('bridge.ListenerManager.add');
                var id = _next();
                _listeners[id] = l;
                return id;
            },
            resolve: function (id, action, data, keep) {
                if (debug) console.log('bridge.ListenerManager.resolve, id: ' + id + ', action: ' + action);
                keep = !!keep;
                var l = _listeners[id];
                if (l) {
                    var cm = Callbacks.getInstance();
                    cm.call(l.cid, action, [data], keep);
                }
                return l;
            },
            remove: function (id) {
                if (debug) console.log('bridge.ListenerManager.remove, id: ' + id);
                var l = _listeners[id];
                if (l) {
                    var cm = Callbacks.getInstance();
                    if (l.cid) cm.remove(l.cid);
                    delete _listeners[id];
                }
            },
            attach: function (id, key, value) {
                if (_listeners[id]) {
                    _listeners[id][key] = value;
                    return true;
                }
                return false;
            },
            find: function (key, value) {
                var result = [];
                for (var p in _listeners) {
                    if (_listeners.hasOwnProperty(p)) {
                        var l = _listeners[p];
                        if (l[key] === value) result.push({id: p, listener: l});
                    }
                }
                return result;
            }
        }

        return {
            getInstance: function () {
                return this.instance || (this.instance = new ListenerManager);
            }
        };
    })();

    var Listener = function () {
        if (debug) console.log('bridge: Listener constructor');
        this.cid = null;
    };
    Listener.prototype = {
        then: function (c) {
            if (debug) console.log('bridge.Listener.then');
            var cm = Callbacks.getInstance();
            this.cid = cm.add(c, this.cid);
            return this;
        }
    };

    var Bridge = function () {};
    Bridge.prototype = {
        sync: function (data) {
            var json = JSON.stringify({
              cmd: data.cmd,
              args: data
            });
            if (debug) console.log('bridge.sync, json: ' + json);
            var result = extension.internal.sendSyncMessage(json);
            var obj = JSON.parse(result);
            if (obj.error)
                throw new WebAPIException(obj.code, obj.name, obj.message);
            return obj.result;
        },
        async: function (data) {
            var l = new Listener();
            data.cid = Listeners.getInstance().add(l);
            var json = JSON.stringify({
                cmd: data.cmd,
                args: data
            });
            if (debug) console.log('bridge.async, json: ' + json);
            setTimeout(function () {
                extension.postMessage(json);
            });
            return l;
        },
        listener: function (c) {
            var l = (new Listener()).then(c);
            var cid = Listeners.getInstance().add(l);
            return cid;
        },
        attach: function (id, key, value) {
            return Listeners.getInstance().attach(id, key, value);
        },
        find: function (key, value) {
            return Listeners.getInstance().find(key, value);
        },
        remove: function (id) {
            Listeners.getInstance().remove(id);
        }
    };

    extension.setMessageListener(function (json) {
        /*
         *Expected response:
         *{
         *    cid: 23,                        // callback id
         *    action: 'success',              // expected callback action
         *    keep: false                     // optional param
         *    args: {...}                     // data pased to callback
         *}
         */

        if (debug) console.log('bridge.setMessageListener, json: ' + json);
        var data = JSON.parse(json);
        if (data.cid && data.action) {
            setTimeout(function() {
                Listeners.getInstance().resolve(data.cid, data.action, data.args, data.keep);
            }, 0);
        }
    });

    return new Bridge;
});

// WebAPIException and WebAPIError definition moved to Utils for compliance
// reasons with blink-wrt environment.
// In blink-wrt the original Tizen module is loaded, which is not providing exception constructor.
// As modules needs exceptions internally so they are loaded here for now.
// See http://168.219.209.56/gerrit/#/c/23472/ for more details.
// In future exception definition could be moved back to Tizen module.
function __isObject(object) {
  return object instanceof _global.Object;
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

  if (code_ > errors.DATA_CLONE_ERR) {
    code_ = 0;
  }

  // attributes
  Object.defineProperties(this, {
    code: {value: code_, writable: false, enumerable: true},
    name: {value: name_, writable: false, enumerable: true},
    message: {value: message_, writable: false, enumerable: true}
  });

  this.constructor.prototype.__proto__ = Error.prototype;
  Error.captureStackTrace && Error.captureStackTrace(this, this.constructor); // V8-specific code
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


// Export WebAPIException and WebAPIError into global scope.
// For compliance reasons their constructors should not be exported in tizen namespace,
// but should be available internally to allow throwing exceptions from modules.
var scope;
if (typeof window !== 'undefined') {
  scope = window;
} else if(typeof global !== 'undefined') {
  scope = global;
}
scope = scope || {};
scope.WebAPIException = WebAPIException;
scope.WebAPIError = WebAPIException;

Utils.prototype.dateConverter = _dateConverter;
Utils.prototype.type = _type;
Utils.prototype.converter = _converter;
Utils.prototype.validator = _validator;
Utils.prototype.NativeManager = NativeManager;
Utils.prototype.NativeBridge = NativeBridge;

var native_ = new NativeManager(extension);

exports = new Utils();
