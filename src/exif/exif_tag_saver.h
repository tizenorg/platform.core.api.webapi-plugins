//
// Tizen Web Device API
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Apache License, Version 2.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

/**
 * @file  exif_tag_saver.h
 */

#ifndef EXIF_EXIF_TAG_SAVER_H__
#define EXIF_EXIF_TAG_SAVER_H__

#include <libexif/exif-data.h>

#include <string>
#include <vector>

#include "ExifGPSLocation.h"

namespace extension {
namespace exif {

class ExifTagSaver {
 public:
  static void removeExifEntryWithTag(const ExifTag tag, ExifData* exif_data);

  static void saveToExif(long int value,
                         ExifTag tag,
                         ExifData* exif_data);
  static void saveToExif(const std::string& value, ExifTag tag,
                         ExifData* exif_data,
                         ExifFormat format = EXIF_FORMAT_ASCII,
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
  static ExifIfd deduceIfdSection(ExifTag tag);
  static ExifFormat deduceDataFormat(ExifTag tag);
  static ExifEntry* createNewTag(ExifData* exif_data, ExifIfd ifd,
    ExifFormat format, ExifTag tag);
};

}  // namespace exif
}  // namespace extension

#endif  // EXIF_EXIF_TAG_SAVER_H__
