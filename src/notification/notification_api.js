// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var utils_ = xwalk.utils;
var type_ = utils_.type;
var converter_ = utils_.converter;
var validator_ = utils_.validator;
var types_ = validator_.Types;
var native_ = new xwalk.utils.NativeManager(extension);

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

function NotificationManager() {}


NotificationManager.prototype.post = function(notification) {
  var args = validator_.validateArgs(arguments, [
    {name: 'notification', type: types_.PLATFORM_OBJECT, values: StatusNotification}
  ]);

  var data = {
    notification: args.notification
  };

  var result = native_.callSync('NotificationManager_post', data);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  _edit.allow();
  var d = native_.getResultObject(result);
  notification.id = d.id;
  notification.postedTime = d.postedTime || new Date();
  notification.type = d.type || NotificationType.STATUS;
  _edit.disallow();
};

NotificationManager.prototype.update = function(notification) {
  var args = validator_.validateArgs(arguments, [
    {name: 'notification', type: types_.PLATFORM_OBJECT, values: StatusNotification}
  ]);

  if (!arguments.length) {
    throw new tizen.WebAPIException(tizen.WebAPIException.NOT_FOUND_ERR);
  }

  var data = {
    notification: args.notification
  };

  var result = native_.callSync('NotificationManager_update', data);

  if (native_.isFailure(result)) {
    throw new tizen.WebAPIException(tizen.WebAPIException.NOT_FOUND_ERR,
        native_.getErrorObject(result));
  }
};

NotificationManager.prototype.remove = function(id) {
  var args = validator_.validateArgs(arguments, [
    {name: 'id', type: types_.STRING}
  ]);

  if (!arguments.length) {
    throw new tizen.WebAPIException(tizen.WebAPIException.NOT_FOUND_ERR);
  }

  var data = {
    id: args.id
  };

  var result = native_.callSync('NotificationManager_remove', data);

  if (native_.isFailure(result)) {
    throw new tizen.WebAPIException(tizen.WebAPIException.NOT_FOUND_ERR,
        native_.getErrorObject(result));
  }
};

NotificationManager.prototype.removeAll = function() {
  var result = native_.callSync('NotificationManager_removeAll', {});

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

NotificationManager.prototype.get = function(id) {
  var args = validator_.validateArgs(arguments, [
    {name: 'id', type: types_.STRING}
  ]);

  if (!arguments.length) {
    throw new tizen.WebAPIException(tizen.WebAPIException.NOT_FOUND_ERR);
  }

  var data = {
    id: args.id
  };

  var result = native_.callSync('NotificationManager_get', data);

  if (native_.isFailure(result)) {
    throw new tizen.WebAPIException(tizen.WebAPIException.NOT_FOUND_ERR,
        native_.getErrorObject(result));
  }

  _edit.allow();
  var returnObject = new StatusNotification(native_.getResultObject(result));
  _edit.disallow();

  return returnObject;
};

NotificationManager.prototype.getAll = function() {
  var result = native_.callSync('NotificationManager_getAll', {});

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  var n = native_.getResultObject(result);
  var notifications = [];

  _edit.allow();
  for (var i = 0; i < n.length; i++) {
    notifications.push(new StatusNotification(n[i]));
  }
  _edit.disallow();

  return notifications;
};

function NotificationInitDict(data) {
  var _iconPath = null;
  var _soundPath = null;
  var _vibration = false;
  var _isUnified = false;
  var _appControl = null;
  var _appId = false;
  var _progressType = NotificationProgressType.PERCENTAGE;
  var _progressValue = 0;
  var _number = null;
  var _subIconPath = null;
  var _detailInfo = [];
  var checkDetailInfo = function(v) {
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
  var _ledColor = null;
  var isHex = function(v) {
    return v.length === 7 && v.substr(0, 1) === '#' && (/^([0-9A-Fa-f]{2})+$/).test(v.substr(1, 7));
  };
  var _ledOnPeriod = 0;
  var _ledOffPeriod = 0;
  var _backgroundImagePath = null;
  var _thumbnails = [];
  var checkThumbnails = function(v) {
    if (!type_.isArray(v)) {
      return false;
    }
    for (var i = 0; i < v.length; ++i) {
      if (!type_.isString(v[i]) && !(/\s/.test(v))) {
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
        _iconPath = type_.isString(v) ? v : _iconPath;
      },
      enumerable: true
    },
    soundPath: {
      get: function() {
        return _soundPath;
      },
      set: function(v) {
        _soundPath = type_.isString(v) ? v : _soundPath;
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
    isUnified: {
      get: function() {
        return _isUnified;
      },
      set: function(v) {
        _isUnified = type_.isBoolean(v) ? v : _isUnified;
      },
      enumerable: true
    },
    appControl: {
      get: function() {
        return _appControl;
      },
      set: function(v) {
        _appControl = v instanceof tizen.ApplicationControl ? v : _appControl;
      },
      enumerable: true
    },
    appId: {
      get: function() {
        return _appId;
      },
      set: function(v) {
        _appId = type_.isString(v) && !(/\s/.test(v)) ? v : _appId;
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
        return _progressValue;
      },
      set: function(v) {
        _progressValue = this.statusType === StatusNotificationType.PROGRESS
                && (v >= 0 && v <= 100) ? v : _progressValue;
      },
      enumerable: true
    },
    number: {
      get: function() {
        return _number;
      },
      set: function(v) {
        _number = type_.isNumber(v) ? v : _number;
      },
      enumerable: true
    },
    subIconPath: {
      get: function() {
        return _subIconPath;
      },
      set: function(v) {
        _subIconPath = type_.isString(v) && !(/\s/.test(v))  ? v : _subIconPath;
      },
      enumerable: true
    },
    detailInfo: {
      get: function() {
        return _detailInfo;
      },
      set: function(v) {
        _detailInfo = checkDetailInfo(v) ? v : _detailInfo;
      },
      enumerable: true
    },
    ledColor: {
      get: function() {
        return _ledColor;
      },
      set: function(v) {
        _ledColor = (type_.isString(v) && isHex(v)) || v === null ? v : _ledColor;
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
        _backgroundImagePath = type_.isString(v) && !(/\s/.test(v))  ? v : _backgroundImagePath;
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

  if (data instanceof Object) {
    for (var prop in data) {
      if (this.hasOwnProperty(prop)) {
        this[prop] = data[prop];
      }
    }
  }
}

function Notification(data) {
  NotificationInitDict.call(this, data);

  var _id = undefined;
  var _type = NotificationType.STATUS;
  var _postedTime = undefined;
  var _content = null;

  Object.defineProperties(this, {
    id: {
      get: function() {
        return _id;
      },
      set: function(v) {
        _id = _edit.canEdit ? v : _id;
      },
      enumerable: true
    },
    type: {
      get: function() {
        return _type;
      },
      set: function(v) {
        _type = _edit.canEdit ? v : _id;
      },
      enumerable: true
    },
    postedTime: {
      get: function() {
        return _postedTime;
      },
      set: function(v) {
        _postedTime = _edit.canEdit ? new Date(v) : _postedTime;
      },
      enumerable: true
    },
    content: {
      get: function() {
        return _content;
      },
      set: function(v) {
        _content = type_.isString(v) ? v : _content;
      },
      enumerable: true
    }
  });

  if (data instanceof Object) {
    for (var prop in data) {
      if (this.hasOwnProperty(prop)) {
        this[prop] = data[prop];
      }
    }
  }
}

function StatusNotification(statusType, title, notificationInitDict) {
  validator_.isConstructorCall(this, StatusNotification);
  Notification.call(this, notificationInitDict);

  var _statusType = (Object.keys(StatusNotificationType)).indexOf(statusType) >= 0
      ? statusType : StatusNotificationType.SIMPLE;
  var _title = converter_.toString(title);

  Object.defineProperties(this, {
    statusType: {
      get: function() {
        return _statusType;
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
