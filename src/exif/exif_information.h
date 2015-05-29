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

#ifndef EXIF_EXIF_INFORMATION_H_
#define EXIF_EXIF_INFORMATION_H_

#include <libexif/exif-loader.h>

#include <map>
#include <string>
#include <vector>

#include "common/picojson.h"
#include "common/platform_result.h"

#include "exif/exif_gps_location.h"

namespace extension {
namespace exif {

#define EI_URI "uri"
#define EI_WIDTH "width"
#define EI_HEIGHT "height"
#define EI_DEVICE_MAKER "deviceMaker"
#define EI_DEVICE_MODEL "deviceModel"
#define EI_ORIGINAL_TIME "originalTime"
#define EI_ORIENTATION "orientation"
#define EI_FNUMBER "fNumber"
#define EI_ISO_SPEED_RATINGS "isoSpeedRatings"
#define EI_EXPOSURE_TIME "exposureTime"
#define EI_EXPOSURE_PROGRAM "exposureProgram"
#define EI_FLASH "flash"
#define EI_FOCAL_LENGTH "focalLength"
#define EI_WHITE_BALANCE "whiteBalance"
#define EI_GPS_LOCATION "gpsLocation"
#define EI_GPS_ALTITUDE "gpsAltitude"
#define EI_GPS_PROCESSING_METHOD "gpsProcessingMethod"
#define EI_GPS_TIME "gpsTime"
#define EI_USER_COMMENT "userComment"

class ExifInformation;
typedef std::shared_ptr<ExifInformation> ExifInformationPtr;
typedef std::map<std::string, std::string> AttributeMap;
typedef std::vector<long long int> IsoSpeedRatingsVector;

extern const std::size_t EXIF_UNDEFINED_TYPE_LENGTH;
extern const std::string EXIF_UNDEFINED_TYPE_ASCII;
extern const std::string EXIF_UNDEFINED_TYPE_JIS;
extern const std::string EXIF_UNDEFINED_TYPE_UNICODE;
extern const std::string EXIF_UNDEFINED_TYPE_UNDEFINED;

enum ExifInformationAttribute{
  EXIF_INFORMATION_ATTRIBUTE_URI,
  EXIF_INFORMATION_ATTRIBUTE_WIDTH,
  EXIF_INFORMATION_ATTRIBUTE_HEIGHT,
  EXIF_INFORMATION_ATTRIBUTE_DEVICE_MAKER,
  EXIF_INFORMATION_ATTRIBUTE_DEVICE_MODEL,
  EXIF_INFORMATION_ATTRIBUTE_ORIGINAL_TIME,
  EXIF_INFORMATION_ATTRIBUTE_ORIENTATION,
  EXIF_INFORMATION_ATTRIBUTE_FNUMBER,
  EXIF_INFORMATION_ATTRIBUTE_ISO_SPEED_RATINGS,
  EXIF_INFORMATION_ATTRIBUTE_EXPOSURE_TIME,
  EXIF_INFORMATION_ATTRIBUTE_EXPOSURE_PROGRAM,
  EXIF_INFORMATION_ATTRIBUTE_FLASH,
  EXIF_INFORMATION_ATTRIBUTE_FOCAL_LENGTH,
  EXIF_INFORMATION_ATTRIBUTE_WHITE_BALANCE,
  EXIF_INFORMATION_ATTRIBUTE_GPS_ALTITUDE,
  EXIF_INFORMATION_ATTRIBUTE_GPS_ALTITUDE_REF,
  EXIF_INFORMATION_ATTRIBUTE_GPS_LOCATION,
  EXIF_INFORMATION_ATTRIBUTE_GPS_PROCESSING_METHOD,
  EXIF_INFORMATION_ATTRIBUTE_GPS_TIME,
  EXIF_INFORMATION_ATTRIBUTE_USER_COMMENT,
  EXIF_INFORMATION_ATTRIBUTE_NUMBER_OF_ATTRIBUTES
};

enum GpsAltitudeRef {
  GPS_ALTITUDE_REF_ABOVE_SEA = 0,
  GPS_ALTITUDE_REF_BELOW_SEA = 1
};

const AttributeMap ExifInformationAttributeMap = {
  {"uri", "uri"},
  {"width", "width"},
  {"height", "height"},
  {"deviceMaker", "deviceMaker"},
  {"deviceModel", "deviceModel"},
  {"originalTime", "originalTime"},
  {"orientation", "orientation"},
  {"fNumber", "fNumber"},
  {"isoSpeedRatings", "isoSpeedRatings"},
  {"exposureTime", "exposureTime"},
  {"exposureProgram", "exposureProgram"},
  {"flash", "flash"},
  {"focalLength", "focalLength"},
  {"whiteBalance", "whiteBalance"},
  {"gpsLocation", "gpsLocation"},
  {"gpsAltitude", "gpsAltitude"},
  {"gpsProcessingMethod", "gpsProcessingMethod"},
  {"gpsTime", "gpsTime"},
  {"userComment", "userComment"}
};

class ExifInformation {
 public:
  ExifInformation();
  explicit ExifInformation(const picojson::value& args);
  ~ExifInformation();

  common::PlatformResult saveToFile(const std::string& file_path);

  const std::string& getUri();
  void setUri(const std::string& uri);

  unsigned long getWidth() const;
  void setWidth(unsigned long width);

  unsigned long getHeight() const;
  void setHeight(unsigned long height);

  const std::string& getDeviceMaker();
  void setDeviceMaker(const std::string& device_maker);

  const std::string& getDeviceModel();
  void setDeviceModel(const std::string& device_model);

  time_t getOriginalTime() const;
  void setOriginalTime(time_t original_time);

  const std::string& getOrientationString();
  ImageOrientation getOrientation();
  void setOrientation(const std::string& orientation);
  void setOrientation(ImageOrientation orientation);

  const Rational& getFNumber() const;
  void setFNumber(Rational f_number);

  const std::vector<long long int>& getIsoSpeedRatings();
  void setIsoSpeedRatings(const std::vector<long long int>& iso_speed_ratings);

  const Rational& getExposureTime();
  void setExposureTime(const Rational& exposure_time);

  const std::string& getExposureProgramString();
  ExposureProgram getExposureProgram();
  void setExposureProgram(const std::string& exposure_program);
  void setExposureProgram(ExposureProgram exposure_program);

  bool getFlash() const;
  void setFlash(bool flash);

  const Rational& getFocalLength() const;
  void setFocalLength(Rational focal_length);

  const std::string& getWhiteBalanceModeString();
  WhiteBalanceMode getWhiteBalanceMode();
  void setWhiteBalanceMode(const std::string& white_balance);
  void setWhiteBalanceMode(WhiteBalanceMode white_balance);

  ExifGPSLocation& getGPSExifLocation();
  void setGPSLocation(ExifGPSLocation gps_location);
  void unsetGPSLocation();

  const Rational& getGpsAltitude() const;
  void setGpsAltitude(Rational gps_altitude);

  GpsAltitudeRef getGpsAltitudeRef() const;
  void setGpsAltitudeRef(const GpsAltitudeRef ref);

  /**
   * gps_altitude can be negative and positive:
   * if gps_altitude < 0.0 GPS_ALTITUDE_REF_BELOW_SEA is set
   * if gps_altitude >= 0.0 GPS_ALTITUDE_REF_ABOVE_SEA is set
   */
  void setGpsAltitudeWithRef(double gps_altitude);

  /**
   * Return gps altitude which can be negative (below sea level)
   * and positive (above sea level)
   */
  double getGpsAltitudeWithRef() const;

  const std::string& getGpsProcessingMethod() const;
  const std::string& getGpsProcessingMethodType() const;
  void setGpsProcessingMethod(const std::string& type,
                              const std::string& processing_method);

  void setGpsTime(time_t time);
  time_t getGpsTime();
  void unsetGPStime();

  const std::string& getUserComment();
  const std::string& getUserCommentType();
  void setUserComment(const std::string& type, const std::string& user_comment);

  bool isSet(ExifInformationAttribute attribute) const;
  void unset(ExifInformationAttribute attribute);
  void set(std::string attributeName, const picojson::value& args);

 private:
  void removeNulledAttributesFromExifData(ExifData* exif_data);
  void updateAttributesInExifData(ExifData* exif_data);

  std::string m_uri;
  unsigned long m_width;
  unsigned long m_height;
  std::string m_device_maker;
  std::string m_device_model;

  time_t m_original_time;

  ImageOrientation m_orientation;
  Rational m_f_number;
  std::vector<long long int> m_iso_speed_ratings;
  Rational m_exposure_time;
  ExposureProgram m_exposure_program;
  bool m_flash;
  Rational m_focal_length;
  WhiteBalanceMode m_white_balance;

  ExifGPSLocation m_gps_location;

  Rational m_gps_altitude;
  GpsAltitudeRef m_gps_altitude_ref;

  std::string m_gps_processing_method;
  std::string m_gps_processing_method_type;

  time_t m_gps_time;

  std::string m_user_comment;
  std::string m_user_comment_type;

  bool m_is_set[EXIF_INFORMATION_ATTRIBUTE_NUMBER_OF_ATTRIBUTES];
};

}  // namespace exif
}  // namespace extension

#endif  // EXIF_EXIF_INFORMATION_H_
