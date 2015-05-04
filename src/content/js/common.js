/* global tizen, xwalk, extension */

// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var utils_ = xwalk.utils;
var type_ = utils_.type;
var converter_ = utils_.converter;
var validator_ = utils_.validator;
var types_ = validator_.Types;
var native_ = new xwalk.utils.NativeManager(extension);
var privilege_ = xwalk.utils.privilege;

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

var SCHEMA = 'file://';

function createContentObject_(data) {
  switch (data.type) {
    case ContentType.IMAGE:
        return new ImageContent(data);
      break;
    case ContentType.AUDIO:
      return new AudioContent(data);
      break;
    case ContentType.VIDEO:
      return new VideoContent(data);
      break;
    case ContentType.OTHER:
      return new Content(data);
      break;
  }

  throw new WebAPIException(WebAPIException.UNKNOWN_ERR, 'Undefined content type');
}

function convertUriToPath_(uri) {
  if (0 === uri.indexOf(SCHEMA)) {
    return uri.substring(SCHEMA.length);
  }

  return uri;
}

function convertPathToUri_(path) {
  if (0 === path.indexOf(SCHEMA)) {
    return path;
  }

  return SCHEMA + path;
}
