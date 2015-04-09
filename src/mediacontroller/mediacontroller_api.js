// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


var utils_ = xwalk.utils;
var type_ = utils_.type;
var converter_ = utils_.converter;
var validator_ = utils_.validator;
var types_ = validator_.Types;
var native_ = new xwalk.utils.NativeManager(extension);


function ListenerManager(native, listenerName) {
  this.listeners = {};
  this.nextId = 1;
  this.nativeSet = false;
  this.native = native;
  this.listenerName = listenerName;
}

ListenerManager.prototype.onListenerCalled = function(msg) {
  for (var key in this.listeners) {
    if (this.listeners.hasOwnProperty(key)) {
      if ('Command' === msg.type) {
        this.listeners[key](msg.clientName, msg.command, msg.data);
      } else if ('RequestPlaybackInfo' === msg.type) {
        if (msg.action === 'onplaybackstaterequest') {
          this.listeners[key][msg.action](msg.state);
        }
        if (msg.action === 'onplaybackpositionrequest') {
          this.listeners[key][msg.action](msg.position);
        }
        if (msg.action === 'onshufflemoderequest' || msg.action === 'onrepeatmoderequest') {
          this.listeners[key][msg.action](msg.mode);
        }
      } else if ('PlaybackInfo' === msg.type) {
        if (msg.action === 'onplaybackstaterequest') {
          this.listeners[key][msg.action](msg.state);
        }
        if (msg.action === 'onplaybackpositionrequest') {
          this.listeners[key][msg.action](msg.position);
        }
        if (msg.action === 'onshufflemoderequest' || msg.action === 'onrepeatmoderequest') {
          this.listeners[key][msg.action](msg.mode);
        }
        if (msg.action === 'onmetadatachanged') {
          this.listeners[key][msg.action](new MediaControllerMetadata(msg.metadata));
        }
      } else if ('Status' === msg.type) {
        this.listeners[key](msg.status);
      }
    }
  }
};

ListenerManager.prototype.addListener = function(callback) {
  var id = this.nextId;
  if (!this.nativeSet) {
    this.native.addListener(this.listenerName, this.onListenerCalled.bind(this));
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

var ServerCommandListener = new ListenerManager(native_, '_ServerCommandListener');
var ServerPlaybackInfoListener = new ListenerManager(native_, '_ServerPlaybackInfoListener');
var ServerInfoStatusListener = new ListenerManager(native_, '_ServerInfoStatusListener');
var ServerInfoPlaybackInfoListener = new ListenerManager(native_, '_ServerInfoPlaybackInfoListener');

var EditManager = function() {
  this.isAllowed = false;
};

EditManager.prototype.allow = function() {
  this.isAllowed = true;
};

EditManager.prototype.disallow = function() {
  this.isAllowed = false;
};

var _edit = new EditManager();


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

  var result = native_.callSync('MediaControllerManager_getClient', {});

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  var client = new MediaControllerClient(native_.getResultObject(result));
  return client;
};

MediaControllerManager.prototype.createServer = function() {
  var result = native_.callSync('MediaControllerManager_createServer', {});
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  var server = new MediaControllerServer(native_.getResultObject(result));
  return server;
};


var MediaControllerMetadata = function(data) {
  var data = data || {};
  Object.defineProperties(this, {
    title: {
      value: data.title || '',
      writable: true,
      enumerable: true
    },
    artist: {
      value: data.artist || '',
      writable: true,
      enumerable: true
    },
    album: {
      value: data.album || '',
      writable: true,
      enumerable: true
    },
    author: {
      value: data.author || '',
      writable: true,
      enumerable: true
    },
    genre: {
      value: data.genre || '',
      writable: true,
      enumerable: true
    },
    duration: {
      value: data.duration || '',
      writable: true,
      enumerable: true
    },
    date: {
      value: data.date || '',
      writable: true,
      enumerable: true
    },
    copyright: {
      value: data.copyright || '',
      writable: true,
      enumerable: true
    },
    description: {
      value: data.description || '',
      writable: true,
      enumerable: true
    },
    trackNum: {
      value: data.trackNum || '',
      writable: true,
      enumerable: true
    },
    picture: {
      value: data.picture || '',
      writable: true,
      enumerable: true
    }
  });
};

var MediaControllerPlaybackInfo = function(data) {
  var data = data || {};
  Object.defineProperties(this, {
    state: {
      value: data.state || 'STOP',
      writable: false,
      enumerable: true
    },
    position: {
      value: data.position || 0,
      writable: false,
      enumerable: true
    },
    shuffleMode: {
      value: data.shuffleMode || false,
      writable: false,
      enumerable: true
    },
    repeatMode: {
      value: data.repeatMode || false,
      writable: false,
      enumerable: true
    },
    metadata: {
      value: new MediaControllerMetadata(data.metadata),
      writable: false,
      enumerable: true
    }
  });
};

function MediaControllerServer(data) {
  var _playbackInfo = new MediaControllerPlaybackInfo(data);
  Object.defineProperties(this, {
    playbackInfo: {
      get: function() {
        return _playbackInfo;
      },
      set: function(v) {
        _playbackInfo = _edit.isAllowed && v ? new MediaControllerPlaybackInfo(v) : _playbackInfo;
      },
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

  _edit.allow();
  this.playbackInfo = new MediaControllerPlaybackInfo(native_.getResultObject(result));
  _edit.disallow();
};

MediaControllerServer.prototype.updatePlaybackPosition = function(position) {
  var args = validator_.validateArgs(arguments, [
    {name: 'position', type: types_.LONG_LONG}
  ]);

  var data = {
    position: args.position
  };

  var result = native_.callSync('MediaControllerServer_updatePlaybackPosition', data);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  _edit.allow();
  this.playbackInfo = new MediaControllerPlaybackInfo(native_.getResultObject(result));
  _edit.disallow();
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

  _edit.allow();
  this.playbackInfo = new MediaControllerPlaybackInfo(native_.getResultObject(result));
  _edit.disallow();
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

  _edit.allow();
  this.playbackInfo = new MediaControllerPlaybackInfo(native_.getResultObject(result));
  _edit.disallow();
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

  _edit.allow();
  this.playbackInfo = new MediaControllerPlaybackInfo(native_.getResultObject(result));
  _edit.disallow();
};

MediaControllerServer.prototype.addChangeRequestPlaybackInfoListener = function(listener) {
  var args = validator_.validateArgs(arguments, [
    {name: 'listener', type: types_.LISTENER, values: ['onplaybackstaterequest', 'onplaybackpositionrequest', 'onshufflemoderequest', 'onrepeatmoderequest']}
  ]);

  if (type_.isEmptyObject(ServerPlaybackInfoListener.listeners)) {
    var result = native_.callSync('MediaControllerServer_addChangeRequestPlaybackInfoListener');
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
    var result = native_.callSync('MediaControllerServer_addCommandListener');
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
      info.push(new MediaControllerServerInfo(data));
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

  var serverInfo = new MediaControllerServerInfo(native_.getResultObject(result));
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
      value: new MediaControllerPlaybackInfo(data.playbackInfo),
      writable: false,
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

  var data = {
    position: args.position
  };

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.successCallback);
  };

  native_.call('MediaControllerServerInfo_sendPlaybackPosition', data, callback);
};

MediaControllerServerInfo.prototype.sendShuffleMode = function(mode, successCallback, errorCallback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'mode', type: types_.BOOLEAN},
    {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var data = {
    mode: args.mode
  };

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.successCallback);
  };

  native_.call('MediaControllerServerInfo_sendShuffleMode', data, callback);
};

MediaControllerServerInfo.prototype.sendRepeatMode = function(mode, successCallback, errorCallback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'mode', type: types_.BOOLEAN},
    {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var data = {
    mode: args.mode
  };

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.successCallback);
  };

  native_.call('MediaControllerServerInfo_sendRepeatMode', data, callback);
};

MediaControllerServerInfo.prototype.sendCommand = function(command, data, successCallback, errorCallback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'command', type: types_.STRING},
    {name: 'data', type: types_.DICTIONARY},
    {name: 'successCallback', type: types_.FUNCTION},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var data = {
    command: args.command,
    data: args.data
  };

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.successCallback);
  };

  native_.call('MediaControllerServerInfo_sendCommand', data, callback);
};

MediaControllerServerInfo.prototype.addServerStatusChangeListener = function(listener) {
  var args = validator_.validateArgs(arguments, [
    {name: 'listener', type: types_.FUNCTION}
  ]);

  if (type_.isEmptyObject(ServerInfoStatusListener.listeners)) {
    var result = native_.callSync('MediaControllerServerInfo_addServerStatusChangeListener');
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
  var args = validator_.validateArgs(arguments, [
    {name: 'listener', type: types_.LISTENER, values: ['onplaybackstatechanged', 'onplaybackpositionchanged', 'onshufflemodechanged', 'onrepeatmodechanged', 'onmetadatachanged']}
  ]);

  if (type_.isEmptyObject(ServerInfoPlaybackInfoListener.listeners)) {
    var result = native_.callSync('MediaControllerServerInfo_addPlaybackInfoChangeListener');
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
