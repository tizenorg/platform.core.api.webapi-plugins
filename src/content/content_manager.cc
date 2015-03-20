// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/content_manager.h"

#include <algorithm>
#include <cstring>
#include <dlog.h>
#include <map>
#include <metadata_extractor.h>
#include <sstream>
#include <string>
#include <unistd.h>

#include "common/converter.h"
#include "common/logger.h"
#include "common/platform_exception.h"
#include "common/platform_result.h"
#include "common/scope_exit.h"
#include "content/content_filter.h"

using namespace std;
using namespace common;

namespace extension {
namespace content {

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

static int get_utc_offset()
{
  time_t zero = 24*60*60L;
  struct tm * timeptr;
  int gmtime_hours;

  /* get the local time for Jan 2, 1900 00:00 UTC */
  timeptr = localtime( &zero );
  gmtime_hours = timeptr->tm_hour;

  if( timeptr->tm_mday < 2 )
    gmtime_hours -= 24;

  return gmtime_hours;
}

static bool isContentUri(const std::string str) {
  std::string schema("file://");
  std::size_t found = str.find(schema);

  if (found == std::string::npos || found != 0) {
      return false;
  }

  return true;
}

static std::string ltrim(const std::string s) {
  std::string str = s;
  std::string::iterator i;
  for (i = str.begin(); i != str.end(); i++) {
      if (!isspace(*i)) {
          break;
      }
  }
  if (i == str.end()) {
      str.clear();
  } else {
      str.erase(str.begin(), i);
  }
  return str;
}


static std::string convertUriToPath(const string str) {
  string result;
  std::string schema ("file://");
  std::string _str = ltrim(str);

  std::string _schema = _str.substr(0,schema.size());

  if(_schema == schema)
  {
      result = _str.substr(schema.size());
  }
  else
  {
      result = _str;
  }
  return result;
}

static std::string convertPathToUri(const string str) {
  string result;
  std::string schema ("file://");
  std::string _str = ltrim(str);

  std::string _schema = _str.substr(0,schema.size());

  if(_schema == schema)
  {
      result = _str;
  }
  else
 {
      result = schema + _str;
  }
  return result;
}


void contentToJson(media_info_h info, picojson::object& o) {
  int ret;
  int tmpInt = 0;
  bool tmpBool = false;
  char* tmpStr = NULL;
  time_t tmpDate;
  double tmpDouble;
  long long unsigned int tmpLong;
  media_content_type_e type;
  ret == media_info_get_media_type(info, &type);
  if(ret != MEDIA_CONTENT_ERROR_NONE) {
    if ( type == MEDIA_CONTENT_TYPE_IMAGE ) {
      o["type"] = picojson::value(std::string("IMAGE"));
      image_meta_h img;
      if(MEDIA_CONTENT_ERROR_NONE == media_info_get_image(info, &img)) {
        if(MEDIA_CONTENT_ERROR_NONE == image_meta_get_date_taken (img, &tmpStr)) {
          if ( tmpStr ) {
            struct tm *result = (struct tm *)calloc(1, sizeof(struct tm));
            if(strptime(tmpStr, "%Y:%m:%d %H:%M:%S", result) == NULL) {
              LoggerE( "Couldn't convert supplied date.");
            }
            else {
              time_t t = mktime( result );// + get_utc_offset() * 3600;
              std::stringstream str_date;
              str_date << t;
              o["releaseDate"] = picojson::value(str_date.str());
              free(tmpStr);
              free(result);
              tmpStr = NULL;
            }
          }
        }
        if(MEDIA_CONTENT_ERROR_NONE == image_meta_get_width(img, &tmpInt) ) {
          o["width"] = picojson::value(static_cast<double>(tmpInt));
        }
        if(MEDIA_CONTENT_ERROR_NONE == image_meta_get_height(img, &tmpInt) ) {
          o["height"] = picojson::value(static_cast<double>(tmpInt));          
        }
        picojson::object geo;
        std::string str_latitude;
        if (MEDIA_CONTENT_ERROR_NONE == media_info_get_latitude(info, &tmpDouble) ) {
          geo["latitude"] = picojson::value(tmpDouble);
        }
        std::string str_longitude;
        if(MEDIA_CONTENT_ERROR_NONE == media_info_get_longitude(info, &tmpDouble) ) {
          geo["longitude"] = picojson::value(tmpDouble);
        }
        o["geolocation"] = picojson::value(geo);
        std::string ori;
        media_content_orientation_e orientation;
        if(MEDIA_CONTENT_ERROR_NONE == image_meta_get_orientation(img, &orientation) ) {
          switch (orientation) {
          case 0:
          case 1:
              ori = "NORMAL";
              break;
          case 2:
              ori = "FLIP_HORIZONTAL";
              break;
          case 3:
              ori = "ROTATE_180";
              break;
          case 4:
              ori = "FLIP_VERTICAL";
              break;
          case 5:
              ori = "TRANSPOSE";
              break;
          case 6:
              ori = "ROTATE_90";
              break;
          case 7:
              ori = "TRANSVERSE";
              break;
          case 8:
              ori = "ROTATE_270";
              break;
          }
          o["orientation"] = picojson::value(ori);
        }
        
      }

    }
    else if( type == MEDIA_CONTENT_TYPE_VIDEO ) {
      o["type"] = picojson::value(std::string("VIDEO"));
      video_meta_h video;
      if(MEDIA_CONTENT_ERROR_NONE == media_info_get_video(info, &video)) {
        if(MEDIA_CONTENT_ERROR_NONE == video_meta_get_width(video, &tmpInt)) {
          o["width"] = picojson::value(static_cast<double>(tmpInt));
        }

        if(MEDIA_CONTENT_ERROR_NONE == video_meta_get_height(video, &tmpInt) ) {
          o["height"] = picojson::value(static_cast<double>(tmpInt));
        }
        if (MEDIA_CONTENT_ERROR_NONE == video_meta_get_artist(video, &tmpStr) ) {
          picojson::array artists;
          if (tmpStr) {
            artists.push_back(picojson::value(std::string(tmpStr)));
          }
          o["artists"] = picojson::value(artists);
        }
        if (MEDIA_CONTENT_ERROR_NONE == video_meta_get_album(video, &tmpStr)) {
          if (tmpStr) {
            o["album"] = picojson::value(tmpStr);
          }
        }
        if (MEDIA_CONTENT_ERROR_NONE == video_meta_get_duration(video, &tmpInt) ) {
          o["duration"] = picojson::value(static_cast<double>(tmpInt));
        }        
      }
      picojson::object geo;
      if (MEDIA_CONTENT_ERROR_NONE == media_info_get_latitude(info, &tmpDouble) ) {
        geo["latitude"] = picojson::value(tmpDouble);
      }
      if (MEDIA_CONTENT_ERROR_NONE == media_info_get_longitude(info, &tmpDouble) ) {
        geo["longitude"] = picojson::value(tmpDouble);
      }      
      o["geolocation"] = picojson::value(geo);
    }
    else if( type == MEDIA_CONTENT_TYPE_SOUND || type == MEDIA_CONTENT_TYPE_MUSIC ) {
      o["type"] = picojson::value(std::string("AUDIO"));
      audio_meta_h audio;
      if(MEDIA_CONTENT_ERROR_NONE == media_info_get_audio(info, &audio)) {
        if (MEDIA_CONTENT_ERROR_NONE == audio_meta_get_recorded_date(audio, &tmpStr) ) {
          if (tmpStr) {
            struct tm *result = (struct tm *)calloc(1, sizeof(struct tm));
            
            if (strptime(tmpStr, "%Y:%m:%d %H:%M:%S", result) == NULL) {
                LoggerD( "Couldn't convert supplied date.");
            }
            time_t t = mktime( result ) + get_utc_offset() * 3600;
            std::stringstream str_date;
            str_date << t;
            o["releaseDate"] = picojson::value(str_date.str());
            free(tmpStr);
            free(result);
            tmpStr = NULL;
          }
        }
        if (MEDIA_CONTENT_ERROR_NONE == audio_meta_get_album(audio, &tmpStr)) {
          if(tmpStr) {
            o["album"] = picojson::value(std::string(tmpStr));
            free(tmpStr);
            tmpStr = NULL;
          }
        }
        if(MEDIA_CONTENT_ERROR_NONE == audio_meta_get_artist(audio, &tmpStr)) {
          if(tmpStr) {
            picojson::array artists;
            if (tmpStr) {
              artists.push_back(picojson::value(std::string(tmpStr)));
            }
            o["artists"] = picojson::value(artists);
            free(tmpStr);
            tmpStr = NULL;
          }
        }
        if(MEDIA_CONTENT_ERROR_NONE == audio_meta_get_genre(audio, &tmpStr)) {
          if(tmpStr) {
            picojson::array genres;
            if (tmpStr) {
              genres.push_back(picojson::value(std::string(tmpStr)));
            }
            o["genres"] = picojson::value(genres);
            free(tmpStr);
            tmpStr = NULL;
          }
        }
        if(MEDIA_CONTENT_ERROR_NONE == audio_meta_get_composer(audio, &tmpStr)) {
          if(tmpStr) {
            picojson::array composers;
            if (tmpStr) {
              composers.push_back(picojson::value(std::string(tmpStr)));
            }
            o["composers"] = picojson::value(composers);
            free(tmpStr);
            tmpStr = NULL;
          }
        }
        if (MEDIA_CONTENT_ERROR_NONE == audio_meta_get_copyright(audio, &tmpStr)) {
          if(tmpStr) {
            o["copyright"] = picojson::value(std::string(tmpStr));
            free(tmpStr);
            tmpStr = NULL;
          }
        }
        if (MEDIA_CONTENT_ERROR_NONE == audio_meta_get_bit_rate(audio, &tmpInt)){
          o["bitrate"] = picojson::value(static_cast<double>(tmpInt));
        }
        if (MEDIA_CONTENT_ERROR_NONE == audio_meta_get_track_num(audio, &tmpStr)){
          if(tmpStr) {
            o["trackNumber"] = picojson::value(static_cast<double>(std::atoi(tmpStr)));
            free(tmpStr);
            tmpStr = NULL;
          }
          else {
            o["trackNumber"] = picojson::value();
          }
        }
        if (MEDIA_CONTENT_ERROR_NONE == audio_meta_get_duration(audio, &tmpInt) ) {
          o["duration"] = picojson::value(static_cast<double>(tmpInt));          
        }
      }
    }
    else {
      o["type"] = picojson::value(std::string("OTHER"));
    }
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
    std::stringstream str_date;
    str_date << tmpDate;
    o["modifiedDate"] = picojson::value(str_date.str());
  }  
}

static int setContent(media_info_h media, picojson::value content) {
  
  int ret;
  std::string name = content.get("name").to_str();
  std::string description = content.get("description").to_str();
  std::string rating = content.get("rating").to_str();
  std::string is_fav = content.get("isFavorite").to_str();
  if (media != NULL) {
    media_content_type_e type;
    ret = media_info_get_media_type(media, &type);
    if (ret != MEDIA_CONTENT_ERROR_NONE ) {
      return ret;
    }
    ret = media_info_set_display_name(media, name.c_str());
    if ( ret != MEDIA_CONTENT_ERROR_NONE) {
      LoggerD("Updating name is failed.");
    }
    ret = media_info_set_description(media, description.c_str());
    if ( ret != MEDIA_CONTENT_ERROR_NONE) {
      LoggerD("Updating description is failed.");
    }
    ret = media_info_set_rating(media, std::stoi(rating));
    if ( ret != MEDIA_CONTENT_ERROR_NONE) {
      LoggerD("Updating rating is failed.");
    }

    if (is_fav == "true") {
      ret = media_info_set_favorite(media, true);
    }
    else if (is_fav == "false") {
      ret = media_info_set_favorite(media, false);
    }

    if ( ret != MEDIA_CONTENT_ERROR_NONE) {
      LoggerD("Updating favorite is failed.");
    }
    if (type == MEDIA_CONTENT_TYPE_IMAGE) {
      std::string orientation = content.get("orientation").to_str();
      auto orientationToSet = orientationMap.find(orientation);

      if (orientationToSet != orientationMap.end()) {
        image_meta_h img;
        if(MEDIA_CONTENT_ERROR_NONE == media_info_get_image(media, &img) &&
           MEDIA_CONTENT_ERROR_NONE == image_meta_set_orientation(img, orientationToSet->second) &&
           MEDIA_CONTENT_ERROR_NONE == image_meta_update_to_db(img)) {
          LoggerD("orientation update was successful");
        } else {
          LoggerD("orientation update failed");
        }
      }
    }
    if (type == MEDIA_CONTENT_TYPE_IMAGE || type == MEDIA_CONTENT_TYPE_VIDEO) {
      picojson::value geo = content.get("geolocation");
      double latitude = atof(geo.get("latitude").to_str().c_str());
      double longitude = atof(geo.get("longitude").to_str().c_str());
      ret = media_info_set_latitude(media, latitude);
      if ( ret != MEDIA_CONTENT_ERROR_NONE) {
        LoggerD("Updating geolocation is failed.");
      }
      ret = media_info_set_longitude(media, longitude);
      if ( ret != MEDIA_CONTENT_ERROR_NONE) {
        LoggerD("Updating geolocation is failed.");
      }
    }
    ret = MEDIA_CONTENT_ERROR_NONE;
  }
  else {
    ret = MEDIA_CONTENT_ERROR_DB_FAILED;
  }
  return ret;
}

static bool media_foreach_directory_cb(media_folder_h folder, void *user_data) {
  std::vector<media_folder_h> *dir = static_cast<std::vector<media_folder_h>*>(user_data);
  media_folder_h nfolder = NULL;
  media_folder_clone (&nfolder, folder);
  dir->push_back(nfolder);
}

static bool media_foreach_content_cb(media_info_h media, void *user_data) {
  picojson::value::array *contents = static_cast<picojson::value::array*>(user_data);
  picojson::value::object o;
  contentToJson(media, o);
  contents->push_back(picojson::value(o));  
  return true;
}

static bool playlist_foreach_cb(media_playlist_h playlist, void *user_data) {

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
        o["thumbnailURI"] = picojson::value(std::string(thumb_path));
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
    if( media_playlist_get_media_count_from_db(id, filter, &cnt) == MEDIA_CONTENT_ERROR_NONE) {
      o["numberOfTracks"] = picojson::value(static_cast<double>(cnt));
    }
    else {
      LoggerD("Invalid count for playlist.");
    }
    playlists->push_back(picojson::value(o));

  }
  return true;
}

static bool playlist_content_member_cb(int playlist_member_id, media_info_h media, void *user_data) {
  
  picojson::value::array *contents = static_cast<picojson::value::array*>(user_data);
  picojson::value::object o;
  char *name = NULL;
  
  media_info_get_display_name(media, &name);
  o["playlist_member_id"] = picojson::value(static_cast<double>(playlist_member_id));
  contentToJson(media, o);
  contents->push_back(picojson::value(o));
  return true;
}


ContentManager::ContentManager() {
  LoggerE("ContentManager called");
  if(media_content_connect() == MEDIA_CONTENT_ERROR_NONE) {
      m_dbConnected = true;
  }
  else
      m_dbConnected = false;
}

ContentManager::~ContentManager() {
  if(m_dbConnected) {
    if(media_content_disconnect() == MEDIA_CONTENT_ERROR_NONE) {
      m_dbConnected = false;
    }
  }
}

ContentManager* ContentManager::getInstance() {
  static ContentManager instance;
  return &instance;
}

bool ContentManager::isConnected() {
  return m_dbConnected;
}

void ContentManager::getDirectories(const std::shared_ptr<ReplyCallbackData>& user_data) {
  
  picojson::value::array pico_dirs;
  
  int ret = MEDIA_CONTENT_ERROR_NONE;
  filter_h filter = NULL;
  std::vector<media_folder_h> dirs;
  
  ret = media_folder_foreach_folder_from_db(filter, media_foreach_directory_cb, &dirs);

  if (ret == MEDIA_CONTENT_ERROR_NONE) {
    for(std::vector<media_folder_h>::iterator it = dirs.begin(); it != dirs.end(); ++it) {
      char *name = NULL;
      char *id = NULL;
      char *path = NULL;
      time_t date;
      media_content_storage_e storageType;
      picojson::value::object o;

      media_folder_get_folder_id(*it, &id);
      media_folder_get_name(*it, &name);
      media_folder_get_path(*it, &path);
      media_folder_get_modified_time(*it, &date);
      media_folder_get_storage_type(*it, &storageType);

      o["id"] = picojson::value(std::string(id));
      o["directoryURI"] = picojson::value(std::string(path));
      o["title"] = picojson::value(std::string(name));

      if (storageType == MEDIA_CONTENT_STORAGE_INTERNAL) {
        o["storageType"] = picojson::value(std::string("INTERNAL"));
      } else if (storageType == MEDIA_CONTENT_STORAGE_EXTERNAL) {
        o["storageType"] = picojson::value(std::string("EXTERNAL"));
      }

      char tmp[128];
      ctime_r(&date, tmp);    
      o["modifiedDate"] = picojson::value(std::string(tmp));
      pico_dirs.push_back(picojson::value(o));

      free(name);
      free(id);
      free(path);
    }
    user_data->isSuccess = true;
    user_data->result = picojson::value(pico_dirs);
  }
  else {
    UnknownException err("Getting the directories is failed.");
    user_data->isSuccess = false;
    user_data->result = err.ToJSON();
  }

}

void ContentManager::find(const std::shared_ptr<ReplyCallbackData>& user_data) {
  int ret;
  double count, offset;
  std::string dirId, attributeName, matchFlag, matchValue;
  std::string sortModeName, sortModeOrder;
  int error_code = 0;
  media_content_order_e order;

  picojson::value::array arrayContent;
  filter_h filter = nullptr;
  media_filter_create(&filter);
  SCOPE_EXIT {
    if (filter) {
      media_filter_destroy(filter);
    }
  };
  if (user_data->args.contains("filter")) {
    ContentFilter filterMechanism;
    std::string query;
    picojson::object argsObject = JsonCast<picojson::object>(user_data->args);
    if (filterMechanism.buildQuery(
            FromJson<picojson::object>(argsObject, "filter"), &query)) {
      ret = media_filter_set_condition(filter, query.c_str(),
                                       MEDIA_CONTENT_COLLATE_DEFAULT);
      if (MEDIA_CONTENT_ERROR_NONE != ret) {
      }
    }
  }
  if(user_data->args.contains("count")) {
    count = user_data->args.get("count").get<double>();
  } else {
    count = 100; //TODO rethink proper default count setting
  }
  if(user_data->args.contains("offset")) {
    offset = user_data->args.get("offset").get<double>();
  }
  else {
    offset = 0;
  }
  ret = media_filter_set_offset(filter, offset, count);
  if ( MEDIA_CONTENT_ERROR_NONE != ret) {
    LoggerD("A platform error occurs in media_filter_set_offset.");
  }
  if(user_data->args.contains("directoryId")) {
    dirId = user_data->args.get("directoryId").get<std::string>();
    ret = media_folder_foreach_media_from_db(dirId.c_str(), filter, media_foreach_content_cb, static_cast<void*>(&arrayContent));
  }
  else {
    ret = media_info_foreach_media_from_db(filter, media_foreach_content_cb, static_cast<void*>(&arrayContent));
  }

  if (ret == MEDIA_CONTENT_ERROR_NONE) {
    user_data->isSuccess = true;
    user_data->result = picojson::value(arrayContent);
  }
  else {
    UnknownException err("The iteration is failed in platform.");
    user_data->isSuccess = false;
    user_data->result = err.ToJSON();
  }
}

int ContentManager::scanFile(std::string& uri) {
  return media_content_scan_file(uri.c_str());
}

int ContentManager::setChangeListener(media_content_db_update_cb callback, void *user_data) {

  int ret = media_content_set_db_updated_cb(callback, user_data);
  if(ret != MEDIA_CONTENT_ERROR_NONE) {
    throw UnknownException("registering the listener is failed.");
  }

}

void ContentManager::unSetChangeListener() {
  int ret = media_content_unset_db_updated_cb();
  if(ret != MEDIA_CONTENT_ERROR_NONE) {
    throw UnknownException("unSetChangeListener is failed.");
  }
}

void ContentManager::createPlaylist(std::string name, 
  const std::shared_ptr<ReplyCallbackData>& user_data) {
  
  media_playlist_h	playlist = NULL;
  
  int ret = media_playlist_insert_to_db(name.c_str(),&playlist);
  if(ret != MEDIA_CONTENT_ERROR_NONE) { 
    UnknownException err("creation of playlist is failed.");
    user_data->isSuccess = false;
    user_data->result = err.ToJSON();    
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
      UnknownException err("loading of playlist is failed.");
      user_data->isSuccess = false;
      user_data->result = err.ToJSON();
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
    if( media_playlist_get_media_count_from_db(id, filter, &cnt) == MEDIA_CONTENT_ERROR_NONE) {
      o["numberOfTracks"] = picojson::value(static_cast<double>(cnt));
    }
    else {
      LoggerD("Invalid count for playlist.");
    }
  }
  user_data->isSuccess = true;
  user_data->result = picojson::value(o);

}

void ContentManager::getPlaylists(const std::shared_ptr<ReplyCallbackData>& user_data) {

  int ret;
  filter_h 	filter;
  media_filter_create(&filter);
  picojson::value::array playlists;
  
  ret = media_playlist_foreach_playlist_from_db(filter, playlist_foreach_cb, static_cast<void*>(&playlists));

  if(ret != MEDIA_CONTENT_ERROR_NONE) {
    UnknownException err("Getting playlist is failed.");
    user_data->isSuccess = false;
    user_data->result = err.ToJSON();      
  }
  user_data->isSuccess = true;
  user_data->result = picojson::value(playlists);
}

void ContentManager::removePlaylist(std::string playlistId, 
  const std::shared_ptr<ReplyCallbackData>& user_data) {
  
  int id = std::atoi(playlistId.c_str());
  if(id == 0) {
    UnknownException err("PlaylistId is wrong.");
    user_data->isSuccess = false;
    user_data->result = err.ToJSON();
    return;
  }

  int ret = media_playlist_delete_from_db(id);
  if(ret != MEDIA_CONTENT_ERROR_NONE) {
    UnknownException err("Removal of playlist is failed.");
    user_data->isSuccess = false;
    user_data->result = err.ToJSON();      
  }
}

int ContentManager::update(picojson::value args) {
  int ret;
  picojson::value content = args.get("content");
  std::string id = content.get("id").to_str();

  media_info_h media = NULL;
  ret = media_info_get_media_from_db (id.c_str(), &media);
  if (media != NULL) {
    setContent(media, content);
    ret = media_info_update_to_db(media);
  }
  else {
    LoggerD("There is no content(%s)",id.c_str());
    ret = MEDIA_CONTENT_ERROR_NONE;
  }
  return ret;
}


int ContentManager::updateBatch(picojson::value args) {
  int ret;
  std::vector<picojson::value> contents = args.get("contents").get<picojson::array>();
  for (picojson::value::array::iterator it = contents.begin(); it != contents.end(); it++) {
    picojson::value content = *it;
    std::string id = content.get("id").to_str();
    media_info_h media = NULL;
    ret = media_info_get_media_from_db (id.c_str(), &media);
    dlog_print(DLOG_INFO, "DYKIM", "ContentManager::update id:%s",id.c_str());          
    if (media != NULL && ret == MEDIA_CONTENT_ERROR_NONE) {
      setContent(media, content);
      ret = media_info_update_to_db(media);
    }
    else {
      return ret;
    }
  }
  return ret;
}



int ContentManager::playlistAdd(std::string playlist_id, std::string content_id) {
  int ret = MEDIA_CONTENT_ERROR_NONE;

  media_playlist_h playlist = NULL;
  ret = media_playlist_get_playlist_from_db(std::stoi(playlist_id), &playlist);

  if (playlist != NULL && ret == MEDIA_CONTENT_ERROR_NONE) {
    ret = media_playlist_add_media(playlist, content_id.c_str());
    if (ret != MEDIA_CONTENT_ERROR_NONE) {
      LoggerD("The content(id:%s) can't add to playlist",content_id.c_str());      
    }

    ret = media_playlist_update_to_db(playlist);
    if (ret != MEDIA_CONTENT_ERROR_NONE) {
      LoggerD("The content(id:%s) can't add to playlist",content_id.c_str());
    }
  }
  else {
    LoggerD("Playlist(id:%s) is not exist",playlist_id.c_str());
  }
  
  media_playlist_destroy(playlist);
  return ret;
}

int ContentManager::playlistRemove(std::string playlist_id, int member_id) {
  int ret = MEDIA_CONTENT_ERROR_NONE;

  media_playlist_h playlist = NULL;
  ret = media_playlist_get_playlist_from_db(std::stoi(playlist_id), &playlist);
  if (playlist != NULL && ret == MEDIA_CONTENT_ERROR_NONE) {
    ret = media_playlist_remove_media(playlist, member_id);
    if (ret != MEDIA_CONTENT_ERROR_NONE) {
      LoggerD("The content can't remove to playlist");      
    }

    ret = media_playlist_update_to_db(playlist);
    if (ret != MEDIA_CONTENT_ERROR_NONE) {
      LoggerD("The content can't remove to playlist");
    }
  }
  else {
    LoggerD("Playlist(id:%s) is not exist",playlist_id.c_str());
  }
  media_playlist_destroy(playlist);
  
  return ret;
}


void ContentManager::playlistAddbatch(const std::shared_ptr<ReplyCallbackData>& user_data) {
  
  int ret = MEDIA_CONTENT_ERROR_NONE;
  std::string playlist_id = user_data->args.get("playlist_id").get<std::string>();

  media_playlist_h playlist = NULL;  
  ret = media_playlist_get_playlist_from_db(std::stoi(playlist_id), &playlist);

  if(ret != MEDIA_CONTENT_ERROR_NONE && playlist == NULL) { 
    UnknownException err("Getting playlist is failed.");
    user_data->isSuccess = false;
    user_data->result = err.ToJSON();
    return;
  }
  
  std::vector<picojson::value> contents = user_data->args.get("contents").get<picojson::array>();
  for (picojson::value::array::iterator it = contents.begin(); it != contents.end(); it++) {
    picojson::value content = *it;
    std::string id = content.get("id").to_str();
    ret = media_playlist_add_media(playlist, id.c_str());
    if (ret != MEDIA_CONTENT_ERROR_NONE) {
      LoggerD("Adding Content(id:%s) is failed.", id.c_str());
    }
  }

  ret = media_playlist_update_to_db(playlist);
  if(ret != MEDIA_CONTENT_ERROR_NONE ) {
    UnknownException err("Adding playlist is failed.");
    user_data->isSuccess = false;
    user_data->result = err.ToJSON();
  }
  media_playlist_destroy(playlist);
}

void ContentManager::playlistGet(const std::shared_ptr<ReplyCallbackData>& user_data) {
  
  int ret = MEDIA_CONTENT_ERROR_NONE;
  media_playlist_h playlist = NULL;  

  std::string playlist_id = user_data->args.get("playlist_id").get<std::string>();
  ret = media_playlist_get_playlist_from_db(std::stoi(playlist_id), &playlist);
  if(ret != MEDIA_CONTENT_ERROR_NONE && playlist == NULL) { 
    UnknownException err("Getting playlist is failed.");
    user_data->isSuccess = false;
    user_data->result = err.ToJSON();
    return;
  }

  filter_h filter = NULL;
  ret = media_filter_create(&filter);
  if (ret != MEDIA_CONTENT_ERROR_NONE) {
    UnknownException err("Creating a filter is failed.");
    user_data->isSuccess = false;
    user_data->result = err.ToJSON();
    return;    
  }

  int count = user_data->args.get("count").get<double>();
  int offset = user_data->args.get("offset").get<double>();
  ret = media_filter_set_offset(filter, offset, count);
  if (ret != MEDIA_CONTENT_ERROR_NONE) {
    LoggerD("Setting a offset/count is failed.");
  }
  picojson::value::array arrayContent;
  ret = media_playlist_foreach_media_from_db(std::stoi(playlist_id),
    filter, playlist_content_member_cb, static_cast<void*>(&arrayContent));

  media_filter_destroy(filter);
  if (ret == MEDIA_CONTENT_ERROR_NONE) {
    user_data->isSuccess = true;
    user_data->result = picojson::value(arrayContent);
  }
  else {
    UnknownException err("Creating a filter is failed.");
    user_data->isSuccess = false;
    user_data->result = err.ToJSON();
  }
}

void ContentManager::playlistRemovebatch(const std::shared_ptr<ReplyCallbackData>& user_data) {
 
  int ret = MEDIA_CONTENT_ERROR_NONE;
  media_playlist_h playlist = NULL;  

  std::string playlist_id = user_data->args.get("playlist_id").get<std::string>();
  ret = media_playlist_get_playlist_from_db(std::stoi(playlist_id), &playlist);
  if(ret != MEDIA_CONTENT_ERROR_NONE && playlist == NULL) { 
    UnknownException err("Getting playlist is failed.");
    user_data->isSuccess = false;
    user_data->result = err.ToJSON();
    return;
  }

  std::vector<picojson::value> members = user_data->args.get("members").get<picojson::array>();
  for( int i = 0; i < members.size(); i++ ) {
    int member_id = static_cast<int>(members.at(i).get<double>());
    ret = media_playlist_remove_media(playlist, member_id);

    if (ret != MEDIA_CONTENT_ERROR_NONE) {
      LoggerD("Removing a content is failed.");
    }
  }

  ret = media_playlist_update_to_db(playlist);
  if (ret != MEDIA_CONTENT_ERROR_NONE) {
    UnknownException err("Removing the contents is failed.");
    user_data->isSuccess = false;
    user_data->result = err.ToJSON();    
  }
  else {
    user_data->isSuccess = true;
  }
}

void ContentManager::playlistSetOrder(const std::shared_ptr<ReplyCallbackData>& user_data) {

  int ret = MEDIA_CONTENT_ERROR_NONE;
  media_playlist_h playlist = NULL;

  std::string playlist_id = user_data->args.get("playlist_id").get<std::string>();
  ret = media_playlist_get_playlist_from_db(std::stoi(playlist_id), &playlist);
  if(ret != MEDIA_CONTENT_ERROR_NONE && playlist == NULL) {
    UnknownException err("Getting playlist is failed.");
    user_data->isSuccess = false;
    user_data->result = err.ToJSON();
    return;
  }

  int cnt;
  std::vector<picojson::value> members = user_data->args.get("members").get<picojson::array>();  
  
  ret = media_playlist_get_media_count_from_db(std::stoi(playlist_id), NULL, &cnt);

  if ( cnt != members.size() ) {
    InvalidValuesException err("The items array does not contain all items from the playlist.");
    user_data->isSuccess = false;
    user_data->result = err.ToJSON();
    return;    
  }
  
  for( int i = 0; i < members.size(); i++ ) {
    int member_id = static_cast<int>(members.at(i).get<double>());
    ret = media_playlist_set_play_order(playlist, member_id, i);
    if (ret != MEDIA_CONTENT_ERROR_NONE) {
      LoggerD("Removing a content is failed.");
    }
  }
  
  ret = media_playlist_update_to_db(playlist);
  if (ret != MEDIA_CONTENT_ERROR_NONE) {
    UnknownException err("Removing the contents is failed.");
    user_data->isSuccess = false;
    user_data->result = err.ToJSON();    
  }
  else {
    user_data->isSuccess = true;
  }
}

void ContentManager::playlistMove(const std::shared_ptr<ReplyCallbackData>& user_data) {
  int ret = MEDIA_CONTENT_ERROR_NONE;
  media_playlist_h playlist = NULL;
  std::string playlist_id = user_data->args.get("playlist_id").get<std::string>();
  ret = media_playlist_get_playlist_from_db(std::stoi(playlist_id), &playlist);
  if(ret != MEDIA_CONTENT_ERROR_NONE && playlist == NULL) {
    UnknownException err("Getting playlist is failed.");
    user_data->isSuccess = false;
    user_data->result = err.ToJSON();
    return;
  }
  int old_order;
  double member_id = user_data->args.get("member_id").get<double>();
  double delta = user_data->args.get("delta").get<double>();
  ret = media_playlist_get_play_order(playlist, static_cast<int>(member_id), &old_order);
  if (ret != MEDIA_CONTENT_ERROR_NONE) {
    UnknownException err("The content can't find form playlist.");
    user_data->isSuccess = false;
    user_data->result = err.ToJSON();
    return;
  }
  int new_order = static_cast<int>(old_order) + static_cast<int>(delta);
  ret = media_playlist_set_play_order(playlist, static_cast<int>(member_id), new_order);
  if (ret != MEDIA_CONTENT_ERROR_NONE) {
    UnknownException err("The content can't update play_order.");
    user_data->isSuccess = false;
    user_data->result = err.ToJSON();
    return;
  }
  ret = media_playlist_update_to_db(playlist);
  if (ret != MEDIA_CONTENT_ERROR_NONE) {
    UnknownException err("Updateing play_order is failed.");
    user_data->isSuccess = false;
    user_data->result = err.ToJSON();    
  }
  else {
    user_data->isSuccess = true;
  }  
}

int ContentManager::getLyrics(const picojson::value& args, picojson::object& result) {
  int ret = METADATA_EXTRACTOR_ERROR_NONE;
  std::string contentURI = convertUriToPath(args.get("contentURI").get<std::string>());
  metadata_extractor_h extractor;
  metadata_extractor_create(&extractor);
  if (!(contentURI.empty())) {
    ret = metadata_extractor_set_path(extractor, contentURI.c_str());
    if (ret != METADATA_EXTRACTOR_ERROR_NONE) {
      return -1;
    }
    picojson::array timestamps;
    picojson::array texts;
    char* strSyncTextNum=NULL;
    metadata_extractor_attr_e attr = METADATA_SYNCLYRICS_NUM;
    ret = metadata_extractor_get_metadata(extractor, attr, &strSyncTextNum);
    if (ret = METADATA_EXTRACTOR_ERROR_NONE && strSyncTextNum ) {
      int nSyncTextNum = atoi(strSyncTextNum);
      free(strSyncTextNum);
      strSyncTextNum = NULL;
      if (nSyncTextNum > 0) {
        result["type"] = picojson::value(std::string("SYNCHRONIZED"));
        for(int i=0; i < nSyncTextNum; i++) {
          unsigned long time_info = 0;
          char * lyrics = NULL;
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
      }
      else {
        char* unSyncText = NULL;
        attr = METADATA_UNSYNCLYRICS;
        ret = metadata_extractor_get_metadata(extractor, attr, &unSyncText);
        if (ret == METADATA_EXTRACTOR_ERROR_NONE) {
          result["type"] = picojson::value(std::string("UNSYNCHRONIZED"));
          texts.push_back(picojson::value(std::string(unSyncText)));
          result["texts"] = picojson::value(texts);
          free(unSyncText);
        }
      }
    }
  }
  return ret;
}


common::PlatformException ContentManager::convertError(int err) {
  switch (err) {
    case MEDIA_CONTENT_ERROR_INVALID_PARAMETER :
      return common::InvalidValuesException("Invalid parameter.");
    case MEDIA_CONTENT_ERROR_OUT_OF_MEMORY :
      return common::UnknownException("Out of memory.");
    case MEDIA_CONTENT_ERROR_INVALID_OPERATION :
      return common::UnknownException("Invalid Operation.");
    case MEDIA_CONTENT_FILE_NO_SPACE_ON_DEVICE :
      return common::UnknownException("No space left on device.");
    case MEDIA_CONTENT_ERROR_PERMISSION_DENIED :
      return common::UnknownException("Permission denied.");
    case MEDIA_CONTENT_ERROR_DB_FAILED :
      return common::UnknownException("DB operation failed.");
    case MEDIA_CONTENT_ERROR_DB_BUSY :
      return common::UnknownException("DB operation BUSY.");
    case MEDIA_CONTENT_ERROR_NETWORK :
      return common::UnknownException("Network Fail.");
    case MEDIA_CONTENT_ERROR_UNSUPPORTED_CONTENT :
      return common::UnknownException("Unsupported Content.");
    default:
      return common::UnknownException("Unknown error.");
  }
}

} // namespace content
} // namespace extension

