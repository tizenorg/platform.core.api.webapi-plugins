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

#include "exif/exif_information.h"

#include <memory>
#include <cmath>

#include "common/assert.h"
#include "common/converter.h"
#include "common/logger.h"
#include "common/platform_result.h"

#include "exif/exif_tag_saver.h"
#include "exif/exif_util.h"
#include "exif/jpeg_file.h"

namespace extension {
namespace exif {

using common::ErrorCode;
using common::PlatformResult;

const std::size_t EXIF_UNDEFINED_TYPE_LENGTH = 8;
const std::string EXIF_UNDEFINED_TYPE_ASCII =
    std::string("ASCII\0\0\0", EXIF_UNDEFINED_TYPE_LENGTH);
const std::string EXIF_UNDEFINED_TYPE_JIS =
    std::string("JIS\0\0\0\0\0", EXIF_UNDEFINED_TYPE_LENGTH);
const std::string EXIF_UNDEFINED_TYPE_UNICODE =
    std::string("UNICODE\0", EXIF_UNDEFINED_TYPE_LENGTH);
const std::string EXIF_UNDEFINED_TYPE_UNDEFINED =
    std::string("\0\0\0\0\0\0\0\0", EXIF_UNDEFINED_TYPE_LENGTH);

namespace {
constexpr unsigned int str2int(const char* str, int h = 0) {
  return !str[h] ? 5381 : (str2int(str, h+1)*33) ^ str[h];
}

IsoSpeedRatingsVector jsonArray2vector(const picojson::value& a) {
  LoggerD("Enter");
  if (!a.is<picojson::array>()) {
    return IsoSpeedRatingsVector();
  }

  IsoSpeedRatingsVector result;

  picojson::array v = a.get<picojson::array>();

  for (picojson::array::iterator it = v.begin(); it != v.end(); ++it) {
    if ((*it).is<double>())
      result.push_back(static_cast<long long int>((*it).get<double>()));
  }

  return result;
}
}  // namespace

ExifInformation::ExifInformation() {
  LoggerD("Enter");
  for (int attr = 0;
      attr < EXIF_INFORMATION_ATTRIBUTE_NUMBER_OF_ATTRIBUTES; attr++) {
    unset(static_cast<ExifInformationAttribute>(attr));
  }
}

ExifInformation::ExifInformation(const picojson::value& args) {
  LoggerD("Enter");
  for (int attr = 0;
      attr < EXIF_INFORMATION_ATTRIBUTE_NUMBER_OF_ATTRIBUTES; attr++) {
    unset(static_cast<ExifInformationAttribute>(attr));
  }

  for (AttributeMap::const_iterator it = ExifInformationAttributeMap.begin();
      it != ExifInformationAttributeMap.end(); ++it) {
    std::string attributeName = it->second;
    picojson::value v = args.get(attributeName);
    if (!common::IsNull(v)) {
      set(attributeName, v);
    }
  }
}

ExifInformation::~ExifInformation() {
  LoggerD("Enter");
 }

const std::string& ExifInformation::getUri() {
  LoggerD("Entered");
  return m_uri;
}

void ExifInformation::setUri(const std::string& uri) {
  LoggerD("Entered");
  LoggerD("URI: %s", uri.c_str());
  m_is_set[EXIF_INFORMATION_ATTRIBUTE_URI] = true;
  m_uri = uri;
}

unsigned long ExifInformation::getWidth() const {
  LoggerD("Entered");
  return m_width;
}

void ExifInformation::setWidth(unsigned long width) {
  LoggerD("Entered");
  m_is_set[EXIF_INFORMATION_ATTRIBUTE_WIDTH] = true;
  m_width = width;
}

unsigned long ExifInformation::getHeight() const {
  LoggerD("Entered");
  return m_height;
}

void ExifInformation::setHeight(unsigned long height) {
  LoggerD("Entered");
  m_is_set[EXIF_INFORMATION_ATTRIBUTE_HEIGHT] = true;
  m_height = height;
}

const std::string& ExifInformation::getDeviceMaker() {
  LoggerD("Entered");
  return m_device_maker;
}

void ExifInformation::setDeviceMaker(const std::string& device_maker) {
  LoggerD("Entered");
  m_is_set[EXIF_INFORMATION_ATTRIBUTE_DEVICE_MAKER] = true;
  m_device_maker = device_maker;
}

const std::string& ExifInformation::getDeviceModel() {
  LoggerD("Entered");
  return m_device_model;
}

void ExifInformation::setDeviceModel(const std::string& device_model) {
  LoggerD("Entered");
  m_is_set[EXIF_INFORMATION_ATTRIBUTE_DEVICE_MODEL] = true;
  m_device_model = device_model;
}

time_t ExifInformation::getOriginalTime() const {
  LoggerD("Entered");
  return m_original_time;
}

void ExifInformation::setOriginalTime(time_t original_time) {
  LoggerD("Entered");
  m_is_set[EXIF_INFORMATION_ATTRIBUTE_ORIGINAL_TIME] = true;
  m_original_time = original_time;
}

const std::string& ExifInformation::getOrientationString() {
  LoggerD("Entered");
  return ExifUtil::orientationToString(m_orientation);
}

ImageOrientation ExifInformation::getOrientation() {
  LoggerD("Entered");
  return m_orientation;
}

void ExifInformation::setOrientation(const std::string& orientation) {
  LoggerD("Entered");
  setOrientation(ExifUtil::stringToOrientation(orientation));
}

void ExifInformation::setOrientation(ImageOrientation orientation) {
  LoggerD("Entered");
  m_is_set[EXIF_INFORMATION_ATTRIBUTE_ORIENTATION] = true;
  m_orientation = orientation;
}

const Rational& ExifInformation::getFNumber() const {
  LoggerD("Entered");
  return m_f_number;
}

void ExifInformation::setFNumber(Rational f_number) {
  LoggerD("Entered");
  if (!f_number.isValid()) {
    LoggerW("Trying to set invalid F-Number: %s", f_number.toString().c_str());
    return;
  }

  m_is_set[EXIF_INFORMATION_ATTRIBUTE_FNUMBER] = true;
  m_f_number = f_number;
}

const std::vector<long long int>& ExifInformation::getIsoSpeedRatings() {
  LoggerD("Entered");
  return m_iso_speed_ratings;
}

void ExifInformation::setIsoSpeedRatings(
const std::vector<long long int>& iso_speed_ratings) {
  LoggerD("Entered");
  m_is_set[EXIF_INFORMATION_ATTRIBUTE_ISO_SPEED_RATINGS] = true;
  m_iso_speed_ratings = iso_speed_ratings;
}

const Rational& ExifInformation::getExposureTime() {
  LoggerD("Entered");
  return m_exposure_time;
}

void ExifInformation::setExposureTime(const Rational& exposure_time) {
  LoggerD("Entered");
  if (!exposure_time.isValid() || 0 == exposure_time.nominator) {
    LoggerW("Trying to set invalid exposure time: [%s]",
        exposure_time.toString().c_str());
    return;
  }

  m_is_set[EXIF_INFORMATION_ATTRIBUTE_EXPOSURE_TIME] = true;
  m_exposure_time = exposure_time;
}

const std::string& ExifInformation::getExposureProgramString() {
  LoggerD("Entered");
  return ExifUtil::exposureProgramToString(m_exposure_program);;
}

ExposureProgram ExifInformation::getExposureProgram() {
  LoggerD("Entered");
  return m_exposure_program;
}

void ExifInformation::setExposureProgram(const std::string& exposure_program) {
  LoggerD("Entered");
  setExposureProgram(ExifUtil::stringToExposureProgram(exposure_program));
}

void ExifInformation::setExposureProgram(ExposureProgram exposure_program) {
  LoggerD("Entered");
  m_is_set[EXIF_INFORMATION_ATTRIBUTE_EXPOSURE_PROGRAM] = true;
  m_exposure_program = exposure_program;
}

bool ExifInformation::getFlash() const {
  LoggerD("Entered");
  return m_flash;
}

void ExifInformation::setFlash(bool flash) {
  LoggerD("Entered");
  m_is_set[EXIF_INFORMATION_ATTRIBUTE_FLASH] = true;
  m_flash = flash;
}

const Rational& ExifInformation::getFocalLength() const {
  LoggerD("Entered");
  return m_focal_length;
}

void ExifInformation::setFocalLength(Rational focal_length) {
  LoggerD("Entered");
  if (!focal_length.isValid()) {
    LoggerW("Trying to set invalid focal length: %s",
        focal_length.toString().c_str());
    return;
  }

  m_is_set[EXIF_INFORMATION_ATTRIBUTE_FOCAL_LENGTH] = true;
  m_focal_length = focal_length;
}

const std::string& ExifInformation::getWhiteBalanceModeString() {
  LoggerD("Entered");
  return ExifUtil::whiteBalanceToString(m_white_balance);
}

WhiteBalanceMode ExifInformation::getWhiteBalanceMode() {
  LoggerD("Entered");
  return m_white_balance;
}

void ExifInformation::setWhiteBalanceMode(const std::string& white_balance) {
  LoggerD("Entered");
  setWhiteBalanceMode(ExifUtil::stringToWhiteBalance(white_balance));
}

void ExifInformation::setWhiteBalanceMode(WhiteBalanceMode white_balance) {
  LoggerD("Entered");
  m_is_set[EXIF_INFORMATION_ATTRIBUTE_WHITE_BALANCE] = true;
  m_white_balance = white_balance;
}

ExifGPSLocation& ExifInformation::getGPSExifLocation() {
  LoggerD("Entered");
  return m_gps_location;
}

void ExifInformation::setGPSLocation(ExifGPSLocation gps_location) {
  LoggerD("Entered");
  m_is_set[EXIF_INFORMATION_ATTRIBUTE_GPS_LOCATION] = true;
  m_gps_location = gps_location;
}

void ExifInformation::unsetGPSLocation() {
  LoggerD("Entered");
  m_is_set[EXIF_INFORMATION_ATTRIBUTE_GPS_LOCATION] = false;
  m_gps_location.unsetAll();
}

const Rational& ExifInformation::getGpsAltitude() const {
  LoggerD("Entered");
  return m_gps_altitude;
}

void ExifInformation::setGpsAltitude(Rational gps_altitude) {
  LoggerD("Entered");
  if (!gps_altitude.isValid()) {
    LoggerW("Trying to set invalid gps altitude: %s",
        gps_altitude.toString().c_str());
    return;
  }

  m_is_set[EXIF_INFORMATION_ATTRIBUTE_GPS_ALTITUDE] = true;
  m_gps_altitude = gps_altitude;
}

GpsAltitudeRef ExifInformation::getGpsAltitudeRef() const {
  LoggerD("Entered");
  return m_gps_altitude_ref;
}

void ExifInformation::setGpsAltitudeRef(const GpsAltitudeRef ref) {
  LoggerD("Entered");
  m_is_set[EXIF_INFORMATION_ATTRIBUTE_GPS_ALTITUDE_REF] = true;
  m_gps_altitude_ref = ref;
}

void ExifInformation::setGpsAltitudeWithRef(double gps_altitude) {
  LoggerD("Entered");
  setGpsAltitude(Rational::createFromDouble(fabs(gps_altitude)));

  if (gps_altitude >= 0.0) {
    setGpsAltitudeRef(GPS_ALTITUDE_REF_ABOVE_SEA);
  } else {
    setGpsAltitudeRef(GPS_ALTITUDE_REF_BELOW_SEA);
  }
}

double ExifInformation::getGpsAltitudeWithRef() const {
  LoggerD("Entered");

  if (!m_is_set[EXIF_INFORMATION_ATTRIBUTE_GPS_ALTITUDE_REF] ||
      GPS_ALTITUDE_REF_ABOVE_SEA == m_gps_altitude_ref) {
    return m_gps_altitude.toDouble();
  } else {
    return -1.0 * m_gps_altitude.toDouble();
  }
}

const std::string& ExifInformation::getGpsProcessingMethod() const {
  LoggerD("Entered");
  return m_gps_processing_method;
}

const std::string& ExifInformation::getGpsProcessingMethodType() const {
  LoggerD("Entered");
  return m_gps_processing_method_type;
}

void ExifInformation::setGpsProcessingMethod(const std::string& type,
    const std::string& processing_method) {
  LoggerD("Entered");
  if (type != EXIF_UNDEFINED_TYPE_ASCII &&
      type != EXIF_UNDEFINED_TYPE_JIS &&
      type != EXIF_UNDEFINED_TYPE_UNICODE &&
      type != EXIF_UNDEFINED_TYPE_UNDEFINED) {
    LoggerW("Trying to set invalid GPSProcessingMethod type: [%s] len:%d",
        type.c_str(), type.length());
    return;
  }

  m_is_set[EXIF_INFORMATION_ATTRIBUTE_GPS_PROCESSING_METHOD] = true;
  m_gps_processing_method = processing_method;
  m_gps_processing_method_type = type;
}

void ExifInformation::setGpsTime(time_t time) {
  LoggerD("Entered");
  m_is_set[EXIF_INFORMATION_ATTRIBUTE_GPS_TIME] = true;
  m_gps_time = time;
}

time_t ExifInformation::getGpsTime() {
  LoggerD("Entered");
  return m_gps_time;
}

void ExifInformation::unsetGPStime() {
  LoggerD("Entered");
  m_is_set[EXIF_INFORMATION_ATTRIBUTE_GPS_TIME] = false;
  m_gps_time = 0;
}

const std::string& ExifInformation::getUserComment() {
  LoggerD("Entered");
  return m_user_comment;
}

const std::string& ExifInformation::getUserCommentType() {
  LoggerD("Entered");
  return m_user_comment_type;
}

void ExifInformation::setUserComment(const std::string& type,
    const std::string& user_comment) {
  LoggerD("Entered");
  if (type != EXIF_UNDEFINED_TYPE_ASCII &&
      type != EXIF_UNDEFINED_TYPE_JIS &&
      type != EXIF_UNDEFINED_TYPE_UNICODE &&
      type != EXIF_UNDEFINED_TYPE_UNDEFINED) {
    LoggerW("Trying to set invalid user comment type: [%s] len:%d",
        type.c_str(), type.length());
    return;
  }

  m_is_set[EXIF_INFORMATION_ATTRIBUTE_USER_COMMENT] = true;
  m_user_comment_type = type;
  m_user_comment = user_comment;
}

bool ExifInformation::isSet(ExifInformationAttribute attribute) const {
  LoggerD("Entered");
  return m_is_set[attribute];
}

void ExifInformation::unset(ExifInformationAttribute attribute) {
  LoggerD("Entered");
  if (attribute >= EXIF_INFORMATION_ATTRIBUTE_NUMBER_OF_ATTRIBUTES) {
    return;
  }

  m_is_set[attribute] = false;
  switch (attribute) {
    case EXIF_INFORMATION_ATTRIBUTE_URI:
      m_uri = std::string();
      break;
    case EXIF_INFORMATION_ATTRIBUTE_WIDTH:
      m_width = 0;
      break;
    case EXIF_INFORMATION_ATTRIBUTE_HEIGHT:
      m_height = 0;
      break;
    case EXIF_INFORMATION_ATTRIBUTE_DEVICE_MAKER:
      m_device_maker = std::string();
      break;
    case EXIF_INFORMATION_ATTRIBUTE_DEVICE_MODEL:
      m_device_model = std::string();
      break;
    case EXIF_INFORMATION_ATTRIBUTE_ORIGINAL_TIME:
      m_original_time = 0;
      break;
    case EXIF_INFORMATION_ATTRIBUTE_ORIENTATION:
      m_orientation = EXIF_ORIENTATION_NOT_VALID;
      break;
    case EXIF_INFORMATION_ATTRIBUTE_FNUMBER:
      m_f_number = Rational::createInvalid();
      break;
    case EXIF_INFORMATION_ATTRIBUTE_ISO_SPEED_RATINGS:
      m_iso_speed_ratings = std::vector<long long int>();
      break;
    case EXIF_INFORMATION_ATTRIBUTE_EXPOSURE_TIME:
      m_exposure_time = Rational::createInvalid();
      break;
    case EXIF_INFORMATION_ATTRIBUTE_EXPOSURE_PROGRAM:
      m_exposure_program = EXIF_EXPOSURE_PROGRAM_NOT_VALID;
      break;
    case EXIF_INFORMATION_ATTRIBUTE_FLASH:
      m_flash = false;
      break;
    case EXIF_INFORMATION_ATTRIBUTE_FOCAL_LENGTH:
      m_focal_length = Rational::createInvalid();
      break;
    case EXIF_INFORMATION_ATTRIBUTE_WHITE_BALANCE:
      m_white_balance = EXIF_WHITE_BALANCE_MODE_NOT_VALID;
      break;
    case EXIF_INFORMATION_ATTRIBUTE_GPS_LOCATION:
      unsetGPSLocation();
      break;
    case EXIF_INFORMATION_ATTRIBUTE_GPS_ALTITUDE:
      m_gps_altitude = Rational::createInvalid();
      break;
    case EXIF_INFORMATION_ATTRIBUTE_GPS_ALTITUDE_REF:
      m_gps_altitude_ref = GPS_ALTITUDE_REF_ABOVE_SEA;
      break;
    case EXIF_INFORMATION_ATTRIBUTE_GPS_PROCESSING_METHOD:
      m_gps_processing_method = std::string();
      m_gps_processing_method_type = EXIF_UNDEFINED_TYPE_ASCII;
      break;
    case EXIF_INFORMATION_ATTRIBUTE_GPS_TIME:
      unsetGPStime();
      break;
    case EXIF_INFORMATION_ATTRIBUTE_USER_COMMENT:
      m_user_comment = std::string();
      m_user_comment_type = EXIF_UNDEFINED_TYPE_ASCII;
      break;
    default:
      break;
  }
}

void ExifInformation::set(std::string attributeName, const picojson::value& v) {
  LoggerD("Entered | name: %s", attributeName.c_str());

  switch (str2int(attributeName.c_str())) {
    case str2int(EI_URI): {
      setUri(v.get<std::string>());
      break;
    }
    case str2int(EI_WIDTH): {
      setWidth(static_cast<int>(v.get<double>()));
      break;
    }
    case str2int(EI_HEIGHT): {
      setHeight(static_cast<int>(v.get<double>()));
      break;
    }
    case str2int(EI_DEVICE_MAKER): {
      setDeviceMaker(v.get<std::string>());
      break;
    }
    case str2int(EI_DEVICE_MODEL): {
      setDeviceModel(v.get<std::string>());
      break;
    }
    case str2int(EI_ORIGINAL_TIME): {
      setOriginalTime(static_cast<unsigned long long>(v.get<double>()));
    break;
    }
    case str2int(EI_ORIENTATION): {
      setOrientation(v.get<std::string>());
      break;
    }
    case str2int(EI_FNUMBER): {
      setFNumber(Rational::createFromDouble(v.get<double>()));
      break;
    }
    case str2int(EI_ISO_SPEED_RATINGS): {
      setIsoSpeedRatings(jsonArray2vector(v));
      break;
    }
    case str2int(EI_EXPOSURE_TIME): {
      setExposureTime(
        Rational::createFromExposureTimeString(v.get<std::string>()));
      break;
    }
    case str2int(EI_EXPOSURE_PROGRAM): {
      setExposureProgram(v.get<std::string>());
      break;
    }
    case str2int(EI_FLASH): {
      setFlash(v.get<bool>());
      break;
    }
    case str2int(EI_FOCAL_LENGTH): {
      setFocalLength(Rational::createFromDouble(v.get<double>()));
      break;
    }
    case str2int(EI_WHITE_BALANCE): {
      setWhiteBalanceMode(v.get<std::string>());
      break;
    }
    case str2int(EI_GPS_LOCATION): {
      setGPSLocation(ExifGPSLocation(v.get("longitude").get<double>(),
       v.get("latitude").get<double>()));
      break;
    }
    case str2int(EI_GPS_ALTITUDE): {
      setGpsAltitudeWithRef(v.get<double>());
      break;
    }
    case str2int(EI_GPS_PROCESSING_METHOD): {
      setGpsProcessingMethod(EXIF_UNDEFINED_TYPE_ASCII, v.get<std::string>());
      break;
    }
    case str2int(EI_GPS_TIME): {
      setGpsTime(static_cast<unsigned long long>(v.get<double>()));
      break;
    }
    case str2int(EI_USER_COMMENT): {
      setUserComment(EXIF_UNDEFINED_TYPE_ASCII, v.get<std::string>());
      break;
    }
    default:
      break;
  }
}

void ExifInformation::removeNulledAttributesFromExifData(ExifData* exif_data) {
  LoggerD("Entered");
  AssertMsg(exif_data, "exif_data is NULL");

  if (!isSet(EXIF_INFORMATION_ATTRIBUTE_WIDTH)) {
    LoggerD("Removing width");
    ExifTagSaver::removeExifEntryWithTag(EXIF_TAG_IMAGE_WIDTH, exif_data);
  }
  if (!isSet(EXIF_INFORMATION_ATTRIBUTE_HEIGHT)) {
    LoggerD("Removing height");
    ExifTagSaver::removeExifEntryWithTag(EXIF_TAG_IMAGE_LENGTH, exif_data);
  }
  if (!isSet(EXIF_INFORMATION_ATTRIBUTE_DEVICE_MAKER)) {
    LoggerD("Removing device maker");
    ExifTagSaver::removeExifEntryWithTag(EXIF_TAG_MAKE, exif_data);
  }
  if (!isSet(EXIF_INFORMATION_ATTRIBUTE_ORIENTATION)) {
    LoggerD("Removing orientation");
    ExifTagSaver::removeExifEntryWithTag(EXIF_TAG_ORIENTATION, exif_data);
  }
  if (!isSet(EXIF_INFORMATION_ATTRIBUTE_EXPOSURE_PROGRAM)) {
    LoggerD("Removing exposure program");
    ExifTagSaver::removeExifEntryWithTag(EXIF_TAG_EXPOSURE_PROGRAM, exif_data);
  }
  if (!isSet(EXIF_INFORMATION_ATTRIBUTE_ISO_SPEED_RATINGS)) {
    LoggerD("Removing iso speed ratings");
    ExifTagSaver::removeExifEntryWithTag(EXIF_TAG_ISO_SPEED_RATINGS, exif_data);
  }
  if (!isSet(EXIF_INFORMATION_ATTRIBUTE_WHITE_BALANCE)) {
    LoggerD("Removing white balance");
    ExifTagSaver::removeExifEntryWithTag(EXIF_TAG_WHITE_BALANCE, exif_data);
  }
  if (!isSet(EXIF_INFORMATION_ATTRIBUTE_DEVICE_MODEL)) {
    LoggerD("Removing device model");
    ExifTagSaver::removeExifEntryWithTag(EXIF_TAG_MODEL, exif_data);
  }
  if (!isSet(EXIF_INFORMATION_ATTRIBUTE_ORIGINAL_TIME)) {
    LoggerD("Removing original time");
    ExifTagSaver::removeExifEntryWithTag(
        EXIF_TAG_DATE_TIME_ORIGINAL, exif_data);
  }
  if (!isSet(EXIF_INFORMATION_ATTRIBUTE_EXPOSURE_TIME)) {
    LoggerD("Removing exposure time");
    ExifTagSaver::removeExifEntryWithTag(EXIF_TAG_EXPOSURE_TIME, exif_data);
  }
  if (!isSet(EXIF_INFORMATION_ATTRIBUTE_FNUMBER)) {
    LoggerD("Removing f-number");
    ExifTagSaver::removeExifEntryWithTag(EXIF_TAG_FNUMBER, exif_data);
  }
  if (!isSet(EXIF_INFORMATION_ATTRIBUTE_FLASH)) {
    LoggerD("Removing flash");
    ExifTagSaver::removeExifEntryWithTag(EXIF_TAG_FLASH, exif_data);
  }
  if (!isSet(EXIF_INFORMATION_ATTRIBUTE_FOCAL_LENGTH)) {
    LoggerD("Removing focal length");
    ExifTagSaver::removeExifEntryWithTag(EXIF_TAG_FOCAL_LENGTH, exif_data);
  }
  if (!isSet(EXIF_INFORMATION_ATTRIBUTE_GPS_TIME)) {
    LoggerD("Removing gps altitude");
    ExifTagSaver::removeExifEntryWithTag(
    static_cast<ExifTag>(EXIF_TAG_GPS_TIME_STAMP), exif_data);
  }
  if (!isSet(EXIF_INFORMATION_ATTRIBUTE_GPS_ALTITUDE)) {
    LoggerD("Removing gps altitude");
    ExifTagSaver::removeExifEntryWithTag(
        static_cast<ExifTag>(EXIF_TAG_GPS_ALTITUDE), exif_data);
  }
  if (!isSet(EXIF_INFORMATION_ATTRIBUTE_GPS_ALTITUDE_REF)) {
    LoggerD("Removing gps altitude ref");
    ExifTagSaver::removeExifEntryWithTag(
        static_cast<ExifTag>(EXIF_TAG_GPS_ALTITUDE_REF), exif_data);
  }
  if (!isSet(EXIF_INFORMATION_ATTRIBUTE_GPS_PROCESSING_METHOD)) {
    LoggerD("Removing gps processing method");
    ExifTagSaver::removeExifEntryWithTag(
        static_cast<ExifTag>(EXIF_TAG_GPS_PROCESSING_METHOD), exif_data);
  }
  if (!isSet(EXIF_INFORMATION_ATTRIBUTE_USER_COMMENT)) {
    LoggerD("Removing user comment");
    ExifTagSaver::removeExifEntryWithTag(EXIF_TAG_USER_COMMENT, exif_data);
  }
  if (!m_gps_location.isSet(EXIF_GPS_LOCATION_ATTRIBUTE_LATITUDE)) {
    LoggerD("Removing latitude");
    ExifTagSaver::removeExifEntryWithTag(
        static_cast<ExifTag>(EXIF_TAG_GPS_LATITUDE), exif_data);
  }
  if (!m_gps_location.isSet(EXIF_GPS_LOCATION_ATTRIBUTE_LATITUDE_REF)) {
    LoggerD("Removing latitude ref");
    ExifTagSaver::removeExifEntryWithTag(
        static_cast<ExifTag>(EXIF_TAG_GPS_LATITUDE_REF), exif_data);
  }
  if (!m_gps_location.isSet(EXIF_GPS_LOCATION_ATTRIBUTE_LONGITUDE)) {
    LoggerD("Removing longitude");
    ExifTagSaver::removeExifEntryWithTag(
        static_cast<ExifTag>(EXIF_TAG_GPS_LONGITUDE), exif_data);
  }
  if (!m_gps_location.isSet(EXIF_GPS_LOCATION_ATTRIBUTE_LONGITUDE_REF)) {
    LoggerD("Removing longitude ref");
    ExifTagSaver::removeExifEntryWithTag(
        static_cast<ExifTag>(EXIF_TAG_GPS_LONGITUDE_REF), exif_data);
  }
}

void ExifInformation::updateAttributesInExifData(ExifData* exif_data) {
  LoggerD("Entered");
  AssertMsg(exif_data, "exif_data is NULL");

  if (isSet(EXIF_INFORMATION_ATTRIBUTE_WIDTH)) {
    LoggerD("Saving width: %d", getWidth());
    ExifTagSaver::saveToExif(getWidth(),
        EXIF_TAG_IMAGE_WIDTH, exif_data);
  }
  if (isSet(EXIF_INFORMATION_ATTRIBUTE_HEIGHT)) {
    LoggerD("Saving height: %d", getHeight());
    ExifTagSaver::saveToExif(getHeight(),
        EXIF_TAG_IMAGE_LENGTH, exif_data);
  }
  if (isSet(EXIF_INFORMATION_ATTRIBUTE_DEVICE_MAKER)) {
    LoggerD("Saving device maker: %s", getDeviceMaker().c_str());
    ExifTagSaver::saveToExif(getDeviceMaker(),
        EXIF_TAG_MAKE, exif_data);
  }
  if (isSet(EXIF_INFORMATION_ATTRIBUTE_ORIENTATION)) {
    LoggerD("Saving orientation: %d", static_cast<int>(getOrientation()));
    ExifTagSaver::saveToExif(static_cast<long int>(getOrientation()),
        EXIF_TAG_ORIENTATION, exif_data);
  }
  if (isSet(EXIF_INFORMATION_ATTRIBUTE_EXPOSURE_PROGRAM)) {
    LoggerD("Saving exposure program: %d",
        static_cast<int>(getExposureProgram()));
    ExifTagSaver::saveToExif(getExposureProgram(),
        EXIF_TAG_EXPOSURE_PROGRAM, exif_data);
  }
  if (isSet(EXIF_INFORMATION_ATTRIBUTE_ISO_SPEED_RATINGS)) {
    std::vector<long long int> iso_ratings = getIsoSpeedRatings();
    LoggerD("Saving iso speed ratings count:%d", iso_ratings.size());
    ExifTagSaver::saveToExif(iso_ratings, EXIF_FORMAT_SHORT,
       EXIF_TAG_ISO_SPEED_RATINGS, exif_data);
  }
  if (isSet(EXIF_INFORMATION_ATTRIBUTE_WHITE_BALANCE)) {
    LoggerD("Saving white balance: %d",
        static_cast<int>(getWhiteBalanceMode()));
    ExifTagSaver::saveToExif(getWhiteBalanceMode(),
        EXIF_TAG_WHITE_BALANCE, exif_data);
  }
  if (isSet(EXIF_INFORMATION_ATTRIBUTE_DEVICE_MODEL)) {
    LoggerD("Saving device model: %s", getDeviceModel().c_str());
    ExifTagSaver::saveToExif(getDeviceModel(),
        EXIF_TAG_MODEL, exif_data);
  }
  if (isSet(EXIF_INFORMATION_ATTRIBUTE_ORIGINAL_TIME)) {
    const time_t o_time = getOriginalTime();
    const std::string o_time_str =
        ExifUtil::timeTToExifDateTimeOriginal(o_time);
    LoggerD("Saving original time time_t:%d, format:%s",
        static_cast<int>(o_time), o_time_str.c_str());

    ExifTagSaver::saveToExif(o_time_str,
        EXIF_TAG_DATE_TIME_ORIGINAL, exif_data);
  }
  if (isSet(EXIF_INFORMATION_ATTRIBUTE_EXPOSURE_TIME)) {
    Rational exposure_time = getExposureTime();
    if (exposure_time.isValid()) {
      LoggerD("Saving exposure time: %s (%s)",
          exposure_time.toString().c_str(),
          exposure_time.toExposureTimeString().c_str());

      ExifTagSaver::saveToExif(exposure_time,
          EXIF_TAG_EXPOSURE_TIME, exif_data);
    }
  }
  if (isSet(EXIF_INFORMATION_ATTRIBUTE_FNUMBER)) {
    auto f_number = getFNumber();
    LoggerD("Saving f-number: %f (%s)", f_number.toDouble(),
        f_number.toString().c_str());
    ExifTagSaver::saveToExif(f_number,
        EXIF_TAG_FNUMBER, exif_data);
  }
  if (isSet(EXIF_INFORMATION_ATTRIBUTE_FLASH)) {
    LoggerD("Saving flash: %s", getFlash() ? "ON" : "OFF");
    ExifTagSaver::saveToExif(getFlash(),
        EXIF_TAG_FLASH, exif_data);
  }
  if (isSet(EXIF_INFORMATION_ATTRIBUTE_FOCAL_LENGTH)) {
    auto f_length = getFocalLength();
    LoggerD("Saving focal length:%f (%s)", f_length.toDouble(),
        f_length.toString().c_str());
    ExifTagSaver::saveToExif(f_length,
        EXIF_TAG_FOCAL_LENGTH, exif_data);
  }
  if (isSet(EXIF_INFORMATION_ATTRIBUTE_GPS_LOCATION)) {
    LoggerD("Saving gps location");
    ExifTagSaver::saveGpsLocationToExif(m_gps_location, exif_data);
  }
  if (isSet(EXIF_INFORMATION_ATTRIBUTE_GPS_ALTITUDE)) {
    LoggerD("Saving gps altitude:%f (%s)", m_gps_altitude.toDouble(),
         m_gps_altitude.toString().c_str());
    ExifTagSaver::saveToExif(m_gps_altitude,
        static_cast<ExifTag>(EXIF_TAG_GPS_ALTITUDE), exif_data);
  }
  if (isSet(EXIF_INFORMATION_ATTRIBUTE_GPS_ALTITUDE_REF)) {
    // Exif spec:
    // 0 = Sea level
    // 1 = Sea level reference (negative value)
    LoggerD("Saving gps altitude ref:%d (%s)",
        static_cast<int>(m_gps_altitude_ref),
        (static_cast<int>(m_gps_altitude_ref) > 0) ? "below sea" : "above sea");
    std::vector<long long int> value = {
      static_cast<long long int>(m_gps_altitude_ref)
    };
    ExifTagSaver::saveToExif(value, EXIF_FORMAT_BYTE,
        static_cast<ExifTag>(EXIF_TAG_GPS_ALTITUDE_REF), exif_data);
  }
  if (isSet(EXIF_INFORMATION_ATTRIBUTE_GPS_PROCESSING_METHOD)) {
    LoggerD("Saving gps processing method: [%s] type:%s",
        getGpsProcessingMethod().c_str(), getGpsProcessingMethodType().c_str());

    const std::string& joined = getGpsProcessingMethodType() +
        getGpsProcessingMethod();
    LoggerD("joined: [%s]", joined.c_str());

    ExifTagSaver::saveToExif(joined,
        static_cast<ExifTag>(EXIF_TAG_GPS_PROCESSING_METHOD), exif_data,
        EXIF_FORMAT_UNDEFINED, false);
  }
  if (isSet(EXIF_INFORMATION_ATTRIBUTE_GPS_TIME)) {
    const time_t gps_time = getGpsTime();
    const Rationals gps_time_vec = ExifUtil::timeTToExifGpsTimeStamp(gps_time);
    const std::string& gps_date_str =
        ExifUtil::timeTToExifGpsDateStamp(gps_time);
    LoggerD("Saving gps time stamp time_t: %d", static_cast<int>(gps_time));

    ExifTagSaver::saveToExif(gps_time_vec,
        static_cast<ExifTag>(EXIF_TAG_GPS_TIME_STAMP), exif_data);

    LoggerD("Saving gps date stamp: %s", gps_date_str.c_str());

    ExifTagSaver::saveToExif(gps_date_str,
        static_cast<ExifTag>(EXIF_TAG_GPS_DATE_STAMP), exif_data,
        EXIF_FORMAT_ASCII, false);
  }
  if (isSet(EXIF_INFORMATION_ATTRIBUTE_USER_COMMENT)) {
    LoggerD("Saving user comment: %s (type:%s)", getUserComment().c_str(),
        getUserCommentType().c_str());

    const std::string& joined = getUserCommentType() + getUserComment();
    LoggerD("joined: [%s]", joined.c_str());

    ExifTagSaver::saveToExif(joined,
        EXIF_TAG_USER_COMMENT, exif_data, EXIF_FORMAT_UNDEFINED, false);
  }
}

PlatformResult ExifInformation::saveToFile(const std::string& file_path) {
  LoggerD("Entered");
  LoggerD("Using JpegFile to read: [%s] and Exif if present",
      file_path.c_str());

  JpegFilePtr jpg_file;
  PlatformResult result = JpegFile::loadFile(file_path, &jpg_file);
  if (!result)
    return result;

  ExifData* exif_data = jpg_file->getExifData();
  bool exif_data_is_new = false;

  // Exif is not present in file - create new ExifData
  if (!exif_data) {
    LoggerD("Exif is not present in file: [%s] creating new",
        file_path.c_str());

    exif_data = exif_data_new();
    if (!exif_data) {
      LoggerE("Couldn't allocate new ExifData");
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Memory allocation failed");
    }

    exif_data_set_option(exif_data, EXIF_DATA_OPTION_FOLLOW_SPECIFICATION);
    exif_data_set_data_type(exif_data, EXIF_DATA_TYPE_COMPRESSED);
    exif_data_set_byte_order(exif_data, EXIF_BYTE_ORDER_MOTOROLA);
    exif_data_is_new = true;
  }

  LoggerD("Exif data type: %d", exif_data_get_data_type(exif_data));
  LoggerD("Exif byte order: %d", exif_data_get_byte_order(exif_data));
  exif_data_set_option(exif_data, EXIF_DATA_OPTION_FOLLOW_SPECIFICATION);

  // If we have created new ExifData there is nothing to remove
  if (!exif_data_is_new) {
    // Remove attributes that have been nulled
    removeNulledAttributesFromExifData(exif_data);
  }
  updateAttributesInExifData(exif_data);

  LoggerD("Using JpegFile to save new Exif in: [%s]", file_path.c_str());
  if (exif_data_is_new) {
    result = jpg_file->setNewExifData(exif_data);
  }

  exif_data_unref(exif_data);

  if (!result) {
    return result;
  }

  return jpg_file->saveToFile(file_path);
}

}  // namespace exif
}  // namespace extension
