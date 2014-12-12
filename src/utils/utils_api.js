/* global tizen */

// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


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



/** @constructor */
function Utils() {}


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



/////////////////////////////////////////////////////////////////////////////
/** @constructor */
var Type = function() {};

Type.prototype.isBoolean = function(obj) {
  return typeof obj === 'boolean';
};

Type.prototype.isObject = function(obj) {
  return obj instanceof Object;
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
    throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR,
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
  var t;
  if (_type.isArray(types)) {
    t = types;
  } else {
    t = [types];
  }
  var match = false;
  for (var i = 0; i < t.length; ++i) {
    match = match || (val instanceof t[i]);
  }
  if (match) {
    return val;
  }

  throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR,
      'Cannot convert ' + String(val) + ' to ' + String(t[0].name) + '.');
}

Converter.prototype.toPlatformObject = function(val, types, nullable) {
  return _nullableGeneric(_toPlatformObject, nullable, val, types);
};

function _toFunction(val) {
  if (_type.isFunction(val)) {
    return val;
  }

  throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR,
      'Cannot convert ' + String(val) + ' to function.');
}

Converter.prototype.toFunction = function(val, nullable) {
  return _nullableGeneric(_toFunction, nullable, val);
};

function _toArray(val) {
  if (_type.isArray(val)) {
    return val;
  }

  throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR,
      'Cannot convert ' + String(val) + ' to array.');
}

Converter.prototype.toArray = function(val, nullable) {
  return _nullableGeneric(_toArray, nullable, val);
};

function _toDictionary(val) {
  if (_type.isObject(val) || _type.isFunction(val)) {
    return val;
  }

  throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR,
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

  throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR,
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
              throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR,
                  'Argument "' + name + '" cannot be null.');
            }
          } else {
            if (!_type.isObject(val)) {
              throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR,
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
                    throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR,
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
          throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR,
              'Unknown type: "' + type + '".');
      }

      var _validator = d[i].validator;

      if (_type.isFunction(_validator) && !_validator(val)) {
        throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR,
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
    throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR,
        'Constructor cannot be called as function.');
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

  // check extension prototype
  if (!extension || !extension.internal ||
      !_type.isFunction(extension.postMessage) ||
      !_type.isFunction(extension.internal.sendSyncMessage) ||
      !_type.isFunction(extension.setMessageListener)) {
    throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR,
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

      this.callbacks_[id](msg);
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

      this.listeners_[id](msg);

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

  var request = JSON.stringify({
    cmd: cmd,
    args: args
  });

  this.extension.postMessage(request);
};

NativeManager.prototype.callSync = function(cmd, args) {
  var request = JSON.stringify({
    cmd: cmd,
    args: args || {}
  });

  return JSON.parse(this.extension.internal.sendSyncMessage(request));
};

NativeManager.prototype.addListener = function(name, callback) {
  if (!_type.isString(name) || !name.length) {
    throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR);
  }

  this.listeners_[name] = callback;
};

NativeManager.prototype.removeListener = function(name) {
  if (this.listeners_.hasOwnProperty(name)) {
    delete this.listeners_[name];
  }
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
  return new tizen.WebAPIException(result.error.code, result.error.message, result.error.name);
};


Utils.prototype.type = _type;
Utils.prototype.converter = _converter;
Utils.prototype.validator = _validator;
Utils.prototype.NativeManager = NativeManager;

exports = new Utils();
