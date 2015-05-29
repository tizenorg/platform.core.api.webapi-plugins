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

#ifndef EXIF_EXIF_UTIL_H_
#define EXIF_EXIF_UTIL_H_

#include <libexif/exif-data.h>
#include <libexif/exif-entry.h>
#include <libexif/exif-utils.h>
#include <string>
#include <vector>

#include "rational.h"

namespace extension {
namespace exif {

enum ImageOrientation {
  EXIF_ORIENTATION_NORMAL = 1,
  EXIF_ORIENTATION_FLIP_HORIZONTAL = 2,
  EXIF_ORIENTATION_ROTATE_180 = 3,
  EXIF_ORIENTATION_FLIP_VERTICAL = 4,
  EXIF_ORIENTATION_TRANSPOSE = 5,
  EXIF_ORIENTATION_ROTATE_90 = 6,
  EXIF_ORIENTATION_TRANSVERSE = 7,
  EXIF_ORIENTATION_ROTATE_270 = 8,
  EXIF_ORIENTATION_NOT_VALID,
};

enum WhiteBalanceMode {
  EXIF_WHITE_BALANCE_MODE_AUTO = 0,
  EXIF_WHITE_BALANCE_MODE_MANUAL = 1,
  EXIF_WHITE_BALANCE_MODE_NOT_VALID
};

enum ExposureProgram {
  EXIF_EXPOSURE_PROGRAM_NOT_DEFINED = 0,
  EXIF_EXPOSURE_PROGRAM_MANUAL = 1,
  EXIF_EXPOSURE_PROGRAM_NORMAL = 2,
  EXIF_EXPOSURE_PROGRAM_APERTURE_PRIORITY = 3,
  EXIF_EXPOSURE_PROGRAM_SHUTTER_PRIORITY = 4,
  EXIF_EXPOSURE_PROGRAM_CREATIVE_PROGRAM = 5,
  EXIF_EXPOSURE_PROGRAM_ACTION_PROGRAM = 6,
  EXIF_EXPOSURE_PROGRAM_PORTRAIT_MODE = 7,
  EXIF_EXPOSURE_PROGRAM_LANDSCAPE_MODE = 8,
  EXIF_EXPOSURE_PROGRAM_NOT_VALID
};

/**
 * From Exif 2.2 specification:
 *  The following types are used in Exif:
 *  1 = BYTE An 8-bit unsigned integer.,
 *  2 = ASCII An 8-bit byte containing one 7-bit ASCII code. The final byte is terminated
 *    with NULL.,
 *  3 = SHORT A 16-bit (2-byte) unsigned integer,
 *  4 = LONG A 32-bit (4-byte) unsigned integer,
 *  5 = RATIONAL Two LONGs. The first LONG is the numerator and the second LONG expresses
 *    the denominator.,
 *  7 = UNDEFINED An 8-bit byte that can take any value depending on the field definition,
 *  9 = SLONG A 32-bit (4-byte) signed integer (2's complement notation),
 * 10 = SRATIONAL Two SLONGs. The first SLONG is the numerator and the second SLONG is the
 *    denominator.
 */
struct ExifTypeInfo {

  /**
   * Number of bytes used by each exif type
   */
  static const std::size_t ByteSize;       // 1 byte
  static const std::size_t ASCIISize;      // 1 byte (*N)
  static const std::size_t ShortSize;      // 2 bytes
  static const std::size_t LongSize;       // 4 bytes
  static const std::size_t RationalSize;   // 8 bytes
  static const std::size_t UndefinedSize;  // 1 byte (*N)
  static const std::size_t SLongSize;      // 4 bytes
  static const std::size_t SRationalSize;  // 8 bytes

  /**
   * Id values used by Exif to identify type
   */
  static const ExifByte ByteId;       // 1
  static const ExifByte ASCIIId;      // 2
  static const ExifByte ShortId;      // 3
  static const ExifByte LongId;       // 4
  static const ExifByte RationalId;   // 5
  static const ExifByte UndefinedId;  // 7
  static const ExifByte SLongId;      // 9
  static const ExifByte SRationalId;  // 10
};

class ExifUtil {
 public:
  ExifUtil();
  virtual ~ExifUtil();

  static ImageOrientation stringToOrientation(const std::string& orientation);
  static const std::string& orientationToString(ImageOrientation value);

  static WhiteBalanceMode stringToWhiteBalance(
      const std::string& white_balance);
  static const std::string& whiteBalanceToString(WhiteBalanceMode value);

  static ExposureProgram stringToExposureProgram(
      const std::string& exposure_program);
  static const std::string& exposureProgramToString(ExposureProgram value);

  static std::string convertUriToPath(const std::string& str);
  static std::string ltrim(const std::string& s);

  static time_t exifDateTimeOriginalToTimeT(const char* string);
  static std::string timeTToExifDateTimeOriginal(time_t time);
  static const Rationals timeTToExifGpsTimeStamp(time_t time);
  static std::string timeTToExifGpsDateStamp(time_t time);

  static std::size_t getSizeOfExifFormatType(ExifFormat format);
  static void  printExifEntryInfo(ExifEntry* entry, ExifData* exif_data);

  static void extractFromTimeT(const time_t time,
      int& out_year, int& out_month, int& out_day,
      int& out_hour, int& out_min, int& out_sec);

  static time_t convertToTimeT(int year, int month, int day,
      int hour, int min, int sec);
};

}  // namespace exif
}  // namespace extension

#endif  // EXIF_EXIF_UTIL_H_
