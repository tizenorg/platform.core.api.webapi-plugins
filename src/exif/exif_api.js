// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function _sendSyncMessage(cmd) {
  var msg = {
    'cmd': cmd
  };
  return JSON.parse(extension.internal.sendSyncMessage(JSON.stringify(msg)));
}

function ExifManager() {
}

ExifManager.prototype.getExifInfo = function(uri, successCallback, errorCallback) {
  throw 'Not implemented';
};

ExifManager.prototype.saveExifInfo = function(exifInfo, successCallback, errorCallback) {
  throw 'Not implemented';
};

ExifManager.prototype.getThumbnail = function(uri, successCallback, errorCallback) {
  throw 'Not implemented';
};

tizen.ExifInformation = function(ExifInitDict) {
  var uri_;
  var width_;
  var height_;
  var deviceMaker_;
  var deviceModel_;
  var originalTime_;
  var orientation_; //ImageContentOrientation
  var fNumber_;
  var isoSpeedRatings_;
  var exposureTime_;
  var exposureProgram_; //ExposureProgram
  var flash_;
  var focalLength_;
  var whiteBalance_; //WhiteBalanceMode
  var gpsLocation_; //SimpleCoordinates
  var gpsAltitude_;
  var gpsProcessingMethod_;
  var gpsTime_; //TZDate
  var userComment_;

  throw 'Not implemented';
};

exports = new ExifManager();
