// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var validator_ = xwalk.utils.validator;
var types_ = validator_.Types;
var native_ = new xwalk.utils.NativeManager(extension);

function EditManager() {
  this.canEdit = false;
}

EditManager.prototype.allow = function() {
  this.canEdit = true;
};

EditManager.prototype.disallow = function() {
  this.canEdit = false;
};

var _edit = new EditManager();

function BookmarkManager() {}

BookmarkManager.prototype.get = function() {
  var args = validator_.validateArgs(arguments, [
    {
      name: 'parentFolder',
      type: types_.PLATFORM_OBJECT,
      values: [tizen.BookmarkFolder, tizen.BookmarkItem],
      optional: true,
      nullable: true
    },
    {
      name: 'recursive',
      type: types_.BOOLEAN,
      optional: true,
      nullable: true
    }
  ]);
  var result;

  if (arguments.length === 0 || args.parentFolder === null) {
    result = provider.getFolderItems(provider.getRootId(), args.recursive);
    if (!result)
      throw new tizen.WebAPIException(tizen.WebAPIException.NOT_FOUND_ERR);
    return result;
  }
  if (args.parentFolder.id === null || args.parentFolder.id === 0)
    throw new tizen.WebAPIException(tizen.WebAPIException.NOT_FOUND_ERR);

  result = provider.getFolderItems(args.parentFolder.id, args.recursive);
  if (!result)
    throw new tizen.WebAPIException(tizen.WebAPIException.NOT_FOUND_ERR);
  return result;
};

BookmarkManager.prototype.add = function() {
  var args = validator_.validateArgs(arguments, [
    {
      name: 'bookmark',
      type: types_.PLATFORM_OBJECT,
      values: [tizen.BookmarkFolder, tizen.BookmarkItem],
      optional: false,
      nullable: false
    },
    {
      name: 'parentFolder',
      type: types_.PLATFORM_OBJECT,
      values: tizen.BookmarkFolder,
      optional: true,
      nullable: true
    }
  ]);
  if (arguments.length == 1 || args.parentFolder === null) {
    if (args.bookmark.id) {
      throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR);
    }
    if (!provider.addToFolder(args.bookmark, provider.getRootId())) {
      throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR);
    }
    return;
  }
  if (!args.parentFolder.id) {
    throw new tizen.WebAPIException(tizen.WebAPIException.NOT_FOUND_ERR);
  }
  if (!provider.addToFolder(args.bookmark, args.parentFolder.id)) {
    throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR);
  }
};

BookmarkManager.prototype.remove = function() {
  var args = validator_.validateArgs(arguments, [
    {
      name: 'bookmark',
      type: types_.PLATFORM_OBJECT,
      values: [tizen.BookmarkFolder, tizen.BookmarkItem],
      optional: true,
      nullable: true
    }
  ]);

  if (!arguments.length || args.bookmark === null) {
    if (native_.isFailure(native_.callSync('Bookmark_removeAll')))
      throw new tizen.WebAPIException(tizen.WebAPIException.SECURITY_ERR);
    return;
  }
  if (!args.bookmark.id)
    throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR);
  if (native_.isFailure(native_.callSync('Bookmark_remove', {
    id: args.bookmark.id})))
    throw new tizen.WebAPIException(tizen.WebAPIException.SECURITY_ERR);

  _edit.allow();
  args.bookmark.id = null;
  args.bookmark.parent = undefined;
  _edit.disallow();
};

function BookmarkProvider() {}

BookmarkProvider.prototype.addToFolder = function() {
  var args = validator_.validateArgs(arguments, [
    {
      name: 'bookmark',
      type: types_.PLATFORM_OBJECT,
      values: [tizen.BookmarkFolder, tizen.BookmarkItem],
      optional: true,
      nullable: true
    },
    {
      name: 'parentId',
      type: types_.DOUBLE,
      optional: false,
      nullable: false}
  ]);
  var ret = native_.callSync('Bookmark_add',
      {
        title: args.bookmark.title,
        url: args.bookmark.url || '/',
        parentId: args.parentId,
        type: args.bookmark instanceof tizen.BookmarkFolder ? 1 : 0
      }
      );
  if (native_.isFailure(ret)) {
    return false;
  }
  var ret_id = native_.getResultObject(ret);
  _edit.allow();
  args.bookmark.id = ret_id;
  args.bookmark.parent = this.getFolder(args.parentId);
  _edit.disallow();
  return true;
};

BookmarkProvider.prototype.getFolder = function() {
  var args = validator_.validateArgs(arguments, [
    {
      name: 'id',
      type: types_.DOUBLE,
      optional: false,
      nullable: false
    }
  ]);
  if (arguments.length === 0 || args.id <= 0)
    return null;
  if (args.id == this.getRootId())
    return null;

  var ret = native_.callSync('Bookmark_get', {
    id: args.id,
    shouldGetItems: 0
  });

  if (native_.isFailure(ret)) {
    throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR);
  }

  var folder = native_.getResultObject(ret);
  if (folder === undefined || folder === null)
    throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR);

  var obj = new tizen.BookmarkFolder(folder[0].title);
  obj.id = folder[0].id;
  obj.parent = this.getFolder(folder[0].parentId);
  return obj;
};

BookmarkProvider.prototype.getFolderItems = function() {
  var args = validator_.validateArgs(arguments, [
    {
      name: 'id',
      type: types_.DOUBLE,
      optional: false,
      nullable: false
    },
    {
      name: 'recursive',
      type: types_.BOOLEAN,
      optional: true,
      nullable: true
    }
  ]);

  var ret = native_.callSync('Bookmark_get', {
    id: Number(args.id),
    shouldGetItems: 1
  });

  if (native_.isFailure(ret)) {
    throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR);
  }

  var folder = native_.getResultObject(ret);
  if (folder === undefined)
    throw new tizen.WebAPIException(tizen.WebAPIException.INVALID_VALUES_ERR);

  var item;
  var obj;
  var result = [];
  var len = folder.length;
  for (var i = 0; item = folder[i], i < len; i++) {
    if (item.type !== 0) {
      obj = new tizen.BookmarkItem(item.title, item.url);
      _edit.allow();
      obj.id = item.id;
      obj.parent = this.getFolder(item.parentId);
      _edit.disallow();
      result.push(obj);
    } else {
      obj = new tizen.BookmarkFolder(item.title);
      _edit.allow();
      obj.id = item.id;
      obj.parent = this.getFolder(item.parentId);
      _edit.disallow();
      result.push(obj);
      if (args.recursive) {
        result = result.concat(this.getFolderItems(item.id, true));
      }
    }
  }
  return result;
};

BookmarkProvider.prototype.getRootId = function() {
  var ret = native_.callSync('Bookmark_getRootId');
  if (native_.isFailure(ret)) {
    throw native_.getErrorObject(ret);
  }
  var rootId = native_.getResultObject(ret);
  return Number(rootId);
};

var provider = new BookmarkProvider();

tizen.BookmarkItem = function() {
  validator_.isConstructorCall(this, tizen.BookmarkItem);
  var args = validator_.validateArgs(arguments, [
    {
      name: 'title',
      type: types_.STRING,
      optional: false
    },
    {
      name: 'url',
      type: types_.STRING,
      optional: false
    }
  ]);
  var parent_;
  var id_ = null;

  Object.defineProperties(this, {
    parent: {
      get: function() {
        return parent_;
      },
      set: function(new_parent) {
        if (_edit.canEdit)
          parent_ = new_parent;
      },
      enumerable: true,
      nullable: true
    },
    title: {
      get: function() {
        return args.title;
      },
      enumerable: true,
      nullable: false
    },
    url: {
      get: function() {
        return args.url;
      },
      enumerable: true,
      nullable: false
    },
    id: {
      get: function() {
        return id_;
      },
      set: function(new_id) {
        if (_edit.canEdit)
          id_ = new_id;
      },
      enumerable: false,
      nullable: true
    }
  });
};

tizen.BookmarkFolder = function() {
  validator_.isConstructorCall(this, tizen.BookmarkFolder);
  var args = validator_.validateArgs(arguments, [
    {
      name: 'title',
      type: types_.STRING,
      optional: false,
      nullable: false
    }
  ]);

  var parent_;
  var id_ = null;

  Object.defineProperties(this, {
    parent: {
      get: function() {
        return parent_;
      },
      set: function(new_parent) {
        if (_edit.canEdit)
          parent_ = new_parent;
      },
      enumerable: true,
      nullable: true
    },
    title: {
      get: function() {
        return args.title;
      },
      enumerable: true,
      nullable: false
    },
    id: {
      get: function() {
        return id_;
      },
      set: function(new_id) {
        if (_edit.canEdit)
          id_ = new_id;
      },
      enumerable: false,
      nullable: true
    }
  });
};
exports = new BookmarkManager();
