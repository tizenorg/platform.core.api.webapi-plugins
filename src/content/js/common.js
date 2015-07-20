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
  var content;
  switch (data.type) {
    case ContentType.IMAGE:
      content = new ImageContent(data);
      break;
    case ContentType.AUDIO:
        content = new AudioContent(data);
      break;
    case ContentType.VIDEO:
      content = new VideoContent(data);
      break;
    case ContentType.OTHER:
      content = new Content(data);
      break;
    default:
      throw new WebAPIException(WebAPIException.UNKNOWN_ERR, 'Undefined content type');
  }
  // below constructor overwriting is needed because of backward compatibility
  var object = {};
  content.constructor = object.constructor;
  return content;
}

function createContentDirObject_(data) {
  return new ContentDirectory(data);
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
