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
 
#include "exif/exif_util.h"

#include <iomanip>
#include <sstream>

#include "common/platform_result.h"
#include "common/logger.h"

namespace extension {
namespace exif {

namespace {
const std::string ORIENTATION_NORMAL = "NORMAL";
const std::string ORIENTATION_FLIP_HORIZONTAL = "FLIP_HORIZONTAL";
const std::string ORIENTATION_ROTATE_180 = "ROTATE_180";
const std::string ORIENTATION_FLIP_VERTICAL = "FLIP_VERTICAL";
const std::string ORIENTATION_TRANSPOSE = "TRANSPOSE";
const std::string ORIENTATION_ROTATE_90 = "ROTATE_90";
const std::string ORIENTATION_TRANSVERSE = "TRANSVERSE";
const std::string ORIENTATION_ROTATE_270 = "ROTATE_270";

const std::string WHITE_BALANCE_MODE_AUTO = "AUTO";
const std::string WHITE_BALANCE_MODE_MANUAL = "MANUAL";

const std::string EXPOSURE_PROGRAM_NOT_DEFINED = "NOT_DEFINED";
const std::string EXPOSURE_PROGRAM_MANUAL = "MANUAL";
const std::string EXPOSURE_PROGRAM_NORMAL = "NORMAL";
const std::string EXPOSURE_PROGRAM_APERTURE_PRIORITY = "APERTURE_PRIORITY";
const std::string EXPOSURE_PROGRAM_SHUTTER_PRIORITY = "SHUTTER_PRIORITY";
const std::string EXPOSURE_PROGRAM_CREATIVE_PROGRAM = "CREATIVE_PROGRAM";
const std::string EXPOSURE_PROGRAM_ACTION_PROGRAM = "ACTION_PROGRAM";
const std::string EXPOSURE_PROGRAM_PORTRAIT_MODE = "PORTRAIT_MODE";
const std::string EXPOSURE_PROGRAM_LANDSCAPE_MODE = "LANDSCAPE_MODE";

const std::string DUMMY = "";  // For unexpected input handling

const std::string URI_PREFIX = "file://";
}  // namespace

const std::size_t ExifTypeInfo::ByteSize = 1;
const std::size_t ExifTypeInfo::ASCIISize = 1;
const std::size_t ExifTypeInfo::ShortSize = 2;
const std::size_t ExifTypeInfo::LongSize = 4;
const std::size_t ExifTypeInfo::RationalSize = 8;
const std::size_t ExifTypeInfo::UndefinedSize = 1;
const std::size_t ExifTypeInfo::SLongSize = 4;
const std::size_t ExifTypeInfo::SRationalSize = 8;

const ExifByte ExifTypeInfo::ByteId = 1;
const ExifByte ExifTypeInfo::ASCIIId = 2;
const ExifByte ExifTypeInfo::ShortId = 3;
const ExifByte ExifTypeInfo::LongId = 4;
const ExifByte ExifTypeInfo::RationalId = 5;
const ExifByte ExifTypeInfo::UndefinedId = 7;
const ExifByte ExifTypeInfo::SLongId = 9;
const ExifByte ExifTypeInfo::SRationalId = 10;

ExifUtil::ExifUtil() {
}

ExifUtil::~ExifUtil() {
}

ImageOrientation ExifUtil::stringToOrientation(const std::string& orientation) {
  LoggerD("Entered");
  if (ORIENTATION_NORMAL == orientation) {
    return ImageOrientation::EXIF_ORIENTATION_NORMAL;
  }
  if (ORIENTATION_FLIP_HORIZONTAL == orientation) {
    return ImageOrientation::EXIF_ORIENTATION_FLIP_HORIZONTAL;
  }
  if (ORIENTATION_ROTATE_180 == orientation) {
    return ImageOrientation::EXIF_ORIENTATION_ROTATE_180;
  }
  if (ORIENTATION_FLIP_VERTICAL == orientation) {
    return ImageOrientation::EXIF_ORIENTATION_FLIP_VERTICAL;
  }
  if (ORIENTATION_TRANSPOSE == orientation) {
    return ImageOrientation::EXIF_ORIENTATION_TRANSPOSE;
  }
  if (ORIENTATION_ROTATE_90 == orientation) {
    return ImageOrientation::EXIF_ORIENTATION_ROTATE_90;
  }
  if (ORIENTATION_TRANSVERSE == orientation) {
    return ImageOrientation::EXIF_ORIENTATION_TRANSVERSE;
  }
  if (ORIENTATION_ROTATE_270 == orientation) {
    return ImageOrientation::EXIF_ORIENTATION_ROTATE_270;
  }
  return ImageOrientation::EXIF_ORIENTATION_NOT_VALID;
}

const std::string& ExifUtil::orientationToString(ImageOrientation orientation) {
  LoggerD("Entered");
  switch (orientation) {
    case ImageOrientation::EXIF_ORIENTATION_NORMAL:
      return ORIENTATION_NORMAL;
    case ImageOrientation::EXIF_ORIENTATION_FLIP_HORIZONTAL:
      return ORIENTATION_FLIP_HORIZONTAL;
    case ImageOrientation::EXIF_ORIENTATION_ROTATE_180:
      return ORIENTATION_ROTATE_180;
    case ImageOrientation::EXIF_ORIENTATION_FLIP_VERTICAL:
      return ORIENTATION_FLIP_VERTICAL;
    case ImageOrientation::EXIF_ORIENTATION_TRANSPOSE:
      return ORIENTATION_TRANSPOSE;
    case ImageOrientation::EXIF_ORIENTATION_ROTATE_90:
      return ORIENTATION_ROTATE_90;
    case ImageOrientation::EXIF_ORIENTATION_TRANSVERSE:
      return ORIENTATION_TRANSVERSE;
    case ImageOrientation::EXIF_ORIENTATION_ROTATE_270:
      return ORIENTATION_ROTATE_270;
    default:
      return DUMMY;
  }
}

WhiteBalanceMode ExifUtil::stringToWhiteBalance(
    const std::string& white_balance) {
  LoggerD("Entered");
  if (WHITE_BALANCE_MODE_AUTO == white_balance) {
    return WhiteBalanceMode::EXIF_WHITE_BALANCE_MODE_AUTO;
  }
  if (WHITE_BALANCE_MODE_MANUAL == white_balance) {
    return WhiteBalanceMode::EXIF_WHITE_BALANCE_MODE_MANUAL;
  }
  return WhiteBalanceMode::EXIF_WHITE_BALANCE_MODE_NOT_VALID;
}

const std::string& ExifUtil::whiteBalanceToString(WhiteBalanceMode value) {
  LoggerD("Entered");
  switch (value) {
    case WhiteBalanceMode::EXIF_WHITE_BALANCE_MODE_AUTO:
      return WHITE_BALANCE_MODE_AUTO;
    case WhiteBalanceMode::EXIF_WHITE_BALANCE_MODE_MANUAL:
      return WHITE_BALANCE_MODE_MANUAL;
    default:
      return DUMMY;
  }
}

ExposureProgram ExifUtil::stringToExposureProgram(
    const std::string& exposure_program) {
  LoggerD("Entered");
  if (EXPOSURE_PROGRAM_NOT_DEFINED == exposure_program) {
    return EXIF_EXPOSURE_PROGRAM_NOT_DEFINED;
  }
  if (EXPOSURE_PROGRAM_MANUAL == exposure_program) {
    return EXIF_EXPOSURE_PROGRAM_MANUAL;
  }
  if (EXPOSURE_PROGRAM_NORMAL == exposure_program) {
    return EXIF_EXPOSURE_PROGRAM_NORMAL;
  }
  if (EXPOSURE_PROGRAM_APERTURE_PRIORITY == exposure_program) {
    return EXIF_EXPOSURE_PROGRAM_APERTURE_PRIORITY;
  }
  if (EXPOSURE_PROGRAM_SHUTTER_PRIORITY == exposure_program) {
    return EXIF_EXPOSURE_PROGRAM_SHUTTER_PRIORITY;
  }
  if (EXPOSURE_PROGRAM_CREATIVE_PROGRAM == exposure_program) {
    return EXIF_EXPOSURE_PROGRAM_CREATIVE_PROGRAM;
  }
  if (EXPOSURE_PROGRAM_ACTION_PROGRAM == exposure_program) {
    return EXIF_EXPOSURE_PROGRAM_ACTION_PROGRAM;
  }
  if (EXPOSURE_PROGRAM_PORTRAIT_MODE == exposure_program) {
    return EXIF_EXPOSURE_PROGRAM_PORTRAIT_MODE;
  }
  if (EXPOSURE_PROGRAM_LANDSCAPE_MODE == exposure_program) {
    return EXIF_EXPOSURE_PROGRAM_LANDSCAPE_MODE;
  }
  return EXIF_EXPOSURE_PROGRAM_NOT_VALID;
}

const std::string& ExifUtil::exposureProgramToString(ExposureProgram value) {
  LoggerD("Entered");
  switch (value) {
    case ExposureProgram::EXIF_EXPOSURE_PROGRAM_NOT_DEFINED:
      return EXPOSURE_PROGRAM_NOT_DEFINED;
    case ExposureProgram::EXIF_EXPOSURE_PROGRAM_MANUAL:
      return EXPOSURE_PROGRAM_MANUAL;
    case ExposureProgram::EXIF_EXPOSURE_PROGRAM_NORMAL:
      return EXPOSURE_PROGRAM_NORMAL;
    case ExposureProgram::EXIF_EXPOSURE_PROGRAM_APERTURE_PRIORITY:
      return EXPOSURE_PROGRAM_APERTURE_PRIORITY;
    case ExposureProgram::EXIF_EXPOSURE_PROGRAM_SHUTTER_PRIORITY:
      return EXPOSURE_PROGRAM_SHUTTER_PRIORITY;
    case ExposureProgram::EXIF_EXPOSURE_PROGRAM_CREATIVE_PROGRAM:
      return EXPOSURE_PROGRAM_CREATIVE_PROGRAM;
    case ExposureProgram::EXIF_EXPOSURE_PROGRAM_ACTION_PROGRAM:
      return EXPOSURE_PROGRAM_ACTION_PROGRAM;
    case ExposureProgram::EXIF_EXPOSURE_PROGRAM_PORTRAIT_MODE:
      return EXPOSURE_PROGRAM_PORTRAIT_MODE;
    case ExposureProgram::EXIF_EXPOSURE_PROGRAM_LANDSCAPE_MODE:
      return EXPOSURE_PROGRAM_LANDSCAPE_MODE;
    default:
      return DUMMY;
  }
}

// Example:
// in: uri = file:///opt/usr/media/Images/exif.jpg
// out: path = /opt/usr/media/Images/exif.jpg
std::string ExifUtil::convertUriToPath(const std::string& str) {
  std::string path = ltrim(str);
  std::string prefix = path.substr(0, URI_PREFIX.size());

  if (prefix == URI_PREFIX) {
    return path.substr(URI_PREFIX.size());
  } else {
    return path;
  }
}

std::string ExifUtil::ltrim(const std::string& s) {
  std::string str = s;
  std::string::iterator i;
  for (i = str.begin(); i != str.end(); ++i) {
    if (!isspace(*i)) {
      break;
    }
  }

  if (i == str.end()) {
    str.clear();
  } else {
    str.erase(str.begin(), i);
  }
  return str;
}

time_t ExifUtil::exifDateTimeOriginalToTimeT(const char* string) {
  int year, month, day, hour, min, sec;
  if (sscanf(string, "%5d:%5d:%5d %5d:%5d:%5d",
      &year, &month, &day, &hour, &min, &sec) >= 6) {
    return convertToTimeT(year, month, day, hour, min, sec);
  }

  return 0;
}

std::string ExifUtil::timeTToExifDateTimeOriginal(time_t time) {
  int year, month, day, hour, min, sec;
  extractFromTimeT(time, year, month, day, hour, min, sec);

  std::ostringstream ss;
  ss << std::setfill('0') << std::setw(4) << year << ':';
  ss << std::setfill('0') << std::setw(2) << month << ':';
  ss << std::setfill('0') << std::setw(2) << day << ' ';

  ss << std::setfill('0') << std::setw(2) << hour << ':';
  ss << std::setfill('0') << std::setw(2) << min << ':';
  ss << std::setfill('0') << std::setw(2) << sec;
  return ss.str();
}

const Rationals ExifUtil::timeTToExifGpsTimeStamp(time_t time) {
  int year, month, day, hour, min, sec;
  extractFromTimeT(time, year, month, day, hour, min, sec);

  Rational hourRational = Rational::createFromDouble(static_cast<double>(hour));
  Rational minRational = Rational::createFromDouble(static_cast<double>(min));
  Rational secRational = Rational::createFromDouble(static_cast<double>(sec));

  Rationals result;
  result.push_back(hourRational);
  result.push_back(minRational);
  result.push_back(secRational);

  return result;
}

std::string ExifUtil::timeTToExifGpsDateStamp(time_t time) {
  int year, month, day, hour, min, sec;
  extractFromTimeT(time, year, month, day, hour, min, sec);

  LoggerD("year: %d", year);
  LoggerD("month: %d", month);
  LoggerD("day: %d", day);

  std::ostringstream ss;
  ss << std::setfill('0') << std::setw(4) << year << ':';
  ss << std::setfill('0') << std::setw(2) << month << ':';
  ss << std::setfill('0') << std::setw(2) << day;

  LoggerD("SS: %s", ss.str().c_str());

  return ss.str();
}

size_t  ExifUtil::getSizeOfExifFormatType(ExifFormat format) {
  std::size_t size_per_member = 0;
  switch (format) {
    case EXIF_FORMAT_BYTE:
      size_per_member = 1;
      break;

    case EXIF_FORMAT_SHORT:
    case EXIF_FORMAT_SSHORT:
      size_per_member = 2;
      break;

    case EXIF_FORMAT_LONG:
    case EXIF_FORMAT_SLONG:
      size_per_member = 4;
      break;

    case EXIF_FORMAT_RATIONAL:
    case EXIF_FORMAT_SRATIONAL:
      size_per_member = 8;
      break;

    default:
      LoggerE("output ExifFormat: %d is not supported!");
      return 0;
  }

  return size_per_member;
}

void ExifUtil::printExifEntryInfo(ExifEntry* entry, ExifData* exif_data) {
  char buf[2000];
  std::stringstream ss;

  if (!entry) {
    LoggerE("entry is null");
    return;
  }

  if (!entry->data) {
    LoggerE("entry data is null");
    return;
  }

  unsigned char* read_buf_ptr = entry->data;

  std::size_t size_per_member = getSizeOfExifFormatType(entry->format);
  if (0 == size_per_member) {
    size_per_member = 1;  // display as array of bytes
  }

  for (unsigned long compi = 0; compi < entry->components; ++compi) {
    if (compi > 0) {
      ss << " ";
    }

    for (size_t i = 0; i < size_per_member; ++i) {
      unsigned int value = read_buf_ptr[i];
      ss << std::hex << std::setw(2) << std::setfill('0') << value;
    }

    read_buf_ptr += size_per_member;
  }

  LoggerD("Entry{name:%s type:%s size:%d components:%d value:%s RAW DATA:[%s]}",
      exif_tag_get_name(entry->tag),
      exif_format_get_name(entry->format),
      static_cast<int>(entry->size),
      static_cast<int>(entry->components),
      exif_entry_get_value(entry, buf, sizeof(buf)),
      ss.str().c_str());
}

void ExifUtil::extractFromTimeT(const time_t time,
                                int& out_year, int& out_month, int& out_day,
                                int& out_hour, int& out_min, int& out_sec) {
  struct tm utc;
  gmtime_r(&time, &utc);

  out_year = utc.tm_year + 1900;
  out_month = utc.tm_mon + 1;
  out_day = utc.tm_mday;
  out_hour = utc.tm_hour;
  out_min = utc.tm_min;
  out_sec = utc.tm_sec;
}

time_t ExifUtil::convertToTimeT(int year, int month, int day,
      int hour, int min, int sec) {
  struct tm timeinfo = { 0 };
  time_t tmp_time = 0;
  tzset();
  localtime_r(&tmp_time, &timeinfo);

  timeinfo.tm_year = year - 1900;
  timeinfo.tm_mon = month - 1;
  timeinfo.tm_mday = day;

  timeinfo.tm_hour = hour;
  timeinfo.tm_min = min;
  timeinfo.tm_sec = sec;

  // From mktime documentation:
  // "The values of the members tm_wday and tm_yday of timeptr are ignored"
  return timegm(&timeinfo);
}

}  // namespace exif
}  // namespace extension
