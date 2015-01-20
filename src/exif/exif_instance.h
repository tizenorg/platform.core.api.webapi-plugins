// Copyright (c) 2014 Samsung Electronics Co., Ltd. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXIF_EXIF_INSTANCE_H_
#define EXIF_EXIF_INSTANCE_H_

#include "common/extension.h"
#include "common/picojson.h"

namespace extension {
namespace exif {
class ExifInstance : public common::ParsedInstance {
 public:
  ExifInstance();
  virtual ~ExifInstance();

 private:
  void getExifInfo(const picojson::value& args, picojson::object& out);
  void saveExifInfo(const picojson::value& args, picojson::object& out);
  void getThumbnail(const picojson::value& args, picojson::object& out);
};

}  // namespace exif
}  // namespace extension
#endif  // EXIF_EXIF_INSTANCE_H_
