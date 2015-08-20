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

function ContentDirectory(data) {
  var id;
  var directoryURI;
  var title;
  var storageType;
  var modifiedDate = null;

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
    directoryURI: {
      get: function() {
        return directoryURI;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          directoryURI = convertPathToUri_(converter_.toString(v, false));
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
    storageType: {
      get: function() {
        return storageType;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          storageType = converter_.toEnum(v, Object.keys(ContentDirectoryStorageType), false);
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
          modifiedDate = v > 0 ? new Date(v * 1000) : null;
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
          releaseDate = v > 0 ? new Date(v * 1000) : null;
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
          modifiedDate = v > 0 ? new Date(v * 1000) : null;
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
        if (!type_.isNull(v) && v >= 0 && v <= 10) {
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

  var editableAttributes = this.editableAttributes;
  editableAttributes.push('geolocation');

  var geolocation;
  var album;
  var artists;
  var duration;
  var width;
  var height;

    Object.defineProperties(this, {
    editableAttributes: {
      value: editableAttributes,
      writable: false,
      enumerable: true
    },
    geolocation: {
      get: function() {
        return geolocation;
      },
      set: function(v) {
        if (!type_.isNull(v)) {
          var latitude = converter_.toDouble(v.latitude, false);
          var longitude = converter_.toDouble(v.longitude, false);
          geolocation = new tizen.SimpleCoordinates(latitude, longitude);
        }
      },
      enumerable: true
    },
    album: {
      get: function() {
        return album;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          album = converter_.toString(v, false);
        }
      },
      enumerable: true
    },
    artists: {
      get: function() {
        return artists;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          artists = converter_.toArray(v, true);
        }
      },
      enumerable: true
    },
    duration: {
      get: function() {
        return duration;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          duration = converter_.toUnsignedLong(v, false);
        }
      },
      enumerable: true
    },
    width: {
      get: function() {
        return width;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          width = converter_.toUnsignedLong(v, false);
        }
      },
      enumerable: true
    },
    height: {
      get: function() {
        return height;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          height = converter_.toUnsignedLong(v, false);
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

VideoContent.prototype = new Content();
VideoContent.prototype.constructor = VideoContent;


function AudioContentLyrics(data) {
  var type;
  var timestamps;
  var texts;

  Object.defineProperties(this, {
    type: {
      get: function() {
        return type;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          type = converter_.toEnum(v, Object.keys(AudioContentLyricsType), false);
        }
      },
      enumerable: true
    },
    timestamps: {
      get: function() {
        return timestamps;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          timestamps = converter_.toArray(v, true);
        }
      },
      enumerable: true
    },
    texts: {
      get: function() {
        return texts;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          texts = converter_.toArray(v, false);
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


function AudioContent(data) {
  Content.call(this, data);

  var album;
  var genres;
  var artists;
  var composers;
  var lyrics;
  var copyright;
  var bitrate;
  var trackNumber;
  var duration;

  var getLyrics = function() {
    var data = {
      contentURI: convertUriToPath_(this.contentURI)
    };

    var result = native_.callSync('ContentManager_getLyrics', data);

    if (native_.isFailure(result)) {
      console.log('Getting lyrics failed for ' + data.contentURI);
      return;
    }

    return new AudioContentLyrics(native_.getResultObject(result));
  }.bind(this);

  Object.defineProperties(this, {
    album: {
      get: function() {
        return album;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          album = converter_.toString(v, false);
        }
      },
      enumerable: true
    },
    genres: {
      get: function() {
        return genres;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          genres = converter_.toArray(v, true);
        }
      },
      enumerable: true
    },
    artists: {
      get: function() {
        return artists;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          artists = converter_.toArray(v, true);
        }
      },
      enumerable: true
    },
    composers: {
      get: function() {
        return composers;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          composers = converter_.toArray(v, true);
        }
      },
      enumerable: true
    },
    lyrics: {
      get: function() {
        if (lyrics === undefined) {
          lyrics = getLyrics();
        }
        return lyrics;
      },
      set: function(v) {
        if (edit_.isAllowed && type_.isObject(v)) {
          lyrics = new AudioContentLyrics(v);
        }
      },
      enumerable: true
    },
    copyright: {
      get: function() {
        return copyright;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          copyright = converter_.toString(v, false);
        }
      },
      enumerable: true
    },
    bitrate: {
      get: function() {
        return bitrate;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          bitrate = converter_.toUnsignedLong(v, false);
        }
      },
      enumerable: true
    },
    trackNumber: {
      get: function() {
        return trackNumber;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          trackNumber = converter_.toUnsignedLong(v, false);
        }
      },
      enumerable: true
    },
    duration: {
      get: function() {
        return duration;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          duration = converter_.toUnsignedLong(v, false);
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

AudioContent.prototype = new Content();
AudioContent.prototype.constructor = AudioContent;


function ImageContent(data) {
  Content.call(this, data);

  var editableAttributes = this.editableAttributes;
  editableAttributes.push('geolocation');
  editableAttributes.push('orientation');

  var geolocation;
  var width;
  var height;
  var orientation;

  Object.defineProperties(this, {
    editableAttributes: {
      value: editableAttributes,
      writable: false,
      enumerable: true
    },
    geolocation: {
      get: function() {
        return geolocation;
      },
      set: function(v) {
        if (!type_.isNull(v)) {
          var latitude = converter_.toDouble(v.latitude, false);
          var longitude = converter_.toDouble(v.longitude, false);
          geolocation = new tizen.SimpleCoordinates(latitude, longitude);
        }
      },
      enumerable: true
    },
    width: {
      get: function() {
        return width;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          width = converter_.toUnsignedLong(v, false);
        }
      },
      enumerable: true
    },
    height: {
      get: function() {
        return height;
      },
      set: function(v) {
        if (edit_.isAllowed) {
          height = converter_.toUnsignedLong(v, false);
        }
      },
      enumerable: true
    },
    orientation: {
      get: function() {
        return orientation;
      },
      set: function(v) {
        if (!type_.isNull(v)) {
          orientation = converter_.toEnum(v, Object.keys(ImageContentOrientation), false);
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

ImageContent.prototype = new Content();
ImageContent.prototype.constructor = ImageContent;


function PlaylistItem(data) {
  var content = data;

  Object.defineProperties(this, {
    content: {
      get: function() {
        return content;
      },
      set: function(v) {
        if (edit_.isAllowed && v instanceof Content) {
          content = v;
        }
      },
      enumerable: true
    }
  });
}
