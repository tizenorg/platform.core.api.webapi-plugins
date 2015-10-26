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

var utils_ = xwalk.utils;
var type_ = utils_.type;
var converter_ = utils_.converter;
var validator_ = utils_.validator;
var types_ = validator_.Types;
var privilege_ = xwalk.utils.privilege;
var native_ = new xwalk.utils.NativeManager(extension);

function convertColorToInt(rgbaColor) {
  var color = rgbaColor.length === 7 ? rgbaColor + 'ff' : rgbaColor;
  var isLengthOk = color.length === 9;
  var isHash = color.substr(0, 1) === '#';
  var hex = '0123456789abcdefABCDEF';
  var isHex = true;
  var c = color.replace('#', '');

  for (var i = 0; i < c.length; i++) {
    if (hex.indexOf(c[i]) < 0) {
      isHex = false;
    }
  }

  if (!isLengthOk || !isHash || !isHex) {
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR, 'invalid value');
  }

  return parseInt('0x' + c);
}

var EditManager = function() {
  this.canEdit = false;
};

EditManager.prototype.allow = function() {
  this.canEdit = true;
};

EditManager.prototype.disallow = function() {
  this.canEdit = false;
};

var _edit = new EditManager();

var NotificationType = {
  STATUS: 'STATUS'
};

var StatusNotificationType = {
  SIMPLE: 'SIMPLE',
  THUMBNAIL: 'THUMBNAIL',
  ONGOING: 'ONGOING',
  PROGRESS: 'PROGRESS'
};

var NotificationProgressType = {
  PERCENTAGE: 'PERCENTAGE',
  BYTE: 'BYTE'
};

var LEDCustomFlags = {
  LED_CUSTOM_DUTY_ON: 'LED_CUSTOM_DUTY_ON',
  LED_CUSTOM_DEFAULT: 'LED_CUSTOM_DEFAULT'
};

function NotificationManager() {}


NotificationManager.prototype.post = function(notification) {
  xwalk.utils.checkPrivilegeAccess(privilege_.NOTIFICATION);

  var args = validator_.validateArgs(arguments, [
    {name: 'notification', type: types_.PLATFORM_OBJECT, values: StatusNotification}
  ]);

  var data = {
    notification: args.notification
  };

  var result = native_.callSync('NotificationManager_post', data);

  if (native_.isFailure(result)) {
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR);
  }

  _edit.allow();
  var d = native_.getResultObject(result);
  notification.id = d.id;
  notification.postedTime = new Date(d.postedTime) || new Date();
  notification.type = d.type || NotificationType.STATUS;
  _edit.disallow();
};

NotificationManager.prototype.update = function(notification) {
  xwalk.utils.checkPrivilegeAccess(privilege_.NOTIFICATION);

  var args = validator_.validateArgs(arguments, [
    {name: 'notification', type: types_.PLATFORM_OBJECT, values: StatusNotification}
  ]);

  if (!arguments.length) {
    throw new WebAPIException(WebAPIException.NOT_FOUND_ERR);
  }
  if (!args.notification.id) {
    throw new WebAPIException(WebAPIException.UNKNOWN_ERR);
  }

  var data = {
    notification: args.notification
  };

  var result = native_.callSync('NotificationManager_update', data);

  if (native_.isFailure(result)) {
    throw new WebAPIException(WebAPIException.NOT_FOUND_ERR,
        native_.getErrorObject(result));
  }
};

NotificationManager.prototype.remove = function(id) {
  xwalk.utils.checkPrivilegeAccess(privilege_.NOTIFICATION);

  var args = validator_.validateArgs(arguments, [
    {name: 'id', type: types_.STRING}
  ]);

  if (!arguments.length || !(parseInt(arguments[0]) > 0)) {
    throw new WebAPIException(WebAPIException.NOT_FOUND_ERR);
  }

  var data = {
    id: args.id
  };

  var result = native_.callSync('NotificationManager_remove', data);

  if (native_.isFailure(result)) {
    throw new WebAPIException(WebAPIException.NOT_FOUND_ERR,
        native_.getErrorObject(result));
  }
};

NotificationManager.prototype.removeAll = function() {
  xwalk.utils.checkPrivilegeAccess(privilege_.NOTIFICATION);

  var result = native_.callSync('NotificationManager_removeAll', {});

  if (native_.isFailure(result)) {
    throw new WebAPIException(WebAPIException.UNKNOWN_ERR,
        native_.getErrorObject(result));
  }
};

NotificationManager.prototype.get = function(id) {
  var args = validator_.validateArgs(arguments, [
    {name: 'id', type: types_.STRING}
  ]);

  if (!arguments.length) {
    throw new WebAPIException(WebAPIException.NOT_FOUND_ERR);
  }

  var data = {
    id: args.id
  };

  var result = native_.callSync('NotificationManager_get', data);

  if (native_.isFailure(result)) {
    throw new WebAPIException(WebAPIException.NOT_FOUND_ERR,
        native_.getErrorObject(result));
  }

  var n = native_.getResultObject(result);

  _edit.allow();
  var returnObject = new StatusNotification(n.statusType, n.title, n);
  _edit.disallow();

  return returnObject;
};

NotificationManager.prototype.getAll = function() {
  var result = native_.callSync('NotificationManager_getAll', {});

  if (native_.isFailure(result)) {
    throw new WebAPIException(WebAPIException.NOT_FOUND_ERR,
        native_.getErrorObject(result));
  }

  var n = native_.getResultObject(result);
  var notifications = [];

  _edit.allow();
  for (var i = 0; i < n.length; i++) {
    notifications.push(new StatusNotification(n[i].statusType, n[i].title, n[i]));
  }
  _edit.disallow();

  return notifications;
};

/**
 * Plays the custom effect of the service LED that is located to the front of a device.
 *
 * @param timeOn Number
 * @param timeOff Number
 * @param color String
 * @param flags Array
 */
NotificationManager.prototype.playLEDCustomEffect = function(timeOn, timeOff, color, flags) {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.LED);

  var args = validator_.validateArgs(arguments, [
    {name: 'timeOn', type: types_.LONG},
    {name: 'timeOff', type: types_.LONG},
    {name: 'color', type: types_.STRING},
    {name: 'flags', type: types_.ARRAY, values: types_.STRING}
  ]);

  for (var i = 0; i < args.flags.length; ++i) {
    if (Object.keys(LEDCustomFlags).indexOf(args.flags[i]) < 0) {
      throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR, 'invalid value');
    }
  }

  args.color = convertColorToInt(args.color);
  var result = native_.callSync('NotificationManager_playLEDCustomEffect', args);
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

/**
 * Stops the custom effect of the service LED that is located to the front of a device.
 */
NotificationManager.prototype.stopLEDCustomEffect = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.LED);

  var result = native_.callSync('NotificationManager_stopLEDCustomEffect');
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};


function NotificationInitDict(data) {
  var _iconPath = null;
  var _soundPath = null;
  var _vibration = false;
  var _appControl = null;
  var _appId = null;
  var _progressType = NotificationProgressType.PERCENTAGE;
  var _progressValue = null;
  var checkProgressValue = function(v) {
    if ((_progressType === NotificationProgressType.PERCENTAGE && (v >= 0 && v <= 100))
            || (_progressType === NotificationProgressType.BYTE &&
            converter_.toUnsignedLong(v) >= 0)) {
      return true;
    }
    return false;
  };
  var _number = null;
  var _subIconPath = null;
  var _detailInfo = [];
  var checkDetailInfo = function(v) {
    if (type_.isNull(v)) {
      return true;
    }
    if (!type_.isArray(v)) {
      return false;
    }
    for (var i = 0; i < v.length; ++i) {
      if (!(v[i] instanceof tizen.NotificationDetailInfo)) {
        return false;
      }
    }
    return true;
  };
  var setDetailInfo = function(v) {
    var _d = [];
    for (var i = 0; i < v.length; ++i) {
      _d.push(new tizen.NotificationDetailInfo(v[i].mainText, v[i].subText || null));
    }
    return _d;
  };
  var _ledColor = null;
  var isHex = function(v) {
    return v.length === 7 && v.substr(0, 1) === '#' && (/^([0-9A-Fa-f]{2})+$/).test(v.substr(1, 7));
  };
  var _ledOnPeriod = 0;
  var _ledOffPeriod = 0;
  var _backgroundImagePath = null;
  var _thumbnails = [];
  var checkThumbnails = function(v) {
    if (type_.isNull(v)) {
      return true;
    }
    if (!type_.isArray(v)) {
      return false;
    }
    for (var i = 0; i < v.length; ++i) {
      if (!type_.isString(v[i])) {
        return false;
      }
    }
    return true;
  };

  Object.defineProperties(this, {
    iconPath: {
      get: function() {
        return _iconPath;
      },
      set: function(v) {
        _iconPath = type_.isString(v) || type_.isNull(v) ? v : _iconPath;
      },
      enumerable: true
    },
    soundPath: {
      get: function() {
        return _soundPath;
      },
      set: function(v) {
        _soundPath = type_.isString(v) || type_.isNull(v) ? v : _soundPath;
      },
      enumerable: true
    },
    vibration: {
      get: function() {
        return _vibration;
      },
      set: function(v) {
        _vibration = type_.isBoolean(v) ? v : _vibration;
      },
      enumerable: true
    },
    appControl: {
      get: function() {
        return _appControl;
      },
      set: function(v) {
        _appControl = _edit.canEdit && v
            ? new tizen.ApplicationControl(v.operation, v.uri || null, v.mime || null, v.category
                    || null, v.data || [])
            : v instanceof tizen.ApplicationControl || type_.isNull(v) ? v : _appControl;
      },
      enumerable: true
    },
    appId: {
      get: function() {
        return _appId;
      },
      set: function(v) {
        _appId = type_.isString(v) && !(/\s/.test(v)) || type_.isNull(v) ? v : _appId;
      },
      enumerable: true
    },
    progressType: {
      get: function() {
        return _progressType;
      },
      set: function(v) {
        _progressType = Object.keys(NotificationProgressType).indexOf(v) >= 0 ? v : _progressType;
      },
      enumerable: true
    },
    progressValue: {
      get: function() {
        if (null === _progressValue) {
          return _progressValue;
        }

        return (_progressType === NotificationProgressType.PERCENTAGE)
            ? _progressValue * 100
            : _progressValue;
      },
      set: function(v) {
        if (type_.isNull(v)) {
          _progressValue = v;
          return;
        }
        if (checkProgressValue(v)) {
          _progressValue = (_progressType === NotificationProgressType.PERCENTAGE)
            ? v / 100
            : converter_.toUnsignedLong(v);
        }
      },
      enumerable: true
    },
    number: {
      get: function() {
        return _number;
      },
      set: function(v) {
        _number = type_.isNumber(v) || type_.isNull(v) ? v : _number;
      },
      enumerable: true
    },
    subIconPath: {
      get: function() {
        return _subIconPath;
      },
      set: function(v) {
        _subIconPath = type_.isString(v) || type_.isNull(v) ? v : _subIconPath;
      },
      enumerable: true
    },
    detailInfo: {
      get: function() {
        return _detailInfo;
      },
      set: function(v) {
        _detailInfo = _edit.canEdit && v ? setDetailInfo(v) : checkDetailInfo(v) ? v : _detailInfo;
      },
      enumerable: true
    },
    ledColor: {
      get: function() {
        return _ledColor;
      },
      set: function(v) {
        _ledColor = (type_.isString(v) && isHex(v)) || type_.isNull(v) ? v : _ledColor;
      },
      enumerable: true
    },
    ledOnPeriod: {
      get: function() {
        return _ledOnPeriod;
      },
      set: function(v) {
        _ledOnPeriod = type_.isNumber(v) ? v : _ledOnPeriod;
      },
      enumerable: true
    },
    ledOffPeriod: {
      get: function() {
        return _ledOffPeriod;
      },
      set: function(v) {
        _ledOffPeriod = type_.isNumber(v) ? v : _ledOffPeriod;
      },
      enumerable: true
    },
    backgroundImagePath: {
      get: function() {
        return _backgroundImagePath;
      },
      set: function(v) {
        _backgroundImagePath = type_.isString(v) || type_.isNull(v) ? v : _backgroundImagePath;
      },
      enumerable: true
    },
    thumbnails: {
      get: function() {
        return _thumbnails;
      },
      set: function(v) {
        _thumbnails = checkThumbnails(v) ? v : _thumbnails;
      },
      enumerable: true
    }
  });

  if (data instanceof _global.Object) {
    for (var prop in data) {
      if (this.hasOwnProperty(prop)) {
        this[prop] = data[prop];
      }
    }
  }
}

function Notification(data) {
  var _id;
  var _type = NotificationType.STATUS;
  var _postedTime;
  var _title;
  var _content = null;

  Object.defineProperties(this, {
    id: {
      get: function() {
        return _id;
      },
      set: function(v) {
        _id = _edit.canEdit && v ? v : _id;
      },
      enumerable: true
    },
    type: {
      get: function() {
        return _type;
      },
      set: function(v) {
        _type = _edit.canEdit
            ? converter_.toEnum(v, Object.keys(NotificationType), false)
            : _type;
      },
      enumerable: true
    },
    postedTime: {
      get: function() {
        return _postedTime;
      },
      set: function(v) {
        _postedTime = _edit.canEdit && v ? new Date(v) : _postedTime;
      },
      enumerable: true
    },
    title: {
      get: function() {
        return _title;
      },
      set: function(v) {
        _title = converter_.toString(v);
      },
      enumerable: true
    },
    content: {
      get: function() {
        return _content;
      },
      set: function(v) {
        _content = type_.isString(v) || type_.isNull(v) ? v : _content;
      },
      enumerable: true
    }
  });

  if (data instanceof _global.Object) {
    for (var prop in data) {
      if (data.hasOwnProperty(prop) && this.hasOwnProperty(prop)) {
        this[prop] = data[prop];
      }
    }
  }
}

function StatusNotification(statusType, title, notificationInitDict) {
  validator_.isConstructorCall(this, StatusNotification);
  type_.isObject(notificationInitDict) ?
      notificationInitDict.title = title :
      notificationInitDict = {title: title};
  NotificationInitDict.call(this, notificationInitDict);
  Notification.call(this, notificationInitDict);

  var _statusType = (Object.keys(StatusNotificationType)).indexOf(statusType) >= 0
      ? statusType : StatusNotificationType.SIMPLE;

  Object.defineProperties(this, {
    statusType: {
      get: function() {
        return _statusType;
      },
      set: function(v) {
        _statusType = (Object.keys(StatusNotificationType)).indexOf(v) >= 0 && _edit.canEdit
            ? v : _statusType;
      },
      enumerable: true
    }
  });
}

StatusNotification.prototype = new Notification();
StatusNotification.prototype.constructor = StatusNotification;


function NotificationDetailInfo(mainText, subText) {
  validator_.isConstructorCall(this, NotificationDetailInfo);

  var _mainText = type_.isString(mainText) ? mainText : '';
  var _subText = type_.isString(subText) ? subText : null;

  Object.defineProperties(this, {
    mainText: {
      get: function() {
        return _mainText;
      },
      set: function(v) {
        _mainText = type_.isString(v) ? v : _mainText;
      },
      enumerable: true
    },
    subText: {
      get: function() {
        return _subText;
      },
      set: function(v) {
        _subText = type_.isString(v) ? v : _subText;
      },
      enumerable: true
    }
  });
}

exports = new NotificationManager();
tizen.StatusNotification = StatusNotification;
tizen.NotificationDetailInfo = NotificationDetailInfo;
