// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
