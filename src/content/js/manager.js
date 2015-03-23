// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function _ContentManagerChangeCallback(result) {
  if (result.state === 'oncontentadded' || result.state === 'oncontentupdated') {
    var content = native_.getResultObject(result);
    native_.callIfPossible(this[result.state], createContentObject_(content));
  }
  if (result.state === 'oncontentremoved') {
    native_.callIfPossible(this.oncontentremoved, native_.getResultObject(result));
  }
}


function ContentManager() {
}

ContentManager.prototype.update = function(content) {
  var args = validator_.validateArgs(arguments, [
    {name: 'content', type: types_.PLATFORM_OBJECT, values: Content}
  ]);

  var data = {
    content: args.content
  };

  var result = native_.callSync('ContentManager_update', data);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

ContentManager.prototype.updateBatch = function(contents, successCallback, errorCallback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'contents', type: types_.ARRAY, values: Content},
    {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var data = {
    contents: args.contents
  };

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.successCallback);
  };

  native_.call('ContentManager_updateBatch', data, callback);
};

ContentManager.prototype.getDirectories = function(successCallback, errorCallback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'successCallback', type: types_.FUNCTION},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }

    var out = [];
    result = native_.getResultObject(result);
    for (var i = 0, max = result.length; i < max; i++) {
      out.push(new ContentDirectory(result[i]));
    }
    native_.callIfPossible(args.successCallback, out);
  };

  native_.call('ContentManager_getDirectories', null, callback);
};

ContentManager.prototype.find = function(successCallback, errorCallback, directoryId, filter, sortMode, count, offset) {
  var args = validator_.validateArgs(arguments, [
    {name: 'successCallback', type: types_.FUNCTION},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'directoryId', type: types_.STRING, optional: true, nullable: true},
    {
      name: 'filter',
      type: types_.PLATFORM_OBJECT,
      values: [tizen.AttributeFilter, tizen.AttributeRangeFilter, tizen.CompositeFilter],
      optional: true,
      nullable: true
    },
    {name: 'sortMode', type: types_.PLATFORM_OBJECT, values: tizen.SortMode, optional: true, nullable: true},
    {name: 'count', type: types_.UNSIGNED_LONG, optional: true, nullable: true},
    {name: 'offset', type: types_.UNSIGNED_LONG, optional: true, nullable: true}
  ]);

  var data = {
    directoryId: args.directoryId,
    filter: utils_.repackFilter(args.filter),
    sortMode: args.sortMode,
    count: args.count,
    offset: args.offset
  };

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }

    var out = [];
    result = native_.getResultObject(result);
    try {
      for (var i = 0, max = result.length; i < max; i++) {
        out.push(createContentObject_(result[i]));
      }
    } catch(e) {
      native_.callIfPossible(args.errorCallback, e);
      return;
    }

    native_.callIfPossible(args.successCallback, out);
  };

  native_.call('ContentManager_find', data, callback);
};

ContentManager.prototype.scanFile = function(contentURI, successCallback, errorCallback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'contentURI', type: types_.STRING},
    {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var path = args.contentURI.trim();
  if (!path.length) {
    throw new WebAPIException(WebAPIException.INVALID_VALUES_ERR, 'File path is not valid.');
  }

  var data = {
    contentURI: convertUriToPath_(path)
  };

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.successCallback, args.contentURI);
  };

  native_.call('ContentManager_scanFile', data, callback);
};

ContentManager.prototype.setChangeListener = function(changeCallback) {
  var args = validator_.validateArgs(arguments, [{
    name: 'changeCallback',
    type: types_.LISTENER,
    values: ['oncontentadded', 'oncontentupdated', 'oncontentremoved']
  }]);

  var listenerId = 'ContentManagerChangeCallback';

  var data = {
    listenerId: listenerId
  };

  var callbacks = {
    oncontentadded: args.changeCallback.oncontentadded,
    oncontentupdated: args.changeCallback.oncontentupdated,
    oncontentremoved: args.changeCallback.oncontentremoved
  };

  native_.addListener('ContentManagerChangeCallback',
      _ContentManagerChangeCallback.bind(callbacks));

  var result = native_.callSync('ContentManager_setChangeListener', data);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

ContentManager.prototype.unsetChangeListener = function() {
  var data = {};

  var result = native_.callSync('ContentManager_unsetChangeListener', data);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

ContentManager.prototype.getPlaylists = function(successCallback, errorCallback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'successCallback', type: types_.FUNCTION},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var data = {};

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }
    var out = [];
    result = native_.getResultObject(result);
    for (var i = 0, max = result.length; i < max; i++) {
      out.push(new Playlist(result[i]));
    }
    native_.callIfPossible(args.successCallback, out);
  };

  native_.call('ContentManager_getPlaylists', data, callback);
};

ContentManager.prototype.createPlaylist = function(name, successCallback, errorCallback, sourcePlaylist) {
  var args = validator_.validateArgs(arguments, [
    {name: 'name', type: types_.STRING},
    {name: 'successCallback', type: types_.FUNCTION},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'sourcePlaylist', type: types_.PLATFORM_OBJECT, values: Playlist, optional: true, nullable: true}
  ]);

  if (!arguments.length || !type_.isString(arguments[0]) ||
    (type_.isString(arguments[0]) && !arguments[0].length)) {
    setTimeout(function() {
      args.errorCallback(new WebAPIException(WebAPIException.INVALID_VALUES_ERR));
    }, 0);
    return;
  }

  var data = {
    name: args.name,
    sourcePlaylist: args.sourcePlaylist
  };

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.successCallback, new Playlist(native_.getResultObject(result)));
  };

  native_.call('ContentManager_createPlaylist', data, callback);
};

ContentManager.prototype.removePlaylist = function(id, successCallback, errorCallback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'id', type: types_.STRING},
    {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var data = {
    id: args.id
  };

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.successCallback);
  };

  native_.call('ContentManager_removePlaylist', data, callback);
};

exports = new ContentManager();
