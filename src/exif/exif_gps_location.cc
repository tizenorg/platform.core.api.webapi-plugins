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

#include "exif/exif_gps_location.h"

#include <string>
#include <sstream>
#include <cmath>

#include "common/assert.h"
#include "common/logger.h"

namespace extension {
namespace exif {

GCSPosition::GCSPosition() {
  LoggerD("Enter");
}

GCSPosition::GCSPosition(Rational _degrees, Rational _minutes,
                         Rational _seconds) :
    degrees(_degrees),
    minutes(_minutes),
    seconds(_seconds) {
      LoggerD("Enter");
}

bool GCSPosition::isValid() const {
  LoggerD("Enter");
  if (!(degrees.isValid() && minutes.isValid() && seconds.isValid())) {
    return false;
  }

  if ((degrees.toDouble() > 180.0f) ||
      (minutes.toDouble() > 60.0f) ||
      (seconds.toDouble() > 60.0f)) {
    return false;
  }

  return toDouble() <= 180.0f;
}

double GCSPosition::toDouble() const {
  LoggerD("Enter");
  const double degrees_value = degrees.toDouble();
  const double minutes_value = minutes.toDouble();
  const double seconds_value = seconds.toDouble();
  return (degrees_value + (minutes_value/60.0) + (seconds_value/3600.0));
}

Rationals GCSPosition::toRationalsVector() const {
  LoggerD("Enter");
  Rationals vec;
  vec.push_back(degrees);
  vec.push_back(minutes);
  vec.push_back(seconds);
  return vec;
}

std::string GCSPosition::toDebugString() const {
  LoggerD("Enter");
  std::stringstream ss;
  ss << degrees.toString() << "d ";
  ss << minutes.toString() << "m ";
  ss << seconds.toString() << "s";
  return ss.str();
}

GCSPosition GCSPosition::createFromDouble(double value) {
  LoggerD("Entered value:%f");
  if (value < 0) {
    LoggerW("Trying to create GCSPosition with double < 0: %f", value);
    return GCSPosition();
  }

  if (value > 180.0) {
    LoggerW("Trying to create GCSPosition with double > 180.0: %f", value);
    return GCSPosition();
  }

  double d_degrees = floor(value);
  double left = value - d_degrees;

  double d_minutes = floor(left * 60.0);
  left -= d_minutes / 60.0;

  double d_seconds = round(left * 3600.0);

  if (d_seconds >= 60.0) {
    d_seconds -= 60.0;
    d_minutes++;
  }

  if (d_minutes >= 60.0) {
    d_minutes -= 60.0;
    d_degrees++;
  }

  Assert(d_degrees <= 180.0);

  LoggerD("d_degrees:%f d_minutes:%f d_seconds:%f",
      d_degrees, d_minutes, d_seconds);

  GCSPosition pos;
  pos.degrees = Rational(static_cast<ExifLong>(d_degrees), 1);
  pos.minutes = Rational(static_cast<ExifLong>(d_minutes), 1);
  pos.seconds = Rational::createFromDouble(d_seconds);

  return pos;
}

ExifGPSLocation::ExifGPSLocation() :
    m_longitude_ref(GPS_LOCATION_WEST),
    m_latitude_ref(GPS_LOCATION_NORTH) {
  for (int i = 0; i < EXIF_GPS_LOCATION_ATTRIBUTE_NUMBER_OF_ATTRIBUTES; ++i) {
    LoggerD("Enter");
    m_is_set[i] = false;
  }
  LoggerE("ExifGPSLocation::ExifGPSLocation()");
}

ExifGPSLocation::ExifGPSLocation(double longitude, double latitude) {
  LoggerD("Enter");
  for (int i = 0; i < EXIF_GPS_LOCATION_ATTRIBUTE_NUMBER_OF_ATTRIBUTES; ++i) {
    m_is_set[i] = false;
  }

  setLongitude(GCSPosition::createFromDouble(longitude));
  setLatitude(GCSPosition::createFromDouble(latitude));

  if (longitude < 0) {
    setLongitudeRef(GPS_LOCATION_WEST);
  } else {
    setLongitudeRef(GPS_LOCATION_EAST);
  }

  if (latitude < 0) {
    setLatitudeRef(GPS_LOCATION_SOUTH);
  } else {
    setLatitudeRef(GPS_LOCATION_NORTH);
  }
}

void ExifGPSLocation::setLongitude(const GCSPosition& longitude) {
  LoggerD("Enter");
  if (!longitude.isValid()) {
    LoggerW("longitude is not valid!");
    return;
  }

  m_is_set[EXIF_GPS_LOCATION_ATTRIBUTE_LONGITUDE] = true;
  m_longitude = longitude;
}

const GCSPosition& ExifGPSLocation::getLongitude() const {
  LoggerD("Enter");
  return m_longitude;
}

void ExifGPSLocation::setLongitudeRef(GPSLocationDirectionLongitude ref) {
  LoggerD("Enter");
  m_is_set[EXIF_GPS_LOCATION_ATTRIBUTE_LONGITUDE_REF] = true;
  m_longitude_ref = ref;
}

GPSLocationDirectionLongitude ExifGPSLocation::getLongitudeRef() const {
  LoggerD("Enter");
  return m_longitude_ref;
}

void ExifGPSLocation::setLatitude(const GCSPosition& latitude) {
  LoggerD("Enter");
  if (!latitude.isValid()) {
    LoggerW("latitude is not valid!");
    return;
  }

  m_is_set[EXIF_GPS_LOCATION_ATTRIBUTE_LATITUDE] = true;
  m_latitude = latitude;
}

const GCSPosition& ExifGPSLocation::getLatitude() const {
  LoggerD("Enter");
  return m_latitude;
}

void ExifGPSLocation::setLatitudeRef(GPSLocationDirectionLatitude ref) {
  LoggerD("Enter");
  m_is_set[EXIF_GPS_LOCATION_ATTRIBUTE_LATITUDE_REF] = true;
  m_latitude_ref = ref;
}

GPSLocationDirectionLatitude ExifGPSLocation::getLatitudeRef() const {
  LoggerD("Enter");
  return m_latitude_ref;
}

bool ExifGPSLocation::isSet(ExifGPSLocationAttributes attribute) const {
  LoggerD("Enter");
  return m_is_set[attribute];
}

void ExifGPSLocation::unset(ExifGPSLocationAttributes attribute) {
  LoggerD("Enter");
  m_is_set[attribute] = false;
}

void ExifGPSLocation::unsetAll() {
  LoggerD("Enter");
  m_is_set[EXIF_GPS_LOCATION_ATTRIBUTE_LONGITUDE] = false;
  m_is_set[EXIF_GPS_LOCATION_ATTRIBUTE_LONGITUDE_REF] = false;
  m_longitude =  GCSPosition();

  m_is_set[EXIF_GPS_LOCATION_ATTRIBUTE_LATITUDE] = false;
  m_is_set[EXIF_GPS_LOCATION_ATTRIBUTE_LATITUDE_REF] = false;
  m_latitude = GCSPosition();
}

bool ExifGPSLocation::isComplete() const {
  LoggerD("Enter");
  return m_is_set[EXIF_GPS_LOCATION_ATTRIBUTE_LONGITUDE] &&
      m_is_set[EXIF_GPS_LOCATION_ATTRIBUTE_LONGITUDE_REF] &&
      m_is_set[EXIF_GPS_LOCATION_ATTRIBUTE_LATITUDE] &&
      m_is_set[EXIF_GPS_LOCATION_ATTRIBUTE_LATITUDE_REF];
}


bool ExifGPSLocation::isValid() const {
  LoggerD("Enter");
  return isComplete() && m_latitude.isValid() && m_longitude.isValid();
}

double ExifGPSLocation::getLongitudeValue() const {
  LoggerD("Enter");
  const double longitude_dir =
      (m_longitude_ref == GPS_LOCATION_WEST) ? -1.0f : 1.0f;
  const double longitude = m_longitude.toDouble() * longitude_dir;
  return longitude;
}

double ExifGPSLocation::getLatitudeValue() const {
  LoggerD("Enter");
  const double latitude_dir =
      (m_latitude_ref == GPS_LOCATION_SOUTH) ? -1.0f : 1.0f;
  const double latitude = m_latitude.toDouble() * latitude_dir;
  return latitude;
}
}  // namespace exif
}  // namespace extension
