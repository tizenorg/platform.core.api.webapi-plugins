// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function throwException_(err) {
  throw new tizen.WebAPIException(err.code, err.name, err.message);
}

function _sendSyncMessage(msg) {
  var data = null;
  data = JSON.stringify(msg);

  return JSON.parse(extension.internal.sendSyncMessage(data));
}

function BookmarkManager() {
}

BookmarkManager.prototype.get = function() {
  return new BookmarkManager();
};

BookmarkManager.prototype.add = function(parent_folder, recursive) {
  var msg = {
    cmd: 'add',
    arg: ''
  };

  return _sendSyncMessage(msg).value;
};


BookmarkManager.prototype.remove = function(bookmark) {
  var msg = {
    cmd: 'remove',
    arg: ''
  };

  return _sendSyncMessage(msg).value;
};

tizen.BookmarkItem = function(title, url) {
  var parent;
  var title_;
  var url_;
};

tizen.BookmarkFolder = function(title) {
  var parent;
  var title_;
};

exports = new BookmarkManager();
