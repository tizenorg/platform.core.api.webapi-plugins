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
#include "exif/exif_instance.h"

#include <libexif/exif-data.h>
#include <libexif/exif-entry.h>
#include <libexif/exif-loader.h>
#include <libexif/exif-utils.h>

#include <string>
#include <sstream>

#include "common/logger.h"
#include "common/platform_result.h"
#include "common/task-queue.h"

#include "exif/exif_information.h"
#include "exif/exif_util.h"
#include "exif/get_exif_info.h"
#include "exif/jpeg_file.h"

namespace extension {
namespace exif {

using common::PlatformResult;
using common::ErrorCode;

ExifInstance::ExifInstance() {
  using std::placeholders::_1;
  using std::placeholders::_2;

#define REGISTER_ASYNC(c, x) RegisterSyncHandler(c, std::bind(&ExifInstance::x, this, _1, _2));
  REGISTER_ASYNC("ExifManager_getExifInfo", ExifManagerGetExifInfo);
  REGISTER_ASYNC("ExifManager_saveExifInfo", ExifManagerSaveExifInfo);
  REGISTER_ASYNC("ExifManager_getThumbnail", ExifManagerGetThumbnail);
#undef REGISTER_ASYNC
}

ExifInstance::~ExifInstance() {}

void ExifInstance::ExifManagerGetExifInfo(const picojson::value& args, picojson::object& out) {
  LoggerD("enter");

  const std::string& uri = args.get("uri").get<std::string>();

  const double callback_id = args.get("callbackId").get<double>();
  auto get = [=](const std::shared_ptr<JsonValue>& response)->void {
      JsonValue result = JsonValue(JsonObject());
      PlatformResult status(ErrorCode::NO_ERROR);

      // TODO(r.galka) it can be done on JS side
      const std::string &file_path = ExifUtil::convertUriToPath(uri);
      LoggerD("file_path = %s", file_path.c_str());

      status = GetExifInfo::LoadFromURI(uri, &result);
      if (status)
        ReportSuccess(result, response->get<picojson::object>());
      else
        ReportError(status, &response->get<picojson::object>());
  };

  auto get_response = [callback_id, this](const std::shared_ptr<JsonValue>& response)->void {
      picojson::object& obj = response->get<picojson::object>();
      obj.insert(std::make_pair("callbackId", picojson::value(callback_id)));
      Instance::PostMessage(this, response->serialize().c_str());
  };

  common::TaskQueue::GetInstance().Queue<JsonValue>(
      get, get_response, std::shared_ptr<JsonValue>(new JsonValue(JsonObject())));

  LoggerD("exit");
}

void ExifInstance::ExifManagerSaveExifInfo(const picojson::value& args,
                                           picojson::object& out) {
  LoggerD("Entered");

  const double callback_id = args.get("callbackId").get<double>();
  auto get = [=](const std::shared_ptr<JsonValue>& response) -> void {
      JsonValue result = JsonValue(JsonObject());
      PlatformResult status(ErrorCode::NO_ERROR);

      ExifInformationPtr exifInfo(new ExifInformation(args));
      const std::string& uri = exifInfo->getUri();
      // TODO(r.galka) it can be done on JS side
      const std::string& path = ExifUtil::convertUriToPath(uri);
      status = exifInfo->saveToFile(path);

      if (status)
        ReportSuccess(result, response->get<picojson::object>());
      else
        ReportError(status, &response->get<picojson::object>());
  };

  auto get_response = [callback_id, this](const std::shared_ptr<JsonValue>& response) -> void {
      picojson::object& obj = response->get<picojson::object>();
      obj.insert(std::make_pair("callbackId", picojson::value(callback_id)));
      Instance::PostMessage(this, response->serialize().c_str());
  };

  common::TaskQueue::GetInstance().Queue<JsonValue>(get, get_response,
      std::shared_ptr<JsonValue>(new JsonValue(JsonObject())));
}

void ExifInstance::ExifManagerGetThumbnail(const picojson::value& args,
                                           picojson::object& out) {
  LoggerD("Entered");
  const std::string& uri = args.get("uri").get<std::string>();

  const double callback_id = args.get("callbackId").get<double>();
  auto get = [=](const std::shared_ptr<JsonValue> &response) -> void {
      PlatformResult status(ErrorCode::NO_ERROR);

      // TODO(r.galka) it can be done on JS side
      const std::string &file_path = ExifUtil::convertUriToPath(uri);
      JsonValue result = JsonValue(JsonObject());
      JsonObject &result_obj = result.get<JsonObject>();

      std::string ext = file_path.substr(file_path.find_last_of(".") + 1);
      std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

      if ("jpg" == ext) {
        ext = "jpeg";
      }

      if ("jpeg" != ext && "png" != ext && "gif" != ext) {
        LoggerE("extension: %s is not valid (jpeg/jpg/png/gif is supported)",
            ext.c_str());
        status = PlatformResult(ErrorCode::INVALID_VALUES_ERR,
            "getThumbnail support only jpeg/jpg/png/gif");
        ReportError(status, &response->get<picojson::object>());
        return;
      }

      LoggerD("Get thumbnail from Exif in file: [%s]", file_path.c_str());
      ExifData *exif_data = exif_data_new_from_file(file_path.c_str());
      if (!exif_data) {
        LoggerE("Error reading from file [%s]", file_path.c_str());
        status = PlatformResult(ErrorCode::UNKNOWN_ERR,
            "Error reading from file");
        ReportError(status, &response->get<picojson::object>());
        return;
      }

      if (!exif_data->data || !exif_data->size) {
        exif_data_unref(exif_data);
        LoggerE("File [%s] doesn't contain thumbnail", file_path.c_str());
        status = PlatformResult(ErrorCode::UNKNOWN_ERR,
            "File doesn't contain thumbnail");
        ReportError(status, &response->get<picojson::object>());
        return;
      }

      gchar *ch_uri = g_base64_encode(exif_data->data, exif_data->size);
      exif_data_unref(exif_data);
      std::string base64 = "data:image/" + ext + ";base64," + ch_uri;

      std::pair<std::string, picojson::value> pair;
      pair = std::make_pair("src", picojson::value(base64));
      result_obj.insert(pair);

      ReportSuccess(result, response->get<picojson::object>());
  };

  auto get_response = [callback_id, this](const std::shared_ptr<JsonValue>& response)->void {
      picojson::object& obj = response->get<picojson::object>();
      obj.insert(std::make_pair("callbackId", picojson::value(callback_id)));
      Instance::PostMessage(this, response->serialize().c_str());
  };

  common::TaskQueue::GetInstance().Queue<JsonValue>(
      get, get_response, std::shared_ptr<JsonValue>(new JsonValue(JsonObject())));
}

}  // namespace exif
}  // namespace extension
