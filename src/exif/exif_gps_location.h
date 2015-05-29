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
 
#ifndef EXIF_EXIF_GPS_LOCATION_H_
#define EXIF_EXIF_GPS_LOCATION_H_

#include <string>
#include <vector>

#include "exif/exif_util.h"
#include "exif/rational.h"

namespace extension {
namespace exif {

enum GPSLocationDirectionLongitude {
  GPS_LOCATION_WEST,
  GPS_LOCATION_EAST
};

enum GPSLocationDirectionLatitude {
  GPS_LOCATION_NORTH,
  GPS_LOCATION_SOUTH
};

enum ExifGPSLocationAttributes {
  EXIF_GPS_LOCATION_ATTRIBUTE_LONGITUDE = 0,
  EXIF_GPS_LOCATION_ATTRIBUTE_LONGITUDE_REF,
  EXIF_GPS_LOCATION_ATTRIBUTE_LATITUDE,
  EXIF_GPS_LOCATION_ATTRIBUTE_LATITUDE_REF,
  EXIF_GPS_LOCATION_ATTRIBUTE_NUMBER_OF_ATTRIBUTES
};

/**
 * This class represents Global Coordinate System using
 * three Rational numbers (as stored in Exif)
 */
struct GCSPosition {
  GCSPosition();
  GCSPosition(Rational degrees, Rational minutes, Rational seconds);

  /**
   * Create GCSPosition from degrees represented as double value
   */
  static GCSPosition createFromDouble(double value);

  /**
   * Verify that all components are valid Rational numbers and
   * each component is within valid range. Sum of degrees,
   * minutes and seconds should not be bigger then 180.0
   */
  bool isValid() const;

  /**
   * Return position in degrees
   */
  double toDouble() const;

  /**
   * Return vector of three rationals: degrees, minutes, seconds
   */
  Rationals toRationalsVector() const;

  /**
   * Return string for debugging purposes
   */
  std::string toDebugString() const;

  Rational degrees;
  Rational minutes;
  Rational seconds;
};

class ExifGPSLocation {
 public:
  ExifGPSLocation();
  ExifGPSLocation(double longitude, double latitude);

  void setLongitude(const GCSPosition& longitude);
  const GCSPosition& getLongitude() const;

  void setLongitudeRef(GPSLocationDirectionLongitude ref);
  GPSLocationDirectionLongitude getLongitudeRef() const;

  void setLatitude(const GCSPosition& latitude);
  const GCSPosition& getLatitude() const;

  void setLatitudeRef(GPSLocationDirectionLatitude ref);
  GPSLocationDirectionLatitude getLatitudeRef() const;

  bool isSet(ExifGPSLocationAttributes attribute) const;
  void unset(ExifGPSLocationAttributes attribute);
  void unsetAll();

  /**
   * Returns true only if all components are set.
   */
  bool isComplete() const;

  /**
   * Returns true only if all components are set and valid.
   */
  bool isValid() const;

 private:
  double getLongitudeValue() const;
  double getLatitudeValue() const;

  GCSPosition m_longitude;
  GPSLocationDirectionLongitude m_longitude_ref;

  GCSPosition m_latitude;
  GPSLocationDirectionLatitude m_latitude_ref;

  bool m_is_set[EXIF_GPS_LOCATION_ATTRIBUTE_NUMBER_OF_ATTRIBUTES];
};

}  // namespace exif
}  // namespace extension

#endif  // EXIF_EXIF_GPS_LOCATION_H_
