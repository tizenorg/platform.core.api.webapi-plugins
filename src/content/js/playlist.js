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
function Playlist(data) {
  var id;
  var numberOfTracks;

  Object.defineProperties(this, {
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
        var result = native_.callSync('ContentPlaylist_getName', {'id' : Number(id)});
        if (native_.isFailure(result)) {
          throw native_.getErrorObject(result);
        }
        return native_.getResultObject(result);
      },
      set: function(v) {
        xwalk.utils.checkPrivilegeAccess(privilege_.CONTENT_WRITE);
        if (!type_.isNull(v)) {
          var name = converter_.toString(v, false);
          var result = native_.callSync('ContentPlaylist_setName',
                  {'id' : Number(id), 'name' : name});
          if (native_.isFailure(result)) {
            throw native_.getErrorObject(result);
          }
        }
      },
      enumerable: true
    },
    numberOfTracks: {
      get: function() {
        var result = native_.callSync('ContentPlaylist_getNumberOfTracks', {'id' : Number(id)});
        if (native_.isFailure(result)) {
          throw native_.getErrorObject(result);
        }
        return native_.getResultObject(result);
      },
      set: function() {},
      enumerable: true
    },
    thumbnailURI: {
      get: function() {
        var result = native_.callSync('ContentPlaylist_getThumbnailUri', {'id' : Number(id)});
        if (native_.isFailure(result)) {
          throw native_.getErrorObject(result);
        }
        var res = native_.getResultObject(result);
        //CoreAPI not support empty thumbnail, so one space must be used instead null thumbnail
        return res === " " ? null : res;
      },
      set: function(v) {
        xwalk.utils.checkPrivilegeAccess(privilege_.CONTENT_WRITE);
        var thumbnailURI = converter_.toString(v, true);
        if (type_.isNullOrUndefined(thumbnailURI)) {
          //CoreAPI not support empty thumbnail, so one space must be used instead null thumbnail
          thumbnailURI = " ";
        }
        //TODO probably thumbnailURI should be converted here to absolute uri in case of virtual
        var result = native_.callSync('ContentPlaylist_setThumbnailUri',
                {'id' : Number(id), 'uri' : thumbnailURI});
        if (native_.isFailure(result)) {
          throw native_.getErrorObject(result);
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
  xwalk.utils.checkPrivilegeAccess(privilege_.CONTENT_WRITE);

  var args = validator_.validateArgs(arguments, [
    {name: 'item', type: types_.PLATFORM_OBJECT, values: Content}
  ]);

  var data = {
    contentId: args.item.id,
    playlistId: this.id,
  };

  var result = native_.callSync('ContentPlaylist_add', data);
  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

Playlist.prototype.addBatch = function (items, successCallback, errorCallback) {
  xwalk.utils.checkPrivilegeAccess(privilege_.CONTENT_WRITE);

  var args = validator_.validateArgs(arguments, [
    {name: 'items', type: types_.ARRAY, values: Content},
    {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var data = {
    playlistId: this.id,
    contents: args.items
  };

  var callback = function (result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.successCallback);
  };

  native_.call('ContentPlaylist_addBatch', data, callback);
};

Playlist.prototype.remove = function (item) {
  xwalk.utils.checkPrivilegeAccess(privilege_.CONTENT_WRITE);

  var args = validator_.validateArgs(arguments, [
    {name: 'item', type: types_.PLATFORM_OBJECT, values: PlaylistItem}
  ]);

  var data = {
    playlistId: this.id,
    memberId: args.item.content.memberId
  };
  var result = native_.callSync('ContentPlaylist_remove', data);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

Playlist.prototype.removeBatch = function (items, successCallback, errorCallback) {
  xwalk.utils.checkPrivilegeAccess(privilege_.CONTENT_WRITE);

  var args = validator_.validateArgs(arguments, [
    {name: 'items', type: types_.ARRAY, values: PlaylistItem},
    {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var members = [];
  for (var i = 0; i < args.items.length; i++) {
    members.push(args.items[i].content.memberId);
  }

  var data = {
    playlistId: this.id,
    members: members
  };

  var callback = function (result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.successCallback);
  };

  native_.call('ContentPlaylist_removeBatch', data, callback);
};

Playlist.prototype.get = function (successCallback, errorCallback, count, offset) {
  xwalk.utils.checkPrivilegeAccess(privilege_.CONTENT_READ);

  var args = validator_.validateArgs(arguments, [
    {name: 'successCallback', type: types_.FUNCTION},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'count', type: types_.LONG, optional: true},
    {name: 'offset', type: types_.LONG, optional: true}
  ]);

  if (args.offset < 0 || args.count < 0) {
    setTimeout(function() {
      args.errorCallback(new WebAPIException(WebAPIException.INVALID_VALUES_ERR));
    }, 0);
    return;
  }

  var data = {
    playlistId: this.id,
    count: type_.isNullOrUndefined(args.count) ? -1 : args.count,
    offset: type_.isNullOrUndefined(args.offset) ? -1 : args.offset
  };

  var callback = function (result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }
    result = native_.getResultObject(result);
    var out = [];
    for (var i = 0, max = result.length; i < max; i++) {
      var itemToPush = createContentObject_(result[i]);
      itemToPush['memberId'] = result[i]['playlist_member_id'];
      out.push(new PlaylistItem(itemToPush));
    }
    native_.callIfPossible(args.successCallback, out);
  };

  native_.call('ContentPlaylist_get', data, callback);
};

Playlist.prototype.setOrder = function (items, successCallback, errorCallback) {
  xwalk.utils.checkPrivilegeAccess(privilege_.CONTENT_WRITE);

  var args = validator_.validateArgs(arguments, [
    {name: 'items', type: types_.ARRAY, values: PlaylistItem},
    {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  if (!args.items.length) {
    setTimeout(function() {
      args.errorCallback(new WebAPIException(WebAPIException.INVALID_VALUES_ERR));
    }, 0);
    return;
  }

  var members = [];
  for (var i = 0; i < args.items.length; i++) {
    members.push(args.items[i].content.memberId);
  }

  var data = {
    playlistId: this.id,
    members: members,
  };

  var callback = function (result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.successCallback);
  };

  native_.call('ContentPlaylist_setOrder', data, callback);
};

Playlist.prototype.move = function (item, delta, successCallback, errorCallback) {
  xwalk.utils.checkPrivilegeAccess(privilege_.CONTENT_WRITE);

  var args = validator_.validateArgs(arguments, [
    {name: 'item', type: types_.PLATFORM_OBJECT, values: PlaylistItem},
    {name: 'delta', type: types_.LONG},
    {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
  ]);

  var data = {
    playlistId: this.id,
    memberId: args.item.content.memberId,
    delta: args.delta
  };

  var callback = function (result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }
    native_.callIfPossible(args.successCallback);
  };
  native_.call('ContentPlaylist_move', data, callback);
};
