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

#ifndef WEBAPI_PLUGINS_EXIF_GET_EXIF_INFO_H_
#define WEBAPI_PLUGINS_EXIF_GET_EXIF_INFO_H_

#include <libexif/exif-loader.h>
#include <string>

#include "common/extension.h"
#include "common/picojson.h"
#include "common/platform_result.h"

#include "exif/exif_gps_location.h"

typedef picojson::value JsonValue;
typedef picojson::object JsonObject;
typedef picojson::array JsonArray;
typedef std::string JsonString;

namespace extension {
namespace exif {

extern const std::size_t EXIF_UNDEFINED_TYPE_LENGTH;

class GetExifInfo {
 public:
  static common::PlatformResult ProcessEntry(ExifEntry* entry,
                                             ExifData* exif_data,
                                             JsonObject* result_obj);

  static common::PlatformResult LoadFromURI(const std::string& uri,
                                            JsonValue* result);

 private:
  GetExifInfo() { }  // private ctor - class can not be created

  static void ContentForeachFunctionProxy(ExifEntry* entry, void* user_data);
  static void DataForeachFunction(ExifContent* content, void* user_data);
};

typedef std::shared_ptr<GetExifInfo> GetExifInfoPtr;

}  // namespace exif
}  // namespace extension

#endif  // WEBAPI_PLUGINS_EXIF_GET_EXIF_INFO_H__
