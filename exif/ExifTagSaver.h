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
 * @file    ExifTagSaver.h
 */

#ifndef __TIZEN_EXIF_EXIF_TAG_SAVER_H__
#define __TIZEN_EXIF_EXIF_TAG_SAVER_H__

#include <string>
#include <libexif/exif-data.h>

#include "ExifGPSLocation.h"
#include "ExifGPSTime.h"

namespace DeviceAPI {
namespace Exif {

class ExifTagSaver
{
public:
    static void removeExifEntryWithTag(const ExifTag tag, ExifData* exif_data);

    static void saveToExif(long int value, ExifTag tag, ExifData* exif_data);
    static void saveToExif(const std::string& value, ExifTag tag, ExifData* exif_data,
            bool add_zero_character = true);
    static void saveToExif(const Rational& value, ExifTag tag, ExifData* exif_data);
    static void saveToExif(const Rationals& value, ExifTag tag, ExifData* exif_data);
    static void saveToExif(std::vector<long long int>& value, ExifFormat store_as,
            ExifTag tag, ExifData* exif_data);
    static void saveGpsLocationToExif(const ExifGPSLocation& gps_info,
            ExifData* exif_data);
    static void saveGpsTimeToExif(const ExifGPSTime& gps_time,
            ExifData* exif_data);

private:
    static ExifEntry* prepareEntry(ExifData* exif_data, ExifTag tag);
    static ExifIfd deduceIfdSection(ExifTag tag);
    static ExifFormat deduceDataFormat(ExifTag tag);
    static ExifEntry* createNewTag(ExifData* exif_data, ExifIfd ifd,
        ExifFormat format, ExifTag tag);
};

} // Exif
} // DeviceAPI

#endif // __TIZEN_EXIF_EXIF_TAG_SAVER_H__
