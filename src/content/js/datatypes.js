// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var ContentDirectoryStorageType = {
  INTERNAL: 'INTERNAL',
  EXTERNAL: 'EXTERNAL'
};

var ContentType = {
  IMAGE: 'IMAGE',
  VIDEO: 'VIDEO',
  AUDIO: 'AUDIO',
  OTHER: 'OTHER'
};

var AudioContentLyricsType = {
  SYNCHRONIZED: 'SYNCHRONIZED',
  UNSYNCHRONIZED: 'UNSYNCHRONIZED'
};

var ImageContentOrientation = {
  NORMAL: 'NORMAL',
  FLIP_HORIZONTAL: 'FLIP_HORIZONTAL',
  ROTATE_180: 'ROTATE_180',
  FLIP_VERTICAL: 'FLIP_VERTICAL',
  TRANSPOSE: 'TRANSPOSE',
  ROTATE_90: 'ROTATE_90',
  TRANSVERSE: 'TRANSVERSE',
  ROTATE_270: 'ROTATE_270'
};

function ContentDirectory() {
  // TODO(r.galka)
  //SetReadOnlyProperty(this, 'id', null); // read only property
  //SetReadOnlyProperty(this, 'directoryURI', null); // read only property
  //SetReadOnlyProperty(this, 'title', null); // read only property
  //SetReadOnlyProperty(this, 'storageType', null); // read only property
  //SetReadOnlyProperty(this, 'modifiedDate', null); // read only property
}


function Content(data) {
  var editableAttributes = ['name', 'rating', 'description'];
  var id;
  var name;
  var type;
  var mimeType;
  var title;
  var contentURI;
  var thumbnailURIs = null;
  var releaseDate = null;
  var modifiedDate = null;
  var size;
  var description = null;
  var rating;
  var isFavorite;

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
        if (!type_.isNull(v)) {
          name = converter_.toString(v, false);
        }
      },
      enumerable: true
    },
    type: {
      get: function() {
        return type;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          type = converter_.toEnum(v, Object.keys(ContentType), false);
        }
      },
      enumerable: true
    },
    mimeType: {
      get: function() {
        return mimeType;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          mimeType = converter_.toString(v, false);
        }
      },
      enumerable: true
    },
    title: {
      get: function() {
        return title;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          title = converter_.toString(v, false);
        }
      },
      enumerable: true
    },
    contentURI: {
      get: function() {
        return contentURI;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          contentURI = convertPathToUri_(v);
        }
      },
      enumerable: true
    },
    thumbnailURIs: {
      get: function() {
        return thumbnailURIs;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          thumbnailURIs = converter_.toArray(v, true);
        }
      },
      enumerable: true
    },
    releaseDate: {
      get: function() {
        return releaseDate;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          releaseDate = !type_.isNull(v) ? new Date(v) : null;
        }
      },
      enumerable: true
    },
    modifiedDate: {
      get: function() {
        return modifiedDate;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          modifiedDate = !type_.isNull(v) ? new Date(v) : null;
        }
      },
      enumerable: true
    },
    size: {
      get: function() {
        return size;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          size = converter_.toUnsignedLong(v, false);
        }
      },
      enumerable: true
    },
    description: {
      get: function() {
        return description;
      },
      set: function(v) {
        description = converter_.toString(v, true);
      },
      enumerable: true
    },
    rating: {
      get: function() {
        return rating;
      },
      set: function(v) {
        if (!type_.isNull(v)) {
          rating = converter_.toUnsignedLong(v, false);
        }
      },
      enumerable: true
    },
    isFavorite: {
      get: function() {
        return isFavorite;
      },
      set: function(v) {
        if (!type_.isNull(v)) {
          isFavorite = converter_.toBoolean(v, false);
        }
      },
      enumerable: true
    }
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


function VideoContent(data) {
  Content.call(this, data);

  // TODO(r.galka)
  //this.geolocation = null;
  //SetReadOnlyProperty(this, 'album', null); // read only property
  //SetReadOnlyProperty(this, 'artists', null); // read only property
  //SetReadOnlyProperty(this, 'duration', null); // read only property
  //SetReadOnlyProperty(this, 'width', null); // read only property
  //SetReadOnlyProperty(this, 'height', null); // read only property
}

VideoContent.prototype = new Content();
VideoContent.prototype.constructor = VideoContent;


function AudioContentLyrics() {
  // TODO(r.galka)
  //SetReadOnlyProperty(this, 'type', null); // read only property
  //SetReadOnlyProperty(this, 'timestamps', null); // read only property
  //SetReadOnlyProperty(this, 'texts', null); // read only property
}


function AudioContent(data) {
  Content.call(this, data);

  // TODO(r.galka)
  //SetReadOnlyProperty(this, 'album', null); // read only property
  //SetReadOnlyProperty(this, 'genres', null); // read only property
  //SetReadOnlyProperty(this, 'artists', null); // read only property
  //SetReadOnlyProperty(this, 'composers', null); // read only property
  //SetReadOnlyProperty(this, 'lyrics', null); // read only property
  //SetReadOnlyProperty(this, 'copyright', null); // read only property
  //SetReadOnlyProperty(this, 'bitrate', null); // read only property
  //SetReadOnlyProperty(this, 'trackNumber', null); // read only property
  //SetReadOnlyProperty(this, 'duration', null); // read only property
}

AudioContent.prototype = new Content();
AudioContent.prototype.constructor = AudioContent;


function ImageContent(data) {
  Content.call(this, data);

  var geolocation;
  var width;
  var height;
  var orientation;

  //this.geolocation = null;
  //SetReadOnlyProperty(this, 'width', null); // read only property
  //SetReadOnlyProperty(this, 'height', null); // read only property
  //this.orientation = null;
}

ImageContent.prototype = new Content();
ImageContent.prototype.constructor = ImageContent;


function PlaylistItem() {
  // TODO(r.galka)
  //SetReadOnlyProperty(this, 'content', null); // read only property
}
