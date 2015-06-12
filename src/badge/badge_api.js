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
 
//maximum value of badge as found in old plugin implementation
var MAX_BADGE_COUNT = 999;

var validator_ = xwalk.utils.validator;
var converter_ = xwalk.utils.converter;
var types_ = validator_.Types;
var Type = xwalk.utils.type;
var native_ = new xwalk.utils.NativeManager(extension);

var _badgeListenerRegistered = false;
var _badgeCallbackMap = {};
var _currentWatchId = 1;
var _getNextWatchId = function() {
  return _currentWatchId++;
};

var _badgeChangeListener = function(result) {
  if (result.appId && result.count && _badgeCallbackMap.hasOwnProperty(result.appId)) {
    for (var functionToCall in _badgeCallbackMap[result.appId]) {
      native_.callIfPossible(_badgeCallbackMap[result.appId][functionToCall],
                             result.appId, converter_.toLong(result.count));
    }
  }
};



/**
 * This class provides functions to request and release badge resource.
 * @constructor
 */
function BadgeManager() {
  Object.defineProperties(this, {
    'maxBadgeCount': { value: MAX_BADGE_COUNT, emumerable: true, writable: false}
  });
}


/**
 * Sets the badge count for the designated application.
 */
BadgeManager.prototype.setBadgeCount = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NOTIFICATION);

  var args = validator_.validateArgs(arguments, [
    {name: 'appId', type: types_.STRING},
    {name: 'count', type: types_.LONG}
  ]);

  if (args.count < 0) {
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
                              'Count parameter is negative!');
  }

  var ret = native_.callSync('BadgeManager_setBadgeCount', {
    appId: args.appId,
    count: args.count
  });

  if (native_.isFailure(ret)) {
    throw native_.getErrorObject(ret);
  }
};


/**
 * Gets the badge count for the designated application.
 * @return {number} long Count of the badge
 */
BadgeManager.prototype.getBadgeCount = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NOTIFICATION);

  var args = validator_.validateArgs(arguments, [
    {name: 'appId', type: types_.STRING}
  ]);

  var ret = native_.callSync('BadgeManager_getBadgeCount', {
    appId: args.appId
  });

  if (native_.isFailure(ret)) {
    throw native_.getErrorObject(ret);
  }

  return parseInt(native_.getResultObject(ret));
};


/**
 * Gets the badge count for the designated application.
 */
BadgeManager.prototype.addChangeListener = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NOTIFICATION);

  var args = validator_.validateArgs(arguments, [
    {
      name: 'appIdList',
      type: types_.ARRAY,
      values: types_.STRING,
      optional: false,
      nullable: false
    },
    {
      name: 'successCallback',
      type: types_.FUNCTION,
      optional: false,
      nullable: false
    }
  ]);

  var result = native_.callSync('BadgeManager_addChangeListener', args);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  if (!_badgeListenerRegistered) {
    _badgeListenerRegistered = true;
    native_.addListener('BadgeChangeListener', _badgeChangeListener);
  }
  for (var i = 0; i < args.appIdList.length; i++) {
    if (!_badgeCallbackMap.hasOwnProperty(args.appIdList[i])) {
      _badgeCallbackMap[args.appIdList[i]] = [];
    }
    _badgeCallbackMap[args.appIdList[i]].push(args.successCallback);
  }
  return;
};


/**
 * Gets the badge count for the designated application.
 */
BadgeManager.prototype.removeChangeListener = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.NOTIFICATION);

  var args = validator_.validateArgs(arguments, [
    {
      name: 'appIdList',
      type: types_.ARRAY,
      values: types_.STRING,
      optional: false,
      nullable: false
    }
  ]);

  for (var i = 0; i < args.appIdList.length; i++) {
    if (_badgeCallbackMap.hasOwnProperty(args.appIdList[i]))
      delete _badgeCallbackMap[args.appIdList[i]];
  }

  if (Type.isEmptyObject(_badgeCallbackMap)) {
    native_.removeListener('BadgeChangeListener', _badgeChangeListener);
    _badgeListenerRegistered = false;
  }

  var result = native_.callSync('BadgeManager_removeChangeListener', args);
  if (native_.isFailure(result))
    throw native_.getErrorObject(result);
};

exports = new BadgeManager();
