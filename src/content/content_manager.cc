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
 
#include "content/content_manager.h"

#include <algorithm>
#include <cstring>
#include <map>
#include <metadata_extractor.h>
#include <sstream>
#include <string>
#include <unistd.h>
#include <stdlib.h>

#include "common/converter.h"
#include "common/logger.h"
#include "common/scope_exit.h"
#include "common/virtual_fs.h"
#include "content/content_filter.h"
#include "common/virtual_fs.h"

using namespace std;
using namespace common;

namespace extension {
namespace content {

namespace {
static const std::string uri_prefix = "file://";
static const std::string uri_absolute_prefix = "file:///";
}

const std::map<std::string, media_content_orientation_e> orientationMap = {
    {"NORMAL", MEDIA_CONTENT_ORIENTATION_NORMAL},
    {"FLIP_HORIZONTAL", MEDIA_CONTENT_ORIENTATION_HFLIP},
    {"ROTATE_180", MEDIA_CONTENT_ORIENTATION_ROT_180},
    {"FLIP_VERTICAL", MEDIA_CONTENT_ORIENTATION_VFLIP},
    {"TRANSPOSE", MEDIA_CONTENT_ORIENTATION_TRANSPOSE},
    {"ROTATE_90", MEDIA_CONTENT_ORIENTATION_ROT_90},
    {"TRANSVERSE", MEDIA_CONTENT_ORIENTATION_TRANSVERSE},
    {"ROTATE_270", MEDIA_CONTENT_ORIENTATION_ROT_270},
};

std::string get_date(char* tmpStr) {
  LoggerD("Enter");
  if (tmpStr) {
    struct tm* result = (struct tm*) calloc(1, sizeof(struct tm));
    if (nullptr != result) {
      if (strptime(tmpStr, "%Y:%m:%d %H:%M:%S", result) == NULL) {
        free(result);
        return std::string();
      } else {
        time_t t = mktime(result);// + get_utc_offset() * 3600;
        std::stringstream str_date;
        str_date << t;
        free(result);
        return str_date.str();
      }
    }
  }
  return std::string();
}

void ContentToJson(media_info_h info, picojson::object& o) {
  LoggerD("Enter");
  int ret;
  int tmpInt = 0;
  bool tmpBool = false;
  char* tmpStr = NULL;
  time_t tmpDate;
  double tmpDouble;
  long long unsigned int tmpLong;
  media_content_type_e type;

  ret = media_info_get_media_type(info, &type);

  if (ret != MEDIA_CONTENT_ERROR_NONE) {
    LOGGER(ERROR) << "Get media type failed: " << ret;
    type = MEDIA_CONTENT_TYPE_OTHERS;
  }

  if (type == MEDIA_CONTENT_TYPE_IMAGE) {
    o["type"] = picojson::value(std::string("IMAGE"));
    image_meta_h img;
    if (MEDIA_CONTENT_ERROR_NONE == media_info_get_image(info, &img)) {
      std::unique_ptr<std::remove_pointer<image_meta_h>::type, int(*)(image_meta_h)>
          img_ptr(img, &image_meta_destroy); // automatically release the memory
      if (MEDIA_CONTENT_ERROR_NONE == image_meta_get_date_taken(img, &tmpStr)) {
        if (tmpStr) {
          o["releaseDate"] = picojson::value(get_date(tmpStr));
          free(tmpStr);
          tmpStr = NULL;
        }
      }
      if (MEDIA_CONTENT_ERROR_NONE == image_meta_get_width(img, &tmpInt)) {
        o["width"] = picojson::value(static_cast<double>(tmpInt));
      }
      if (MEDIA_CONTENT_ERROR_NONE == image_meta_get_height(img, &tmpInt)) {
        o["height"] = picojson::value(static_cast<double>(tmpInt));
      }
      picojson::object geo;
      if (MEDIA_CONTENT_ERROR_NONE == media_info_get_latitude(info, &tmpDouble)) {
        geo["latitude"] = picojson::value(tmpDouble);
      }
      if (MEDIA_CONTENT_ERROR_NONE == media_info_get_longitude(info, &tmpDouble)) {
        geo["longitude"] = picojson::value(tmpDouble);
      }
      o["geolocation"] = picojson::value(geo);
      std::string ori;
      media_content_orientation_e orientation;
      if (MEDIA_CONTENT_ERROR_NONE == image_meta_get_orientation(img, &orientation)) {
        switch (orientation) {
          case MEDIA_CONTENT_ORIENTATION_NOT_AVAILABLE:
          case MEDIA_CONTENT_ORIENTATION_NORMAL:
            ori = "NORMAL";
            break;
          case MEDIA_CONTENT_ORIENTATION_HFLIP:
            ori = "FLIP_HORIZONTAL";
            break;
          case MEDIA_CONTENT_ORIENTATION_ROT_180:
            ori = "ROTATE_180";
            break;
          case MEDIA_CONTENT_ORIENTATION_VFLIP:
            ori = "FLIP_VERTICAL";
            break;
          case MEDIA_CONTENT_ORIENTATION_TRANSPOSE:
            ori = "TRANSPOSE";
            break;
          case MEDIA_CONTENT_ORIENTATION_ROT_90:
            ori = "ROTATE_90";
            break;
          case MEDIA_CONTENT_ORIENTATION_TRANSVERSE:
            ori = "TRANSVERSE";
            break;
          case MEDIA_CONTENT_ORIENTATION_ROT_270:
            ori = "ROTATE_270";
            break;
        }
        o["orientation"] = picojson::value(ori);
      }
    }
  } else if (type == MEDIA_CONTENT_TYPE_VIDEO) {
    o["type"] = picojson::value(std::string("VIDEO"));
    video_meta_h video;
    if (MEDIA_CONTENT_ERROR_NONE == media_info_get_video(info, &video)) {
      std::unique_ptr<std::remove_pointer<video_meta_h>::type, int(*)(video_meta_h)>
          video_ptr(video, &video_meta_destroy); // automatically release the memory
      if (MEDIA_CONTENT_ERROR_NONE == video_meta_get_width(video, &tmpInt)) {
        o["width"] = picojson::value(static_cast<double>(tmpInt));
      }

      if (MEDIA_CONTENT_ERROR_NONE == video_meta_get_height(video, &tmpInt)) {
        o["height"] = picojson::value(static_cast<double>(tmpInt));
      }
      if (MEDIA_CONTENT_ERROR_NONE == video_meta_get_artist(video, &tmpStr)) {
        picojson::array artists;
        if (tmpStr) {
          artists.push_back(picojson::value(std::string(tmpStr)));
          free(tmpStr);
          tmpStr = NULL;
        }
        o["artists"] = picojson::value(artists);
      }
      if (MEDIA_CONTENT_ERROR_NONE == video_meta_get_album(video, &tmpStr)) {
        if (tmpStr) {
          o["album"] = picojson::value(tmpStr);
          free(tmpStr);
          tmpStr = NULL;
        }
      }
      if (MEDIA_CONTENT_ERROR_NONE == video_meta_get_duration(video, &tmpInt)) {
        o["duration"] = picojson::value(static_cast<double>(tmpInt));
      }
      if (MEDIA_CONTENT_ERROR_NONE == video_meta_get_recorded_date(video, &tmpStr)) {
        if (tmpStr) {
          o["releaseDate"] = picojson::value(get_date(tmpStr));
          free(tmpStr);
          tmpStr = NULL;
        }
      }
    }
    picojson::object geo;
    if (MEDIA_CONTENT_ERROR_NONE == media_info_get_latitude(info, &tmpDouble)) {
      geo["latitude"] = picojson::value(tmpDouble);
    }
    if (MEDIA_CONTENT_ERROR_NONE == media_info_get_longitude(info, &tmpDouble)) {
      geo["longitude"] = picojson::value(tmpDouble);
    }
    o["geolocation"] = picojson::value(geo);
  } else if (type == MEDIA_CONTENT_TYPE_SOUND || type == MEDIA_CONTENT_TYPE_MUSIC) {
    o["type"] = picojson::value(std::string("AUDIO"));
    audio_meta_h audio;
    if (MEDIA_CONTENT_ERROR_NONE == media_info_get_audio(info, &audio)) {
      std::unique_ptr<std::remove_pointer<audio_meta_h>::type, int(*)(audio_meta_h)>
          audio_ptr(audio, &audio_meta_destroy); // automatically release the memory
      if (MEDIA_CONTENT_ERROR_NONE == audio_meta_get_recorded_date(audio, &tmpStr)) {
        if(tmpStr) {
          o["releaseDate"] = picojson::value(get_date(tmpStr));
          free(tmpStr);
          tmpStr = NULL;
        }
      }
      if (MEDIA_CONTENT_ERROR_NONE == audio_meta_get_album(audio, &tmpStr)) {
        if (tmpStr) {
          o["album"] = picojson::value(std::string(tmpStr));
          free(tmpStr);
          tmpStr = NULL;
        }
      }
      if (MEDIA_CONTENT_ERROR_NONE == audio_meta_get_artist(audio, &tmpStr)) {
        if (tmpStr) {
          picojson::array artists;
          artists.push_back(picojson::value(std::string(tmpStr)));
          o["artists"] = picojson::value(artists);
          free(tmpStr);
          tmpStr = NULL;
        }
      }
      if (MEDIA_CONTENT_ERROR_NONE == audio_meta_get_genre(audio, &tmpStr)) {
        if (tmpStr) {
          picojson::array genres;
          genres.push_back(picojson::value(std::string(tmpStr)));
          o["genres"] = picojson::value(genres);
          free(tmpStr);
          tmpStr = NULL;
        }
      }
      if (MEDIA_CONTENT_ERROR_NONE == audio_meta_get_composer(audio, &tmpStr)) {
        if (tmpStr) {
          picojson::array composers;
          composers.push_back(picojson::value(std::string(tmpStr)));
          o["composers"] = picojson::value(composers);
          free(tmpStr);
          tmpStr = NULL;
        }
      }
      if (MEDIA_CONTENT_ERROR_NONE == audio_meta_get_copyright(audio, &tmpStr)) {
        if (tmpStr) {
          o["copyright"] = picojson::value(std::string(tmpStr));
          free(tmpStr);
          tmpStr = NULL;
        }
      }
      if (MEDIA_CONTENT_ERROR_NONE == audio_meta_get_bit_rate(audio, &tmpInt)) {
        o["bitrate"] = picojson::value(static_cast<double>(tmpInt));
      }
      if (MEDIA_CONTENT_ERROR_NONE == audio_meta_get_track_num(audio, &tmpStr)) {
        if (tmpStr) {
          o["trackNumber"] = picojson::value(static_cast<double>(std::atoi(tmpStr)));
          free(tmpStr);
          tmpStr = NULL;
        }
        else {
          o["trackNumber"] = picojson::value();
        }
      }
      if (MEDIA_CONTENT_ERROR_NONE == audio_meta_get_duration(audio, &tmpInt)) {
        o["duration"] = picojson::value(static_cast<double>(tmpInt));
      }
    }
  } else {
    o["type"] = picojson::value(std::string("OTHER"));
  }

  ret = media_info_get_media_id(info, &tmpStr);
  if(ret == MEDIA_CONTENT_ERROR_NONE) {
    if(tmpStr) {
      o["id"] = picojson::value(std::string(tmpStr));
      free(tmpStr);
      tmpStr = NULL;
    }
  }
  ret = media_info_get_display_name(info, &tmpStr);
  if(ret == MEDIA_CONTENT_ERROR_NONE) {
    if(tmpStr) {
      o["name"] = picojson::value(std::string(tmpStr));
      free(tmpStr);
      tmpStr = NULL;
    }
  }

  ret = media_info_get_mime_type(info, &tmpStr);
  if(ret == MEDIA_CONTENT_ERROR_NONE) {
    if(tmpStr) {
      o["mimeType"] = picojson::value(std::string(tmpStr));
      free(tmpStr);
      tmpStr = NULL;
    }
  }
  ret = media_info_get_title(info, &tmpStr);
  if(ret == MEDIA_CONTENT_ERROR_NONE) {
    if(tmpStr) {
      o["title"] = picojson::value(std::string(tmpStr));
      free(tmpStr);
      tmpStr = NULL;
    }
  }
  ret = media_info_get_file_path(info, &tmpStr);
  if(ret == MEDIA_CONTENT_ERROR_NONE) {
    if(tmpStr) {
      o["contentURI"] = picojson::value(std::string(tmpStr));
      free(tmpStr);
      tmpStr = NULL;
    }
  }
  ret = media_info_get_thumbnail_path(info, &tmpStr);
  if(ret == MEDIA_CONTENT_ERROR_NONE) {
    if(tmpStr) {
      picojson::array thumbnails;
      thumbnails.push_back(picojson::value(std::string(tmpStr)));
      o["thumbnailURIs"] = picojson::value(thumbnails);
      free(tmpStr);
      tmpStr = NULL;
    }
  }
  ret = media_info_get_description(info, &tmpStr);
  if(ret == MEDIA_CONTENT_ERROR_NONE) {
    if(tmpStr) {
      o["description"] = picojson::value(std::string(tmpStr));
      free(tmpStr);
      tmpStr = NULL;
    }
  }
  ret = media_info_get_rating(info, &tmpInt);
  if(ret == MEDIA_CONTENT_ERROR_NONE) {
    o["rating"] = picojson::value(static_cast<double>(tmpInt));
  }
  ret = media_info_get_size(info, &tmpLong);
  if(ret == MEDIA_CONTENT_ERROR_NONE) {
    o["size"] = picojson::value(static_cast<double>(tmpLong));
  }
  ret = media_info_get_favorite(info, &tmpBool);
  if(ret == MEDIA_CONTENT_ERROR_NONE) {
    o["isFavorite"] = picojson::value(tmpBool);
  }
  ret = media_info_get_modified_time(info, &tmpDate);
  if(ret == MEDIA_CONTENT_ERROR_NONE) {
    o["modifiedDate"] = picojson::value(static_cast<double>(tmpDate));
  }
}

void ContentDirToJson(media_folder_h folder, picojson::object& o) {
  LoggerD("Enter");
  int ret;
  char* tmpStr = NULL;
  media_content_storage_e storage_type;
  time_t tmpDate;

  // id
  ret = media_folder_get_folder_id(folder, &tmpStr);
  if(ret == MEDIA_CONTENT_ERROR_NONE) {
    if(tmpStr) {
      o["id"] = picojson::value(std::string(tmpStr));
      free(tmpStr);
      tmpStr = NULL;
    }
  }

  // directoryURI
  ret = media_folder_get_path(folder, &tmpStr);
  if(ret == MEDIA_CONTENT_ERROR_NONE) {
    if(tmpStr) {
      o["directoryURI"] = picojson::value(std::string(tmpStr));
      free(tmpStr);
      tmpStr = NULL;
    }
  }

  // title
  ret = media_folder_get_name(folder, &tmpStr);
  if(ret == MEDIA_CONTENT_ERROR_NONE) {
    if(tmpStr) {
      o["title"] = picojson::value(std::string(tmpStr));
      free(tmpStr);
      tmpStr = NULL;
    }
  }

  // storageType
  ret = media_folder_get_storage_type(folder, &storage_type);
  if(ret == MEDIA_CONTENT_ERROR_NONE) {
    if (storage_type == MEDIA_CONTENT_STORAGE_INTERNAL) {
      o["storageType"] = picojson::value(std::string("INTERNAL"));
    } else if (storage_type == MEDIA_CONTENT_STORAGE_EXTERNAL) {
      o["storageType"] = picojson::value(std::string("EXTERNAL"));
    } else if (storage_type == MEDIA_CONTENT_STORAGE_CLOUD) {
      o["storageType"] = picojson::value(std::string("CLOUD"));
    }
  }

  // modifiedData
  ret = media_folder_get_modified_time(folder, &tmpDate);
  if(ret == MEDIA_CONTENT_ERROR_NONE) {
    o["modifiedDate"] = picojson::value(static_cast<double>(tmpDate));
  }
}

static int setContent(media_info_h media, const picojson::value& content) {
  LoggerD("Enter");

  int ret;
  std::string name = content.get("name").to_str();
  std::string description = content.get("description").to_str();
  int rating = std::stoi(content.get("rating").to_str());
  bool is_fav = content.get("isFavorite").get<bool>();

  if (NULL == media) {
    LoggerE("MEDIA_CONTENT_ERROR_DB_FAILED");
    return MEDIA_CONTENT_ERROR_DB_FAILED;
  }

  media_content_type_e type;
  ret = media_info_get_media_type(media, &type);
  if (ret != MEDIA_CONTENT_ERROR_NONE) {
    LoggerE("Failed: media_info_get_media_type()");
    return ret;
  }

  ret = media_info_set_display_name(media, name.c_str());
  if (ret != MEDIA_CONTENT_ERROR_NONE) {
    LoggerE("Updating name failed.");
  }

  ret = media_info_set_description(media, description.c_str());
  if (ret != MEDIA_CONTENT_ERROR_NONE) {
    LoggerE("Updating description failed.");
  }

  ret = media_info_set_rating(media, rating);
  if (ret != MEDIA_CONTENT_ERROR_NONE) {
    LoggerE("Updating rating failed.");
  }

  ret = media_info_set_favorite(media, is_fav);
  if (ret != MEDIA_CONTENT_ERROR_NONE) {
    LoggerE("Updating isFavorite failed.");
  }

  if (ret != MEDIA_CONTENT_ERROR_NONE) {
    LoggerE("Updating favorite failed.");
  }

  if (type == MEDIA_CONTENT_TYPE_IMAGE) {
    std::string orientation = content.get("orientation").to_str();
    auto orientationToSet = orientationMap.find(orientation);

    if (orientationToSet != orientationMap.end()) {
      image_meta_h img;
      if (MEDIA_CONTENT_ERROR_NONE == media_info_get_image(media, &img) &&
          MEDIA_CONTENT_ERROR_NONE == image_meta_set_orientation(img, orientationToSet->second) &&
          MEDIA_CONTENT_ERROR_NONE == image_meta_update_to_db(img)) {
        LoggerD("orientation update was successful");
      } else {
        LoggerE("orientation update failed");
      }
      image_meta_destroy(img);
    }
  }

  if (type == MEDIA_CONTENT_TYPE_IMAGE || type == MEDIA_CONTENT_TYPE_VIDEO) {
    picojson::value geo = content.get("geolocation");
    if (geo.evaluate_as_boolean()) {
      LoggerD("geolocation is not null");
      double latitude = atof(geo.get("latitude").to_str().c_str());
      double longitude = atof(geo.get("longitude").to_str().c_str());
      ret = media_info_set_latitude(media, latitude);
      if (ret != MEDIA_CONTENT_ERROR_NONE) {
        LoggerE("Updating geolocation is failed.");
      }
      ret = media_info_set_longitude(media, longitude);
      if (ret != MEDIA_CONTENT_ERROR_NONE) {
        LoggerD("Updating geolocation is failed.");
      }
    } else {
      LoggerD("geolocation is null");
    }
  }

  return MEDIA_CONTENT_ERROR_NONE;
}

static void FolderToJson(media_folder_h folder, picojson::object* out) {
  LoggerD("Enter");

  char* name = NULL;
  char* id = NULL;
  char* path = NULL;
  time_t date;
  media_content_storage_e storageType;

  int ret;

  ret = media_folder_get_folder_id(folder, &id);
  if(ret!=MEDIA_CONTENT_ERROR_NONE)
  {
    LoggerE("Failed: media_folder_get_folder_id");
    common::tools::ReportError(ContentManager::convertError(ret), out);
    return;
  }

  ret = media_folder_get_name(folder, &name);
  if(ret!=MEDIA_CONTENT_ERROR_NONE)
  {
    LoggerE("Failed: media_folder_get_name");
    common::tools::ReportError(ContentManager::convertError(ret), out);
    free(id);
    return;
  }

  ret = media_folder_get_path(folder, &path);
  if(ret!=MEDIA_CONTENT_ERROR_NONE)
  {
    LoggerE("Failed: media_folder_get_path");
    common::tools::ReportError(ContentManager::convertError(ret), out);
    free(id);
    free(name);
    return;
  }

  ret = media_folder_get_modified_time(folder, &date);
  if(ret!=MEDIA_CONTENT_ERROR_NONE)
  {
    LoggerE("Failed: media_folder_get_path");
    common::tools::ReportError(ContentManager::convertError(ret), out);
    free(id);
    free(name);
    free(path);
    return;
  }

  ret = media_folder_get_storage_type(folder, &storageType);
  if(ret!=MEDIA_CONTENT_ERROR_NONE)
  {
    free(id);
    free(name);
    free(path);
    LoggerE("Failed: media_folder_get_storage_type");
    common::tools::ReportError(ContentManager::convertError(ret), out);
    return;
  }


  (*out)["id"] = picojson::value(std::string(id));
  (*out)["directoryURI"] = picojson::value(std::string(path));
  (*out)["title"] = picojson::value(std::string(name));

  if (storageType == MEDIA_CONTENT_STORAGE_INTERNAL) {
    (*out)["storageType"] = picojson::value(std::string("INTERNAL"));
  } else if (storageType == MEDIA_CONTENT_STORAGE_EXTERNAL) {
    (*out)["storageType"] = picojson::value(std::string("EXTERNAL"));
  }

  (*out)["modifiedDate"] = picojson::value(static_cast<double>(date));

  free(name);
  free(id);
  free(path);
}

static bool media_foreach_directory_cb(media_folder_h folder, void *user_data) {
  LoggerD("Enter");
  picojson::array *array = static_cast<picojson::array*>(user_data);
  picojson::object json;
  FolderToJson(folder, &json);
  array->push_back(picojson::value(json));
  return true;
}

static bool media_foreach_content_cb(media_info_h media, void *user_data) {
  LoggerD("Enter");
  picojson::value::array *contents = static_cast<picojson::value::array*>(user_data);
  picojson::value::object o;
  ContentToJson(media, o);
  contents->push_back(picojson::value(o));
  return true;
}

static bool playlist_foreach_cb(media_playlist_h playlist, void *user_data) {

  LoggerD("Enter");
  picojson::value::array *playlists = static_cast<picojson::value::array*>(user_data);
  picojson::value::object o;
  if (playlist != NULL) {
    int id,cnt;
    char* thumb_path = NULL;
    char* name = NULL;
    filter_h filter = NULL;
    if( media_playlist_get_playlist_id(playlist, &id) == MEDIA_CONTENT_ERROR_NONE) {
      std::stringstream str_id;
      str_id << id;
      o["id"] = picojson::value(std::to_string(id));
    }
    else {
      LoggerD("Invalid ID for playlist.");
    }
    if( media_playlist_get_thumbnail_path(playlist, &thumb_path) == MEDIA_CONTENT_ERROR_NONE) {
      if (thumb_path != NULL) {
        std::string thumbnail_uri (thumb_path);
        if (thumbnail_uri != " ") {
          thumbnail_uri = uri_prefix + thumbnail_uri;
        }
        o["thumbnailURI"] = picojson::value(thumbnail_uri);
        free(thumb_path);
      }
      else {
       o["thumbnailURI"] = picojson::value();//picojson::value(std::string(""));
      }
    }
    else {
      LoggerD("Invalid thumbnail path for playlist.");
    }
    if( media_playlist_get_name(playlist, &name) == MEDIA_CONTENT_ERROR_NONE) {
      o["name"] = picojson::value(std::string(name));
      free(name);
    }
    else {
      LoggerD("Invalid name for playlist.");
    }

    media_filter_create(&filter);
    std::unique_ptr<std::remove_pointer<filter_h>::type, int(*)(filter_h)>
        filter_ptr(filter, &media_filter_destroy); // automatically release the memory
    if( media_playlist_get_media_count_from_db(id, filter, &cnt) == MEDIA_CONTENT_ERROR_NONE) {
      o["numberOfTracks"] = picojson::value(static_cast<double>(cnt));
    }
    else {
      LoggerE("Invalid count for playlist.");
    }
    playlists->push_back(picojson::value(o));

  }
  return true;
}

static bool playlist_content_member_cb(int playlist_member_id, media_info_h media, void *user_data) {

  LoggerD("Enter");
  picojson::value::array *contents = static_cast<picojson::value::array*>(user_data);
  picojson::value::object o;

  o["playlist_member_id"] = picojson::value(static_cast<double>(playlist_member_id));
  ContentToJson(media, o);
  contents->push_back(picojson::value(o));
  return true;
}


ContentManager::ContentManager() {
  LoggerD("ContentManager called");
  if(media_content_connect() == MEDIA_CONTENT_ERROR_NONE) {
      m_dbConnected = true;
  }
  else
      m_dbConnected = false;
}

ContentManager::~ContentManager() {
  LoggerD("Enter");
  if(m_dbConnected) {
    if(media_content_disconnect() == MEDIA_CONTENT_ERROR_NONE) {
      m_dbConnected = false;
    }
  }
}

ContentManager* ContentManager::getInstance() {
  LoggerD("Enter");
  static ContentManager instance;
  return &instance;
}

bool ContentManager::isConnected() {
  LoggerD("Enter");
  return m_dbConnected;
}

void ContentManager::getDirectories(const std::shared_ptr<ReplyCallbackData>& user_data) {

  LoggerD("Enter");
  int ret;
  filter_h filter = NULL;
  ret = media_filter_create(&filter);
  if (ret != MEDIA_CONTENT_ERROR_NONE) {
    LoggerE("Failed: media_filter_create failed");
    return;
  }

  SCOPE_EXIT {
    media_filter_destroy(filter);
  };

  std::string condition = "(FOLDER_STORAGE_TYPE = 0 OR FOLDER_STORAGE_TYPE = 1)";
  media_filter_set_condition(filter, condition.c_str(), MEDIA_CONTENT_COLLATE_DEFAULT);

  picojson::array pico_dirs;
  ret = media_folder_foreach_folder_from_db(filter, media_foreach_directory_cb, &pico_dirs);
  if (ret != MEDIA_CONTENT_ERROR_NONE) {
    LoggerE("Failed: Getting the directories failed");
    PlatformResult err(ErrorCode::UNKNOWN_ERR, "Getting the directories failed.");
    user_data->isSuccess = err;
    return;
  }

  user_data->result = picojson::value(pico_dirs);
}

void ContentManager::find(const std::shared_ptr<ReplyCallbackData>& user_data) {
  LoggerD("Enter");

  int ret;
  int count, offset;
  std::string dirId;

  picojson::value::array arrayContent;
  filter_h filter = nullptr;
  media_filter_create(&filter);
  SCOPE_EXIT {
    if (filter) {
      media_filter_destroy(filter);
    }
  };

  if (!IsNull(user_data->args.get("filter"))) {
    ContentFilter filterMechanism;
    std::string query;
    picojson::object argsObject = JsonCast<picojson::object>(user_data->args);
    if (filterMechanism.BuildQuery(
        FromJson<picojson::object>(argsObject, "filter"), &query)) {
      LOGGER(DEBUG) << "Filter query: " << query;
      ret = media_filter_set_condition(filter, query.c_str(),
          MEDIA_CONTENT_COLLATE_DEFAULT);
      if (MEDIA_CONTENT_ERROR_NONE != ret) {
        LoggerE("Platform filter setting failed, error %d", ret);
      }
    }
  }

  if (user_data->args.contains("sortMode")) {
    picojson::value vSortMode = user_data->args.get("sortMode");

    if (vSortMode.is<picojson::object>()) {
      std::string sortModeName, sortModeOrder;

      ContentFilter::MapField(vSortMode.get("attributeName").to_str(), &sortModeName);

      sortModeOrder = vSortMode.get("order").to_str();
      if (!sortModeOrder.empty()) {
        media_content_order_e order = MEDIA_CONTENT_ORDER_ASC;

        if (sortModeOrder == "ASC") {
          order = MEDIA_CONTENT_ORDER_ASC;
        } else if (sortModeOrder == "DESC") {
          order = MEDIA_CONTENT_ORDER_DESC;
        }

        ret = media_filter_set_order(filter, order, sortModeName.c_str(),
            MEDIA_CONTENT_COLLATE_DEFAULT);
        if (MEDIA_CONTENT_ERROR_NONE != ret) {
          LoggerE("Platform SortMode setting failed, error: %d", ret);
        }
      }
    }
  }

  if (!IsNull(user_data->args.get("count"))) {
    count = static_cast<int>(user_data->args.get("count").get<double>());
  } else {
    count = -1;
  }
  if (!IsNull(user_data->args.get("offset"))) {
    offset = static_cast<int>(user_data->args.get("offset").get<double>());
  } else {
    offset = -1;
  }
  ret = media_filter_set_offset(filter, offset, count);
  if (MEDIA_CONTENT_ERROR_NONE != ret) {
    LoggerE("A platform error occurs in media_filter_set_offset: %d", ret);
  }
  if (!IsNull(user_data->args.get("directoryId"))) {
    dirId = user_data->args.get("directoryId").get<std::string>();
    ret = media_folder_foreach_media_from_db(dirId.c_str(), filter, media_foreach_content_cb, static_cast<void*>(&arrayContent));
  } else {
    ret = media_info_foreach_media_from_db(filter, media_foreach_content_cb, static_cast<void*>(&arrayContent));
  }

  if (ret == MEDIA_CONTENT_ERROR_NONE) {
    user_data->result = picojson::value(arrayContent);
  } else {
    LoggerE("The iteration failed in platform: %d", ret);
    PlatformResult err(ErrorCode::UNKNOWN_ERR, "The iteration failed in platform");
    user_data->isSuccess = err;
  }
}

int ContentManager::scanFile(std::string& uri) {
  LoggerD("Enter");
  return media_content_scan_file(uri.c_str());
}

PlatformResult ContentManager::scanDirectory(media_scan_completed_cb callback, ReplyCallbackData* cbData) {
  LoggerD("Enter");
  const std::string& contentDirURI = cbData->args.get("contentDirURI").get<std::string>();
  std::string real_path = common::VirtualFs::GetInstance().GetRealPath(contentDirURI);
  const bool recursive = cbData->args.get("recursive").get<bool>();

  int ret = media_content_scan_folder(real_path.c_str(), recursive, callback, (void*) cbData);

  if (ret != MEDIA_CONTENT_ERROR_NONE) {
    LoggerE("Scan folder failed in platform: %d", ret);
    if (MEDIA_CONTENT_ERROR_INVALID_PARAMETER == ret) {
      return PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Scanning content directory failed");
    } else {
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Scanning content directory failed");
    }
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ContentManager::cancelScanDirectory(const std::string& content_dir_uri) {
  LoggerD("Enter");

  int ret = media_content_cancel_scan_folder(content_dir_uri.c_str());
  if (ret != MEDIA_CONTENT_ERROR_NONE) {
    LoggerE("Cancel scan folder failed in platform: %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cancel scan content directory failed");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ContentManager::setChangeListener(media_content_db_update_cb callback,
                                                 void *user_data) {
  LoggerD("Enter");

  int ret = media_content_set_db_updated_cb(callback, user_data);
  if(ret != MEDIA_CONTENT_ERROR_NONE) {
    LoggerE("Failed: registering the listener is failed");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, ("registering the listener is failed."));
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ContentManager::unSetChangeListener() {
  LoggerD("Enter");

  int ret = media_content_unset_db_updated_cb();
  if(ret != MEDIA_CONTENT_ERROR_NONE) {
    LoggerE("Failed: unregistering the listener is failed");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, ("unregistering the listener is failed."));
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ContentManager::setV2ChangeListener(media_content_noti_h* noti_handle,
                                                 media_content_db_update_cb callback,
                                                 void *user_data) {
  LoggerD("Enter");
  if (nullptr == *noti_handle) {
    int ret = media_content_set_db_updated_cb_v2(noti_handle, callback, user_data);
    if(ret != MEDIA_CONTENT_ERROR_NONE) {
      LoggerE("Failed: registering the listener of cb_v2 is failed");
      return PlatformResult(ErrorCode::UNKNOWN_ERR, ("registering the listener is failed."));
    }
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ContentManager::unSetV2ChangeListener(media_content_noti_h* noti_handle) {
  LoggerD("Enter");

  int ret = media_content_unset_db_updated_cb_v2(*noti_handle);
  if(ret != MEDIA_CONTENT_ERROR_NONE) {
    LoggerE("Failed: unregistering the listener of cb_v2 is failed");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, ("unregistering the listener is failed."));
  }
  *noti_handle = nullptr;

  return PlatformResult(ErrorCode::NO_ERROR);
}


void ContentManager::createPlaylist(std::string name,
  const std::shared_ptr<ReplyCallbackData>& user_data) {
  LoggerD("Enter");
  media_playlist_h	playlist = NULL;

  int ret = media_playlist_insert_to_db(name.c_str(),&playlist);
  std::unique_ptr<std::remove_pointer<media_playlist_h>::type, int(*)(media_playlist_h)>
      playlist_ptr(playlist, &media_playlist_destroy); // automatically release the memory
  if(ret != MEDIA_CONTENT_ERROR_NONE) {
    LoggerE("Failed: creation of playlist is failed: %d", ret);
    // MEDIA_CONTENT_ERROR_DB_FAILED means that playlist probably already exists
    PlatformResult err(MEDIA_CONTENT_ERROR_DB_FAILED == ret ? ErrorCode::INVALID_VALUES_ERR : ErrorCode::UNKNOWN_ERR, "Creation of playlist has failed.");
    user_data->isSuccess = err;
    return;
  }
  picojson::value::object o;

  if( playlist != NULL) {
    int id,cnt;
    char* thumb_path = NULL;
    char* name = NULL;
    filter_h filter = NULL;
    if( media_playlist_get_playlist_id(playlist, &id) == MEDIA_CONTENT_ERROR_NONE) {
      o["id"] = picojson::value(std::to_string(id));
    }
    else {
      LoggerE("Failed: loading of playlist is failed");
      PlatformResult err(ErrorCode::UNKNOWN_ERR, "loading of playlist is failed.");
      user_data->isSuccess = err;
      return;
    }
    if( media_playlist_get_thumbnail_path(playlist, &thumb_path) == MEDIA_CONTENT_ERROR_NONE) {
      if (thumb_path != NULL) {
        o["thumbnailURI"] = picojson::value(std::string(thumb_path));
        free(thumb_path);
      }
      else {
        o["thumbnailURI"] = picojson::value();
      }
    }
    else {
      LoggerE("Invalid thumbnail path for playlist.");
    }
    if( media_playlist_get_name(playlist, &name) == MEDIA_CONTENT_ERROR_NONE) {
      o["name"] = picojson::value(std::string(name));
      free(name);
    }
    else {
      LoggerE("Invalid name for playlist.");
    }
    media_filter_create(&filter);
    std::unique_ptr<std::remove_pointer<filter_h>::type, int(*)(filter_h)>
        filter_ptr(filter, &media_filter_destroy); // automatically release the memory

    if( media_playlist_get_media_count_from_db(id, filter, &cnt) == MEDIA_CONTENT_ERROR_NONE) {
      o["numberOfTracks"] = picojson::value(static_cast<double>(cnt));
    }
    else {
      LoggerE("Invalid count for playlist.");
    }
  }

  user_data->result = picojson::value(o);
}

void ContentManager::getPlaylists(const std::shared_ptr<ReplyCallbackData>& user_data) {

  LoggerD("Enter");
  int ret;
  filter_h 	filter = nullptr;
  media_filter_create(&filter);
  std::unique_ptr<std::remove_pointer<filter_h>::type, int(*)(filter_h)>
      filter_ptr(filter, &media_filter_destroy); // automatically release the memory
  picojson::value::array playlists;

  ret = media_playlist_foreach_playlist_from_db(filter, playlist_foreach_cb, static_cast<void*>(&playlists));

  if(ret != MEDIA_CONTENT_ERROR_NONE) {
    LoggerE("Failed: Getting playlist is failed");
    PlatformResult err(ErrorCode::UNKNOWN_ERR, "Getting playlist is failed.");
    user_data->isSuccess = err;
  }

  user_data->result = picojson::value(playlists);
}

void ContentManager::removePlaylist(std::string playlistId,
  const std::shared_ptr<ReplyCallbackData>& user_data) {

  LoggerD("Enter");
  int id = std::atoi(playlistId.c_str());
  if(id == 0) {
    LoggerE("Failed: PlaylistId is wrong");
    PlatformResult err(ErrorCode::UNKNOWN_ERR, "PlaylistId is wrong.");
    user_data->isSuccess = err;
    return;
  }

  int ret = media_playlist_delete_from_db(id);
  if(ret != MEDIA_CONTENT_ERROR_NONE) {
    LoggerE("Failed: Removal of playlist is failed");
    PlatformResult err(ErrorCode::UNKNOWN_ERR, "Removal of playlist is failed.");
    user_data->isSuccess = err;
  }
}

int ContentManager::update(picojson::value args) {
  LoggerD("Enter");

  int ret;
  picojson::value content = args.get("content");
  std::string id = content.get("id").to_str();

  media_info_h media = NULL;
  ret = media_info_get_media_from_db(id.c_str(), &media);
  if (ret == MEDIA_CONTENT_ERROR_NONE) {
    setContent(media, content);
    ret = media_info_update_to_db(media);
    media_info_destroy(media);
  }

  return ret;
}

int ContentManager::updateBatch(picojson::value args) {
  LoggerD("Enter");
  int ret = 0;
  std::vector<picojson::value> contents = args.get("contents").get<picojson::array>();

  for (picojson::value::array::iterator it = contents.begin(); it != contents.end(); ++it) {
    picojson::value content = *it;
    std::string id = content.get("id").to_str();
    media_info_h media = NULL;
    ret = media_info_get_media_from_db(id.c_str(), &media);
    if (media != NULL && ret == MEDIA_CONTENT_ERROR_NONE) {
      setContent(media, content);
      ret = media_info_update_to_db(media);
      media_info_destroy(media);
    } else {
      return ret;
    }
  }
  return ret;
}

int ContentManager::playlistAdd(std::string playlist_id, std::string content_id) {
  LoggerD("Enter");

  media_playlist_h playlist = NULL;
  int ret = media_playlist_get_playlist_from_db(std::stoi(playlist_id), &playlist);

  if (playlist != NULL && ret == MEDIA_CONTENT_ERROR_NONE) {
    ret = media_playlist_add_media(playlist, content_id.c_str());
    if (ret != MEDIA_CONTENT_ERROR_NONE) {
      LoggerE("The content(id:%s) can't add to playlist",content_id.c_str());
    }

    ret = media_playlist_update_to_db(playlist);
    if (ret != MEDIA_CONTENT_ERROR_NONE) {
      LoggerE("The content(id:%s) can't add to playlist",content_id.c_str());
    }
  }
  else {
    LoggerE("Playlist(id:%s) is not exist",playlist_id.c_str());
  }

  media_playlist_destroy(playlist);
  return ret;
}

int ContentManager::playlistRemove(std::string playlist_id, int member_id) {
  LoggerD("Enter");

  media_playlist_h playlist = NULL;
  int ret = media_playlist_get_playlist_from_db(std::stoi(playlist_id), &playlist);
  if (playlist != NULL && ret == MEDIA_CONTENT_ERROR_NONE) {
    ret = media_playlist_remove_media(playlist, member_id);
    if (ret != MEDIA_CONTENT_ERROR_NONE) {
      LoggerE("The content can't remove to playlist");
    }

    ret = media_playlist_update_to_db(playlist);
    if (ret != MEDIA_CONTENT_ERROR_NONE) {
      LoggerE("The content can't remove to playlist");
    }
  }
  else {
    LoggerE("Playlist(id:%s) is not exist",playlist_id.c_str());
  }
  media_playlist_destroy(playlist);

  return ret;
}


void ContentManager::playlistAddbatch(const std::shared_ptr<ReplyCallbackData>& user_data) {

  LoggerD("Enter");
  std::string playlist_id = user_data->args.get("playlistId").get<std::string>();

  media_playlist_h playlist = NULL;
  int ret = media_playlist_get_playlist_from_db(std::stoi(playlist_id), &playlist);

  if(ret != MEDIA_CONTENT_ERROR_NONE && playlist == NULL) {
    PlatformResult err(ErrorCode::UNKNOWN_ERR, "Getting playlist is failed.");
    user_data->isSuccess = err;
    return;
  }

  std::vector<picojson::value> contents = user_data->args.get("contents").get<picojson::array>();
  for (picojson::value::array::iterator it = contents.begin(); it != contents.end(); ++it) {
    picojson::value content = *it;
    std::string id = content.get("id").to_str();
    ret = media_playlist_add_media(playlist, id.c_str());
    if (ret != MEDIA_CONTENT_ERROR_NONE) {
      LoggerE("Adding Content(id:%s) is failed.", id.c_str());
    }
  }

  ret = media_playlist_update_to_db(playlist);
  if(ret != MEDIA_CONTENT_ERROR_NONE ) {
    PlatformResult err(ErrorCode::UNKNOWN_ERR, "Adding playlist is failed.");
    user_data->isSuccess = err;
  }
  media_playlist_destroy(playlist);
}

void ContentManager::playlistGet(const std::shared_ptr<ReplyCallbackData>& user_data) {

  LoggerD("Enter");
  media_playlist_h playlist = NULL;
  media_content_order_e order = MEDIA_CONTENT_ORDER_ASC;
  const std::string playOrder("play_order");

  SCOPE_EXIT {
    if (playlist) {
      media_playlist_destroy(playlist);
    }
  };

  std::string playlist_id = user_data->args.get("playlistId").get<std::string>();
  int ret = media_playlist_get_playlist_from_db(std::stoi(playlist_id), &playlist);
  if(ret != MEDIA_CONTENT_ERROR_NONE && playlist == NULL) {
    LoggerE("Failed: Getting playlist is failed");
    PlatformResult err(ErrorCode::UNKNOWN_ERR, "Getting playlist is failed.");
    user_data->isSuccess = err;
    return;
  }

  filter_h filter = NULL;
  ret = media_filter_create(&filter);
  if (ret != MEDIA_CONTENT_ERROR_NONE) {
    LoggerE("Failed: Creating a filter is failed");
    PlatformResult err(ErrorCode::UNKNOWN_ERR, "Creating a filter is failed.");
    user_data->isSuccess = err;
    return;
  }

  int count = user_data->args.get("count").get<double>();
  int offset = user_data->args.get("offset").get<double>();
  ret = media_filter_set_offset(filter, offset, count);
  if (ret != MEDIA_CONTENT_ERROR_NONE) {
    LoggerD("Setting a offset/count is failed.");
  }
  ret = media_filter_set_order(filter, order, playOrder.c_str(), MEDIA_CONTENT_COLLATE_DEFAULT);
  if (ret != MEDIA_CONTENT_ERROR_NONE) {
    LoggerD("Setting a offset/count is failed.");
  }

  picojson::value::array arrayContent;
  ret = media_playlist_foreach_media_from_db(std::stoi(playlist_id),
    filter, playlist_content_member_cb, static_cast<void*>(&arrayContent));

  media_filter_destroy(filter);
  if (ret == MEDIA_CONTENT_ERROR_NONE) {
    user_data->result = picojson::value(arrayContent);
  }
  else {
    LoggerE("Failed: Creating a filter is failed");
    PlatformResult err(ErrorCode::UNKNOWN_ERR, "Creating a filter is failed.");
    user_data->isSuccess = err;
  }
}

void ContentManager::playlistRemovebatch(const std::shared_ptr<ReplyCallbackData>& user_data) {

  LoggerD("Enter");
  media_playlist_h playlist = NULL;

  SCOPE_EXIT {
    if (playlist) {
      media_playlist_destroy(playlist);
    }
  };

  std::string playlist_id = user_data->args.get("playlistId").get<std::string>();
  int ret = media_playlist_get_playlist_from_db(std::stoi(playlist_id), &playlist);
  if(ret != MEDIA_CONTENT_ERROR_NONE && playlist == NULL) {
    PlatformResult err(ErrorCode::UNKNOWN_ERR, "Getting playlist is failed.");
    user_data->isSuccess = err;
    return;
  }

  std::vector<picojson::value> members = user_data->args.get("members").get<picojson::array>();
  std::size_t members_size = members.size();
  for (std::size_t i = 0; i < members_size; ++i) {
    int member_id = static_cast<int>(members.at(i).get<double>());
    ret = media_playlist_remove_media(playlist, member_id);

    if (ret != MEDIA_CONTENT_ERROR_NONE) {
      LoggerD("Removing a content is failed.");
    }
  }

  ret = media_playlist_update_to_db(playlist);
  if (ret != MEDIA_CONTENT_ERROR_NONE) {
    LoggerE("Failed: Removing the contents is failed");
    PlatformResult err(ErrorCode::UNKNOWN_ERR, "Removing the contents is failed.");
    user_data->isSuccess = err;
  }
}

void ContentManager::playlistSetOrder(const std::shared_ptr<ReplyCallbackData>& user_data) {

  LoggerD("Enter");
  media_playlist_h playlist = NULL;

  SCOPE_EXIT {
    if (playlist) {
      media_playlist_destroy(playlist);
    }
  };

  std::string playlist_id = user_data->args.get("playlistId").get<std::string>();
  int ret = media_playlist_get_playlist_from_db(std::stoi(playlist_id), &playlist);
  if(ret != MEDIA_CONTENT_ERROR_NONE && playlist == NULL) {
    PlatformResult err(ErrorCode::UNKNOWN_ERR, "Getting playlist is failed.");
    user_data->isSuccess = err;
    return;
  }

  int cnt;
  std::vector<picojson::value> members = user_data->args.get("members").get<picojson::array>();

  ret = media_playlist_get_media_count_from_db(std::stoi(playlist_id), NULL, &cnt);
  if(ret != MEDIA_CONTENT_ERROR_NONE)
  {
    LoggerE("Failed: media_playlist_get_media_count_from_db");
    PlatformResult err = convertError(ret);
    user_data->isSuccess = err;
    return;
  }
  std::size_t members_size = members.size();
  if (cnt < 0 || static_cast<size_t>(cnt) != members_size ) {
    LoggerE("Failed: The items array does not contain all items from the playlist");
    PlatformResult err(ErrorCode::INVALID_VALUES_ERR, "The items array does not contain all items from the playlist.");
    user_data->isSuccess = err;
    return;
  }

  for (std::size_t i = 0; i < members_size; ++i) {
    int member_id = static_cast<int>(members.at(i).get<double>());
    ret = media_playlist_set_play_order(playlist, member_id, i);
    if (ret != MEDIA_CONTENT_ERROR_NONE) {
      LoggerD("Removing a content is failed.");
    }
  }

  ret = media_playlist_update_to_db(playlist);
  if (ret != MEDIA_CONTENT_ERROR_NONE) {
    LoggerE("Failed: Removing the contents is failed");
    PlatformResult err(ErrorCode::UNKNOWN_ERR, "Removing the contents is failed.");
    user_data->isSuccess = err;
  }
}

void ContentManager::playlistMove(const std::shared_ptr<ReplyCallbackData>& user_data) {
  LoggerD("Enter");
  media_playlist_h playlist = NULL;

  SCOPE_EXIT {
    if (playlist) {
      media_playlist_destroy(playlist);
    }
  };

  std::string playlist_id = user_data->args.get("playlistId").get<std::string>();
  int ret = media_playlist_get_playlist_from_db(std::stoi(playlist_id), &playlist);
  if(ret != MEDIA_CONTENT_ERROR_NONE && playlist == NULL) {
    LoggerE("Failed: Getting playlist is failed");
    PlatformResult err(ErrorCode::UNKNOWN_ERR, "Getting playlist is failed.");
    user_data->isSuccess = err;
    return;
  }
  int old_order;
  double member_id = user_data->args.get("memberId").get<double>();
  double delta = user_data->args.get("delta").get<double>();
  ret = media_playlist_get_play_order(playlist, static_cast<int>(member_id), &old_order);
  if (ret != MEDIA_CONTENT_ERROR_NONE) {
    LoggerE("Failed: The content can't find form playlist");
    PlatformResult err(ErrorCode::UNKNOWN_ERR, "The content can't find form playlist.");
    user_data->isSuccess = err;
    return;
  }
  int new_order = static_cast<int>(old_order) + static_cast<int>(delta);
  ret = media_playlist_set_play_order(playlist, static_cast<int>(member_id), new_order);
  if (ret != MEDIA_CONTENT_ERROR_NONE) {
    LoggerE("Failed: The content can't update play_order");
    PlatformResult err(ErrorCode::UNKNOWN_ERR, "The content can't update play_order.");
    user_data->isSuccess = err;
    return;
  }
  ret = media_playlist_update_to_db(playlist);
  if (ret != MEDIA_CONTENT_ERROR_NONE) {
    LoggerE("Failed: Updateing play_order is failed");
    PlatformResult err(ErrorCode::UNKNOWN_ERR, "Updateing play_order is failed.");
    user_data->isSuccess = err;
  }
}

int ContentManager::getLyrics(const picojson::value& args, picojson::object& result) {
  LoggerD("Enter");

  int ret = METADATA_EXTRACTOR_ERROR_NONE;
  const std::string& contentURI = args.get("contentURI").to_str();
  if (contentURI.empty()) {
    LOGGER(ERROR) << "contentURI empty - skipping media extractor";
    return -1;
  }

  metadata_extractor_h extractor;
  metadata_extractor_create(&extractor);
  std::unique_ptr<std::remove_pointer<metadata_extractor_h>::type, int(*)(metadata_extractor_h)>
      extractor_ptr(extractor, &metadata_extractor_destroy); // automatically release the memory

  ret = metadata_extractor_set_path(extractor, contentURI.c_str());
  if (ret != METADATA_EXTRACTOR_ERROR_NONE) {
    LOGGER(ERROR) << "metadata_extractor_set_path failed, error: " << ret;
    return ret;
  }
  picojson::array timestamps;
  picojson::array texts = picojson::array();
  char* strSyncTextNum = NULL;

  ret = metadata_extractor_get_metadata(extractor,
      METADATA_SYNCLYRICS_NUM, &strSyncTextNum);
  if (ret != METADATA_EXTRACTOR_ERROR_NONE) {
    LOGGER(ERROR) << "Media extractor error " << ret;
    return ret;
  }

  int nSyncTextNum = 0;
  if (strSyncTextNum) {
    nSyncTextNum = atoi(strSyncTextNum);
    free(strSyncTextNum);
    strSyncTextNum = NULL;
  }
  if (nSyncTextNum > 0) {
    result["type"] = picojson::value(std::string("SYNCHRONIZED"));
    for (int i = 0; i < nSyncTextNum; i++) {
      unsigned long time_info = 0;
      char* lyrics = NULL;
      ret = metadata_extractor_get_synclyrics(extractor, i, &time_info, &lyrics);
      if (ret == METADATA_EXTRACTOR_ERROR_NONE) {
        timestamps.push_back(picojson::value(static_cast<double>(time_info)));
        texts.push_back(picojson::value(std::string(lyrics)));
        free(lyrics);
      }
    }
    result["texts"] = picojson::value(texts);
    result["timestamps"] = picojson::value(timestamps);
    ret = METADATA_EXTRACTOR_ERROR_NONE;
  } else {
    char* unSyncText = nullptr;
    ret = metadata_extractor_get_metadata(extractor, METADATA_UNSYNCLYRICS,
                                          &unSyncText);
    if (ret == METADATA_EXTRACTOR_ERROR_NONE) {
      result["type"] = picojson::value(std::string("UNSYNCHRONIZED"));
      if (nullptr == unSyncText) {
        LOGGER(ERROR) << "Unsynchronized lyrics text is NULL";
      }
      texts.push_back(picojson::value(unSyncText ? unSyncText : ""));
      result["texts"] = picojson::value(texts);
      free(unSyncText);
    }
  }

  return ret;
}

media_playlist_h getPlaylistHandle(int id)
{
  LoggerD("Entered");
  media_playlist_h playlist_handle = nullptr;
  int ret_code = media_playlist_get_playlist_from_db(id, &playlist_handle);
  if(MEDIA_CONTENT_ERROR_NONE != ret_code ||
      playlist_handle == nullptr) {
    LoggerE("could not get playlist handle for id: %d", id);
    return nullptr;
  }

  return playlist_handle;
}

void destroyMediaPlaylistHandle(media_playlist_h& playlist_handle)
{
  LoggerD("Entered");
  if(playlist_handle) {
    int ret_code = media_playlist_destroy(playlist_handle);
    playlist_handle = nullptr;

    if(MEDIA_CONTENT_ERROR_NONE != ret_code) {
      LoggerE("media_playlist_destroy failed");
    }
  }
}

int ContentManager::getPlaylistName(int id, std::string* result) {
  LoggerD("Entered");
  media_playlist_h playlist_handle = getPlaylistHandle(id);
  PlaylistUniquePtr playlist_ptr(playlist_handle, destroyMediaPlaylistHandle);

  char* tmp_playlist_name = nullptr;
  const int ret_code = media_playlist_get_name(playlist_handle, &tmp_playlist_name);

  if(MEDIA_CONTENT_ERROR_NONE != ret_code) {
    LoggerE("media_playlist_get_name failed");
    return TIZEN_ERROR_UNKNOWN;
  }

  std::string playlist_name;
  if(tmp_playlist_name) {
    playlist_name = tmp_playlist_name;
    free(tmp_playlist_name);
    tmp_playlist_name = nullptr;
  }

  *result = playlist_name;
  return MEDIA_CONTENT_ERROR_NONE;
}

int updatePlaylistInDB(media_playlist_h playlist_handle)
{
  LoggerD("Entered");
  int ret_code = media_playlist_update_to_db(playlist_handle);
  if(MEDIA_CONTENT_ERROR_NONE != ret_code) {
    LoggerE("media_playlist_update_to_db failed");
    return ret_code;
  }
  return MEDIA_CONTENT_ERROR_NONE;
}

int ContentManager::setPlaylistName(int id, const std::string& name)
{
  LoggerD("Entered");
  if(name.empty()) {
    LoggerE("Cannot set empty playlist name!");
    return MEDIA_CONTENT_ERROR_INVALID_PARAMETER;
  }

  media_playlist_h playlist_handle = getPlaylistHandle(id);
  PlaylistUniquePtr playlist_ptr(playlist_handle, destroyMediaPlaylistHandle);

  const int ret_code = media_playlist_set_name(playlist_handle, name.c_str());
  if(MEDIA_CONTENT_ERROR_NONE != ret_code) {
    LoggerE("media_playlist_set_name failed");
    //Setting name that is used by other playlist does not return bad error code here.
    //MEDIA_CONTENT_ERROR_INVALID_OPERATION is being returned in updatePlaylistInDB
    return TIZEN_ERROR_UNKNOWN;
  }

  int ret = updatePlaylistInDB(playlist_handle);
  if (MEDIA_CONTENT_ERROR_NONE != ret) {
    LoggerE("Error while updating playlist: %d", ret);
    if (MEDIA_CONTENT_ERROR_DB_FAILED == ret) {
      //We could fetch list of playlists and check if other playlist is using this
      //name, but that seems to be to much work in synchronous method
      LoggerE("Playlist name: %s is probably already used", name.c_str());
      return MEDIA_CONTENT_ERROR_INVALID_PARAMETER;
    }
    return ret;
  }
  return MEDIA_CONTENT_ERROR_NONE;
}

int ContentManager::getThumbnailUri(int id, std::string* result)
{
  LoggerD("Entered");
  media_playlist_h playlist_handle = getPlaylistHandle(id);
  PlaylistUniquePtr playlist_ptr(playlist_handle, destroyMediaPlaylistHandle);

  char* tmp_playlist_thb_path = nullptr;
  const int ret_code = media_playlist_get_thumbnail_path(playlist_handle, &tmp_playlist_thb_path);

  if(MEDIA_CONTENT_ERROR_NONE != ret_code) {
    LoggerE("media_playlist_get_name failed");
    return TIZEN_ERROR_UNKNOWN;
  }

  std::string playlist_thb_path;
  if(tmp_playlist_thb_path) {
    playlist_thb_path = tmp_playlist_thb_path;
    free(tmp_playlist_thb_path);
    tmp_playlist_thb_path = nullptr;
  }

  if (playlist_thb_path != " ") {
    playlist_thb_path = uri_prefix + playlist_thb_path;
  }

  *result = playlist_thb_path;
  return MEDIA_CONTENT_ERROR_NONE;
}

int ContentManager::setThumbnailUri(int id, const std::string& thb_uri)
{
  LoggerD("Entered");

  //Allow setting empty URI, unfortunately Core API does not allow to set empty
  //path so we need to set one empty space. This is probably issue of Core API.
  if(!thb_uri.empty() && " " != thb_uri) {
    if(thb_uri.find(uri_absolute_prefix) != 0) {
      LoggerE("thumbnail URI is not valid: [%s]", thb_uri.c_str());
      return MEDIA_CONTENT_ERROR_INVALID_PARAMETER;
    }
  }

  media_playlist_h playlist_handle = getPlaylistHandle(id);
  PlaylistUniquePtr playlist_ptr(playlist_handle, destroyMediaPlaylistHandle);

  std::string real_path = VirtualFs::GetInstance().GetRealPath(thb_uri);
  const int ret_code = media_playlist_set_thumbnail_path(playlist_handle,
                                                         real_path.c_str());
  if(MEDIA_CONTENT_ERROR_NONE != ret_code) {
    LoggerE("media_playlist_set_thumbnail_path failed");
    return TIZEN_ERROR_UNKNOWN;
  }

  int ret = updatePlaylistInDB(playlist_handle);
  return ret;
}

int ContentManager::getNumberOfTracks(int id, int* result) {
  LoggerD("Entered");

  int count = 0;
  const int ret_code = media_playlist_get_media_count_from_db(id, nullptr,
                                                              &count);

  if (MEDIA_CONTENT_ERROR_NONE != ret_code) {
    LoggerE("media_playlist_get_media_count_from_db failed");
    return TIZEN_ERROR_UNKNOWN;
  }

  *result = count;
  return MEDIA_CONTENT_ERROR_NONE;
}

PlatformResult ContentManager::convertError(int err) {
  switch (err) {
    case MEDIA_CONTENT_ERROR_INVALID_PARAMETER :
      return PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid parameter.");
    case MEDIA_CONTENT_ERROR_OUT_OF_MEMORY :
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Out of memory.");
    case MEDIA_CONTENT_ERROR_INVALID_OPERATION :
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Invalid Operation.");
    case MEDIA_CONTENT_FILE_NO_SPACE_ON_DEVICE :
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "No space left on device.");
    case MEDIA_CONTENT_ERROR_PERMISSION_DENIED :
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Permission denied.");
    case MEDIA_CONTENT_ERROR_DB_FAILED :
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "DB operation failed.");
    case MEDIA_CONTENT_ERROR_DB_BUSY :
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "DB operation BUSY.");
    case MEDIA_CONTENT_ERROR_NETWORK :
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Network Fail.");
    case MEDIA_CONTENT_ERROR_UNSUPPORTED_CONTENT :
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Unsupported Content.");
    default:
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown error.");
  }
}

} // namespace content
} // namespace extension
