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

#include "ExifGPSLocation.h"

#include <sstream>

#include <Logger.h>

namespace DeviceAPI {
namespace Exif {

GCSPosition::GCSPosition()
{
}

GCSPosition::GCSPosition(Rational _degrees, Rational _minutes, Rational _seconds) :
        degrees(_degrees),
        minutes(_minutes),
        seconds(_seconds)
{
}

bool GCSPosition::isValid() const
{
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

double GCSPosition::toDouble() const
{
    const double degrees_value = degrees.toDouble();
    const double minutes_value = minutes.toDouble();
    const double seconds_value = seconds.toDouble();
    return (degrees_value + (minutes_value/60.0) + (seconds_value/3600.0));
}

Rationals GCSPosition::toRationalsVector() const
{
    Rationals vec;
    vec.push_back(degrees);
    vec.push_back(minutes);
    vec.push_back(seconds);
    return vec;
}

std::string GCSPosition::toDebugString() const
{
    std::stringstream ss;
    ss << degrees.toString() << "d ";
    ss << minutes.toString() << "m ";
    ss << seconds.toString() << "s";
    return ss.str();
}

GCSPosition GCSPosition::createFromDouble(double value)
{
    LOGD("Entered value:%f");
    if (value < 0) {
        LOGW("Trying to create GCSPosition with double < 0: %f", value);
        return GCSPosition();
    }

    if (value > 180.0) {
        LOGW("Trying to create GCSPosition with double > 180.0: %f", value);
        return GCSPosition();
    }

    const double d_degrees = floor(value);
    double left = value - d_degrees;

    const double d_minutes = floor(left * 60.0);
    left -= d_minutes / 60.0;

    const double d_seconds = left * 3600.0;

    LOGD("d_degrees:%f d_minutes:%f d_seconds:%f", d_degrees, d_minutes, d_seconds);

    GCSPosition pos;
    pos.degrees = Rational(static_cast<ExifLong>(d_degrees), 1);
    pos.minutes = Rational(static_cast<ExifLong>(d_minutes), 1);
    pos.seconds = Rational::createFromDouble(d_seconds);
    return pos;
}

ExifGPSLocation::ExifGPSLocation() :
        m_longitude_ref(GPS_LOCATION_WEST),
        m_latitude_ref(GPS_LOCATION_NORTH)
{
    for(int i = 0; i < EXIF_GPS_LOCATION_ATTRIBUTE_NUMBER_OF_ATTRIBUTES; ++i) {
        m_is_set[i] = false;
    }
}

void ExifGPSLocation::setLongitude(const GCSPosition& longitude)
{
    if (!longitude.isValid()) {
        LOGW("longitude is not valid!");
        return;
    }

    m_is_set[EXIF_GPS_LOCATION_ATTRIBUTE_LONGITUDE] = true;
    m_longitude = longitude;
}

const GCSPosition& ExifGPSLocation::getLongitude() const
{
    return m_longitude;
}

void ExifGPSLocation::setLongitudeRef(GPSLocationDirectionLongitude ref)
{
    m_is_set[EXIF_GPS_LOCATION_ATTRIBUTE_LONGITUDE_REF] = true;
    m_longitude_ref = ref;
}

GPSLocationDirectionLongitude ExifGPSLocation::getLongitudeRef() const
{
    return m_longitude_ref;
}

void ExifGPSLocation::setLatitude(const GCSPosition& latitude)
{
    if (!latitude.isValid()) {
        LOGW("latitude is not valid!");
        return;
    }

    m_is_set[EXIF_GPS_LOCATION_ATTRIBUTE_LATITUDE] = true;
    m_latitude = latitude;
}

const GCSPosition& ExifGPSLocation::getLatitude() const
{
    return m_latitude;
}

void ExifGPSLocation::setLatitudeRef(GPSLocationDirectionLatitude ref)
{
    m_is_set[EXIF_GPS_LOCATION_ATTRIBUTE_LATITUDE_REF] = true;
    m_latitude_ref = ref;
}

GPSLocationDirectionLatitude ExifGPSLocation::getLatitudeRef() const
{
    return m_latitude_ref;
}

bool ExifGPSLocation::isSet(ExifGPSLocationAttributes attribute) const
{
    return m_is_set[attribute];
}

void ExifGPSLocation::unset(ExifGPSLocationAttributes attribute)
{
    m_is_set[attribute] = false;
}

void ExifGPSLocation::unsetAll()
{
    m_is_set[EXIF_GPS_LOCATION_ATTRIBUTE_LONGITUDE] = false;
    m_is_set[EXIF_GPS_LOCATION_ATTRIBUTE_LONGITUDE_REF] = false;
    m_longitude =  GCSPosition();

    m_is_set[EXIF_GPS_LOCATION_ATTRIBUTE_LATITUDE] = false;
    m_is_set[EXIF_GPS_LOCATION_ATTRIBUTE_LATITUDE_REF] = false;
    m_latitude = GCSPosition();
}

bool ExifGPSLocation::isComplete() const
{
    return m_is_set[EXIF_GPS_LOCATION_ATTRIBUTE_LONGITUDE] &&
            m_is_set[EXIF_GPS_LOCATION_ATTRIBUTE_LONGITUDE_REF] &&
            m_is_set[EXIF_GPS_LOCATION_ATTRIBUTE_LATITUDE] &&
            m_is_set[EXIF_GPS_LOCATION_ATTRIBUTE_LATITUDE_REF];
}


bool ExifGPSLocation::isValid() const
{
    return isComplete() && m_latitude.isValid() && m_longitude.isValid();
}

Tizen::SimpleCoordinatesPtr ExifGPSLocation::getSimpleCoordinates() const
{
    if (!isComplete()) {
        LOGW("Some attributes are not set!");
        return Tizen::SimpleCoordinatesPtr();
    }

    if (!isValid()) {
        LOGW("Some attributes are not valid!");
        return Tizen::SimpleCoordinatesPtr();
    }

    const double cur_latitude = getLatitudeValue();
    const double cur_longitude = getLongitudeValue();

    return Tizen::SimpleCoordinatesPtr(
            new Tizen::SimpleCoordinates(cur_latitude, cur_longitude));
}

double ExifGPSLocation::getLongitudeValue() const
{
    const double longitude_dir = (m_longitude_ref == GPS_LOCATION_WEST) ? -1.0f : 1.0f;
    const double longitude = m_longitude.toDouble() * longitude_dir;
    return longitude;
}

double ExifGPSLocation::getLatitudeValue() const
{
    const double latitude_dir = (m_latitude_ref == GPS_LOCATION_SOUTH) ? -1.0f : 1.0f;
    const double latitude = m_latitude.toDouble() * latitude_dir;
    return latitude;
}

bool ExifGPSLocation::set(Tizen::SimpleCoordinatesPtr scoords)
{
    LOGD("Entered");
    if (!scoords) {
        LOGW("Trying to set null SimpleCoordinates!");
        return false;
    }

    const double longitude = scoords->getLongitude();
    const double latitude = scoords->getLatitude();
    bool updateLongitude = true;
    bool updateLatitude = true;

    if(isComplete()) {
        updateLatitude = getLatitudeValue() != latitude;
        updateLongitude = getLongitudeValue() != longitude;
    }

    LOGD("latitude:%f longitude:%f", latitude, longitude);

    GCSPosition gcs_longitude = GCSPosition::createFromDouble(fabs(longitude));
    LOGD("gcs_longitude deg:%s min:%s sec:%s", gcs_longitude.degrees.toString().c_str(),
            gcs_longitude.minutes.toString().c_str(),
            gcs_longitude.seconds.toString().c_str());

    GCSPosition gcs_latitude = GCSPosition::createFromDouble(fabs(latitude));
    LOGD("gcs_latitude deg:%s min:%s sec:%s", gcs_latitude.degrees.toString().c_str(),
            gcs_latitude.minutes.toString().c_str(),
            gcs_latitude.seconds.toString().c_str());

    if (!gcs_latitude.isValid() || !gcs_longitude.isValid()) {
        return false;
    }

    if(updateLongitude) {
        setLongitude(gcs_longitude);
        if (longitude >= 0.0) {
            setLongitudeRef(GPS_LOCATION_EAST);
        }
        else {
            setLongitudeRef(GPS_LOCATION_WEST);
        }
    }

    if(updateLatitude) {
        setLatitude(gcs_latitude);
        if (latitude >= 0.0) {
            setLatitudeRef(GPS_LOCATION_NORTH);
        }
        else {
            setLatitudeRef(GPS_LOCATION_SOUTH);
        }
    }

    return true;
}


} // Exif
} // DeviceAPI
