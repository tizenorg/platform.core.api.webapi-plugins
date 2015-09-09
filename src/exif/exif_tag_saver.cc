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
#include "exif/exif_tag_saver.h"

#include <libexif/exif-format.h>
#include <sstream>
#include <cstring>

#include "common/platform_result.h"
#include "common/logger.h"

#include "exif/exif_util.h"

namespace extension {
namespace exif {

void ExifTagSaver::removeExifEntryWithTag(const ExifTag tag,
                                          ExifData* exif_data) {
  LoggerD("Entered tag:%d (0x%x)", tag, tag);
  ExifEntry* exif_entry = exif_data_get_entry(exif_data, tag);
  if (!exif_entry) {
    LoggerE("Exif entry with tag:%d (0x%x) is not present", tag, tag);
    return;
  }

  exif_content_remove_entry(exif_entry->parent, exif_entry);
}

void ExifTagSaver::saveToExif(long int value, ExifTag tag,
                              ExifData* exif_data) {
  LoggerD("Entered");
  ExifEntry* entry = prepareEntry(exif_data, tag);
  if (!entry) {
    // TODO return PlatformResult and handle error
    LoggerE("Exif entry is null");
    return;
  }

  ExifByteOrder order = exif_data_get_byte_order(exif_data);

  LoggerD("entry->format: %d", entry->format);
  LoggerD("EXIF_FORMAT_BYTE: %d", EXIF_FORMAT_BYTE);

  switch (entry->format) {
    case EXIF_FORMAT_BYTE: {
      entry->data[0] = static_cast<unsigned char>(value);
      break;
    }
    case EXIF_FORMAT_SHORT: {
      exif_set_short(entry->data, order, value);
      break;
    }
    case EXIF_FORMAT_LONG: {
      exif_set_long(entry->data, order, value);
      break;
    }
    case EXIF_FORMAT_SLONG: {
      exif_set_slong(entry->data, order, value);
      break;
    }
    default: {
      LoggerE("Error: wrong format: %d \n", entry->format);
    }
  }
}

void ExifTagSaver::saveToExif(const std::string& value, ExifTag tag,
                              ExifData* exif_data, ExifFormat format,
                              bool add_zero_character) {
  LoggerD("Entered");
  ExifEntry* entry = prepareEntry(exif_data, tag);
  if (!entry) {
    // TODO return PlatformResult and handle error
    LoggerE("Exif entry is null");
    return;
  }

  if (!value.empty()) {
    if (entry->data) {
      free(entry->data);
      entry->data = NULL;
    }

    std::size_t new_len = value.length();
    if (add_zero_character) {
      ++new_len;
    }

    entry->format = format;
    entry->size = new_len;
    entry->components = new_len;

    entry->data = static_cast<unsigned char*>(malloc(entry->size));
    if (entry->data == nullptr) {
      LoggerE("Function malloc returned nullptr");
      return;
    }

    memcpy(entry->data, value.c_str(), value.length());
    if (add_zero_character) {
      entry->data[value.length()] = '\0';
    }
  }
}

void ExifTagSaver::saveToExif(const Rational& value, ExifTag tag,
                              ExifData* exif_data) {
  LoggerD("Entered");
  ExifEntry* entry = prepareEntry(exif_data, tag);
  if (!entry) {
    // TODO return PlatformResult and handle error
    LoggerE("Exif entry is null");
    return;
  }
  entry->format = EXIF_FORMAT_RATIONAL;

  if (ExifTypeInfo::RationalSize != entry->size) {
    if (entry->data) {
      free(entry->data);
      entry->data = NULL;
    }

    entry->size = ExifTypeInfo::RationalSize;
    entry->data = static_cast<unsigned char*>(malloc(entry->size));
    if (entry->data == nullptr) {
      LoggerE("Function malloc returned nullptr");
      return;
    }
    memset(entry->data, 0, entry->size);
  }

  entry->components = 1;

  ExifByteOrder order = exif_data_get_byte_order(exif_data);
  ExifRational r;
  r.numerator = value.nominator;
  r.denominator = value.denominator;
  exif_set_rational(entry->data, order, r);
}

void ExifTagSaver::saveToExif(const Rationals& value, ExifTag tag,
                              ExifData* exif_data) {
  LoggerD("Entered");
  ExifEntry* entry = prepareEntry(exif_data, tag);
  if (!entry) {
    // TODO return PlatformResult and handle error
    LoggerE("Exif entry is null");
    return;
  }
  ExifByteOrder order = exif_data_get_byte_order(exif_data);
  entry->format = EXIF_FORMAT_RATIONAL;

  const unsigned int required_size = ExifTypeInfo::RationalSize * value.size();
  if (required_size != entry->size) {
    if (entry->data) {
      free(entry->data);
      entry->data = NULL;
    }

    entry->size = required_size;
    entry->data = static_cast<unsigned char*>(malloc(entry->size));
    if (entry->data == nullptr) {
      LoggerE("Function malloc returned nullptr");
      return;
    }
    memset(entry->data, 0, entry->size);
  }

  entry->components = value.size();
  for (std::size_t i = 0; i < value.size(); ++i) {
    ExifRational r;
    r.numerator = value[i].nominator;
    r.denominator = value[i].denominator;
    exif_set_rational(entry->data + i * ExifTypeInfo::RationalSize, order, r);
  }
}

void ExifTagSaver::saveToExif(std::vector<long long int>& value,
                              ExifFormat store_as,
                              ExifTag tag, ExifData* exif_data) {
  LoggerD("Entered");
  ExifEntry* entry = prepareEntry(exif_data, tag);
  if (!entry) {
    // TODO return PlatformResult and handle error
    LoggerE("Exif entry is null");
    return;
  }
  const ExifByteOrder order = exif_data_get_byte_order(exif_data);

  const std::size_t size_per_member = ExifUtil::getSizeOfExifFormatType(store_as);
  switch (store_as) {
    case EXIF_FORMAT_BYTE:
    case EXIF_FORMAT_SHORT:
    case EXIF_FORMAT_SSHORT:
    case EXIF_FORMAT_LONG:
    case EXIF_FORMAT_SLONG:
      break;
    default:
      LoggerE("output ExifFormat: %d is not supported!", store_as);
      return;
  }
  entry->format = store_as;

  const std::size_t num_elements = value.size();
  const unsigned int required_size = size_per_member * num_elements;
  if (required_size != entry->size) {
    if (entry->data) {
      free(entry->data);
      entry->data = NULL;
    }

    entry->size = required_size;
    entry->data = static_cast<unsigned char*>(malloc(entry->size));
    if (entry->data == nullptr) {
      LoggerE("Function malloc returned nullptr");
      return;
    }
    memset(entry->data, 0, entry->size);
  }
  entry->components = num_elements;


  switch (store_as) {
    case EXIF_FORMAT_BYTE: {
      for (std::size_t i = 0; i < num_elements; ++i) {
        entry->data[i] = static_cast<ExifByte>(value[i]);
      }
      break;
    }
    case EXIF_FORMAT_SHORT: {
      for (std::size_t i = 0; i < num_elements; ++i) {
        exif_set_short(entry->data + i * size_per_member, order,
            static_cast<ExifShort>(value[i]));
      }
      break;
    }
    case EXIF_FORMAT_SSHORT: {
      for (std::size_t i = 0; i < num_elements; ++i) {
        exif_set_sshort(entry->data + i * size_per_member, order,
            static_cast<ExifSShort>(value[i]));
      }
      break;
    }
    case EXIF_FORMAT_LONG: {
      for (std::size_t i = 0; i < num_elements; ++i) {
        exif_set_long(entry->data + i * size_per_member, order,
            static_cast<ExifLong>(value[i]));
      }
      break;
    }
    case EXIF_FORMAT_SLONG: {
      for (std::size_t i = 0; i < num_elements; ++i) {
        exif_set_slong(entry->data + i * size_per_member, order,
            static_cast<ExifSLong>(value[i]));
      }
      break;
    }
  }

  LoggerD("entry after save:");
  ExifUtil::printExifEntryInfo(entry, exif_data);
}

void ExifTagSaver::saveGpsLocationToExif(const ExifGPSLocation& gps_info,
                                         ExifData* exif_data) {
  LoggerD("Entered");
  if (gps_info.isSet(EXIF_GPS_LOCATION_ATTRIBUTE_LATITUDE)) {
    auto latitude = gps_info.getLatitude();
    LoggerD("Saving latitude: %s", latitude.toDebugString().c_str());
    saveToExif(latitude.toRationalsVector(),
        static_cast<ExifTag>(EXIF_TAG_GPS_LATITUDE), exif_data);
  }

  if (gps_info.isSet(EXIF_GPS_LOCATION_ATTRIBUTE_LATITUDE_REF)) {
    std::string lat_ref =
        (gps_info.getLatitudeRef() == GPS_LOCATION_NORTH) ? "N" : "S";
    LoggerD("Saving latitude ref: %s", lat_ref.c_str());
    saveToExif(lat_ref, static_cast<ExifTag>(EXIF_TAG_GPS_LATITUDE_REF),
        exif_data, EXIF_FORMAT_ASCII, false);
  }

  if (gps_info.isSet(EXIF_GPS_LOCATION_ATTRIBUTE_LONGITUDE)) {
    auto longitude = gps_info.getLongitude();
    LoggerD("Saving longitude: %s", longitude.toDebugString().c_str());
    saveToExif(longitude.toRationalsVector(),
        static_cast<ExifTag>(EXIF_TAG_GPS_LONGITUDE), exif_data);
  }

  if (gps_info.isSet(EXIF_GPS_LOCATION_ATTRIBUTE_LONGITUDE_REF)) {
    std::string long_ref =
        (gps_info.getLongitudeRef() == GPS_LOCATION_WEST) ? "W" : "E";
    LoggerD("Saving longitude ref: %s", long_ref.c_str());
    saveToExif(long_ref, static_cast<ExifTag>(EXIF_TAG_GPS_LONGITUDE_REF),
        exif_data, EXIF_FORMAT_ASCII, false);
  }
}

ExifEntry* ExifTagSaver::prepareEntry(ExifData* exif_data, ExifTag tag) {
  LoggerD("Entered m_tag:%d", tag);

  ExifEntry* exif_entry = exif_data_get_entry(exif_data, tag);
  if (!exif_entry) {
    ExifIfd exif_ifd;
    common::PlatformResult ret = deduceIfdSection(tag, &exif_ifd);
    if (!ret) {
      LoggerE("Couldn't deduce ifd section: %s", ret.message().c_str());
      return nullptr;
    }

    ExifFormat exif_format;
    ret = deduceDataFormat(tag, &exif_format);
    if (!ret) {
      LoggerE("Couldn't deduce data format: %s", ret.message().c_str());
      return nullptr;
    }
    exif_entry = createNewTag(exif_data, exif_ifd, exif_format, tag);
  }

  if (!exif_entry) {
    LoggerE("Couldn't create new Exif tag");
    return nullptr;
  }

  exif_entry_initialize(exif_entry, tag);

  return exif_entry;
}

ExifEntry* ExifTagSaver::createNewTag(ExifData* exif_data, ExifIfd ifd,
                                      ExifFormat format, ExifTag tag) {
  LoggerD("Creating new tag: %d", tag);

  ExifEntry* new_entry = exif_entry_new();
  if (new_entry == nullptr) {
    LoggerE("Function exif_entry_new returned nullptr");
  } else {
    new_entry->tag = tag;
    new_entry->format = format;
    exif_content_add_entry(exif_data->ifd[ifd], new_entry);
    exif_entry_initialize(new_entry, tag);
  }
  return new_entry;
}

common::PlatformResult ExifTagSaver::deduceIfdSection(ExifTag tag, ExifIfd* exif_ifd) {
  LoggerD("Entered");
  // TODO EXIF_TAG_* and EXIF_TAG_GPS_* are sharing same values,
  // they shouldn't be used in one switch statement.

  switch (static_cast<unsigned int>(tag)) {
    // Tags in IFD_0 Section
    case EXIF_TAG_MAKE:
    case EXIF_TAG_MODEL:
    case EXIF_TAG_IMAGE_WIDTH:
    case EXIF_TAG_IMAGE_LENGTH:
    case EXIF_TAG_ORIENTATION:
      *exif_ifd = EXIF_IFD_0;
      break;

    // Tags in IFD_EXIF Section
    case EXIF_TAG_USER_COMMENT:
    case EXIF_TAG_DATE_TIME_ORIGINAL:
    case EXIF_TAG_EXPOSURE_TIME:
    case EXIF_TAG_FNUMBER:
    case EXIF_TAG_EXPOSURE_PROGRAM:
    case EXIF_TAG_ISO_SPEED_RATINGS:
    case EXIF_TAG_WHITE_BALANCE:
    case EXIF_TAG_FLASH:
    case EXIF_TAG_FOCAL_LENGTH:
      *exif_ifd = EXIF_IFD_EXIF;
      break;

    // Tags in IFD_GPS Section
    case EXIF_TAG_GPS_LATITUDE_REF:
    case EXIF_TAG_GPS_LONGITUDE_REF:
    case EXIF_TAG_GPS_LATITUDE:
    case EXIF_TAG_GPS_LONGITUDE:
    case EXIF_TAG_GPS_ALTITUDE:
    case EXIF_TAG_GPS_ALTITUDE_REF:
    case EXIF_TAG_GPS_TIME_STAMP:
    case EXIF_TAG_GPS_PROCESSING_METHOD:
    case EXIF_TAG_GPS_DATE_STAMP:
      *exif_ifd = EXIF_IFD_GPS;
      break;

    // Tags in other sections
    default:
      LoggerE("Unsupported tag: %d", tag);
      return common::PlatformResult(common::ErrorCode::UNKNOWN_ERR, "Unsupported tag");
  }

  return common::PlatformResult(common::ErrorCode::NO_ERROR);
}

common::PlatformResult ExifTagSaver::deduceDataFormat(ExifTag tag, ExifFormat* exif_format) {
  LoggerD("Entered");
  // TODO EXIF_TAG_* and EXIF_TAG_GPS_* are sharing same values,
  // they shouldn't be used in one switch statement.

  switch (static_cast<unsigned int>(tag)) {
    // Tags with byte type:
    case EXIF_TAG_GPS_ALTITUDE_REF:
      *exif_format = EXIF_FORMAT_BYTE;
      break;

    // Tags with long type:
    case EXIF_TAG_IMAGE_WIDTH:
    case EXIF_TAG_IMAGE_LENGTH:
      *exif_format = EXIF_FORMAT_LONG;
      break;

    // Tags with short type:
    case EXIF_TAG_ORIENTATION:
    case EXIF_TAG_EXPOSURE_PROGRAM:
    case EXIF_TAG_WHITE_BALANCE:
    case EXIF_TAG_FLASH:
      *exif_format = EXIF_FORMAT_SHORT;
      break;

    // Tags with ASCII type:
    case EXIF_TAG_MAKE:
    case EXIF_TAG_MODEL:
    case EXIF_TAG_DATE_TIME_ORIGINAL:
    case EXIF_TAG_GPS_LATITUDE_REF:
    case EXIF_TAG_GPS_LONGITUDE_REF:
    case EXIF_TAG_GPS_DATE_STAMP:
      *exif_format = EXIF_FORMAT_ASCII;
      break;

    // Tags with rational type:
    case EXIF_TAG_EXPOSURE_TIME:
    case EXIF_TAG_FNUMBER:
    case EXIF_TAG_FOCAL_LENGTH:
    case EXIF_TAG_GPS_LATITUDE:
    case EXIF_TAG_GPS_LONGITUDE:
    case EXIF_TAG_GPS_ALTITUDE:
    case EXIF_TAG_GPS_TIME_STAMP:
    case EXIF_TAG_ISO_SPEED_RATINGS:
      *exif_format = EXIF_FORMAT_RATIONAL;
      break;

    // Tags with undefined type:
    case EXIF_TAG_USER_COMMENT:
    case EXIF_TAG_GPS_PROCESSING_METHOD:
      *exif_format = EXIF_FORMAT_UNDEFINED;
      break;

    // Unsupported tags:
    default:
      LoggerE("Unsupported tag: %d", tag);
      return common::PlatformResult(common::ErrorCode::UNKNOWN_ERR, "Unsupported tag");
  }

  return common::PlatformResult(common::ErrorCode::NO_ERROR);
}

}  // namespace exif
}  // namespace extension
