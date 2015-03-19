// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function Playlist(data) {
  var editableAttributes = ['name', 'thumbnailURI'];
  var id;
  var name;
  var numberOfTracks;
  var thumbnailURI;

  Object.defineProperties(this, {
    editableAttributes: {
      value: editableAttributes,
      writable: false,
      enumerable: true
    },
    id: {
      get: function() {
        return id;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          id = converter_.toString(v, false);
        }
      },
      enumerable: true
    },
    name: {
      get: function() {
        return name;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          name = converter_.toString(v, false);
        }
      },
      enumerable: true
    },
    numberOfTracks: {
      get: function() {
        return numberOfTracks;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          numberOfTracks = converter_.toUnsignedLong(v, false);
        }
      },
      enumerable: true
    },
    thumbnailURI: {
      get: function() {
        return thumbnailURI;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          thumbnailURI = converter_.toString(v, false);
        }
      },
      enumerable: true
    },
  });

  if (type_.isObject(data)) {
    // fill object with data
    edit_.allow();
    for (var key in data) {
      if (data.hasOwnProperty(key) && this.hasOwnProperty(key)) {
        this[key] = data[key];
      }
    }
    edit_.disallow();
  }
}

Playlist.prototype.add = function (item) {
  var args = validator_.validateArgs(arguments, [
    {name: 'item', type: types_.PLATFORM_OBJECT, values: tizen.Content}
  ]);

  var data = {
    item: args.item
  };

  var result = native_.callSync('Playlist_add', data);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

Playlist.prototype.addBatch = function (items, successCallback, errorCallback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'items', type: types_.PLATFORM_OBJECT, values: tizen.Content},
    {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var data = {
    items: args.items
  };

  var callback = function (result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.successCallback);
  };

  native_.call('Playlist_addBatch', data, callback);
};

Playlist.prototype.remove = function (item) {
  var args = validator_.validateArgs(arguments, [
    {name: 'item', type: types_.PLATFORM_OBJECT, values: tizen.PlaylistItem}
  ]);

  var data = {
    item: args.item
  };

  var result = native_.callSync('Playlist_remove', data);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

Playlist.prototype.removeBatch = function (items, successCallback, errorCallback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'items', type: types_.PLATFORM_OBJECT, values: tizen.PlaylistItem},
    {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var data = {
    items: args.items
  };

  var callback = function (result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.successCallback);
  };

  native_.call('Playlist_removeBatch', data, callback);
};

Playlist.prototype.get = function (successCallback, errorCallback, count, offset) {
  var args = validator_.validateArgs(arguments, [
    {name: 'successCallback', type: types_.FUNCTION},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'count', type: types_.LONG, optional: true},
    {name: 'offset', type: types_.LONG, optional: true}
  ]);

  var data = {
    count: args.count,
    offset: args.offset
  };

  var callback = function (result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.successCallback, native_.getResultObject(result));
  };

  native_.call('Playlist_get', data, callback);
};

Playlist.prototype.setOrder = function (items, successCallback, errorCallback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'items', type: types_.PLATFORM_OBJECT, values: tizen.PlaylistItem},
    {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var data = {
    items: args.items
  };

  var callback = function (result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.successCallback);
  };

  native_.call('Playlist_setOrder', data, callback);
};

Playlist.prototype.move = function (item, delta, successCallback, errorCallback) {
  var args = validator_.validateArgs(arguments, [
    {name: 'item', type: types_.PLATFORM_OBJECT, values: tizen.PlaylistItem},
    {name: 'delta', type: types_.LONG},
    {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var data = {
    item: args.item,
    delta: args.delta
  };

  var callback = function (result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.successCallback);
  };

  native_.call('Playlist_move', data, callback);
};
