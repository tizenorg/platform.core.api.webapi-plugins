// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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

  var callArgs = {
    uri: args.uri
  };

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
    } else {
      var exifInfo = native_.getResultObject(result);
      args.successCallback(exifInfo);
    }
  };

  native_.call('Exif_getExifInfo', callArgs, callback);
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

  native_.call('Exif_saveExifInfo', json, callback);
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

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback,
          native_.getErrorObject(result));
    } else {
      var thumb = native_.getResultObject(result);
      args.successCallback(thumb.src);
    }
  };

  native_.call('Exif_getThumbnail', {'uri': args.uri}, callback);
};

// this function passes ExifInformation_exposureProgram_attribute test:
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

  var uri_ = null;
  var width_ = null;
  var height_ = null;
  var deviceMaker_ = null;
  var deviceModel_ = null;
  var originalTime_ = null;
  var orientation_ = null;
  var fNumber_ = null;
  var isoSpeedRatings_ = null;
  var exposureTime_ = null;
  var exposureProgram_ = null;
  var flash_ = null;
  var focalLength_ = null;
  var whiteBalance_ = null;
  var gpsLocation_ = null;
  var gpsAltitude_ = null;
  var gpsProcessingMethod_ = null;
  var gpsTime_ = null;
  var userComment_ = null;

  var exifInitDict = args.ExifInitDict;
  if (exifInitDict) {
    if (exifInitDict.uri === null) {
      throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR,
          'Parameter "uri" is required.');
    }
  }

  Object.defineProperties(this, {
    uri: {
      get: function() {
        return uri_;
      },
      set: function(v) {
        uri_ = v ? converter_.toString(v, true) : uri_;
      },
      enumerable: true
    },
    width: {
      get: function() {
        return width_;
      },
      set: function(v) {
        width_ = v ? converter_.toLong(v, true) : width_;
      },
      enumerable: true
    },
    height: {
      get: function() {
        return height_;
      },
      set: function(v) {
        height_ = v ? converter_.toLong(v, true) : height_;
      },
      enumerable: true
    },
    deviceMaker: {
      get: function() {
        return deviceMaker_;
      },
      set: function(val) {
        deviceMaker_ = val ? converter_.toString(val, true) : deviceMaker_;
      },
      enumerable: true
    },
    deviceModel: {
      get: function() {
        return deviceModel_;
      },
      set: function(val) {
        deviceModel_ = val ? converter_.toString(val, true) : deviceModel_;
      },
      enumerable: true
    },
    originalTime: {
      get: function() {
        return originalTime_;
      },
      set: function(val) {
        originalTime_ = val instanceof Date ? val : originalTime_;
      },
      enumerable: true
    },
    orientation: {
      get: function() {
        return orientation_;
      },
      set: function(v) {
        orientation_ = v ? converter_.toEnum(v, Object.keys(ImageContentOrientation), true) :
            orientation_;
      },
      enumerable: true
    },
    fNumber: {
      get: function() {
        return fNumber_;
      },
      set: function(val) {
        fNumber_ = val ? converter_.toDouble(val, true) : fNumber_;
      },
      enumerable: true
    },
    isoSpeedRatings: {
      get: function() {
        return isoSpeedRatings_;
      },
      set: function(val) {
        isoSpeedRatings_ = type_.isArray(val) ? val : isoSpeedRatings_;
      },
      enumerable: true
    },
    exposureTime: {
      get: function() {
        return exposureTime_;
      },
      set: function(val) {
        exposureTime_ = val ? converter_.toString(val, true) : exposureTime_;
      },
      enumerable: true
    },
    exposureProgram: {
      get: function() {
        return exposureProgram_;
      },
      set: function(v) {
        exposureProgram_ = v ? converter_.toEnum(v, Object.keys(ExposureProgram), true) :
            exposureProgram_;
      },
      enumerable: true
    },
    flash: {
      get: function() {
        return flash_;
      },
      set: function(val) {
        flash_ = converter_.toBoolean(val, true);
      },
      enumerable: true
    },
    focalLength: {
      get: function() {
        return focalLength_;
      },
      set: function(val) {
        focalLength_ = val ? converter_.toDouble(val, true) : focalLength_;
      },
      enumerable: true
    },
    whiteBalance: {
      get: function() {
        return whiteBalance_;
      },
      set: function(v) {
        whiteBalance_ = v ? converter_.toEnum(v, Object.keys(WhiteBalanceMode), true) :
            whiteBalance_;
      },
      enumerable: true
    },
    gpsLocation: {
      get: function() {
        return gpsLocation_;
      },
      set: function(val) {
        gpsLocation_ = val instanceof tizen.SimpleCoordinates ? val : gpsLocation_;
      },
      enumerable: true
    },
    gpsAltitude: {
      get: function() {
        return gpsAltitude_;
      },
      set: function(val) {
        gpsAltitude_ = val ? converter_.toDouble(val, true) : gpsAltitude_;
      },
      enumerable: true
    },
    gpsProcessingMethod: {
      get: function() {
        return gpsProcessingMethod_;
      },
      set: function(val) {
        gpsProcessingMethod_ = val ? converter_.toString(val, true) : gpsProcessingMethod_;
      },
      enumerable: true
    },
    gpsTime: {
      enumerable: true,
      get: function() {
        return gpsTime_;
      },
      set: function(val) {
        gpsTime_ = val instanceof tizen.TZDate ? val : gpsTime_;
      }
    },
    userComment: {
      enumerable: true,
      get: function() {
        return userComment_;
      },
      set: function(val) {
        userComment_ = val ? converter_.toString(val, true) : userComment_;
      }
    }
  });

  //--- copy values from exifInitDict using setters above.
  if (exifInitDict instanceof Object) {
    for (var prop in exifInitDict) {
      if (this.hasOwnProperty(prop)) {
        this[prop] = exifInitDict[prop];
      }
    }
  }
};

exports = new ExifManager();
