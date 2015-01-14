// Copyright (c) 2015 Samsung Electronics Co., Ltd. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "exif/exif_instance.h"

#include <libexif/exif-data.h>
#include <libexif/exif-entry.h>
#include <libexif/exif-utils.h>
#include <libexif/exif-loader.h>

#include <string>
#include <sstream>

#include "common/platform_exception.h"
#include "common/task-queue.h"
#include "common/logger.h"

#include "exif_util.h"
#include "jpeg_file.h"
#include "exif_information.h"

namespace extension {
namespace exif {

typedef picojson::value JsonValue;
typedef picojson::object JsonObject;
typedef picojson::array JsonArray;
typedef std::string JsonString;

namespace {
const char kGetExifInfoCmd[] = "Exif_getExifInfo";
const char kSaveExifInfoCmd[] = "Exif_saveExifInfo";
const char kGetThumbnailCmd[] = "Exif_getThumbnail";
}  // namespace

ExifInstance::ExifInstance() {
  using namespace std::placeholders;

#define REGISTER_ASYNC(c, x) \
  RegisterHandler(c, std::bind(&ExifInstance::x, this, _1, _2));
  REGISTER_ASYNC(kGetExifInfoCmd, getExifInfo);
  REGISTER_ASYNC(kSaveExifInfoCmd, saveExifInfo);
  REGISTER_ASYNC(kGetThumbnailCmd, getThumbnail);
#undef REGISTER_ASYNC
}

ExifInstance::~ExifInstance() { }

void ExifInstance::getExifInfo(const picojson::value& args,
                               picojson::object& out) {
  LoggerD("ExifInstance::getExifInfo() in c++ A");

  const std::string& uri = args.get("uri").get<std::string>();

  const double callback_id = args.get("callbackId").get<double>();
  auto get = [=](const std::shared_ptr<JsonValue>& response) -> void {
    try {
      // uri = file:///opt/usr/media/Images/exif.jpg
      // file_path = /opt/usr/media/Images/exif.jpg
      const std::string file_path = ExifUtil::convertUriToPath(uri);
      LoggerD("file_path = %s\n", file_path.c_str());

      LoggerD("ExifInformation::loadFromURI... %s", file_path.c_str());
      ExifInformationPtr exif_info = ExifInformation::loadFromURI(uri);
      if (!exif_info) {
        LoggerE("ExifInformation::loadFromURI failed for %s",
            file_path.c_str());
      }

      unsigned long width = exif_info->getWidth();
      LoggerD("width = %d", width);
      unsigned long height = exif_info->getHeight();
      LoggerD("height = %d", height);
      const std::string& device_maker = exif_info->getDeviceMaker();
      LoggerD("device_maker = %s", device_maker.c_str());
      const std::string& device_model = exif_info->getDeviceModel();
      LoggerD("device_model = %s", device_model.c_str());
      const time_t original_time = exif_info->getOriginalTime();
      LoggerD("original_time = %s", asctime(localtime(&original_time)));
      //...

      /* todo: all fields that need to be implemented:
      DOMString uri;
      unsigned long width;
      unsigned long height;
      DOMString deviceMaker;
      DOMString deviceModel;
      Date originalTime;
      ImageContentOrientation orientation;
      double fNumber;
      unsigned short[] isoSpeedRatings;
      DOMString exposureTime;
      ExposureProgram exposureProgram;
      boolean flash;
      double focalLength;
      WhiteBalanceMode whiteBalance;
      SimpleCoordinates gpsLocation;
      double gpsAltitude;
      DOMString gpsProcessingMethod;
      Date gpsTime;
      DOMString userComment;
*/

      JsonValue result = JsonValue(JsonObject());
      JsonObject& result_obj = result.get<JsonObject>();
      std::ostringstream ss_width;
      ss_width << width;
      result_obj.insert(std::make_pair("width", ss_width.str().c_str()));
      std::ostringstream ss_height;
      ss_height << height;
      result_obj.insert(std::make_pair("height", ss_height.str().c_str()));
      result_obj.insert(std::make_pair("device_maker", device_maker.c_str()));
      result_obj.insert(std::make_pair("device_model", device_model.c_str()));
      // todo: convert to js type: Date
      result_obj.insert(std::make_pair("original_time",
          asctime(localtime(&original_time))));

      // todo: implement remaining fields

      ReportSuccess(result, response->get<picojson::object>());
    } catch (const common::PlatformException& e) {
      ReportError(e, response->get<picojson::object>());
    }
  };

  auto get_response =
      [callback_id, this](const std::shared_ptr<JsonValue>& response) -> void {
        picojson::object& obj = response->get<picojson::object>();
        obj.insert(std::make_pair("callbackId", callback_id));
        LoggerD("callback is %s", response->serialize().c_str());
        PostMessage(response->serialize().c_str());
      };

  common::TaskQueue::GetInstance().Queue<JsonValue>(
      get, get_response,
      std::shared_ptr<JsonValue>(new JsonValue(JsonObject())));

  LoggerD("ExifInstance::getExifInfo() END (c++)");
}

void ExifInstance::saveExifInfo(const picojson::value& args,
                                picojson::object& out) {
  LoggerD("Entered");
  const std::string& uri = args.get("uri").get<std::string>();

  const double callback_id = args.get("callbackId").get<double>();
  auto get = [=](const std::shared_ptr<JsonValue>& response) -> void {
    try {
      ExifInformationPtr exifInfo(new ExifInformation(args));
      std::string uri = exifInfo->getUri();
      std::string path = ExifUtil::convertUriToPath(uri);
      exifInfo->saveToFile(path);

      ReportSuccess(args, response->get<picojson::object>());
    } catch (const common::PlatformException& e) {
      ReportError(e, response->get<picojson::object>());
    }
  };

  auto get_response =
      [callback_id, this](const std::shared_ptr<JsonValue>& response) -> void {
        picojson::object& obj = response->get<picojson::object>();
        obj.insert(std::make_pair("callbackId", callback_id));
        LoggerD("callback is %s", response->serialize().c_str());
        PostMessage(response->serialize().c_str());
      };

  common::TaskQueue::GetInstance().Queue<JsonValue>(
      get, get_response,
      std::shared_ptr<JsonValue>(new JsonValue(JsonObject())));
}

void ExifInstance::getThumbnail(const picojson::value& args,
                                picojson::object& out) {
  LoggerD("Entered");
  const std::string& uri = args.get("uri").get<std::string>();

  const double callback_id = args.get("callbackId").get<double>();
  auto get = [=](const std::shared_ptr<JsonValue>& response) -> void {
    try {
      const std::string file_path = ExifUtil::convertUriToPath(uri);
      ExifData* exif_data = nullptr;
      JsonValue result = JsonValue(JsonObject());;
      JsonObject& result_obj = result.get<JsonObject>();

      std::string ext = file_path.substr(file_path.find_last_of(".") + 1);
      std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

      if ("jpg" == ext) {
        ext = "jpeg";
      }

      if ("jpeg" == ext || "png" == ext || "gif" == ext) {
        LoggerD("Get thumbnail from Exif in file: [%s]", file_path.c_str());

        exif_data = exif_data_new_from_file(file_path.c_str());
        if (!exif_data) {
          throw common::UnknownException("File can not be loaded");
        }

        if (exif_data->data && exif_data->size) {
          gchar* ch_uri = g_base64_encode(exif_data->data, exif_data->size);
          std::string base64 = "data:image/"+ext+";base64," + ch_uri;

          exif_data_unref(exif_data);

          std::pair<std::string, std::string> pair;
          pair = std::make_pair("src", base64);
          result_obj.insert(pair);
        } else {
          exif_data_unref(exif_data);
          LoggerE("File [%s] doesn't contain thumbnail", file_path.c_str());
          throw common::InvalidValuesException("File doesn't contain"
              " thumbnail");
        }
      } else {
        LoggerE("extension: %s is not valid (jpeg/jpg/png/gif is supported)",
            ext.c_str());
        throw common::NotSupportedException("getThumbnail support only"
            " jpeg/jpg/png/gif");
      }

      ReportSuccess(result, response->get<picojson::object>());
    } catch (const common::PlatformException& e) {
      ReportError(e, response->get<picojson::object>());
    }
  };

  auto get_response =
      [callback_id, this](const std::shared_ptr<JsonValue>& response) -> void {
        picojson::object& obj = response->get<picojson::object>();
        obj.insert(std::make_pair("callbackId", callback_id));
        LoggerD("callback is %s", response->serialize().c_str());
        PostMessage(response->serialize().c_str());
      };

  common::TaskQueue::GetInstance().Queue<JsonValue>(
      get, get_response,
      std::shared_ptr<JsonValue>(new JsonValue(JsonObject())));
}

}  // namespace exif
}  // namespace extension
