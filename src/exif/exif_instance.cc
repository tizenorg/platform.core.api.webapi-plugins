// Copyright (c) 2014 Samsung Electronics Co., Ltd. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "exif/exif_instance.h"

#include <string>

#include "common/logger.h"

namespace extension {
namespace exif {

namespace {
const char kGetExifInfoCmd[] = "Exif_getExifInfo";
const char kSaveExifInfoCmd[] = "Exif_saveExifInfo";
const char kGetThumbnailCmd[] = "Exif_getThumbnail";
}

ExifInstance::ExifInstance() {
  using namespace std::placeholders;
  #define REGISTER_SYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&ExifInstance::x, this, _1, _2));
  REGISTER_SYNC(kGetExifInfoCmd, getExifInfo);
  REGISTER_SYNC(kSaveExifInfoCmd, saveExifInfo);
  REGISTER_SYNC(kGetThumbnailCmd, getThumbnail);
  #undef REGISTER_SYNC
}

ExifInstance::~ExifInstance() {}

void ExifInstance::getExifInfo(const picojson::value& args, picojson::object& out) {
  LoggerE("getExifInfo is not implemented");
}

void ExifInstance::saveExifInfo(const picojson::value& args, picojson::object& out) {
  LoggerE("saveExifInfo is not implemented");
}

void ExifInstance::getThumbnail(const picojson::value& args, picojson::object& out) {
  LoggerE("getThumbnail is not implemented");
}
}  // namespace exif
}  // namespace extension
