/* global tizen, xwalk, extension */

// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


var validator_ = xwalk.utils.validator;
var types_ = validator_.Types;


function SetReadOnlyProperty(obj, n, v) {
  Object.defineProperty(obj, n, {'value': v, 'writable': false});
}

var NotificationType = {
  'STATUS': 'STATUS'
};
var StatusNotificationType = {
  'SIMPLE': 'SIMPLE',
  'THUMBNAIL': 'THUMBNAIL',
  'ONGOING': 'ONGOING',
  'PROGRESS': 'PROGRESS'
};
var NotificationProgressType = {
  'PERCENTAGE': 'PERCENTAGE',
  'BYTE': 'BYTE'
};


function NotificationManager() {
  // constructor of NotificationManager

}


NotificationManager.prototype.post = function(notification) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'notification', 'type': types_.PLATFORM_OBJECT, 'values': ['Notification']}
  ]);

  var nativeParam = notificationToNativeParam(notification);

  var id = notification.id;
  var type = notification.type;
  var ptime = notification.postedTime;
  var title = notification.title;
  var content = notification.content;

  var nativeParam = {
    'type': notification.type,
    'title' : notification.title,
    'content' : notification.content
  };


  try {
    var syncResult = callNative('NotificationManager_post', nativeParam);
    // if you need synchronous result from native function using 'syncResult'.
  } catch (e) {
    throw e;
  }

};

NotificationManager.prototype.update = function(notification) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'notification', 'type': types_.PLATFORM_OBJECT, 'values': ['Notification']}
  ]);

  var nativeParam = {
  };


  try {
    var syncResult = callNative('NotificationManager_update', nativeParam);
    // if you need synchronous result from native function using 'syncResult'.
  } catch (e) {
    throw e;
  }

};

NotificationManager.prototype.remove = function(id) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'id', 'type': types_.STRING}
  ]);

  var nativeParam = {
    'id': args.id
  };


  try {
    var syncResult = callNative('NotificationManager_remove', nativeParam);
    // if you need synchronous result from native function using 'syncResult'.
  } catch (e) {
    throw e;
  }

};

NotificationManager.prototype.removeAll = function() {

  var nativeParam = {
  };


  try {
    var syncResult = callNative('NotificationManager_removeAll', nativeParam);
    // if you need synchronous result from native function using 'syncResult'.
  } catch (e) {
    throw e;
  }

};

NotificationManager.prototype.get = function(id) {
  var args = validator_.validateArgs(arguments, [
    {'name': 'id', 'type': types_.STRING}
  ]);

  var nativeParam = {
    'id': args.id
  };


  try {
    var syncResult = callNative('NotificationManager_get', nativeParam);
    // if you need synchronous result from native function using 'syncResult'.
  } catch (e) {
    throw e;
  }

  var returnObject = new Notification();
  return returnObject;
};

NotificationManager.prototype.getAll = function() {

  var nativeParam = {
  };


  try {
    var syncResult = callNative('NotificationManager_getAll', nativeParam);
    // if you need synchronous result from native function using 'syncResult'.
  } catch (e) {
    throw e;
  }

  var returnObject = new Notification();
  return returnObject;
};



function Notification() {
  // constructor of Notification

  SetReadOnlyProperty(this, 'id', null); // read only property
  SetReadOnlyProperty(this, 'type', null); // read only property
  SetReadOnlyProperty(this, 'postedTime', null); // read only property
  this.title = null;
  this.content = null;
}


function notificationToNativeParam(n) {
  var i;

  var detailInfo = [];
  if (n.detailInfo) {
    for (i in n.detailInfo) {
      detailInfo[i] = n.detailInfo[i];
    }
  }
  var thumbnails = [];
  if (n.thumbnails) {
    for (i in n.thumbnails) {
      thumbnails[i] = n.thumbnails[i];
    }
  }

  var appControl = {};
  if (n.appControl) {
    appControl.operation = n.appControl.operation;
    appControl.uri = n.appControl.uri;
    appControl.mime = n.appControl.mime;
    appControl.category = n.appControl.category;
    appControl.data = [];
    if (n.appControl.data) {
      for (i in n.appControl.data) {
        var values = n.appControl.data[i].value;
        var key = n.appControl.data[i].key;
        appControl.data[i] = {
          'key': key,
          'value': values
        };
      }
    }
  }

  return {
    'statusType': args.statusType,
    'title': args.title,
    'content': n.content,
    'iconPath': n.iconPath,
    'soundPath': n.soundPath,
    'vibration': n.vibration,
    'appControl': appControl,
    'appId': n.appId,
    'progressType': n.progressType,
    'progressValue': n.progressValue,
    'number': n.number,
    'subIconPath': n.subIconPath,
    'detailInfo': detailInfo,
    'ledColor': n.ledColor,
    'ledOnPeriod': n.ledOnPeriod,
    'ledOffPeriod': n.ledOffPeriod,
    'backgroundImagePath': n.backgroundImagePath,
    'thumbnails': thumbnails
  };
}


// private constructor
function StatusNotification(statusType, title, notificationInitDict) {
  // constructor of StatusNotification

  SetHidedProperty(this, 'id', undefined);
  SetReadOnlyProperty(this, 'type', 'STATUS');
  SetHidedProperty(this, 'postedTime', undefined);

  this.title = title;
  this.content = notificationInitDict.content;
  SetReadOnlyProperty(this, 'statusType', statusType); // read only property
  this.iconPath = notificationInitDict.iconPath;
  this.subIconPath = notificationInitDict.subIconPath;
  this.number = notificationInitDict.number;
  this.detailInfo = notificationInitDict.detailInfo;
  this.ledColor = notificationInitDict.ledColor;
  this.ledOnPeriod = notificationInitDict.ledOnPeriod;
  this.ledOffPeriod = notificationInitDict.ledOffPeriod;
  this.backgroundImagePath = notificationInitDict.backgroundImagePath;
  this.thumbnails = notificationInitDict.thumbnails;
  this.soundPath = notificationInitDict.soundPath;
  this.vibration = notificationInitDict.vibration;
  this.appControl = notificationInitDict.appControl;
  this.appId = notificationInitDict.appId;
  this.progressType = notificationInitDict.progressType;
  this.progressValue = notificationInitDict.progressValue;
}

StatusNotification.prototype = new Notification();
StatusNotification.prototype.constructor = StatusNotification;



function NotificationDetailInfo(mainText, subText) {
  // constructor of NotificationDetailInfo
  var nativeParam = {
    'mainText': args.mainText,
    'subText': args.subText
  };
  var syncResult = callNative('NotificationDetailInfo_constructor', nativeParam);

  this.mainText = mainText;
  this.subText = subText;
}




exports = new NotificationManager();
tizen.StatusNotification = StatusNotification;

