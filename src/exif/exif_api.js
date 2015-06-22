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

var _global = window || global || {};
 
var validator_ = xwalk.utils.validator;
var type_ = xwalk.utils.type;
var native_ = new xwalk.utils.NativeManager(extension);
var converter_ = xwalk.utils.converter;

function ExifManager() {}

var ImageContentOrientation = {
  NORMAL: 'NORMAL',
  FLIP_HORIZONTAL: 'FLIP_HORIZONTAL',
  ROTATE_180: 'ROTATE_180',
  FLIP_VERTICAL: 'FLIP_VERTICAL',
  TRANSPOSE: 'TRANSPOSE',
  ROTATE_90: 'ROTATE_90',
  TRANSVERSE: 'TRANSVERSE',
  ROTATE_270: 'ROTATE_270'
};

var ExposureProgram = {
  NOT_DEFINED: 'NOT_DEFINED',
  MANUAL: 'MANUAL',
  NORMAL: 'NORMAL',
  APERTURE_PRIORITY: 'APERTURE_PRIORITY',
  SHUTTER_PRIORITY: 'SHUTTER_PRIORITY',
  CREATIVE_PROGRAM: 'CREATIVE_PROGRAM',
  ACTION_PROGRAM: 'ACTION_PROGRAM',
  PORTRAIT_MODE: 'PORTRAIT_MODE',
  LANDSCAPE_MODE: 'LANDSCAPE_MODE'
};

var WhiteBalanceMode = {
  AUTO: 'AUTO',
  MANUAL: 'MANUAL'
};

var propertiesList = {
  URI: 'uri',
  WIDTH: 'width',
  HEIGHT: 'height',
  DEVICE_MAKER: 'deviceMaker',
  DEVICE_MODEL: 'deviceModel',
  ORIGINAL_TIME: 'originalTime',
  ORIENTATION: 'orientation',
  FNUMBER: 'fNumber',
  ISO_SPEED_RATINGS: 'isoSpeedRatings',
  EXPOSURE_TIME: 'exposureTime',
  EXPOSURE_PROGRAM: 'exposureProgram',
  FLASH: 'flash',
  FOCAL_LENGTH: 'focalLength',
  WHITE_BALANCE: 'whiteBalance',
  GPS_LOCATION: 'gpsLocation',
  GPS_ALTITUDE: 'gpsAltitude',
  GPS_PROCESSING_METHOD: 'gpsProcessingMethod',
  GPS_TIME: 'gpsTime',
  USER_COMMENT: 'userComment'
};

var URI_ABSOLUTE_PREFIX = "file:///";

function _isValidAbsoluteURI(uri) {
  return 0 === uri.indexOf(URI_ABSOLUTE_PREFIX);
}

function _getJsonFromExifInformation(exifInfo) {
  var json = {};

  for (var prop in propertiesList) {
    var propName = propertiesList[prop];

    if (exifInfo[propName] !== null) {
      if (propName === 'originalTime') {
        json[propName] = Math.floor(exifInfo[propName].getTime() / 1000);
      } else if (propName === 'gpsTime') {
        var str = exifInfo[propName].toLocaleString().split(', ');
        var res = str[1] + ', ' + str[2] + ', ' + str[3];

        json[propName] = Math.floor(Date.parse(res) / 1000);
      } else {
        json[propName] = exifInfo[propName];
      }
    }
  }

  return json;
}

function _calculateDegDecimal(degrees, minutes, seconds) {
  return parseInt(degrees) + parseInt(minutes) / 60.0 + parseInt(seconds) / 3600.0;
}

function _calculateExifInfo(exifInfoNative) {
  // copy all properties that share name from
  // exifInfoNative to exifInfo
  var exifInfo = new tizen.ExifInformation(exifInfoNative);

  // copy all remaining properties that do not share name or need extra calculations
  if (exifInfoNative.originalTimeSeconds) {
    exifInfo.originalTime = new Date(exifInfoNative.originalTimeSeconds * 1000);
  }

  if (parseInt(exifInfoNative.whiteBalanceValue) === 0) {  // 0=AUTO
    exifInfo.whiteBalance = WhiteBalanceMode.AUTO;
  } else if (parseInt(exifInfoNative.whiteBalanceValue) === 1) {  // 1=MANUAL
    exifInfo.whiteBalance = WhiteBalanceMode.MANUAL;
  }

  // gpsLocation
  if (exifInfoNative.gpsLatitudeDegrees &&
      exifInfoNative.gpsLongitudeDegrees &&
      exifInfoNative.gpsLatitudeRef &&
      exifInfoNative.gpsLongitudeRef) {
    exifInfo.gpsLocation = new tizen.SimpleCoordinates();
    exifInfo.gpsLocation.latitude = _calculateDegDecimal(exifInfoNative.gpsLatitudeDegrees,
        exifInfoNative.gpsLatitudeMinutes, exifInfoNative.gpsLatitudeSeconds);
    exifInfo.gpsLocation.longitude = _calculateDegDecimal(exifInfoNative.gpsLongitudeDegrees,
        exifInfoNative.gpsLongitudeMinutes, exifInfoNative.gpsLongitudeSeconds);

    if (exifInfoNative.gpsLatitudeRef === 'SOUTH') {
      exifInfo.gpsLocation.latitude = -exifInfo.gpsLocation.latitude;
    } else if (exifInfoNative.gpsLatitudeRef !== 'NORTH') {
      exifInfo.gpsLocation.latitude = null;  // invalid gpsLatitudeRef
    }

    if (exifInfoNative.gpsLongitudeRef === 'WEST') {
      exifInfo.gpsLocation.longitude = -exifInfo.gpsLocation.longitude;
    } else if (exifInfoNative.gpsLongitudeRef !== 'EAST') {
      exifInfo.gpsLocation.longitude = null;  // invalid gpsLongitudeRef
    }
  }

  // gpsAltitude
  if (exifInfoNative.gpsAltitude && exifInfoNative.gpsAltitudeRef) {
    if (parseInt(exifInfoNative.gpsAltitudeRef) === 0) {  // 0=ABOVE SEA LEVEL
      exifInfo.gpsAltitude = exifInfoNative.gpsAltitude;
    } else if (parseInt(exifInfoNative.gpsAltitudeRef) === 1) {  // 1=BELOW SEA LEVEL
      exifInfo.gpsAltitude = -exifInfoNative.gpsAltitude;
    }
  }

  // gpsTime
  if (exifInfoNative.gpsExifDate) {
    var dateSplit = exifInfoNative.gpsExifDate.split(':');
    exifInfo.gpsTime = new Date(
        dateSplit[0],  // year
        dateSplit[1],  // month
        dateSplit[2],  // day
        exifInfoNative.gpsExifTimeHours,
        exifInfoNative.gpsExifTimeMinutes,
        exifInfoNative.gpsExifTimeSeconds);
  }

  return exifInfo;
}

ExifManager.prototype.getExifInfo = function() {
  var args = validator_.validateArgs(arguments, [
    {
      name: 'uri',
      type: validator_.Types.STRING,
      optional: false,
      nullable: false
    },
    {
      name: 'successCallback',
      type: validator_.Types.FUNCTION,
      optional: false,
      nullable: false
    },
    {
      name: 'errorCallback',
      type: validator_.Types.FUNCTION,
      optional: true,
      nullable: true
    }
  ]);

  if (!_isValidAbsoluteURI(args.uri)) {
    setTimeout(function() {
      native_.callIfPossible(args.errorCallback, new WebAPIException(
          WebAPIException.INVALID_VALUES_ERR,
          'Invalid URI.'));
    }, 0);
    return;
  }

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
    } else {

      // call to c++ code. Fields that do not exist are undefined.
      var exifInfoNative = native_.getResultObject(result);

      // calculate ExifInformation struct. All fields are initially null.
      // Fields that do not exist in jpg EXIF must remain null.
      var exifInfo = _calculateExifInfo(exifInfoNative);

      // make successCalback and pass exifInfo
      args.successCallback(exifInfo);
    }
  };

  tizen.filesystem.resolve(args.uri,
      function() {
        native_.call('ExifManager_getExifInfo', {'uri': args.uri}, callback);
      },
      function() {
        native_.callIfPossible(args.errorCallback, new WebAPIException(
            WebAPIException.NOT_FOUND_ERR,
            'File can not be found.'));
      });
};

ExifManager.prototype.saveExifInfo = function() {
  var args = validator_.validateArgs(arguments, [
    {
      name: 'exifInfo',
      type: validator_.Types.PLATFORM_OBJECT,
      values: tizen.ExifInformation,
      optional: false,
      nullable: false
    },
    {
      name: 'successCallback',
      type: validator_.Types.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'errorCallback',
      type: validator_.Types.FUNCTION,
      optional: true,
      nullable: true
    }
  ]);

  if (!_isValidAbsoluteURI(args.exifInfo.uri)) {
    setTimeout(function() {
      native_.callIfPossible(args.errorCallback, new WebAPIException(
          WebAPIException.INVALID_VALUES_ERR,
          'Invalid URI.'));
    }, 0);
    return;
  }

  var json = _getJsonFromExifInformation(args.exifInfo);
  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback,
          native_.getErrorObject(result));
    } else {
      var exifInfo = native_.getResultObject(result);
      args.successCallback(exifInfo);
    }
  };

  native_.call('ExifManager_saveExifInfo', json, callback);
};

ExifManager.prototype.getThumbnail = function() {
  var args = validator_.validateArgs(arguments, [
    {
      name: 'uri',
      type: validator_.Types.STRING,
      optional: false,
      nullable: false
    },
    {
      name: 'successCallback',
      type: validator_.Types.FUNCTION,
      optional: false,
      nullable: false
    },
    {
      name: 'errorCallback',
      type: validator_.Types.FUNCTION,
      optional: true,
      nullable: true
    }
  ]);

  if (!_isValidAbsoluteURI(args.uri)) {
    setTimeout(function() {
      native_.callIfPossible(args.errorCallback, new WebAPIException(
          WebAPIException.INVALID_VALUES_ERR,
          'Invalid URI.'));
    }, 0);
    return;
  }

  var _callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback,
          native_.getErrorObject(result));
    } else {
      var thumb = native_.getResultObject(result);
      args.successCallback(thumb.src);
    }
  };

  tizen.filesystem.resolve(args.uri,
      function() {
        native_.call('ExifManager_getThumbnail', {'uri': args.uri}, _callback);
      },
      function() {
        native_.callIfPossible(args.errorCallback, new WebAPIException(
            WebAPIException.NOT_FOUND_ERR,
            'File can not be found.'));
      });
};

tizen.ExifInformation = function() {
  validator_.isConstructorCall(this, tizen.ExifInformation);

  var args = validator_.validateArgs(arguments, [
    {
      name: 'ExifInitDict',
      type: validator_.Types.DICTIONARY,
      optional: true,
      nullable: false
    }
  ]);

  var uri_ = null,
      width_ = null,
      height_ = null,
      deviceMaker_ = null,
      deviceModel_ = null,
      originalTime_ = null,
      orientation_ = null,
      fNumber_ = null,
      isoSpeedRatings_ = null,
      exposureTime_ = null,
      exposureProgram_ = null,
      flash_ = null,
      focalLength_ = null,
      whiteBalance_ = null,
      gpsLocation_ = null,
      gpsAltitude_ = null,
      gpsProcessingMethod_ = null,
      gpsTime_ = null,
      userComment_ = null;

  function _validateISOSpeedRatings(v) {
    var valid = false;
    if (type_.isArray(v)) {
      for (var i = 0; i < v.length; i++) {
        var data = v[i]; // todo: uncomment when array conversion is implemented.
        //if (!type_.isNumber(data)) {
        //  return false;
        //}
      }
      valid = true;
    }
    return valid;
  }


  var exifInitDict = args.ExifInitDict;
  if (exifInitDict) {
    if (exifInitDict.uri === null) {
      throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
          'Parameter "uri" is required.');
    }
  }

  Object.defineProperties(this, {
    uri: {
      get: function() {
        return uri_;
      },
      set: function(v) {
        uri_ = v ? converter_.toString(v) : uri_;
      },
      enumerable: true
    },
    width: {
      get: function() {
        return width_;
      },
      set: function(v) {
        width_ = (!type_.isUndefined(v)) ? converter_.toLong(v, true) : width_;
      },
      enumerable: true
    },
    height: {
      get: function() {
        return height_;
      },
      set: function(v) {
        height_ = (!type_.isUndefined(v)) ? converter_.toLong(v, true) : height_;
      },
      enumerable: true
    },
    deviceMaker: {
      get: function() {
        return deviceMaker_;
      },
      set: function(v) {
        deviceMaker_ = (!type_.isUndefined(v)) ?
            converter_.toString(v, true) : deviceMaker_;
      },
      enumerable: true
    },
    deviceModel: {
      get: function() {
        return deviceModel_;
      },
      set: function(v) {
        deviceModel_ = (!type_.isUndefined(v)) ?
            converter_.toString(v, true) : deviceModel_;
      },
      enumerable: true
    },
    originalTime: {
      get: function() {
        return originalTime_;
      },
      set: function(v) {
        if (!type_.isUndefined(v)) {
          if (v === null || v instanceof Date) originalTime_ = v;
        }
      },
      enumerable: true
    },
    orientation: {
      get: function() {
        return orientation_;
      },
      set: function(v) {
        orientation_ = (!type_.isUndefined(v)) ?
            converter_.toEnum(v, Object.keys(ImageContentOrientation), true) : orientation_;
      },
      enumerable: true
    },
    fNumber: {
      get: function() {
        return fNumber_;
      },
      set: function(v) {
        fNumber_ = (!type_.isUndefined(v)) ? converter_.toDouble(v, true) : fNumber_;
      },
      enumerable: true
    },
    isoSpeedRatings: {
      get: function() {
        return isoSpeedRatings_;
      },
      set: function(v) {
        // todo: convert string array into unsigned short array
        if (!type_.isUndefined(v)) {
          if (v === null || _validateISOSpeedRatings(v)) isoSpeedRatings_ = v;
        }
      },
      enumerable: true
    },
    exposureTime: {
      get: function() {
        return exposureTime_;
      },
      set: function(v) {
        exposureTime_ = (!type_.isUndefined(v)) ?
            converter_.toString(v, true) : exposureTime_;
      },
      enumerable: true
    },
    exposureProgram: {
      get: function() {
        return exposureProgram_;
      },
      set: function(v) {
        exposureProgram_ = (!type_.isUndefined(v)) ?
            converter_.toEnum(v, Object.keys(ExposureProgram), true) : exposureProgram_;
      },
      enumerable: true
    },
    flash: {
      get: function() {
        return flash_;
      },
      set: function(v) {
        flash_ = (!type_.isUndefined(v)) ? converter_.toBoolean(v, true) : flash_;
      },
      enumerable: true
    },
    focalLength: {
      get: function() {
        return focalLength_;
      },
      set: function(v) {
        focalLength_ = (!type_.isUndefined(v)) ?
            converter_.toDouble(v, true) : focalLength_;
      },
      enumerable: true
    },
    whiteBalance: {
      get: function() {
        return whiteBalance_;
      },
      set: function(v) {
        whiteBalance_ = (!type_.isUndefined(v)) ?
            converter_.toEnum(v, Object.keys(WhiteBalanceMode), true) : whiteBalance_;
      },
      enumerable: true
    },
    gpsLocation: {
      get: function() {
        return gpsLocation_;
      },
      set: function(v) {
        if (!type_.isUndefined(v)) {
          if (v === null || v instanceof tizen.SimpleCoordinates) gpsLocation_ = v;
        }
      },
      enumerable: true
    },
    gpsAltitude: {
      get: function() {
        return gpsAltitude_;
      },
      set: function(v) {
        gpsAltitude_ = (!type_.isUndefined(v)) ?
            converter_.toDouble(v, true) : gpsAltitude_;
      },
      enumerable: true
    },
    gpsProcessingMethod: {
      get: function() {
        return gpsProcessingMethod_;
      },
      set: function(v) {
        gpsProcessingMethod_ = (!type_.isUndefined(v)) ?
            converter_.toString(v, true) : gpsProcessingMethod_;
      },
      enumerable: true
    },
    gpsTime: {
      enumerable: true,
      get: function() {
        return gpsTime_;
      },
      set: function(v) {
        if (!type_.isUndefined(v)) {
          if (v === null || v instanceof Date || v instanceof tizen.TZDate) gpsTime_ = v;
        }
      }
    },
    userComment: {
      enumerable: true,
      get: function() {
        return userComment_;
      },
      set: function(v) {
        userComment_ = (!type_.isUndefined(v)) ?
            converter_.toString(v, true) : userComment_;
      }
    }
  });

  //--- copy values from exifInitDict using setters above.
  if (exifInitDict instanceof _global.Object) {
    for (var prop in exifInitDict) {
      if (this.hasOwnProperty(prop)) {
        this[prop] = exifInitDict[prop];
      }
    }
  }
};

exports = new ExifManager();
