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

var native = new xwalk.utils.NativeManager(extension);
var validator = xwalk.utils.validator;
var validatorType = xwalk.utils.type;


/**
 * An enumerator that defines window types.
 * @enum {string}
 */
var WindowType = {
  MAIN: 'MAIN'
};


/**
 * An enumerator to indicate the tune mode.
 * @enum {string}
 */
var TuneMode = {
  ALL: 'ALL',
  DIGITAL: 'DIGITAL',
  ANALOG: 'ANALOG',
  FAVORITE: 'FAVORITE'
};

function ChannelInfo(dict) {
  for (var key in dict) {
    if (dict.hasOwnProperty(key)) {
      Object.defineProperty(this, key, {
        value: dict[key],
        enumerable: true
      });
    }
  }
}

function dictListToChannelList(list) {
  var result = [],
      i = 0,
      il = list.length;
  for (; i < il; ++i) {
    result.push(new ChannelInfo(list[i]));
  }
  return result;
}

function ProgramInfo(dict) {
  for (var key in dict) {
    if (dict.hasOwnProperty(key)) {
      Object.defineProperty(this, key, {
        value: key === 'startTime' ? new tizen.TZDate(new Date(dict[key])) : dict[key],
        enumerable: true
      });
    }
  }
}

function dictListToProgramList(list) {
  var result = [],
      i = 0,
      il = list.length;
  for (; i < il; ++i) {
    result.push(new ProgramInfo(list[i]));
  }
  return result;
}

function ListenerManager(native, listenerName) {
  this.listeners = {};
  this.nextId = 1;
  native.addListener(listenerName, this.onListenerCalled.bind(this));
}

ListenerManager.prototype.onListenerCalled = function(msg) {
  var channel = new ChannelInfo(msg.channel);
  for (var key in this.listeners) {
    if (this.listeners.hasOwnProperty(key)) {
      this.listeners[key](channel, msg.windowType);
    }
  }
};

ListenerManager.prototype.addListener = function(callback) {
  var id = this.nextId;
  this.listeners[id] = callback;
  ++this.nextId;
  return id;
};

ListenerManager.prototype.removeListener = function(watchId) {
  if (this.listeners.hasOwnProperty(watchId)) {
    delete this.listeners[watchId];
  }
};


/**
 * @const
 * @type {string}
 */
var CHANNEL_CHANGE_LISTENER = 'ChannelChanged';

var channelListener = new ListenerManager(native, CHANNEL_CHANGE_LISTENER);


/**
 * @const
 * @type {string}
 */
var PROGRAMINFO_LISTENER = 'ProgramInfoReceived';

//TVChannelManager interface
function TVChannelManager() {
  if (!(this instanceof TVChannelManager)) {
    throw new TypeError;
  }
}


TVChannelManager.prototype.tune = function(tuneOption, successCallback, errorCallback, windowType) {
  var data = validator.validateArgs(arguments, [
    {
      name: 'tuneOption',
      type: validator.Types.DICTIONARY
    },
    {
      name: 'callback',
      type: validator.Types.LISTENER,
      values: ['onsuccess', 'onnosignal', 'onprograminforeceived']
    },
    {
      name: 'errorCallback',
      type: validator.Types.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'windowType',
      optional: true,
      nullable: true,
      type: validator.Types.ENUM,
      values: validatorType.getValues(WindowType)
    }
  ]);
  native.addListener(PROGRAMINFO_LISTENER, function(msg) {
    if (validatorType.isFunction(data.callback.onprograminforeceived)) {
      data.callback.onprograminforeceived(new ProgramInfo(msg.program), msg.windowType);
    }
  });
  native.call('TVChannelManager_tune', {
    tuneOption: data.tuneOption,
    windowType: data.windowType
  }, function(msg) {
    if (msg.error) {
      if (validatorType.isFunction(data.errorCallback)) {
        data.errorCallback(native.getErrorObject(msg));
      }
    } else if (msg.nosignal) {
      if (validatorType.isFunction(data.callback.onnosignal)) {
        data.callback.onnosignal();
      }
    } else if (msg.success) {
      if (validatorType.isFunction(data.callback.onsuccess)) {
        data.callback.onsuccess(new ChannelInfo(msg.channel), msg.windowType);
      }
    }
  });
};

function tuneUpDown(args, methodName) {
  var data = validator.validateArgs(args, [
    {
      name: 'callback',
      type: validator.Types.LISTENER,
      values: ['onsuccess', 'onnosignal', 'onprograminforeceived']
    },
    {
      name: 'errorCallback',
      type: validator.Types.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'tuneMode',
      optional: true,
      nullable: true,
      type: validator.Types.ENUM,
      values: validatorType.getValues(TuneMode)
    },
    {
      name: 'windowType',
      optional: true,
      nullable: true,
      type: validator.Types.ENUM,
      values: validatorType.getValues(WindowType)
    }
  ]);
  native.addListener(PROGRAMINFO_LISTENER, function(msg) {
    if (validatorType.isFunction(data.callback.onprograminforeceived)) {
      data.callback.onprograminforeceived(new ProgramInfo(msg.program), msg.windowType);
    }
  });
  native.call(methodName, {
    tuneMode: data.tuneMode,
    windowType: data.windowType
  }, function(msg) {
    if (msg.error) {
      if (validatorType.isFunction(data.errorCallback)) {
        data.errorCallback(native.getErrorObject(msg));
      }
    } else if (msg.nosignal) {
      if (validatorType.isFunction(data.callback.onnosignal)) {
        data.callback.onnosignal();
      }
    } else if (msg.success) {
      if (validatorType.isFunction(data.callback.onsuccess)) {
        data.callback.onsuccess(new ChannelInfo(msg.channel), msg.windowType);
      }
    }
  });
}

TVChannelManager.prototype.tuneUp = function(callback, errorCallback, tuneMode, windowType) {
  tuneUpDown(arguments, 'TVChannelManager_tuneUp');
};

TVChannelManager.prototype.tuneDown = function(successCallback,
    errorCallback, tuneMode, windowType) {
  tuneUpDown(arguments, 'TVChannelManager_tuneDown');
};

TVChannelManager.prototype.findChannel = function(major, minor, successCallback, errorCallback) {
  var args = validator.validateArgs(arguments, [
    {
      name: 'major',
      optional: false,
      nullable: false,
      type: validator.Types.LONG
    },
    {
      name: 'minor',
      optional: false,
      nullable: false,
      type: validator.Types.LONG
    },
    {
      name: 'successCallback',
      type: validator.Types.FUNCTION,
      optional: false,
      nullable: false
    },
    {
      name: 'errorCallback',
      type: validator.Types.FUNCTION,
      optional: true,
      nullable: true
    }
  ]);
  native.call('TVChannelManager_findChannel', {
    major: args.major,
    minor: args.minor
  }, function(msg) {
    if (msg.error) {
      if (validatorType.isFunction(args.errorCallback)) {
        args.errorCallback(native.getErrorObject(msg));
      }
    } else {
      args.successCallback(dictListToChannelList(msg.channelInfos));
    }
  });
};

TVChannelManager.prototype.getChannelList = function(successCallback,
    errorCallback, tuneMode, start, number) {
  var args = validator.validateArgs(arguments, [
    {
      name: 'successCallback',
      type: validator.Types.FUNCTION,
      optional: false,
      nullable: false
    },
    {
      name: 'errorCallback',
      type: validator.Types.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'tuneMode',
      optional: true,
      nullable: true,
      type: validator.Types.ENUM,
      values: validatorType.getValues(TuneMode)
    },
    {
      name: 'nStart',
      optional: true,
      nullable: true,
      type: validator.Types.LONG
    },
    {
      name: 'number',
      optional: true,
      nullable: true,
      type: validator.Types.LONG
    }
  ]);
  if (args.nStart < 0) {
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
      "nStart is lower than 0");
  }
  if (args.number < 0) {
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR,
      "number is lower than 0");
  }
  native.call('TVChannelManager_getChannelList', {
    tuneMode: args.tuneMode,
    nStart: args.nStart,
    number: args.number
  }, function(msg) {
    if (msg.error) {
      if (validatorType.isFunction(args.errorCallback)) {
        args.errorCallback(native.getErrorObject(msg));
      }
    } else {
      args.successCallback(dictListToChannelList(msg.channelInfos));
    }
  });
};

TVChannelManager.prototype.getCurrentChannel = function(windowType) {
  var args = validator.validateArgs(arguments, [
    {
      name: 'windowType',
      optional: true,
      nullable: true,
      type: validator.Types.ENUM,
      values: validatorType.getValues(WindowType)
    }
  ]);
  var ret = native.callSync('TVChannelManager_getCurrentChannel', {
    windowType: args.windowType ? args.windowType : WindowType.MAIN
  });
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
  return new ChannelInfo(native.getResultObject(ret));
};

TVChannelManager.prototype.getProgramList = function(channelInfo,
    startTime, successCallback, errorCallback, duration) {
  var args = validator.validateArgs(arguments, [
    {
      name: 'channelInfo',
      type: validator.Types.PLATFORM_OBJECT,
      values: ChannelInfo
    },
    {
      name: 'startTime',
      type: validator.Types.PLATFORM_OBJECT,
      values: tizen.TZDate
    },
    {
      name: 'successCallback',
      type: validator.Types.FUNCTION,
      optional: false,
      nullable: false
    },
    {
      name: 'errorCallback',
      type: validator.Types.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'duration',
      optional: true,
      nullable: true,
      type: validator.Types.LONG
    }
  ]);
  native.call('TVChannelManager_getProgramList', {
    channelId: args.channelInfo._serviceId,
    startTime: args.startTime.getTime(),
    duration: args.duration
  }, function(msg) {
    if (msg.error) {
      if (validatorType.isFunction(args.errorCallback)) {
        args.errorCallback(native.getErrorObject(msg));
      }
    } else {
      args.successCallback(dictListToProgramList(msg.programInfos));
    }
  });
};

TVChannelManager.prototype.getCurrentProgram = function(windowType) {
  var args = validator.validateArgs(arguments, [
    {
      name: 'windowType',
      optional: true,
      nullable: true,
      type: validator.Types.ENUM,
      values: validatorType.getValues(WindowType)
    }
  ]);
  var ret = native.callSync('TVChannelManager_getCurrentProgram', {
    windowType: args.windowType ? args.windowType : WindowType.MAIN
  });
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
  return new ProgramInfo(native.getResultObject(ret));
};

TVChannelManager.prototype.addChannelChangeListener = function(callback, windowType) {
  var args = validator.validateArgs(arguments, [
    {
      name: 'callback',
      type: validator.Types.FUNCTION
    },
    {
      name: 'windowType',
      optional: true,
      nullable: true,
      type: validator.Types.ENUM,
      values: validatorType.getValues(WindowType)
    }
  ]);

  return channelListener.addListener(args.callback);
};

TVChannelManager.prototype.removeChannelChangeListener = function(listenerId) {
  channelListener.removeListener(listenerId);
};

TVChannelManager.prototype.addProgramChangeListener = function(successCallback, windowType) {
  return undefined;
};

TVChannelManager.prototype.removeProgramChangeListener = function(listenerId) {
  return undefined;
};

TVChannelManager.prototype.getNumOfAvailableTuner = function()
    {
  var ret = native.callSync('TVChannelManager_getNumOfAvailableTuner');
  if (native.isFailure(ret)) {
    throw native.getErrorObject(ret);
  }
  return native.getResultObject(ret);
};

exports = new TVChannelManager();
