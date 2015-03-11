// Content

var validator_ = xwalk.utils.validator;
var types_ = validator_.Types;


var callbackId = 0;
var callbacks = {};

extension.setMessageListener(function(json) {
  var result = JSON.parse(json);
  var callback = callbacks[result['callbackId']];
  callback(result);
});

function nextCallbackId() {
  return callbackId++;
}


var ExceptionMap = {
  'UnknownError' : tizen.WebAPIException.UNKNOWN_ERR ,
  'TypeMismatchError' : tizen.WebAPIException.TYPE_MISMATCH_ERR ,
  'InvalidValuesError' : tizen.WebAPIException.INVALID_VALUES_ERR ,
  'IOError' : tizen.WebAPIException.IO_ERR ,
  'ServiceNotAvailableError' : tizen.WebAPIException.SERVICE_NOT_AVAILABLE_ERR ,
  'SecurityError' : tizen.WebAPIException.SECURITY_ERR ,
  'NetworkError' : tizen.WebAPIException.NETWORK_ERR ,
  'NotSupportedError' : tizen.WebAPIException.NOT_SUPPORTED_ERR ,
  'NotFoundError' : tizen.WebAPIException.NOT_FOUND_ERR ,
  'InvalidAccessError' : tizen.WebAPIException.INVALID_ACCESS_ERR ,
  'AbortError' : tizen.WebAPIException.ABORT_ERR ,
  'QuotaExceededError' : tizen.WebAPIException.QUOTA_EXCEEDED_ERR ,
}

function callNative(cmd, args) {
  var json = {'cmd':cmd, 'args':args};
  var argjson = JSON.stringify(json);
  var resultString = extension.internal.sendSyncMessage(argjson)
  var result = JSON.parse(resultString);

  if (typeof result !== 'object') {
    throw new tizen.WebAPIException(tizen.WebAPIException.UNKNOWN_ERR);
  }

  if (result['status'] == 'success') {
    if(result['result']) {
        return result['result'];
    }
    return true;
  } else if (result['status'] == 'error') {
    var err = result['error'];
    if(err) {
        if(ExceptionMap[err.name]) {
            throw new tizen.WebAPIException(ExceptionMap[err.name], err.message);
        } else {
            throw new tizen.WebAPIException(tizen.WebAPIException.UNKNOWN_ERR, err.message);
        }
    }
    return false;
  }
}


function callNativeWithCallback(cmd, args, callback) {
  if(callback) {
    var id = nextCallbackId();
    args['callbackId'] = id;
    callbacks[id] = callback;
  }
  return callNative(cmd, args);
}

function SetReadOnlyProperty(obj, n, v){
  Object.defineProperty(obj, n, {value:v, writable: false});
}

function Playlist(id, name, numberOfTracks, thumbnailURI) {
  var name_ = name;
  Object.defineProperties(this, {
    'id': { writable: false, value: id, enumerable: true },
    'name': { writable: true, value: name, enumerable: true },
    'numberOfTracks': { writable: false, value: numberOfTracks, enumerable: true },
    'thumbnailURI': {writable: true, value: thumbnailURI, enumerable: true }
  });
}

function PlaylistItem(content,playlist_member_id) {
  var content_ = content;
  var member_id = playlist_member_id;
  Object.defineProperties(this, {
    'content': { writable: false, value: content, enumerable: true },
    'member_id': {enumerable: false,
      set: function(v) { if (v != null) member_id = v},
      get: function() { return member_id; }
    }
  });
}


Playlist.prototype.add = function(item) {
  var args = validator_.validateArgs(arguments, [
    {'name' : 'item', 'type': types_.PLATFORM_OBJECT, 'values': Content, 'optional' : false, 'nullable' : false}
  ]);
  var nativeParam = {
    'playlist_id': this.id
  };

  if (args['item']) {
    nativeParam['content_id'] = args.item.id;
  }
  try {
    var syncResult = callNative('ContentPlaylist_add', nativeParam);
  } catch(e) {
    throw e;
  }
}

Playlist.prototype.remove = function(item) {
  var args = validator_.validateArgs(arguments, [
    {'name' : 'item', 'type': types_.PLATFORM_OBJECT, 'values': PlaylistItem, 'optional' : false, 'nullable' : false}
  ]);
  var nativeParam = {
    'playlist_id': this.id,
    'member_id' : args.item.member_id
  };

  try {
    var syncResult = callNative('ContentPlaylist_remove', nativeParam);
  } catch(e) {
    throw e;
  }
}


Playlist.prototype.removeBatch = function(items) {
  var args = validator_.validateArgs(arguments, [
    {'name' : 'items', 'type': types_.ARRAY},
    {'name' : 'successCallback', 'type': types_.FUNCTION, optional : true, nullable : true},
    {'name' : 'errorCallback', 'type': types_.FUNCTION, optional : true, nullable : true}
  ]);

  var nativeParam = {
    'playlist_id': this.id,
  };

  nativeParam["members"] = [];
  if (args['items']) {
    for (var i = 0; i < args.items.length; i++) {
      var c = args.items[i];
      nativeParam.members.push(c.member_id);
    }
  }

  try {
    var syncResult = callNativeWithCallback('ContentPlaylist_removeBatch', nativeParam, function(result) {
      if (result.status == 'success') {
        args.successCallback();
      }
      else if(result.status == 'error') {
        var err = result['value'];
        args.errorCallback(err);
      }
    });
  } catch(e) {
    throw e;
  }    
}


Playlist.prototype.get = function() {
  var args = validator_.validateArgs(arguments, [
    {'name' : 'successCallback', 'type': types_.FUNCTION},  
    {'name' : 'errorCallback', 'type': types_.FUNCTION, optional : true, nullable : true},
    {'name' : 'count', 'type': types_.LONG, optional : true, nullable : true},
    {'name' : 'offset', 'type': types_.LONG, optional : true, nullable : true}
  ]);

  var nativeParam = {
    'playlist_id': this.id
  };

  if (args['count']) {
    nativeParam['count'] = args.count;
  }
  else {
    nativeParam['count'] = -1;
  }
  if (args['offset']) {
    nativeParam['offset'] = args.offset;
  }
  else {
    nativeParam['offset'] = -1;
  }
  
  try {
    var syncResult = callNativeWithCallback('ContentPlaylist_get', nativeParam, function(result) {
      if (result.status == 'success') {
        var items = [];
        for (var i = 0; i < result.value.length; i++) {
          var c = result.value[i];
          var content = createContent(c);
          var item = new PlaylistItem(content,c.playlist_member_id);
          items.push(item);
        }
        args.successCallback(items);
      }
      else if(result.status == 'error') {
        var err = result['value'];
        args.errorCallback(err);
      }
    });
  }
  catch(e) {
    throw e;
  }
}

Playlist.prototype.addBatch = function() {
  var args = validator_.validateArgs(arguments, [
    {'name' : 'items', 'type': types_.ARRAY},
    {'name' : 'successCallback', 'type': types_.FUNCTION, optional : true, nullable : true},
    {'name' : 'errorCallback', 'type': types_.FUNCTION, optional : true, nullable : true}
  ]);

  var nativeParam = {
    'playlist_id': this.id,
    'contents' : args.items
  };

  try {
    var syncResult = callNativeWithCallback('ContentPlaylist_addBatch', nativeParam, function(result) {
      if (result.status == 'success') {
        args.successCallback();
      }
      else if(result.status == 'error') {
        var err = result['value'];
        args.errorCallback(err);
      }
    });
  } catch(e) {
      throw e;
  }
}


Playlist.prototype.setOrder  = function() {
  var args = validator_.validateArgs(arguments, [
    {'name' : 'items', 'type': types_.ARRAY},
    {'name' : 'successCallback', 'type': types_.FUNCTION, optional : true, nullable : true},
    {'name' : 'errorCallback', 'type': types_.FUNCTION, optional : true, nullable : true}
  ]);

  var nativeParam = {
    'playlist_id': this.id,
  };

  nativeParam["members"] = [];
  if (args['items']) {
    for (var i = 0; i < args.items.length; i++) {
      var c = args.items[i];
      nativeParam.members.push(c.member_id);
    }
  }

  try {
    var syncResult = callNativeWithCallback('ContentPlaylist_setOrder', nativeParam, function(result) {
      if (result.status == 'success') {
        args.successCallback();
      }
      else if(result.status == 'error') {
        var err = result['value'];
        args.errorCallback(err);
      }
    });
  } catch(e) {
    throw e;
  }      
}

Playlist.prototype.move  = function(item, delta) {
  var args = validator_.validateArgs(arguments, [
    {'name' : 'item', 'type': types_.PLATFORM_OBJECT, 'values': PlaylistItem},
    {'name' : 'delta', 'type': types_.LONG},
    {'name' : 'successCallback', 'type': types_.FUNCTION, optional : true, nullable : true},
    {'name' : 'errorCallback', 'type': types_.FUNCTION, optional : true, nullable : true}
  ]);

  var nativeParam = {
    'playlist_id': this.id,
    'member_id': args.item.member_id,
    'delta' : args.delta
  };
  
  try {

    var syncResult = callNativeWithCallback('ContentPlaylist_move', nativeParam, function(result) {
      if (result.status == 'success') {
          args.successCallback();
      }
      else if(result.status == 'error') {
          var err = result['value'];
          args.errorCallback(err);
      }
    });
  } catch(e) {
    throw e;
  }
}


function ContentDirectory(id, uri, type, title, date) {
  Object.defineProperties(this, {
    'id': { writable: false, value: id, enumerable: true },    
    'directoryURI': { writable: false, value: uri, enumerable: true },
    'title': { writable: false, value: title, enumerable: true },
    'storageType': { writable: false, value: type, enumerable: true },
    'date': { writable: false, value: date, enumerable: true },
  });
}


function Content(id, name,type, mimeType, title, contentURI, thumbnailURIs, 
  releaseDate, modifiedDate, size, description, rating, isFavorite) {
  var name_ = name;
  var rating_ = rating;
  var editableAttributes_= ["name", "description", "rating", "isFavorite"];

  if (type === "IMAGE") {
    editableAttributes_.push("geolocation");
    editableAttributes_.push("orientation");
  }
  else if(type === "VIDEO") {
    editableAttributes_.push("geolocation");
  }
  Object.defineProperties(this, {
    'editableAttributes': 
      { enumerable: true,
        get: function() { return editableAttributes_; }
      },
    'id': 
      { writable: false, value: id, enumerable: true },
    'name': 
      { enumerable: true,
        set: function(v) { if (v != null) name_ = v},
        get: function() { return name_; }
      },        
    'type': 
      { writable: false, value: type, enumerable: true },
    'mimeType': 
      { writable: false, value: mimeType, enumerable: true },
    'title': 
      { writable: false, value: title, enumerable: true },
    'contentURI': 
      { writable: false, value: contentURI, enumerable: true },
    'thumbnailURIs': 
      { writable: false, value: thumbnailURIs, enumerable: true },
    'releaseDate': 
      { writable: false, value: releaseDate, enumerable: true },
    'modifiedDate': 
      { writable: false, value: modifiedDate, enumerable: true },
    'size': 
      { writable: false, value: size, enumerable: true },
    'description': 
      { writable: true, value: description, enumerable: true },
    'rating': 
      { enumerable: true,
        set: function(v) { if (v != null && v >= 0 && v <= 10) rating_ = v; },
        get: function() { return rating_; }
      },
    'isFavorite': 
      { writable: true, value: isFavorite, enumerable: true }
  });
}

function ImageContent(obj, width, height, orientation, geolocation) {
  Object.defineProperties(obj, {
    'width': 
      { writable: false, value: width, enumerable: true },
    'height': 
      { writable: false, value: height, enumerable: true },
    'orientation': 
      { writable: true, value: orientation, enumerable: true },
    'geolocation': 
      { writable: true, value: geolocation, enumerable: true }
  });
}

function VideoContent(obj, geolocation, album, artists, duration, width, height) {
  Object.defineProperties(obj, {
    'geolocation': 
      { writable: true, value: geolocation, enumerable: true },
    'album': 
      { writable: false, value: album, enumerable: true },
    'artists': 
      { writable: false, value: artists, enumerable: true },
    'duration': 
      { writable: false, value: duration, enumerable: true },
    'width': 
      { writable: false, value: width, enumerable: true },
    'height': 
      { writable: false, value: height, enumerable: true }
  });
}

function AudioContentLyrics(type, timestamps, texts) {
  Object.defineProperties(this, {
    'type': 
      { writable: false, value: type, enumerable: true },
    'timestamps': 
      { writable: false, value: timestamps, enumerable: true },
    'texts': 
      { writable: false, value: texts, enumerable: true }
  });
}

function AudioContent(obj, album, genres, artists, composers, copyright,
  bitrate, trackNumber, duration) {
  var lyrics_ = null;
  function getLyrics(contentURI) {
    var nativeParam = {
      'contentURI': contentURI
    };

    var syncResult = callNative('ContentManager_getLyrics', nativeParam);

    if (syncResult.status == 'success') {
      var l = syncResult['result'];
      lyrics_ = new AudioContentLyrics(l.type, l.timestamps, l.texts);
    }
    else {
      console.log("Getting the lyrics is failed.");
    }
  }
  Object.defineProperties(obj, {
    'album': 
      { writable: false, value: album, enumerable: true },
    'genres': 
      { writable: false, value: genres, enumerable: true },
    'artists': 
      { writable: false, value: artists, enumerable: true },
    'composers': 
      { writable: false, value: composers, enumerable: true },
    'copyright': 
      { writable: false, value: copyright, enumerable: true },
    'bitrate': 
      { writable: false, value: bitrate, enumerable: true },
    'trackNumber': 
      { writable: false, value: trackNumber, enumerable: true },
    'duration': 
      { writable: false, value: duration, enumerable: true },
    'lyrics': 
      { enumerable: true,
        get: function() { 
        	if(lyrics_ === undefined) {
        		getLyrics(obj.contentURI);
        	}
          else if(lyrics_ === null) {
            lyrics_ = undefined;
          }
      		return lyrics_;
        }
      }
  });
}

function createContent(c) {
  var content = new Content(c.id,
    c.name,
    c.type,
    c.mimeType,
    c.title,
    c.contentURI,
    c.thumbnailURIs,
    c.releaseDate,
    c.modifiedDate,
    c.size,
    c.description,
    c.rating,
    c.isFavorite
  );
  if (c.type === "IMAGE") {
    var image = new ImageContent(content,
      c.width,
      c.height, 
      c.orientation,
      c.geolocation
    );
  }
  else if (c.type === "VIDEO") {
    var video = new VideoContent(content,
      c.geolocation,
      c.album,
      c.artists,
      c.duration,
      c.width,
      c.height
    );
  }
  else if(c.type === "AUDIO") {
    var audio = new AudioContent(content,
      c.album,
      c.genres,
      c.artists,
      c.composers,
      c.copyright,
      c.bitrate,
      c.trackNumber,
      c.duration
    );
  }
  return content;
}


function ContentManager() {
    // constructor of ContentManager
}

//void update (Content content)
ContentManager.prototype.update = function(content) {
  var args = validator_.validateArgs(arguments, [
    {'name' : 'content', 'type': types_.PLATFORM_OBJECT, 'values': Content}
  ]);
  var nativeParam = {};
  if (args['content']) {
      nativeParam['content'] = args.content;
  }
  try {
    var syncResult = callNative('ContentManager_update', nativeParam);
  } catch(e) {
    throw e;
  }
}

ContentManager.prototype.updateBatch = function(contents) {
  var args = validator_.validateArgs(arguments, [
    {'name' : 'contents', 'type': types_.ARRAY},  
    {'name' : 'successCallback', 'type': types_.FUNCTION, optional : true, nullable : true},
    {'name' : 'errorCallback', 'type': types_.FUNCTION, optional : true, nullable : true}
  ]);

  var nativeParam = {
    'contents' : args.contents
  };

  try {
    var syncResult = callNativeWithCallback('ContentManager_updateBatch', nativeParam, function(result) {
      if (result.status == 'success') {
        args.successCallback();
      }
      else if(result.status == 'error') {
        var err = result['value'];
        args.errorCallback(err);
      }
    });
  } catch(e) {
      throw e;
  }
}

ContentManager.prototype.getDirectories = function(successCallback) {
  var args = validator_.validateArgs(arguments, [
      {'name' : 'successCallback', 'type': types_.FUNCTION, optional : false, nullable : false},  
      {'name' : 'errorCallback', 'type': types_.FUNCTION, optional : true, nullable : true} 
  ]);

  var nativeParam = {
  };

  try {
    var syncResult = callNativeWithCallback('ContentManager_getDirectories', nativeParam, function(result) {
      if (result.status == 'success') {
        var dirs = [];

        for (var i = 0; i < result.value.length; i++) {
          var res = result.value[i];
          var dir = new ContentDirectory(
            res.id,
            res.directoryURI,
            res.title,
            res.storageType,
            res.modifiedDate
          );
          
          dirs.push(dir);
        }
        args.successCallback(dirs);
      }
      else if(result.status == 'error') {
          var err = result['value'];
          args.errorCallback(err);
      }
    });
      // if you need synchronous result from native function using 'syncResult'.
  }
  catch(e) {
    throw e;
  }
}

ContentManager.prototype.find = function(successCallback) {
  var args = validator_.validateArgs(arguments, [
      {'name' : 'successCallback', 'type': types_.FUNCTION, 'values' : ['onsuccess']},  
      {'name' : 'errorCallback', 'type': types_.FUNCTION, optional : true, nullable : true},  
      {'name' : 'directoryId', 'type': types_.STRING, optional : true, nullable : true},  
      {'name' : 'filter', 'type': types_.DICTIONARY, optional : true, nullable : true},  
      {'name' : 'sortMode', 'type': types_.DICTIONARY, optional : true, nullable : true},  
      {'name' : 'count', 'type': types_.UNSIGNED_LONG, optional : true},
      {'name' : 'offset', 'type': types_.UNSIGNED_LONG, optional : true}
  ]);

  var nativeParam = {
  };
  
  if (args['directoryId']) {
      nativeParam['directoryId'] = args.directoryId;
  }
  if (args['filter']) {
      nativeParam['filter'] = args.filter;
  }
  if (args['sortMode']) {
      nativeParam['sortMode'] = args.sortMode;
  }
  if (args['count']) {
      nativeParam['count'] = args.count;
  }
  if (args['offset']) {
      nativeParam['offset'] = args.offset;
  }
  try {
    var syncResult = callNativeWithCallback('ContentManager_find', nativeParam, function(result) {
      if (result.status == 'success') {
        var contents = [];
        for ( var i = 0; i < result.value.length; i++) {
          var c = result.value[i];

          var content = createContent(c);
          
          contents.push(content);
        }
        args.successCallback(contents);
      }
      else if(result.status == 'error') {
        var err = result['value'];
        args.errorCallback(err);
      }            
    });
      // if you need synchronous result from native function using 'syncResult'.
  } catch(e) {
    throw e;
  }

}

ContentManager.prototype.scanFile = function(contentURI) {
  var args = validator_.validateArgs(arguments, [
    {'name' : 'contentURI', 'type': types_.STRING},  
    {'name' : 'successCallback', 'type': types_.FUNCTION, optional : true, nullable : true},
    {'name' : 'errorCallback', 'type': types_.FUNCTION, optional : true, nullable : true}
  ]);

  var nativeParam = {
    'contentURI': args.contentURI
  };

  try {
    var syncResult = callNativeWithCallback('ContentManager_scanFile', nativeParam, function(result) {
      if (result.status == 'success') {
        if (args.successCallback) {
            var uri = result['contentURI'];
            args.successCallback(uri);
        }
      }
      else if(result.status == 'error') {
        var err = result['value'];
        args.errorCallback(err);
      }
    });
  } catch(e) {
    throw e;
  }

}

ContentManager.prototype.setChangeListener = function(changeCallback) {
  var args = validator_.validateArgs(arguments, [
      {'name' : 'changeCallback', 'type': types_.LISTENER, 'values' : ['oncontentadded', 'oncontentupdated', 'oncontentremoved']} 
  ]);

  var nativeParam = {
  };
  try {
    var syncResult = callNativeWithCallback('ContentManager_setChangeListener', nativeParam, function(result) {
      if (result.status == 'oncontentadded') {
        var c = result['value'];
        var content = createContent(c);
        args.changeCallback.oncontentadded(content);
      }
      if (result.status == 'oncontentupdated') {
        var c = result['value'];
        var content = createContent(c);
        args.changeCallback.oncontentupdated(content);
      }
      if (result.status == 'oncontentremoved') {
        var contentId = result['value'];
        args.changeCallback.oncontentremoved(contentId);
      }
    });
      // if you need synchronous result from native function using 'syncResult'.
  } catch(e) {
      throw e;
  }
}

ContentManager.prototype.unsetChangeListener = function() {
  var nativeParam = {};
  try {
      var syncResult = callNative('ContentManager_unsetChangeListener', nativeParam);
  } catch(e) {
      throw e;
  }

}

ContentManager.prototype.getPlaylists = function(successCallback) {
  var args = validator_.validateArgs(arguments, [
    {'name' : 'successCallback', 'type': types_.FUNCTION},  
    {'name' : 'errorCallback', 'type': types_.FUNCTION, optional : true, nullable : true} 
  ]);

  var nativeParam = {
  };

  try {
    var syncResult = callNativeWithCallback('ContentManager_getPlaylists', nativeParam, function(result) {
      if (result.status == 'success') {
        var playlists = [];
        for ( var i = 0; i < result.value.length; i++) {
          var p = result.value[i];
          var playlist = new Playlist(p.id, p.name, p.numberOfTracks, p.thumbnailURI);
          playlists.push(playlist);
        }
        args.successCallback(playlists);
      }
      else if(result.status == 'error') {
        var err = result['value'];
        args.errorCallback(err);
      }
    });
  } catch(e) {
    throw e;
  }
}

ContentManager.prototype.createPlaylist = function(name, successCallback) {
  var args = validator_.validateArgs(arguments, [
    {'name' : 'name', 'type': types_.STRING},  
    {'name' : 'successCallback', 'type': types_.FUNCTION},  
    {'name' : 'errorCallback', 'type': types_.FUNCTION, optional : true, nullable : true},  
    {'name' : 'sourcePlaylist', 'type': types_.PLATFORM_OBJECT, optional : true, nullable : true}
  ]);

  var nativeParam = {
    'name': args.name
  };

  if (args['sourcePlaylist']) {
    nativeParam['sourcePlaylist'] = args.sourcePlaylist;
  }

  try {
    var syncResult = callNativeWithCallback('ContentManager_createPlaylist', nativeParam, function(result) {
      if (result.status == 'success') {
        var p = result['value'];
        var ret = new Playlist(p.id, p.name, p.numberOfTracks, p.thumbnailURI);
        args.successCallback(ret);
      }
      else if(result.status == 'error') {
        var err = result['value'];
        args.errorCallback(err);
      }
    });
  } catch(e) {
      throw e;
  }
}

ContentManager.prototype.removePlaylist = function(id) {
  var args = validator_.validateArgs(arguments, [
    {'name' : 'id', 'type': types_.STRING},  
    {'name' : 'successCallback', 'type': types_.FUNCTION, optional : true, nullable : true},
    {'name' : 'errorCallback', 'type': types_.FUNCTION, optional : true, nullable : true}
  ]);

  var nativeParam = {
    'id': args.id
  };

  try {
    var syncResult = callNativeWithCallback('ContentManager_removePlaylist', nativeParam, function(result) {
      if (result.status == 'success') {
        args.successCallback();
      }
      else if(result.status == 'error') {
        var err = result['value'];
        args.errorCallback(err);
      }
    });
  } catch(e) {
    throw e;
  }
}



exports = new ContentManager();

