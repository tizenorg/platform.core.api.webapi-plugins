// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @author  Marcin Wasowski (m.wasowski2@samsung.com)
 */

function TVDisplay() {}

var UNKNOWN_ERROR = 'UnknownError';

/**
 * Check if 3D mode is enabled
 * @return {boolean} true if enabled else false
 */
TVDisplay.prototype.is3DModeEnabled = function() {
  var msg = {
    cmd: 'TVDisplay_is3DModeEnabled',
    args: {},
    arg: ''
  };
  var reply = sendSyncMessage(msg);
  if (reply.error) {
    throw new tizen.WebAPIException(0, UNKNOWN_ERROR, reply.error);
  }
  return reply.result;
};

/**
 * Get current 3D effect mode or 'OFF' if no 3D enabled
 * @return {string} current mode name
 */
TVDisplay.prototype.get3DEffectMode = function() {
  var msg = {
    cmd: 'TVDisplay_get3DEffectMode',  // returns index
    args: {},
    arg: ''
  };
  var reply = sendSyncMessage(msg);
  if (reply.error) {
    throw new tizen.WebAPIException(0, reply.error, UNKNOWN_ERROR);
  }
  var mode = Display3DEffectMode[reply.result];
  if (!mode) {
    throw new tizen.WebAPIException(
      0,
      'Unknown 3D effect mode (' + reply.result + ')',
      UNKNOWN_ERROR);
  }
  return mode;
};

/**
 * Get list of supported 3D effects
 *
 * @param {!Mode3DEffectListSupportCallback} successCallback
 * @param {?ErrorCallback} errorCallback
 * @return {Display3DEffectMode[]} mode3DEffects
 */
TVDisplay.prototype.getSupported3DEffectModeList =
function(successCallback, errorCallback) {


  var Types = xwalk.utils.validator.Types;
  var successCallback = {
    name: 'successCallback',
    type: Types.FUNCTION,
    optional: false,
    nullable: false
  };
  var errorCallback = {
    name: 'errorCallback',
    type: Types.FUNCTION,
    optional: true,
    nullable: false
  };

  var msg = {
    cmd: 'TVDisplay_getSupported3DEffectModeList',
    args: {},
    arg: {}  // TODO: here callback id should be passed
  };

  var args = xwalk.utils.validator.validateArgs(
      arguments,
      [successCallback, errorCallback]);
  return sendSyncMessage(msg);  // undefined
};

function sendSyncMessage(msg) {
  var serialized = null;
  serialized = JSON.stringify(msg);
  return JSON.parse(extension.internal.sendSyncMessage(serialized));
}

/**
 *  Allowed values for 3D effect modes mapped to return values from C API
 *
 *  OFF - identifier for 3DEffect mode off
 *  TOP_BOTTOM - Top-bottom: Left at top, right at bottom
 *  SIDE_BY_SIDE - Top-bottom: Left at left side, right at right side
 *  LINE_BY_LINE - Line-by-line: Left and right image interlaced by row
 *  VERTICAL_STRIP - Vertical-strip: Left and right image
 *                   interlaced by column
 *  FRAME_SEQUENCE - Left and right image interlaced by frame
 *  CHECKER_BD - Checkerboard (only for PC or game console sources)
 *  FROM_2D_TO_3D - Left and right image computed from
 *                  No-stereoscopic image
*/
var Display3DEffectMode = [
  'OFF',
  'TOP_BOTTOM',
  'SIDE_BY_SIDE',
  'LINE_BY_LINE',
  'VERTICAL_STRIPE',
  'FRAME_SEQUENCE',
  'CHECKER_BD',
  'FROM_2D_TO_3D'
];
Object.freeze(Display3DEffectMode);

var Display3DModeState = {
  NOT_CONNECTED: 'NOT_CONNECTED',
  NOT_SUPPORTED: 'NOT_SUPPORTED',
  READY: 'READY'
};
Object.freeze(Display3DModeState);

exports = new TVDisplay();
