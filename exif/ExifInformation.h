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

#ifndef __TIZEN_EXIF_EXIFINFORMATION_H_
#define __TIZEN_EXIF_EXIFINFORMATION_H_

#include <libexif/exif-loader.h>
#include <string>

#include <JSVector.h>
#include <TimeDuration.h>

#include "ExifGPSLocation.h"
#include "ExifGPSTime.h"

namespace DeviceAPI {
namespace Exif {

class ExifInformation;
typedef std::shared_ptr<ExifInformation> ExifInformationPtr;

extern const size_t EXIF_UNDEFINED_TYPE_LENGTH;
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
    EXIF_INFORMATION_ATTRIBUTE_GPS_PROCESSING_METHOD,
    EXIF_INFORMATION_ATTRIBUTE_USER_COMMENT,
    EXIF_INFORMATION_ATTRIBUTE_NUMBER_OF_ATTRIBUTES
};

enum GpsAltitudeRef {
    GPS_ALTITUDE_REF_ABOVE_SEA = 0,
    GPS_ALTITUDE_REF_BELOW_SEA = 1
};

class ExifInformation
{
public:
    ExifInformation();
    ~ExifInformation();

    static ExifInformationPtr loadFromURI(const std::string& uri);
    void saveToFile(const std::string& file_path);

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

    ImageOrientation getOrientation() const;
    void setOrientation(ImageOrientation orientation);

    const Rational& getFNumber() const;
    void setFNumber(Rational f_number);

    Common::JSLongLongVector getIsoSpeedRatings();
    void setIsoSpeedRatings(const std::vector<long long int>& iso_speed_ratings);
    void appendIsoSpeedRatings(long long int iso_speed_rating);

    const Rational& getExposureTime();
    void setExposureTime(const Rational& exposure_time);

    ExposureProgram getExposureProgram();
    void setExposureProgram(ExposureProgram exposure_program );

    bool getFlash() const;
    void setFlash(bool flash);

    const Rational& getFocalLength() const;
    void setFocalLength(Rational focal_length);

    WhiteBalanceMode getWhiteBalanceMode() const;
    void setWhiteBalanceMode(WhiteBalanceMode white_balance);

    ExifGPSLocation& getGPSExifLocation();
    Tizen::SimpleCoordinatesPtr getGPSLocation();
    void setGPSLocation(Tizen::SimpleCoordinatesPtr gps_location);
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
     * Return gps altitude which can be negative (below sea level) and positive (above sea
     * level)
     */
    double getGpsAltitudeWithRef() const;

    const std::string& getGpsProcessingMethod() const;
    const std::string& getGpsProcessingMethodType() const;
    void setGpsProcessingMethod(const std::string& type,
            const std::string& processing_method);

    ExifGPSTime& getExifGpsTime();
    const ExifGPSTime& getExifGpsTime() const;
    Time::TZDatePtr getGpsTime();
    void setGpsTime(Time::TZDatePtr new_time);
    void unsetGPStime();

    const std::string& getUserComment();
    const std::string& getUserCommentType();
    void setUserComment(const std::string& type,
            const std::string& user_comment);

    bool isSet(ExifInformationAttribute attribute) const;
    void unset(ExifInformationAttribute attribute);

private:
    void processEntry(ExifEntry* entry, ExifData* exif_data);
    static void contentForeachFunctionProxy(ExifEntry* entry, void* user_data);
    static void dataForeachFunction(ExifContent* content, void* user_data);

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
    Common::JSLongLongVector m_iso_speed_ratings;
    Rational m_exposure_time;
    ExposureProgram m_exposure_program;
    bool m_flash;
    Rational m_focal_length;
    WhiteBalanceMode m_white_balance;

    ExifGPSLocation m_exif_gps_location;
    Tizen::SimpleCoordinatesPtr m_gps_location;

    Rational m_gps_altitude;
    GpsAltitudeRef m_gps_altitude_ref;

    std::string m_gps_processing_method;
    std::string m_gps_processing_method_type;

    ExifGPSTime m_exif_gps_time;
    Time::TZDatePtr m_gps_time;

    std::string m_user_comment;
    std::string m_user_comment_type;

    bool m_is_set[EXIF_INFORMATION_ATTRIBUTE_NUMBER_OF_ATTRIBUTES];
};

} // Exif
} // DeviceAPI

#endif // __TIZEN_EXIF_EXIFINFORMATION_H_
