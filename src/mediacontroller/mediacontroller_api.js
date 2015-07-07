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
var native_ = new xwalk.utils.NativeManager(extension);

// TODO(r.galka) CAPI have no dedicated methods for position/shuffle/repeat change.
// It should be updated when new version of CAPI will be available.
// For now implementation is using internal commands.
var internal_commands_ = {
  sendPlaybackPosition: '__internal_sendPlaybackPosition',
  sendShuffleMode: '__internal_sendShuffleMode',
  sendRepeatMode: '__internal_sendRepeatMode'
};

function ListenerManager(native, listenerName, handle) {
  this.listeners = {};
  this.nextId = 1;
  this.nativeSet = false;
  this.native = native;
  this.listenerName = listenerName;
  this.handle = handle || function(msg, listener, watchId) {};
}

ListenerManager.prototype.addListener = function(callback) {
  var id = this.nextId;
  if (!this.nativeSet) {
    this.native.addListener(this.listenerName, function(msg) {
      for (var watchId in this.listeners) {
        if (this.listeners.hasOwnProperty(watchId)) {
          var stop = this.handle(msg, this.listeners[watchId], watchId);
          if (stop) {
            break;
          }
        }
      }
    }.bind(this));

    this.nativeSet = true;
  }

  this.listeners[id] = callback;
  ++this.nextId;
  return id;
};

ListenerManager.prototype.removeListener = function(watchId) {
  if (this.listeners.hasOwnProperty(watchId)) {
    delete this.listeners[watchId];
  }
};

var ServerCommandListener = new ListenerManager(native_, '_ServerCommandListener', function(msg, listener) {
  var data = undefined;
  data = listener(msg.clientName, msg.command, msg.data);

  if (type_.isUndefined(data)) {
   data = null;
  }

  var nativeData = {
    clientName: msg.clientName,
    replyId: msg.replyId,
    data: data
  };

  var result = native_.callSync('MediaControllerServer_replyCommand', nativeData);
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
});

var ReplyCommandListener = new ListenerManager(native_, '_ReplyCommandListener', function(msg, listener, watchId) {
  if (msg.replyId === watchId) {
    listener(msg.data);
    this.removeListener(watchId);
    return true;
  }

  return false;
});

var ServerPlaybackInfoListener = new ListenerManager(native_, '_ServerPlaybackInfoListener', function(msg, listener) {
  if (msg.action === 'onplaybackstaterequest') {
    listener[msg.action](msg.state);
  }
  if (msg.action === 'onplaybackpositionrequest') {
    listener[msg.action](msg.position);
  }
  if (msg.action === 'onshufflemoderequest' || msg.action === 'onrepeatmoderequest') {
    listener[msg.action](msg.mode);
  }
});

var ServerInfoStatusListener = new ListenerManager(native_, '_ServerInfoStatusListener', function(msg, listener) {
  listener(msg.state);
});

var ServerInfoPlaybackInfoListener = new ListenerManager(native_, '_ServerInfoPlaybackInfoListener', function(msg, listener) {
  if (msg.action === 'onplaybackchanged') {
    listener[msg.action](msg.state, msg.position);
  }
  if (msg.action === 'onshufflemodechanged' || msg.action === 'onrepeatmodechanged') {
    listener[msg.action](msg.mode);
  }
  if (msg.action === 'onmetadatachanged') {
    listener[msg.action](new MediaControllerMetadata(msg.metadata));
  }
});

var EditManager = function() {
  this.isAllowed = false;
};

EditManager.prototype.allow = function() {
  this.isAllowed = true;
};

EditManager.prototype.disallow = function() {
  this.isAllowed = false;
};

var edit_ = new EditManager();


var MediaControllerServerState = {
  ACTIVE: 'ACTIVE',
  INACTIVE: 'INACTIVE'
};

var MediaControllerPlaybackState = {
  PLAY: 'PLAY',
  PAUSE: 'PAUSE',
  STOP: 'STOP',
  NEXT: 'NEXT',
  PREV: 'PREV',
  FORWARD: 'FORWARD',
  REWIND: 'REWIND'
};


function MediaControllerManager() {}

MediaControllerManager.prototype.getClient = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.MEDIACONTROLLER_CLIENT);

  var result = native_.callSync('MediaControllerManager_getClient', {});

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  return new MediaControllerClient(native_.getResultObject(result));
};

MediaControllerManager.prototype.createServer = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.MEDIACONTROLLER_SERVER);

  var result = native_.callSync('MediaControllerManager_createServer', {});
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  return new MediaControllerServer(native_.getResultObject(result));
};


var MediaControllerMetadata = function(data) {
  var _title = '';
  var _artist = '';
  var _album = '';
  var _author = '';
  var _genre = '';
  var _duration = '';
  var _date = '';
  var _copyright = '';
  var _description = '';
  var _trackNum = '';
  var _picture = '';
  Object.defineProperties(this, {
    title: {
      get: function() {return _title;},
      set: function(v) {_title = converter_.toString(v)},
      enumerable: true
    },
    artist: {
      get: function() {return _artist;},
      set: function(v) {_artist = converter_.toString(v)},
      enumerable: true
    },
    album: {
      get: function() {return _album;},
      set: function(v) {_album = converter_.toString(v)},
      enumerable: true
    },
    author: {
      get: function() {return _author;},
      set: function(v) {_author = converter_.toString(v)},
      enumerable: true
    },
    genre: {
      get: function() {return _genre;},
      set: function(v) {_genre = converter_.toString(v)},
      enumerable: true
    },
    duration: {
      get: function() {return _duration;},
      set: function(v) {_duration = converter_.toString(v)},
      enumerable: true
    },
    date: {
      get: function() {return _date;},
      set: function(v) {_date = converter_.toString(v)},
      enumerable: true
    },
    copyright: {
      get: function() {return _copyright;},
      set: function(v) {_copyright = converter_.toString(v)},
      enumerable: true
    },
    description: {
      get: function() {return _description;},
      set: function(v) {_description = converter_.toString(v)},
      enumerable: true
    },
    trackNum: {
      get: function() {return _trackNum;},
      set: function(v) {_trackNum = converter_.toString(v)},
      enumerable: true
    },
    picture: {
      get: function() {return _picture;},
      set: function(v) {_picture = converter_.toString(v)},
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
};

var MediaControllerPlaybackInfo = function(data) {
  var _state = 'STOP';
  var _position = 0;
  var _shuffleMode = false;
  var _repeatMode = false;
  var _metadata = new MediaControllerMetadata();
  Object.defineProperties(this, {
    state: {
      get: function() {
        return _state;
      },
      set: function(v) {
        _state = edit_.isAllowed && v ? v : _state;
      },
      enumerable: true
    },
    position: {
      get: function() {
        return _position;
      },
      set: function(v) {
        _position = edit_.isAllowed && v ? v : _position;
      },
      enumerable: true
    },
    shuffleMode: {
      get: function() {
        return _shuffleMode;
      },
      set: function(v) {
        _shuffleMode = edit_.isAllowed && v ? v : _shuffleMode;
      },
      enumerable: true
    },
    repeatMode: {
      get: function() {
        return _repeatMode;
      },
      set: function(v) {
        _repeatMode = edit_.isAllowed && v ? v : _repeatMode;
      },
      enumerable: true
    },
    metadata: {
      get: function() {
        return _metadata;
      },
      set: function(v) {
        _metadata = edit_.isAllowed && v ? new MediaControllerMetadata(v) : _metadata;
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
};

function MediaControllerServer(data) {
  Object.defineProperties(this, {
    playbackInfo: {
      value: new MediaControllerPlaybackInfo(data),
      writable: false,
      enumerable: true
    }
  });
}

MediaControllerServer.prototype.updatePlaybackState = function(state) {
  var args = validator_.validateArgs(arguments, [
    {name: 'state', type: types_.ENUM, values: Object.keys(MediaControllerPlaybackState)}
  ]);

  var data = {
    state: args.state
  };

  var result = native_.callSync('MediaControllerServer_updatePlaybackState', data);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  edit_.allow();
  this.playbackInfo.state = args.state;
  edit_.disallow();
};

MediaControllerServer.prototype.updatePlaybackPosition = function(position) {
  var args = validator_.validateArgs(arguments, [
    {name: 'position', type: types_.UNSIGNED_LONG_LONG}
  ]);

  var data = {
    position: args.position
  };

  var result = native_.callSync('MediaControllerServer_updatePlaybackPosition', data);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  edit_.allow();
  this.playbackInfo.position = args.position;
  edit_.disallow();
};

MediaControllerServer.prototype.updateShuffleMode = function(mode) {
  var args = validator_.validateArgs(arguments, [
    {name: 'mode', type: types_.BOOLEAN}
  ]);

  var data = {
    mode: args.mode
  };

  var result = native_.callSync('MediaControllerServer_updateShuffleMode', data);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  edit_.allow();
  this.playbackInfo.shuffleMode = args.mode;
  edit_.disallow();
};

MediaControllerServer.prototype.updateRepeatMode = function(mode) {
  var args = validator_.validateArgs(arguments, [
    {name: 'mode', type: types_.BOOLEAN}
  ]);

  var data = {
    mode: args.mode
  };

  var result = native_.callSync('MediaControllerServer_updateRepeatMode', data);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  edit_.allow();
  this.playbackInfo.repeatMode = args.mode;
  edit_.disallow();
};

MediaControllerServer.prototype.updateMetadata = function(metadata) {
  var args = validator_.validateArgs(arguments, [
    {name: 'metadata', type: types_.PLATFORM_OBJECT, values: MediaControllerMetadata}
  ]);

  var data = {
    metadata: args.metadata
  };

  var result = native_.callSync('MediaControllerServer_updateMetadata', data);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  edit_.allow();
  this.playbackInfo.metadata = args.metadata;
  edit_.disallow();
};

MediaControllerServer.prototype.addChangeRequestPlaybackInfoListener = function(listener) {
  var args = validator_.validateArgs(arguments, [{
    name: 'listener',
    type: types_.LISTENER,
    values: [
      'onplaybackstaterequest',
      'onplaybackpositionrequest',
      'onshufflemoderequest',
      'onrepeatmoderequest'
    ]
  }]);

  if (type_.isEmptyObject(ServerPlaybackInfoListener.listeners)) {
    var result = native_.callSync('MediaControllerServer_addChangeRequestPlaybackInfoListener', {
      listenerId: ServerPlaybackInfoListener.listenerName
    });
    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }
  }

  return ServerPlaybackInfoListener.addListener(args.listener);
};

MediaControllerServer.prototype.removeChangeRequestPlaybackInfoListener = function(watchId) {
  var args = validator_.validateArgs(arguments, [
    {name: 'watchId', type: types_.LONG}
  ]);

  ServerPlaybackInfoListener.removeListener(args.watchId);

  if (type_.isEmptyObject(ServerPlaybackInfoListener.listeners)) {
    native_.callSync('MediaControllerServer_removeCommandListener');
  }
};

MediaControllerServer.prototype.addCommandListener = function(listener) {
  var args = validator_.validateArgs(arguments, [
    {name: 'listener', type: types_.FUNCTION}
  ]);

  if (type_.isEmptyObject(ServerCommandListener.listeners)) {
    var result = native_.callSync('MediaControllerServer_addCommandListener', {
      listenerId: ServerCommandListener.listenerName
    });
    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }
  }

  return ServerCommandListener.addListener(args.listener);
};

MediaControllerServer.prototype.removeCommandListener = function(watchId) {
  var args = validator_.validateArgs(arguments, [
    {name: 'watchId', type: types_.LONG}
  ]);

  ServerCommandListener.removeListener(args.watchId);

  if (type_.isEmptyObject(ServerCommandListener.listeners)) {
    native_.callSync('MediaControllerServer_removeCommandListener');
  }
};


function MediaControllerClient() {}

MediaControllerClient.prototype.findServers = function(successCallback, errorCallback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'successCallback', type: types_.FUNCTION},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }
    var info = [];
    var data = native_.getResultObject(result);
    for (var i = 0; i < data.length; i++) {
      info.push(new MediaControllerServerInfo(data[i]));
    }
    native_.callIfPossible(args.successCallback, info);
  };

  native_.call('MediaControllerClient_findServers', {}, callback);
};

MediaControllerClient.prototype.getLatestServerInfo = function() {

  var result = native_.callSync('MediaControllerClient_getLatestServerInfo', {});

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  var serverInfo = native_.getResultObject(result);
  if (serverInfo) {
    serverInfo = new MediaControllerServerInfo(serverInfo);
  }
  return serverInfo;
};


function MediaControllerServerInfo(data) {
  Object.defineProperties(this, {
    name: {
      value: data.name,
      writable: false,
      enumerable: true
    },
    state: {
      value: data.state,
      writable: false,
      enumerable: true
    },
    playbackInfo: {
      get: function () {
        var result = native_.callSync('MediaControllerClient_getPlaybackInfo', {name: this.name});
        if (native_.isFailure(result)) {
          throw new native_.getErrorObject(result);
        }
        edit_.allow();
        var data = native_.getResultObject(result);
        var playbackInfo = new MediaControllerPlaybackInfo(data);
        edit_.disallow();

        return playbackInfo;
      }.bind(this),
      set: function() {},
      enumerable: true
    }
  });
}


MediaControllerServerInfo.prototype.sendPlaybackState = function(state, successCallback, errorCallback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'state', type: types_.ENUM, values: Object.keys(MediaControllerPlaybackState)},
    {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var data = {
    name: this.name,
    state: args.state
  };

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.successCallback);
  };

  native_.call('MediaControllerServerInfo_sendPlaybackState', data, callback);
};

MediaControllerServerInfo.prototype.sendPlaybackPosition = function(position, successCallback, errorCallback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'position', type: types_.LONG_LONG},
    {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  if (args.position < 0) {
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR);
  }

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.successCallback);
  };

  var data = {
    position: args.position
  };

  sendDefinedCommand(this.name, internal_commands_.sendPlaybackPosition, data, callback);
};

MediaControllerServerInfo.prototype.sendShuffleMode = function(mode, successCallback, errorCallback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'mode', type: types_.BOOLEAN},
    {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.successCallback);
  };

  var data = {
    mode: args.mode
  };
  sendDefinedCommand(this.name, internal_commands_.sendShuffleMode, data, callback);
};

MediaControllerServerInfo.prototype.sendRepeatMode = function(mode, successCallback, errorCallback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'mode', type: types_.BOOLEAN},
    {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.successCallback);
  };

  var data = {
    mode: args.mode
  };
  sendDefinedCommand(this.name, internal_commands_.sendRepeatMode, data, callback);
};

function sendDefinedCommand(name_, command_, data_, callback_) {
  var nativeData = {
    command: command_,
    data: data_,
    name: name_
  };

  var replyId = ReplyCommandListener.addListener(callback_);

  nativeData.replyId = replyId;
  nativeData.listenerId = ReplyCommandListener.listenerName;

  native_.call('MediaControllerServerInfo_sendCommand', nativeData, callback_);
};

MediaControllerServerInfo.prototype.sendCommand = function(command, data, successCallback, errorCallback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'command', type: types_.STRING},
    {name: 'data', type: types_.DICTIONARY},
    {name: 'successCallback', type: types_.FUNCTION},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var nativeData = {
    command: args.command,
    data: args.data,
    name: this.name
  };

  var replyId = ReplyCommandListener.addListener(successCallback);

  nativeData.replyId = replyId;
  nativeData.listenerId = ReplyCommandListener.listenerName;
  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }
    args.successCallback(native_.getResultObject(result));
  };

  native_.call('MediaControllerServerInfo_sendCommand', nativeData, callback);
};

MediaControllerServerInfo.prototype.addServerStatusChangeListener = function(listener) {
  var args = validator_.validateArgs(arguments, [
    {name: 'listener', type: types_.FUNCTION}
  ]);

  if (type_.isEmptyObject(ServerInfoStatusListener.listeners)) {
    var result = native_.callSync('MediaControllerServerInfo_addServerStatusChangeListener', {
      listenerId: ServerInfoStatusListener.listenerName
    });
    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }
  }

  return ServerInfoStatusListener.addListener(args.listener);
};

MediaControllerServerInfo.prototype.removeServerStatusChangeListener = function(watchId) {
  var args = validator_.validateArgs(arguments, [
    {name: 'watchId', type: types_.LONG}
  ]);

  ServerInfoStatusListener.removeListener(args.watchId);

  if (type_.isEmptyObject(ServerInfoStatusListener.listeners)) {
    native_.callSync('MediaControllerServerInfo_removeServerStatusChangeListener');
  }
};

MediaControllerServerInfo.prototype.addPlaybackInfoChangeListener = function(listener) {
  var args = validator_.validateArgs(arguments, [{
    name: 'listener',
    type: types_.LISTENER,
    values: [
      'onplaybackchanged',
      'onshufflemodechanged',
      'onrepeatmodechanged',
      'onmetadatachanged'
    ]
  }]);

  if (type_.isEmptyObject(ServerInfoPlaybackInfoListener.listeners)) {
    var result = native_.callSync(
        'MediaControllerServerInfo_addPlaybackInfoChangeListener', {
          listenerId: ServerInfoPlaybackInfoListener.listenerName
        });
    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }
  }

  return ServerInfoPlaybackInfoListener.addListener(args.listener);
};

MediaControllerServerInfo.prototype.removePlaybackInfoChangeListener = function(watchId) {
  var args = validator_.validateArgs(arguments, [
    {name: 'watchId', type: types_.LONG}
  ]);

  ServerInfoPlaybackInfoListener.removeListener(args.watchId);

  if (type_.isEmptyObject(ServerInfoPlaybackInfoListener.listeners)) {
    native_.callSync('MediaControllerServerInfo_removePlaybackInfoChangeListener');
  }
};


exports = new MediaControllerManager();
