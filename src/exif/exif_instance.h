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

#ifndef EXIF_EXIF_INSTANCE_H_
#define EXIF_EXIF_INSTANCE_H_

#include "common/extension.h"
#include "common/picojson.h"

typedef picojson::value JsonValue;
typedef picojson::object JsonObject;
typedef picojson::array JsonArray;
typedef std::string JsonString;

namespace extension {
namespace exif {

class ExifInstance : public common::ParsedInstance {
 public:
  ExifInstance();
  virtual ~ExifInstance();

 private:
  void ExifManagerGetExifInfo(const picojson::value& args, picojson::object& out);
  void ExifManagerSaveExifInfo(const picojson::value& args, picojson::object& out);
  void ExifManagerGetThumbnail(const picojson::value& args, picojson::object& out);
};

}  // namespace exif
}  // namespace extension

#endif  // EXIF_EXIF_INSTANCE_H_
