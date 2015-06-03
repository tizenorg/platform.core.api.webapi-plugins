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
 
/**
 * @author m.wasowski2@samsung.com (Marcin Wasowski)
 */

function TVDisplay() {}

var native = new xwalk.utils.NativeManager(extension);
var validator = xwalk.utils.validator;
var validatorTypes = xwalk.utils.validator.Types;

var UNKNOWN_ERROR = 'UnknownError';


/**
 * Allowed values for is3DModeEnabled return value
 * @type {Display3DModeState}
 */
var Display3DModeState = {
  NOT_CONNECTED: 'NOT_CONNECTED',
  NOT_SUPPORTED: 'NOT_SUPPORTED',
  READY: 'READY'
};
Object.freeze(Display3DModeState);


/**
 * Check if 3D mode is enabled
 * @return {Display3DModeState} mode String informing if 3D content
 *                                   is supported and ready
 */
TVDisplay.prototype.is3DModeEnabled = function() {
  var ret = native.callSync('TVDisplay_is3DModeEnabled', {});
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
  return native.getResultObject(ret);
};


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


/**
 * Get current 3D effect mode or 'OFF' if no 3D enabled
 * @return {string} current mode name
 */
TVDisplay.prototype.get3DEffectMode = function() {
  var ret = native.callSync('TVDisplay_get3DEffectMode', {});
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
  var mode = Display3DEffectMode[native.getResultObject(ret)];
  if (!mode) {
    var error_msg = 'Unknown 3D effect mode (' + reply.result + ')';
    throw new WebAPIException(0, error_msg, UNKNOWN_ERROR);
  }
  return mode;
};


/**
 * Get list of supported 3D effects
 *
 * @param {!Mode3DEffectListSupportCallback} successCallback
 * @param {?ErrorCallback} errorCallback
 */
TVDisplay.prototype.getSupported3DEffectModeList = function(
    successCallback,
    errorCallback) {

  var successCallback = {
    name: 'successCallback',
    type: validatorTypes.FUNCTION,
    optional: false,
    nullable: false
  };
  var errorCallback = {
    name: 'errorCallback',
    type: validatorTypes.FUNCTION,
    optional: true,
    nullable: true
  };

  var args = validator.validateArgs(
      arguments,
      [successCallback, errorCallback]);

  native.call(
      'TVDisplay_getSupported3DEffectModeList',
      {},
      function(msg) {
        if (msg && !msg.error) {
          args.successCallback(msg.result);
        } else if (msg && validatorTypes.isFunction(args.errorCallback)) {
          args.errorCallback(native.getErrorObject(msg.error));
        } else {
          return;
        }
      }
  );

};


exports = new TVDisplay();
