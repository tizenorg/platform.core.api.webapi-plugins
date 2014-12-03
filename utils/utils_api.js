// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var signature_to_type = { 'n': 'number',
                          'f': 'function',
                          'b': 'boolean',
                          's': 'string',
                          'o': 'object'
                        };

/** @constructor */
function Utils() {
}

/** @deprecated */
Utils.prototype.validateArguments_old = function(signature, args) {
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

/** @private */
var converter_ = {
  'boolean': Boolean,
  'number': Number,
  'string': String,
  'object': function(val, ext) {
    var t = Array.isArray(ext) ? ext : [ext];
    var match = false;

    for (var i = 0; i < t.length; ++i) {
      match = match || val instanceof t[i];
    }

    if (!match)
      throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR);

    return val;  
  },
  'function': function(val) {
    if (typeof val !== 'function')
      throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR);

    return val;
  },
  'array': function(val, ext) {
    if (!Array.isArray(val))
      throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR);

    if (ext) {
      var func = converter_[ext];
      if (func) {
        for (var i = 0; i < val.length; ++i) {
          val[i] = func(val[i]);
        }
      }
    }

    return val;
  },
  'long': function (val) {
    var ret = parseInt(val, 10);
    return isNaN(ret) ? (val === true ? 1 : 0) : ret;
  },
  'unsigned long': function (val) {
    var ret = converter_['long'](val);
    return ret >>> 0;
  },
  'double': function (val) {
    var ret = Number(val);
    if (isNaN(ret) || !isFinite(ret)) {
      throw new "TypeMismatch";
    }

    return ret;
  }
};

/**
 * Verifies if arguments passed to function are valid.
 * @param {!Array.<*>} a arguments of a method
 * @param {!Array.<{name:string, type:string, nullable:boolean, optional:boolean}>}
 *    Description of expected aurguments.
 * @return {Array.<?>} Object which holds all converted arguments.
 */
Utils.prototype.validateArguments = function(a, d) {

    // TODO: remove this
    // for compatibility
    if (typeof a === 'string') {
      return validateArguments_old(a, d);
    }

    var args = { has : {} };

    for (var i = 0; i < d.length; ++i) {
        var name = d[i].name;
        args.has[name] = (i < a.length);

        var optional = d[i].optional;
        var nullable = d[i].nullable;
        var val = a[i];

        if (args.has[name] || !optional) {
            var type = d[i].type;
            var ext = d[i].ext;

            if (val !== null || !nullable) {
              var func = converter_[type];
              if (func) {
                val = func(val, ext);
              }
            }

            /* Does it needed?
            var validator = d[i].validator;

            if (validator && !validator(val)) {
              throw new "TypeMismatch";
              //  _throwTypeMismatch('Argument "' + name +
              //          '" did not pass additional validation.');
            }
            */

            args[name] = val;
        }
    }

    return args;
};

exports = new Utils();
