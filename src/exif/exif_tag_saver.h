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

#ifndef EXIF_EXIF_TAG_SAVER_H_
#define EXIF_EXIF_TAG_SAVER_H_

#include <libexif/exif-data.h>

#include <string>
#include <vector>

#include "common/platform_result.h"
#include "exif_gps_location.h"

namespace extension {
namespace exif {

class ExifTagSaver {
 public:
  static void removeExifEntryWithTag(const ExifTag tag, ExifData* exif_data);

  static void saveToExif(long int value, ExifTag tag, ExifData* exif_data);
  static void saveToExif(const std::string& value, ExifTag tag,
      ExifData* exif_data, ExifFormat format = EXIF_FORMAT_ASCII,
      bool add_zero_character = true);
  static void saveToExif(const Rational& value, ExifTag tag,
      ExifData* exif_data);
  static void saveToExif(const Rationals& value, ExifTag tag,
      ExifData* exif_data);
  static void saveToExif(std::vector<long long int>& value, ExifFormat store_as,
      ExifTag tag, ExifData* exif_data);
  static void saveGpsLocationToExif(const ExifGPSLocation& gps_info,
      ExifData* exif_data);

 private:
  static ExifEntry* prepareEntry(ExifData* exif_data, ExifTag tag);
  static common::PlatformResult deduceIfdSection(ExifTag tag, ExifIfd* exif_ifd);
  static common::PlatformResult deduceDataFormat(ExifTag tag, ExifFormat* exif_format);
  static ExifEntry* createNewTag(ExifData* exif_data, ExifIfd ifd,
      ExifFormat format, ExifTag tag);
};

}  // namespace exif
}  // namespace extension

#endif  // EXIF_EXIF_TAG_SAVER_H_
