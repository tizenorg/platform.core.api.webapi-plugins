/* global xwalk, extension, tizen */

// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var validator_ = xwalk.utils.validator;
var types_ = validator_.Types;
var native_ = new xwalk.utils.NativeManager(extension);

/**
 * This class provides functions to request and release badge resource.
 * @constructor
 */
function BadgeManager() {
}

/**
 * Sets the badge count for the designated application.
 * @param {!ApplicationId} ID of the application to update the badge
 * @param {!number} Number to display as the badge on the application icon
 */
BadgeManager.prototype.setBadgeCount = function() {
  throw new tizen.WebAPIException(tizen.WebAPIException.UNKNOWN_ERROR, 'Not implemented');
}

/**
 * Gets the badge count for the designated application.
 * @param {!ApplicationId} ID of the designated application
 * @return {number} long Count of the badge
 */
BadgeManager.prototype.getBadgeCount = function() {
  throw new tizen.WebAPIException(tizen.WebAPIException.UNKNOWN_ERROR, 'Not implemented');
}

/**
 * Gets the badge count for the designated application.
 * @param {!ApplicationId} Array of the ID of the designated application
 * @param {!function} Callback method to be invoked when a badge number change notification is received
 */
BadgeManager.prototype.addChangeListener = function() {
  throw new tizen.WebAPIException(tizen.WebAPIException.UNKNOWN_ERROR, 'Not implemented');
}

/**
 * Gets the badge count for the designated application.
 * @param {!ApplicationId} Array of the ID of the designated application
 */
BadgeManager.prototype.removeChangeListener = function() {
  throw new tizen.WebAPIException(tizen.WebAPIException.UNKNOWN_ERROR, 'Not implemented');
}

exports.badge = new BadgeManager();
