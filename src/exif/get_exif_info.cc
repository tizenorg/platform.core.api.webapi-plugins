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

#include "exif/get_exif_info.h"

#include <math.h>
#include <memory>
#include <string>
#include <utility>

#include "common/platform_result.h"
#include "common/logger.h"

#include "exif/exif_util.h"

namespace extension {
namespace exif {

using common::PlatformResult;
using common::ErrorCode;

struct ExifDataHolder {
  ExifData* exif_data;
  JsonObject* result_obj_ptr;
};

Rational GetRationalFromEntry(ExifEntry *entry, ExifData* exif_data) {
  LoggerD("Entered");
  if (EXIF_FORMAT_RATIONAL == entry->format
      && entry->components >= 1
      && entry->data) {
    const ExifByteOrder order = exif_data_get_byte_order(exif_data);
    return Rational(exif_get_rational(entry->data, order));
  } else {
    return Rational::createInvalid();
  }
}

bool GetRationalsFromEntry(ExifEntry* entry, ExifData* exif_data,
                           unsigned long required_count,
                           Rationals& out_rationals) {
  LoggerD("Entered");
  if (EXIF_FORMAT_RATIONAL == entry->format &&
      entry->components >= required_count &&
      entry->data) {
    const ExifByteOrder order = exif_data_get_byte_order(exif_data);
    unsigned char* ptr = entry->data;

    for (unsigned long i = 0; i < required_count; ++i) {
      out_rationals.push_back(Rational(exif_get_rational(ptr, order)));
      ptr += ExifTypeInfo::RationalSize;
    }

    return true;
  } else {
    return false;
  }
}

bool GetGCSPositionFromEntry(ExifEntry* entry, ExifData* exif_data,
                             GCSPosition& out_pos) {
  // RATIONAL - 3
  LoggerD("Entered");
  if (EXIF_FORMAT_RATIONAL == entry->format &&
      entry->components >= 3 &&
      entry->data) {
    const ExifByteOrder order = exif_data_get_byte_order(exif_data);
    out_pos.degrees = Rational(exif_get_rational(entry->data, order));
    out_pos.minutes = Rational(exif_get_rational(
        entry->data +   ExifTypeInfo::RationalSize, order));
    out_pos.seconds = Rational(exif_get_rational(
        entry->data + 2*ExifTypeInfo::RationalSize, order));
    return true;
  } else {
    return false;
  }
}

bool DecomposeExifUndefined(ExifEntry* entry, std::string& type, std::string& value) {
  LoggerD("Entered");
  if (!entry || !entry->data) {
    LoggerW("exif entry is NULL/empty");
    return false;
  }

  if (entry->size < EXIF_UNDEFINED_TYPE_LENGTH) {
    LoggerW("entry size is invalid %d < EXIF_UNDEFINED_TYPE_LENGTH", entry->size);
    return false;
  }

  const char* ptr = reinterpret_cast<const char*>(entry->data);
  type = std::string(ptr, EXIF_UNDEFINED_TYPE_LENGTH);
  ptr += EXIF_UNDEFINED_TYPE_LENGTH;
  value = std::string(ptr, entry->size - EXIF_UNDEFINED_TYPE_LENGTH);
  return true;
}

PlatformResult GetExifInfo::ProcessEntry(ExifEntry* entry,
                               ExifData* exif_data,
                               JsonObject* result_obj) {
  LoggerD("Entered");
  char buf[2000];
  exif_entry_get_value(entry, buf, sizeof(buf));
  ExifUtil::printExifEntryInfo(entry, exif_data);

  const ExifIfd cur_ifd = exif_entry_get_ifd(entry);
  if (EXIF_IFD_INTEROPERABILITY == cur_ifd || EXIF_IFD_1 == cur_ifd) {
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  std::pair<std::string, JsonValue> pair;
  switch (static_cast<unsigned int>(entry->tag)) {
    case EXIF_TAG_IMAGE_WIDTH: {
      exif_entry_get_value(entry, buf, sizeof(buf));
      LoggerD("Setting ExifInformation width to: [%s]", buf);
      pair = std::make_pair("width", JsonValue(std::string(buf)));
      result_obj->insert(pair);
      break;
    }
    case EXIF_TAG_IMAGE_LENGTH: {
      exif_entry_get_value(entry, buf, sizeof(buf));
      LoggerD("Setting ExifInformation height to: [%s]", buf);
      pair = std::make_pair("height", JsonValue(std::string(buf)));
      result_obj->insert(pair);
      break;
    }
    case EXIF_TAG_MAKE: {
      exif_entry_get_value(entry, buf, sizeof(buf));
      LoggerD("Setting ExifInformation maker to: [%s]", buf);
      pair = std::make_pair("deviceMaker", JsonValue(std::string(buf)));
      result_obj->insert(pair);
      break;
    }
    case EXIF_TAG_MODEL: {
      exif_entry_get_value(entry, buf, sizeof(buf));
      LoggerD("Setting ExifInformation model to: [%s]", buf);
      pair = std::make_pair("deviceModel", JsonValue(std::string(buf)));
      result_obj->insert(pair);
      break;
    }
    case EXIF_TAG_DATE_TIME_ORIGINAL: {
      // ASCII - 20
      exif_entry_get_value(entry, buf, sizeof(buf));
      const time_t time = ExifUtil::exifDateTimeOriginalToTimeT(
          reinterpret_cast<const char*>(entry->data));
      LoggerD("Setting ExifInformation time original to: [%s] time_t:%d", buf,
          static_cast<int>(time));
      // convert time_t (number of seconds) to string
      pair = std::make_pair("originalTimeSeconds",
          JsonValue(static_cast<double>(time)));
      result_obj->insert(pair);
      break;
    }
    case EXIF_TAG_ORIENTATION: {
      // SHORT - 1
      exif_entry_get_value(entry, buf, sizeof(buf));
      const ExifByteOrder order = exif_data_get_byte_order(exif_data);
      const ExifShort orient(exif_get_short(entry->data, order));

      const std::string& orientation = ExifUtil::orientationToString(
          static_cast<ImageOrientation>(orient));
      pair = std::make_pair("orientation", JsonValue(orientation));
      result_obj->insert(pair);

      if (orient < EXIF_ORIENTATION_NORMAL || orient >= EXIF_ORIENTATION_NOT_VALID) {
        LoggerW("Couldn't set ExifInformation - orientation is not valid: %d (%s)",
            orient, buf);
      } else {
        LoggerD("Setting ExifInformation orientation to: %d [%s]", orient, buf);
      }
      break;
    }
    case EXIF_TAG_FNUMBER:
    {
      // RATIONAL - 1
      Rational fnumber = GetRationalFromEntry(entry, exif_data);
      if (fnumber.isValid()) {
        LoggerD("Setting ExifInformation fnumber to: %f (%s)", fnumber.toDouble(),
          fnumber.toString().c_str());
        pair = std::make_pair("fNumber", JsonValue(fnumber.toDouble()));
        result_obj->insert(pair);
      } else {
        LoggerW("Couldn't set ExifInformation - fnumber is not valid: %s",
            fnumber.toString().c_str());
      }
      break;
    }
    case EXIF_TAG_ISO_SPEED_RATINGS: {
      // SHORT - Any
      if (EXIF_FORMAT_SHORT == entry->format &&
          entry->components > 0 &&
          entry->data) {
        const ExifByteOrder order = exif_data_get_byte_order(exif_data);
        unsigned char* read_ptr = entry->data;
        const std::size_t size_per_member =
            ExifUtil::getSizeOfExifFormatType(entry->format);

        JsonArray array = JsonArray();
        for (unsigned long i = 0; i < entry->components; ++i) {
          ExifShort iso_rating = exif_get_short(read_ptr, order);
          array.push_back(JsonValue(std::to_string(iso_rating)));

          LoggerD("Appending ExifInformation speed ratings with: %d",
              static_cast<int>(iso_rating));

          read_ptr += size_per_member;
        }
        pair = std::make_pair("isoSpeedRatings", JsonValue(array));
        result_obj->insert(pair);
      } else {
        LoggerE("iso speed ratings: format or components count is invalid!");
        return PlatformResult(ErrorCode::TYPE_MISMATCH_ERR,
            "iso speed ratings: format or components count is invalid!");
      }
      break;
    }
    case EXIF_TAG_EXPOSURE_TIME: {
      // RATIONAL - 1
      if (EXIF_FORMAT_RATIONAL == entry->format &&
          entry->components > 0 &&
          entry->data) {
        const ExifByteOrder order = exif_data_get_byte_order(exif_data);
        const Rational exp_time(exif_get_rational(entry->data, order));

        if (exp_time.isValid()) {
          LoggerD("Setting ExifInformation exposure time to: %s (%s)",
              exp_time.toString().c_str(),
              exp_time.toExposureTimeString().c_str());
          pair = std::make_pair("exposureTime", JsonValue(exp_time.toDouble()));
          result_obj->insert(pair);
        } else {
          LoggerD("Couldn't set ExifInformation - exposure time is not valid: %s",
              exp_time.toString().c_str());
        }
      } else {
        LoggerE("exposure time: format or components count is invalid!");
        return PlatformResult(ErrorCode::TYPE_MISMATCH_ERR,
            "exposure time: format or components count is invalid!");
      }
      break;
    }
    case EXIF_TAG_EXPOSURE_PROGRAM: {
      // SHORT - 1
      exif_entry_get_value(entry, buf, sizeof(buf));

      const ExifByteOrder order = exif_data_get_byte_order(exif_data);
      const ExifShort exp_program = exif_get_short(entry->data, order);
      if (exp_program >= EXIF_EXPOSURE_PROGRAM_NOT_VALID) {
        LoggerW("ExposureProgram: %d (%s) is not valid!", exp_program, buf);
      } else {
        LoggerD("Setting ExifInformation exposure program to: %d [%s]",
            exp_program, buf);
        std::string exp_program_string =
            ExifUtil::exposureProgramToString(static_cast<ExposureProgram>(exp_program));
        pair = std::make_pair("exposureProgram", JsonValue(exp_program_string));
        result_obj->insert(pair);
      }
      break;
    }
    case EXIF_TAG_FLASH: {
      // SHORT - 1
      exif_entry_get_value(entry, buf, sizeof(buf));

      const ExifByteOrder order = exif_data_get_byte_order(exif_data);
      const ExifShort flash = exif_get_short(entry->data, order);

      LoggerD("Setting ExifInformation flash to: [%s] flash=%d", buf, flash);
      pair = std::make_pair("flash", JsonValue((flash != 0) ? "true" : "false"));
      result_obj->insert(pair);
      break;
    }
    case EXIF_TAG_FOCAL_LENGTH: {
      // RATIONAL - 1
      Rational flength = GetRationalFromEntry(entry, exif_data);
      if (flength.isValid()) {
        LoggerD("Setting ExifInformation focal length to: %f (%s)",
            flength.toDouble(), flength.toString().c_str());
        pair = std::make_pair("focalLength", JsonValue(flength.toDouble()));
        result_obj->insert(pair);
      } else {
        LoggerW("Couldn't set ExifInformation - focal length is not valid: %s",
            flength.toString().c_str());
      }
      break;
    }
    case EXIF_TAG_WHITE_BALANCE: {
      // SHORT - 1
      exif_entry_get_value(entry, buf, sizeof(buf));
      LoggerD("Setting ExifInformation white balance to: [%s]", buf);
      pair = std::make_pair("whiteBalanceValue",
          JsonValue(static_cast<double>(entry->data[0])));
      result_obj->insert(pair);
      break;
    }
    case EXIF_TAG_GPS_LONGITUDE: {
      // RATIONAL - 3
      GCSPosition longitude;
      if (GetGCSPositionFromEntry(entry, exif_data, longitude)) {
        pair = std::make_pair("gpsLongitudeDegrees", JsonValue(longitude.degrees.toDouble()));
        result_obj->insert(pair);
        pair = std::make_pair("gpsLongitudeMinutes", JsonValue(longitude.minutes.toDouble()));
        result_obj->insert(pair);
        pair = std::make_pair("gpsLongitudeSeconds", JsonValue(longitude.seconds.toDouble()));
        result_obj->insert(pair);
        LoggerD("Setting ExifInformation gps longitude to: %s; %s; %s valid:%d",
            longitude.degrees.toString().c_str(),
            longitude.minutes.toString().c_str(),
            longitude.seconds.toString().c_str(),
            longitude.isValid());
      } else {
        LoggerW("Couldn't set longitude pos - data is not valid.");
      }
      break;
    }
    case EXIF_TAG_GPS_LONGITUDE_REF: {
      // ASCII - 2
      if (entry->size < 1) {
        LoggerW("Longitude ref entry do not contain enough data!");
        break;
      }

      const char ref = static_cast<char>(entry->data[0]);
      if ('E' == ref || 'e' == ref) {      // East
        pair = std::make_pair("gpsLongitudeRef", JsonValue("EAST"));
        result_obj->insert(pair);
        LoggerD("Setting ExifInformation gps longitude REF to: EAST");
      } else if ('W' == ref || 'w' == ref) {   // West
        pair = std::make_pair("gpsLongitudeRef", JsonValue("WEST"));
        result_obj->insert(pair);
        LoggerD("Setting ExifInformation gps longitude REF to: WEST");
      } else {
        LoggerW("Unknown longitude ref: %c (0x%x)", ref, static_cast<int>(ref));
      }
      break;
    }
    case EXIF_TAG_GPS_LATITUDE: {
      // RATIONAL - 3
      exif_entry_get_value(entry, buf, sizeof(buf));
      LoggerD("Setting ExifInformation latitude to: [%s], tag->%s",
          buf, exif_tag_get_name(entry->tag) );

      GCSPosition latitude;
      if (GetGCSPositionFromEntry(entry, exif_data, latitude)) {
        pair = std::make_pair("gpsLatitudeDegrees", JsonValue(latitude.degrees.toDouble()));
        result_obj->insert(pair);
        pair = std::make_pair("gpsLatitudeMinutes", JsonValue(latitude.minutes.toDouble()));
        result_obj->insert(pair);
        pair = std::make_pair("gpsLatitudeSeconds", JsonValue(latitude.seconds.toDouble()));
        result_obj->insert(pair);

        LoggerD("Setting ExifInformation gps latitude to: %s; %s; %s valid:%d",
            latitude.degrees.toString().c_str(),
            latitude.minutes.toString().c_str(),
            latitude.seconds.toString().c_str(),
            latitude.isValid());
      } else {
        LoggerW("Couldn't set latitude pos - data is not valid!");
      }
      break;
    }
    case EXIF_TAG_GPS_LATITUDE_REF: {
      // ASCII - 2
      if (entry->size < 1) {
        LoggerW("Latitude ref entry do not contain enough data!");
        break;
      }

      const char ref = static_cast<char>(entry->data[0]);
      if ('N' == ref || 'n' == ref) {      // North
        pair = std::make_pair("gpsLatitudeRef", JsonValue("NORTH"));
        result_obj->insert(pair);
        LoggerD("Setting ExifInformation gps latitude REF to: NORTH");
      } else if ('S' == ref || 's' == ref) {   // South
        pair = std::make_pair("gpsLatitudeRef", JsonValue("SOUTH"));
        result_obj->insert(pair);
        LoggerD("Setting ExifInformation gps latitude REF to: SOUTH");
      } else {
        LoggerW("Unknown latitude ref: %c (0x%x)", ref, static_cast<int>(ref));
      }
      break;
    }
    case EXIF_TAG_GPS_ALTITUDE: {
      // RATIONAL - 1
      Rational gps_altitude = GetRationalFromEntry(entry, exif_data);
      if (gps_altitude.isValid()) {
        LoggerD("Setting ExifInformation gps altitude to: %f (%s)",
            gps_altitude.toDouble(), gps_altitude.toString().c_str());
        pair = std::make_pair("gpsAltitude", JsonValue(gps_altitude.toDouble()));
        result_obj->insert(pair);
      } else {
        LoggerW("Couldn't set ExifInformation - gps altitude is not valid: %s",
            gps_altitude.toString().c_str());
      }
      break;
    }
    case EXIF_TAG_GPS_ALTITUDE_REF: {
      // BYTE - 1
      const ExifByte altitude_ref = static_cast<ExifByte>(entry->data[0]);
      pair = std::make_pair("gpsAltitudeRef", JsonValue(static_cast<double>(altitude_ref)));
      result_obj->insert(pair);
      LoggerD("Setting ExifInformation gps altitude ref to: %d (%s)",
            static_cast<int>(altitude_ref),
            (altitude_ref > 0) ? "below sea level" : "above sea level");

      break;
    }
    case EXIF_TAG_GPS_PROCESSING_METHOD: {
      // UNDEFINED - Any
      std::string type, value;
      if (DecomposeExifUndefined(entry, type, value)) {
        LoggerD("Extracted GPSProcessingMethod: [%s], len:%d, type:%s",
            value.c_str(), value.length(), type.c_str());
        pair = std::make_pair("gpsProcessingMethod", JsonValue(value));
        result_obj->insert(pair);
      } else {
        LoggerW("GPSProcessingMethod tag contains invalid values!");
      }
      break;
    }
    case EXIF_TAG_GPS_DATE_STAMP: {
      // ASCII - 11
      pair = std::make_pair("gpsExifDate", JsonValue(std::string(buf)));
      result_obj->insert(pair);
      LoggerD("Setting ExifInformation gps date stamp to %s", std::string(buf).c_str());
      break;
    }
    case EXIF_TAG_GPS_TIME_STAMP: {
      // Rational - 3
      LoggerD("Setting ExifInformation gps time stamp to: [%s]", buf);

      Rationals time;
      if (GetRationalsFromEntry(entry, exif_data, 3, time)) {
        pair = std::make_pair("gpsExifTimeHours", JsonValue(time[0].toDouble()));
        result_obj->insert(pair);
        pair = std::make_pair("gpsExifTimeMinutes", JsonValue(time[1].toDouble()));
        result_obj->insert(pair);
        pair = std::make_pair("gpsExifTimeSeconds", JsonValue(time[2].toDouble()));
        result_obj->insert(pair);
      }
      break;
    }
    case EXIF_TAG_USER_COMMENT: {
      // UNDEFINED - Any
      std::string type, value;
      if (DecomposeExifUndefined(entry, type, value)) {
        LoggerD("Extracted UserComment: [%s], len:%d, type:%s",
            value.c_str(), value.length(), type.c_str());

        pair = std::make_pair("userComment", JsonValue(value));
        result_obj->insert(pair);
      } else {
        LoggerW("UserComment tag contains invalid values!");
      }
      break;
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

void GetExifInfo::ContentForeachFunctionProxy(ExifEntry *entry, void *user_data) {
  LoggerD("Entered");
  ExifDataHolder* holder = static_cast<ExifDataHolder*>(user_data);
  if (!holder) {
    LoggerE("holder is NULL");
    return;
  }

  if (!holder->exif_data) {
    LoggerE("exif_data is NULL!");
    return;
  }

  JsonObject* result_obj_ptr = holder->result_obj_ptr;

  PlatformResult status = ProcessEntry(entry, holder->exif_data, result_obj_ptr);
  if (!status) {
    LoggerE("Unsupported error while processing Exif entry.");
  }
}

void GetExifInfo::DataForeachFunction(ExifContent *content, void *user_data) {
  exif_content_foreach_entry(content, ContentForeachFunctionProxy, user_data);
}

PlatformResult GetExifInfo::LoadFromURI(const std::string& uri,
                                        JsonValue* result) {
  LoggerD("Entered");
  // TODO(r.galka) it can be done on JS side
  const std::string& file_path = ExifUtil::convertUriToPath(uri);
  ExifData* ed = exif_data_new_from_file(file_path.c_str());
  if (!ed) {
    LoggerE("Error reading exif from file %s", file_path.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
            "Error reading exif from file");
  }

  LoggerD("loadFromURI_into_json exif_data_foreach_content START");

  JsonObject& result_obj = result->get<JsonObject>();

  ExifDataHolder holder;
  holder.exif_data = ed;
  holder.result_obj_ptr = &result_obj;
  exif_data_foreach_content(ed, DataForeachFunction,
      static_cast<void *>(&holder));

  LoggerD("loadFromURI_into_json exif_data_foreach_content END");

  exif_data_unref(ed);

  // uri is not taken from jgp Exif, so we add it here
  holder.result_obj_ptr->insert(std::make_pair("uri", JsonValue(uri)));

  return PlatformResult(ErrorCode::NO_ERROR);
}

}  // namespace exif
}  // namespace extension
