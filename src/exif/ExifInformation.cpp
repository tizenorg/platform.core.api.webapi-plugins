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

#include "ExifInformation.h"

#include <Logger.h>
#include <memory>
#include <PlatformException.h>

#include "ExifTagSaver.h"
#include "ExifUtil.h"
#include "JpegFile.h"

using namespace DeviceAPI::Common;

namespace DeviceAPI {
namespace Exif {

const size_t EXIF_UNDEFINED_TYPE_LENGTH = 8;
const std::string EXIF_UNDEFINED_TYPE_ASCII =
        std::string("ASCII\0\0\0", EXIF_UNDEFINED_TYPE_LENGTH);
const std::string EXIF_UNDEFINED_TYPE_JIS =
        std::string("JIS\0\0\0\0\0", EXIF_UNDEFINED_TYPE_LENGTH);
const std::string EXIF_UNDEFINED_TYPE_UNICODE =
        std::string("UNICODE\0", EXIF_UNDEFINED_TYPE_LENGTH);
const std::string EXIF_UNDEFINED_TYPE_UNDEFINED =
        std::string("\0\0\0\0\0\0\0\0", EXIF_UNDEFINED_TYPE_LENGTH);

ExifInformation::ExifInformation()
{
    for (int attr = 0; attr < EXIF_INFORMATION_ATTRIBUTE_NUMBER_OF_ATTRIBUTES; attr++) {
        unset(static_cast<ExifInformationAttribute>(attr));
    }
}

ExifInformation::~ExifInformation()
{
}

const std::string& ExifInformation::getUri()
{
    LOGD("Entered");
    return m_uri;
}

void ExifInformation::setUri(const std::string& uri)
{
    LOGD("Entered");
    m_is_set[EXIF_INFORMATION_ATTRIBUTE_URI] = true;
    m_uri = uri;
}

unsigned long ExifInformation::getWidth() const
{
    LOGD("Entered");
    return m_width;
}

void ExifInformation::setWidth(unsigned long width)
{
    LOGD("Entered");
    m_is_set[EXIF_INFORMATION_ATTRIBUTE_WIDTH] = true;
    m_width = width;
}

unsigned long ExifInformation::getHeight() const
{
    LOGD("Entered");
    return m_height;
}

void ExifInformation::setHeight(unsigned long height)
{
    LOGD("Entered");
    m_is_set[EXIF_INFORMATION_ATTRIBUTE_HEIGHT] = true;
    m_height = height;
}

const std::string& ExifInformation::getDeviceMaker()
{
    LOGD("Entered");
    return m_device_maker;
}

void ExifInformation::setDeviceMaker(const std::string& device_maker)
{
    LOGD("Entered");
    m_is_set[EXIF_INFORMATION_ATTRIBUTE_DEVICE_MAKER] = true;
    m_device_maker = device_maker;
}

const std::string& ExifInformation::getDeviceModel()
{
    LOGD("Entered");
    return m_device_model;
}

void ExifInformation::setDeviceModel(const std::string& device_model)
{
    LOGD("Entered");
    m_is_set[EXIF_INFORMATION_ATTRIBUTE_DEVICE_MODEL] = true;
    m_device_model = device_model;
}

time_t ExifInformation::getOriginalTime() const
{
    LOGD("Entered");
    return m_original_time;
}

void ExifInformation::setOriginalTime(time_t original_time)
{
    LOGD("Entered");
    m_is_set[EXIF_INFORMATION_ATTRIBUTE_ORIGINAL_TIME] = true;
    m_original_time = original_time;
}

ImageOrientation ExifInformation::getOrientation() const
{
    LOGD("Entered");
    return m_orientation;
}

void ExifInformation::setOrientation(ImageOrientation orientation)
{
    LOGD("Entered");
    if(EXIF_ORIENTATION_NOT_VALID == orientation) {
        LOGW("Trying to set NOT VALID orientation");
        return;
    }

    m_is_set[EXIF_INFORMATION_ATTRIBUTE_ORIENTATION] = true;
    m_orientation = orientation;
}

const Rational& ExifInformation::getFNumber() const
{
    LOGD("Entered");
    return m_f_number;
}

void ExifInformation::setFNumber(Rational f_number)
{
    LOGD("Entered");
    if(!f_number.isValid()) {
        LOGW("Trying to set invalid F-Number: %s", f_number.toString().c_str());
        return;
    }

    m_is_set[EXIF_INFORMATION_ATTRIBUTE_FNUMBER] = true;
    m_f_number = f_number;
}

Common::JSLongLongVector ExifInformation::getIsoSpeedRatings()
{
    LOGD("Entered");
    return m_iso_speed_ratings;
}

void ExifInformation::setIsoSpeedRatings(
        const std::vector<long long int>& iso_speed_ratings)
{
    LOGD("Entered");
    m_is_set[EXIF_INFORMATION_ATTRIBUTE_ISO_SPEED_RATINGS] = true;
    m_iso_speed_ratings = iso_speed_ratings;
}

void ExifInformation::appendIsoSpeedRatings(long long int iso_speed_rating)
{
    m_is_set[EXIF_INFORMATION_ATTRIBUTE_ISO_SPEED_RATINGS] = true;
    m_iso_speed_ratings.push_back(iso_speed_rating);
}

const Rational& ExifInformation::getExposureTime()
{
    LOGD("Entered");
    return m_exposure_time;
}

void ExifInformation::setExposureTime(const Rational& exposure_time)
{
    LOGD("Entered");
    if (!exposure_time.isValid() || 0 == exposure_time.nominator) {
        LOGW("Trying to set invalid exposure time: [%s]",
                exposure_time.toString().c_str());
        return;
    }

    m_is_set[EXIF_INFORMATION_ATTRIBUTE_EXPOSURE_TIME] = true;
    m_exposure_time = exposure_time;
}

ExposureProgram ExifInformation::getExposureProgram()
{
    LOGD("Entered");
    return m_exposure_program;
}

void ExifInformation::setExposureProgram(ExposureProgram exposure_program)
{
    LOGD("Entered");
    if (EXIF_EXPOSURE_PROGRAM_NOT_VALID == exposure_program) {
        LOGW("Trying to set NOT VALID exposure program");
        return;
    }

    m_is_set[EXIF_INFORMATION_ATTRIBUTE_EXPOSURE_PROGRAM] = true;
    m_exposure_program = exposure_program;
}

bool ExifInformation::getFlash() const
{
    LOGD("Entered");
    return m_flash;
}

void ExifInformation::setFlash(bool flash)
{
    LOGD("Entered");
    m_is_set[EXIF_INFORMATION_ATTRIBUTE_FLASH] = true;
    m_flash = flash;
}

const Rational& ExifInformation::getFocalLength() const
{
    LOGD("Entered");
    return m_focal_length;
}

void ExifInformation::setFocalLength(Rational focal_length)
{
    LOGD("Entered");
    if(!focal_length.isValid()) {
        LOGW("Trying to set invalid focal length: %s", focal_length.toString().c_str());
        return;
    }

    m_is_set[EXIF_INFORMATION_ATTRIBUTE_FOCAL_LENGTH] = true;
    m_focal_length = focal_length;
}

WhiteBalanceMode ExifInformation::getWhiteBalanceMode() const
{
    LOGD("Entered");
    return m_white_balance;
}

void ExifInformation::setWhiteBalanceMode(WhiteBalanceMode white_balance)
{
    LOGD("Entered");
    if (EXIF_WHITE_BALANCE_MODE_NOT_VALID == white_balance) {
        LOGW("Trying to set NOT VALID white balance mode");
        return;
    }

    m_is_set[EXIF_INFORMATION_ATTRIBUTE_WHITE_BALANCE] = true;
    m_white_balance = white_balance;
}

ExifGPSLocation& ExifInformation::getGPSExifLocation()
{
    return m_exif_gps_location;
}

Tizen::SimpleCoordinatesPtr ExifInformation::getGPSLocation()
{
    if(m_gps_location) {
        return m_gps_location;
    }


    Tizen::SimpleCoordinatesPtr nscoords = m_exif_gps_location.getSimpleCoordinates();
    if(!nscoords) {
        return nscoords;
    }

    m_gps_location = nscoords;
    return m_gps_location;
}

void ExifInformation::setGPSLocation(Tizen::SimpleCoordinatesPtr gps_location)
{
    if(!gps_location) {
        LOGW("Trying to set NULL gps location!");
        return;
    }

    m_gps_location = gps_location;
}

void ExifInformation::unsetGPSLocation()
{
    m_gps_location = Tizen::SimpleCoordinatesPtr();
    m_exif_gps_location.unsetAll();
}

const Rational& ExifInformation::getGpsAltitude() const
{
    LOGD("Entered");
    return m_gps_altitude;
}

void ExifInformation::setGpsAltitude(Rational gps_altitude)
{
    LOGD("Entered");
    if(!gps_altitude.isValid()) {
        LOGW("Trying to set invalid gps altitude: %s", gps_altitude.toString().c_str());
        return;
    }

    m_is_set[EXIF_INFORMATION_ATTRIBUTE_GPS_ALTITUDE] = true;
    m_gps_altitude = gps_altitude;
}

GpsAltitudeRef ExifInformation::getGpsAltitudeRef() const
{
    LOGD("Entered");
    return m_gps_altitude_ref;
}

void ExifInformation::setGpsAltitudeRef(const GpsAltitudeRef ref)
{
    LOGD("Entered");
    m_is_set[EXIF_INFORMATION_ATTRIBUTE_GPS_ALTITUDE_REF] = true;
    m_gps_altitude_ref = ref;
}

void ExifInformation::setGpsAltitudeWithRef(double gps_altitude)
{
    LOGD("Entered");
    setGpsAltitude(Rational::createFromDouble(fabs(gps_altitude)));

    if(gps_altitude >= 0.0) {
        setGpsAltitudeRef(GPS_ALTITUDE_REF_ABOVE_SEA);
    } else {
        setGpsAltitudeRef(GPS_ALTITUDE_REF_BELOW_SEA);
    }

}

double ExifInformation::getGpsAltitudeWithRef() const
{
    LOGD("Entered");

    if(!m_is_set[EXIF_INFORMATION_ATTRIBUTE_GPS_ALTITUDE_REF] ||
            GPS_ALTITUDE_REF_ABOVE_SEA == m_gps_altitude_ref) {
        return m_gps_altitude.toDouble();
    } else {
        return -1.0 * m_gps_altitude.toDouble();
    }
}

const std::string& ExifInformation::getGpsProcessingMethod() const
{
    LOGD("Entered");
    return m_gps_processing_method;
}

const std::string& ExifInformation::getGpsProcessingMethodType() const
{
    LOGD("Entered");
    return m_gps_processing_method_type;
}

void ExifInformation::setGpsProcessingMethod(const std::string& type,
        const std::string& processing_method)
{
    LOGD("Entered");
    if(type != EXIF_UNDEFINED_TYPE_ASCII &&
            type != EXIF_UNDEFINED_TYPE_JIS &&
            type != EXIF_UNDEFINED_TYPE_UNICODE &&
            type != EXIF_UNDEFINED_TYPE_UNDEFINED) {
        LOGW("Trying to set invalid GPSProcessingMethod type: [%s] len:%d",
                type.c_str(), type.length());
        return;
    }

    m_is_set[EXIF_INFORMATION_ATTRIBUTE_GPS_PROCESSING_METHOD] = true;
    m_gps_processing_method = processing_method;
    m_gps_processing_method_type = type;
}

ExifGPSTime& ExifInformation::getExifGpsTime()
{
    return m_exif_gps_time;
}

const ExifGPSTime& ExifInformation::getExifGpsTime() const
{
    return m_exif_gps_time;
}

Time::TZDatePtr ExifInformation::getGpsTime()
{
    if(m_gps_time) {
        return m_gps_time;
    }

    if(!m_exif_gps_time.isValid()) {
        return Time::TZDatePtr();
    }


    m_gps_time = m_exif_gps_time.getTZDate();
    return m_gps_time;
}

void ExifInformation::setGpsTime(Time::TZDatePtr new_time)
{
    if(!new_time) {
        LOGW("Trying to set null new_time!");
        return;
    }

    m_gps_time = new_time;
}

void ExifInformation::unsetGPStime()
{
    m_exif_gps_time.unsetAll();
    m_gps_time = NULL;
}


const std::string& ExifInformation::getUserComment()
{
    LOGD("Entered");
    return m_user_comment;
}

const std::string& ExifInformation::getUserCommentType()
{
    LOGD("Entered");
    return m_user_comment_type;
}

void ExifInformation::setUserComment(const std::string& type,
        const std::string& user_comment)
{
    LOGD("Entered");
    if(type != EXIF_UNDEFINED_TYPE_ASCII &&
            type != EXIF_UNDEFINED_TYPE_JIS &&
            type != EXIF_UNDEFINED_TYPE_UNICODE &&
            type != EXIF_UNDEFINED_TYPE_UNDEFINED) {
        LOGW("Trying to set invalid user comment type: [%s] len:%d",
                type.c_str(), type.length());
        return;
    }

    m_is_set[EXIF_INFORMATION_ATTRIBUTE_USER_COMMENT] = true;
    m_user_comment_type = type;
    m_user_comment = user_comment;
}

bool ExifInformation::isSet(ExifInformationAttribute attribute) const
{
    LOGD("Entered");
    return m_is_set[attribute];
}

void ExifInformation::unset(ExifInformationAttribute attribute)
{
    LOGD("Entered");
    if(attribute >= EXIF_INFORMATION_ATTRIBUTE_NUMBER_OF_ATTRIBUTES) {
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
        case EXIF_INFORMATION_ATTRIBUTE_USER_COMMENT:
            m_user_comment = std::string();
            m_user_comment_type = EXIF_UNDEFINED_TYPE_ASCII;
            break;
        default:
            break;
    }
}

bool getGCSPositionFromEntry(ExifEntry *entry, ExifData* exif_data, GCSPosition& out_pos)
{
    //RATIONAL - 3
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
    }
    else {
        return false;
    }
}

bool getRationalsFromEntry(ExifEntry *entry, ExifData* exif_data,
            unsigned long required_count, Rationals& out_rationals)
{
    if (EXIF_FORMAT_RATIONAL == entry->format &&
            entry->components >= required_count &&
            entry->data) {
        const ExifByteOrder order = exif_data_get_byte_order(exif_data);
        unsigned char* ptr = entry->data;

        for(unsigned long i = 0; i < required_count; ++i) {
            out_rationals.push_back(Rational(exif_get_rational(ptr, order)));
            ptr += ExifTypeInfo::RationalSize;
        }

        return true;
    }
    else {
        return false;
    }
}

Rational getRationalFromEntry(ExifEntry *entry, ExifData* exif_data)
{
    if (EXIF_FORMAT_RATIONAL == entry->format && entry->components >= 1 && entry->data) {
        const ExifByteOrder order = exif_data_get_byte_order(exif_data);
        return Rational(exif_get_rational(entry->data, order));
    }
    else {
        return Rational::createInvalid();
    }
}

bool decomposeExifUndefined(ExifEntry* entry, std::string& type, std::string& value)
{
    if(!entry || !entry->data) {
        LOGW("exif entry is NULL/empty");
        return false;
    }

    if(entry->size < EXIF_UNDEFINED_TYPE_LENGTH) {
        LOGW("entry size is invalid %d < EXIF_UNDEFINED_TYPE_LENGTH", entry->size);
        return false;
    }

    const char* ptr = reinterpret_cast<const char*>(entry->data);
    type = std::string(ptr, EXIF_UNDEFINED_TYPE_LENGTH);
    ptr += EXIF_UNDEFINED_TYPE_LENGTH;
    value = std::string(ptr, entry->size - EXIF_UNDEFINED_TYPE_LENGTH);
    return true;
}

void ExifInformation::processEntry(ExifEntry* entry, ExifData* exif_data)
{
    char buf[2000];
    exif_entry_get_value(entry, buf, sizeof(buf));
    ExifUtil::printExifEntryInfo(entry, exif_data);

    const ExifIfd cur_ifd = exif_entry_get_ifd(entry);
    if (EXIF_IFD_INTEROPERABILITY == cur_ifd || EXIF_IFD_1 == cur_ifd) {
        return;
    }

    switch (static_cast<unsigned int>(entry->tag)) {
        case EXIF_TAG_IMAGE_WIDTH: {
            //SHORT or LONG - 1
            exif_entry_get_value(entry, buf, sizeof(buf));
            LOGD( "Setting ExifInformation width to: [%s]", buf );
            setWidth(atol(buf));
            break;
        }
        case EXIF_TAG_IMAGE_LENGTH: {
            //SHORT or LONG - 1
            exif_entry_get_value(entry, buf, sizeof(buf));
            LOGD( "Setting ExifInformation height to: [%s]", buf );
            setHeight(atol(buf));
            break;
        }
        case EXIF_TAG_MAKE: {
            //ASCII - Any
            exif_entry_get_value(entry, buf, sizeof(buf));
            LOGD( "Setting ExifInformation maker to: [%s]", buf );
            setDeviceMaker(std::string(buf));
            break;
        }
        case EXIF_TAG_MODEL: {
            //ASCII - Any
            exif_entry_get_value(entry, buf, sizeof(buf));
            LOGD( "Setting ExifInformation model to: [%s]", buf );
            setDeviceModel(std::string(buf));
            break;
        }
        case EXIF_TAG_DATE_TIME_ORIGINAL: {
            //ASCII - 20
            exif_entry_get_value(entry, buf, sizeof(buf));
            const time_t time = ExifUtil::exifDateTimeOriginalToTimeT(
                    reinterpret_cast<const char*>(entry->data));
            LOGD( "Setting ExifInformation time original to: [%s] time_t:%d", buf,
                    (int)time);
            setOriginalTime(time);
        }
        case EXIF_TAG_ORIENTATION: {
            //SHORT - 1
            exif_entry_get_value(entry, buf, sizeof(buf));
            const ExifByteOrder order = exif_data_get_byte_order(exif_data);
            const ExifShort orient(exif_get_short(entry->data, order));

            if(orient < EXIF_ORIENTATION_NORMAL || orient >= EXIF_ORIENTATION_NOT_VALID) {
                LOGW("Couldn't set ExifInformation - orientation is not valid: %d (%s)",
                        orient, buf);
            }
            else {
                LOGD("Setting ExifInformation orientation to: %d [%s]", orient, buf);
                setOrientation(static_cast<ImageOrientation>(orient));
            }
            break;
        }
        case EXIF_TAG_FNUMBER:
        {
            //RATIONAL - 1
            Rational fnumber = getRationalFromEntry(entry, exif_data);
            if(fnumber.isValid()) {
                LOGD("Setting ExifInformation fnumber to: %f (%s)", fnumber.toDouble(),
                    fnumber.toString().c_str());
                setFNumber(fnumber);
            }
            else {
                LOGW("Couldn't set ExifInformation - fnumber is not valid: %s",
                        fnumber.toString().c_str());
            }
            break;
        }
        case EXIF_TAG_ISO_SPEED_RATINGS: {
            //SHORT - Any
            if (EXIF_FORMAT_SHORT == entry->format &&
                    entry->components > 0 &&
                    entry->data) {
                const ExifByteOrder order = exif_data_get_byte_order(exif_data);
                unsigned char* read_ptr = entry->data;
                const size_t size_per_member =
                        ExifUtil::getSizeOfExifFormatType(entry->format);

                for(unsigned long i = 0; i < entry->components; ++i) {
                    ExifShort iso_rating = exif_get_short(read_ptr, order);
                    appendIsoSpeedRatings(iso_rating);

                    LOGD("Appending ExifInformation speed ratings with: %d",
                            static_cast<int>(iso_rating));

                    read_ptr += size_per_member;
                }
            }
            else {
                LOGE("iso speed ratings: format or components count is invalid!");
            }
            break;
        }
        case EXIF_TAG_EXPOSURE_TIME: {
            //RATIONAL - 1
            if (EXIF_FORMAT_RATIONAL == entry->format &&
                    entry->components > 0 &&
                    entry->data) {

                const ExifByteOrder order = exif_data_get_byte_order(exif_data);
                const Rational exp_time(exif_get_rational(entry->data, order));

                if (exp_time.isValid()) {
                    LOGD("Setting ExifInformation exposure time to: %s (%s)",
                            exp_time.toString().c_str(),
                            exp_time.toExposureTimeString().c_str());
                    setExposureTime(exp_time);
                }
                else {
                    LOGD("Couldn't set ExifInformation - exposure time is not valid: %s",
                            exp_time.toString().c_str());
                }
            }
            else {
                LOGE("exposure time: format or components count is invalid!");
            }
            break;
        }
        case EXIF_TAG_EXPOSURE_PROGRAM: {
            //SHORT - 1
            exif_entry_get_value(entry, buf, sizeof(buf));

            const ExifByteOrder order = exif_data_get_byte_order(exif_data);
            const ExifShort exp_program = exif_get_short(entry->data, order);
            if(exp_program >= EXIF_EXPOSURE_PROGRAM_NOT_VALID) {
                LOGW("ExposureProgram: %d (%s) is not valid!", exp_program, buf);
            }
            else {
                LOGD("Setting ExifInformation exposure program to: %d [%s]",
                        exp_program, buf );
                setExposureProgram(static_cast<ExposureProgram>(exp_program));
            }
            break;
        }
        case EXIF_TAG_FLASH: {
            //SHORT - 1
            exif_entry_get_value(entry, buf, sizeof(buf));

            const ExifByteOrder order = exif_data_get_byte_order(exif_data);
            const ExifShort flash = exif_get_short(entry->data, order);

            LOGD( "Setting ExifInformation flash to: [%s] flash=%d", buf, flash);
            setFlash(flash != 0);
            break;
        }
        case EXIF_TAG_FOCAL_LENGTH: {
            //RATIONAL - 1
            Rational flength = getRationalFromEntry(entry, exif_data);
            if(flength.isValid()) {
                LOGD("Setting ExifInformation focal length to: %f (%s)",
                        flength.toDouble(), flength.toString().c_str());
                setFocalLength(flength);
            }
            else {
                LOGW("Couldn't set ExifInformation - focal length is not valid: %s",
                        flength.toString().c_str());
            }
            break;
        }
        case EXIF_TAG_WHITE_BALANCE: {
            //SHORT - 1
            exif_entry_get_value(entry, buf, sizeof(buf));
            LOGD( "Setting ExifInformation white balance to: [%s]", buf );
            if (entry->data[0]) {
                setWhiteBalanceMode(EXIF_WHITE_BALANCE_MODE_MANUAL);
            }
            else {
                setWhiteBalanceMode(EXIF_WHITE_BALANCE_MODE_AUTO);
            }
            break;
        }
        case EXIF_TAG_GPS_LONGITUDE: {
            //RATIONAL - 3
            GCSPosition longitude;
            if (getGCSPositionFromEntry(entry, exif_data, longitude)) {
                m_exif_gps_location.setLongitude(longitude);
                LOGD("Setting ExifInformation gps longitude to: %s; %s; %s valid:%d",
                        longitude.degrees.toString().c_str(),
                        longitude.minutes.toString().c_str(),
                        longitude.seconds.toString().c_str(),
                        longitude.isValid());
            }
            else {
                exif_entry_get_value(entry, buf, sizeof(buf));
                LOGW("Couldn't set longitude pos - data is not valid: [%s]", buf);
            }
            break;
        }
        case EXIF_TAG_GPS_LONGITUDE_REF: {
            //ASCII - 2
            if(entry->size < 1) {
                LOGW("Longtitude ref entry do not contain enought data!");
                break;
            }

            const char ref = static_cast<char>(entry->data[0]);
            if ('E' == ref || 'e' == ref) {          //East
                m_exif_gps_location.setLongitudeRef(GPS_LOCATION_EAST);
                LOGD("Setting ExifInformation gps longitude REF to: EAST");
            }
            else if ('W' == ref || 'w' == ref) {     //West
                m_exif_gps_location.setLongitudeRef(GPS_LOCATION_WEST);
                LOGD("Setting ExifInformation gps longitude REF to: WEST");
            }
            else {
                LOGW("Unknown longitude ref: %c (0x%x)", ref, static_cast<int>(ref));
            }
            break;
        }
        case EXIF_TAG_GPS_LATITUDE: {
            //RATIONAL - 3
            exif_entry_get_value(entry, buf, sizeof(buf));
            LOGD( "Setting ExifInformation latitude to: [%s], tag->%s",
                    buf, exif_tag_get_name(entry->tag) );

            GCSPosition latitude;
            if (getGCSPositionFromEntry(entry, exif_data, latitude)) {
                m_exif_gps_location.setLatitude(latitude);
                LOGD("Setting ExifInformation gps latitude to: %s; %s; %s valid:%d",
                        latitude.degrees.toString().c_str(),
                        latitude.minutes.toString().c_str(),
                        latitude.seconds.toString().c_str(),
                        latitude.isValid());
            }
            else {
                LOGW("Couldn't set latitude pos - data is not valid!");
            }
            break;
        }
        case EXIF_TAG_GPS_LATITUDE_REF: {
            //ASCII - 2
            if(entry->size < 1) {
                LOGW("Latitude ref entry do not contain enought data!");
                break;
            }

            const char ref = static_cast<char>(entry->data[0]);
            if ('N' == ref || 'n' == ref) {          //North
                m_exif_gps_location.setLatitudeRef(GPS_LOCATION_NORTH);
                LOGD("Setting ExifInformation gps latitude REF to: NORTH");
            }
            else if ('S' == ref || 's' == ref) {     //South
                m_exif_gps_location.setLatitudeRef(GPS_LOCATION_SOUTH);
                LOGD("Setting ExifInformation gps latitude REF to: SOUTH");
            }
            else {
                LOGW("Unknown latitude ref: %c (0x%x)", ref, static_cast<int>(ref));
            }
            break;
        }
        case EXIF_TAG_GPS_ALTITUDE: {
            //RATIONAL - 1
            Rational gps_altitude = getRationalFromEntry(entry, exif_data);
            if(gps_altitude.isValid()) {
                LOGD("Setting ExifInformation gps altitude to: %f (%s)",
                        gps_altitude.toDouble(), gps_altitude.toString().c_str());
                setGpsAltitude(gps_altitude);
            }
            else {
                LOGW("Couldn't set ExifInformation - gps altitude is not valid: %s",
                        gps_altitude.toString().c_str());
            }
            break;
        }
        case EXIF_TAG_GPS_ALTITUDE_REF: {
            //BYTE - 1
            const ExifByte altitude_ref = static_cast<ExifByte>(entry->data[0]);
            if(static_cast<ExifByte>(GPS_ALTITUDE_REF_ABOVE_SEA) == altitude_ref ||
                    static_cast<ExifByte>(GPS_ALTITUDE_REF_BELOW_SEA) == altitude_ref) {
                setGpsAltitudeRef(static_cast<GpsAltitudeRef>(altitude_ref));
                LOGD( "Setting ExifInformation gps altitude ref to: %d (%s)",
                        static_cast<int>(altitude_ref),
                        (altitude_ref > 0) ? "below sea" : "above sea");
            } else {
                LOGW("GPS altitude ref is invalid:%d should be 0 or 1!",
                        static_cast<int>(altitude_ref));
            }
            break;
        }
        case EXIF_TAG_GPS_PROCESSING_METHOD: {
            //UNDEFINED - Any
            std::string type, value;
            if(decomposeExifUndefined(entry, type, value)) {
                LOGD("Extracted GPSProcessingMethod: [%s], len:%d, type:%s",
                        value.c_str(), value.length(), type.c_str());
                setGpsProcessingMethod(type, value);

                LOGD("Setting ExifInformation processing method to: [%s], len:%d, type:%s",
                        m_gps_processing_method.c_str(),
                        m_gps_processing_method.length(),
                        m_gps_processing_method_type.c_str());
            }
            else {
                LOGW("GPSProcessingMethod tag contains invalid values!");
            }
            break;
        }
        case EXIF_TAG_GPS_DATE_STAMP: {
            //ASCII - 11
            exif_entry_get_value(entry, buf, sizeof(buf));
            LOGD( "Setting ExifInformation gps date stamp to: [%s]", buf );
            m_exif_gps_time.setDate(buf);
            break;
        }
        case EXIF_TAG_GPS_TIME_STAMP: {
            //Rational - 3
            exif_entry_get_value(entry, buf, sizeof(buf));
            LOGD( "Setting ExifInformation gps time stamp to: [%s]", buf);

            Rationals time;
            if (getRationalsFromEntry(entry, exif_data, 3, time)) {
                m_exif_gps_time.setTime(time);
            }
            break;
        }
        case EXIF_TAG_USER_COMMENT: {
            //UNDEFINED - Any
            std::string type, value;
            if(decomposeExifUndefined(entry, type, value)) {
                LOGD("Extracted UserComment: [%s], len:%d, type:%s",
                        value.c_str(), value.length(), type.c_str());
                setUserComment(type, value);

                LOGD("Setting ExifInformation user comment to: [%s], len:%d, type:%s",
                        m_user_comment.c_str(),
                        m_user_comment.length(),
                        m_user_comment_type.c_str());
            }
            else {
                LOGW("UserComment tag contains invalid values!");
            }

            break;
        }
        default:
            LOGD("Field of tag:%x.H [%s] is not supported, value: [%s]", entry->tag,
                    exif_tag_get_name_in_ifd(entry->tag, cur_ifd),
                    exif_entry_get_value(entry, buf, sizeof(buf)));
    }
}

struct ExifInfoAndDataHolder {
    ExifInformationPtr exif_info;
    ExifData* exif_data;
};

void ExifInformation::contentForeachFunctionProxy(ExifEntry *entry, void *user_data)
{
    ExifInfoAndDataHolder* holder = static_cast<ExifInfoAndDataHolder*>(user_data);
    if (!holder) {
        LOGE("holder is NULL");
    }

    if (!holder->exif_info) {
        LOGE("exif_info is NULL!");
        return;
    }

    if (!holder->exif_data) {
        LOGE("exif_data is NULL!");
        return;
    }

    try {
        holder->exif_info->processEntry(entry, holder->exif_data);
    }
    catch(const BasePlatformException &err) {
        LOGE("processEntry thrown exception: %s : %s", err.getName().c_str(),
                    err.getMessage().c_str());
    }
    catch(...) {
        LOGE("Unsupported error while processing Exif entry.");
    }
}

void ExifInformation::dataForeachFunction(ExifContent *content, void *user_data)
{
    exif_content_foreach_entry(content, contentForeachFunctionProxy, user_data);
}


ExifInformationPtr ExifInformation::loadFromURI(const std::string& uri)
{
    ExifInformationPtr exif_info(new ExifInformation());
    exif_info->setUri(uri);

    const std::string file_path = ExifUtil::convertUriToPath(uri);
    ExifData* ed = exif_data_new_from_file (file_path.c_str());
    if (!ed) {
        LOGE("Error reading exif from file %s", file_path.c_str());
        throw NotFoundException("Error reading exif from file");
    }

    LOGD("exif_data_foreach_content START");

    ExifInfoAndDataHolder holder;
    holder.exif_info = exif_info;
    holder.exif_data = ed;
    exif_data_foreach_content(ed, dataForeachFunction, static_cast<void*>(&holder));

    LOGD("exif_data_foreach_content END");

    exif_data_unref(ed);
    ed = NULL;

    return exif_info;
}


void ExifInformation::removeNulledAttributesFromExifData(ExifData* exif_data)
{
    LOGD("Entered");
    if(!exif_data) {
        LOGE("exif_data is NULL");
        throw UnknownException("Invalid Exif provided");
    }

    if (!isSet(EXIF_INFORMATION_ATTRIBUTE_WIDTH)) {
        LOGD("Removing width");
        ExifTagSaver::removeExifEntryWithTag(EXIF_TAG_IMAGE_WIDTH, exif_data);
    }
    if (!isSet(EXIF_INFORMATION_ATTRIBUTE_HEIGHT)) {
        LOGD("Removing height");
        ExifTagSaver::removeExifEntryWithTag(EXIF_TAG_IMAGE_LENGTH, exif_data);
    }
    if (!isSet(EXIF_INFORMATION_ATTRIBUTE_DEVICE_MAKER)) {
        LOGD("Removing device maker");
        ExifTagSaver::removeExifEntryWithTag(EXIF_TAG_MAKE, exif_data);
    }
    if (!isSet(EXIF_INFORMATION_ATTRIBUTE_ORIENTATION)) {
        LOGD("Removing orientation");
        ExifTagSaver::removeExifEntryWithTag(EXIF_TAG_ORIENTATION, exif_data);
    }
    if (!isSet(EXIF_INFORMATION_ATTRIBUTE_EXPOSURE_PROGRAM)) {
        LOGD("Removing exposure program");
        ExifTagSaver::removeExifEntryWithTag(EXIF_TAG_EXPOSURE_PROGRAM, exif_data);
    }
    if (!isSet(EXIF_INFORMATION_ATTRIBUTE_ISO_SPEED_RATINGS)) {
        LOGD("Removing iso speed ratings");
        ExifTagSaver::removeExifEntryWithTag(EXIF_TAG_ISO_SPEED_RATINGS, exif_data);
    }
    if (!isSet(EXIF_INFORMATION_ATTRIBUTE_WHITE_BALANCE)) {
        LOGD("Removing white balance");
        ExifTagSaver::removeExifEntryWithTag(EXIF_TAG_WHITE_BALANCE, exif_data);
    }
    if (!isSet(EXIF_INFORMATION_ATTRIBUTE_DEVICE_MODEL)) {
        LOGD("Removing device model");
        ExifTagSaver::removeExifEntryWithTag(EXIF_TAG_MODEL, exif_data);
    }
    if (!isSet(EXIF_INFORMATION_ATTRIBUTE_ORIGINAL_TIME)) {
        LOGD("Removing original time");
        ExifTagSaver::removeExifEntryWithTag(EXIF_TAG_DATE_TIME_ORIGINAL, exif_data);
    }
    if (!isSet(EXIF_INFORMATION_ATTRIBUTE_EXPOSURE_TIME)) {
        LOGD("Removing exposure time");
        ExifTagSaver::removeExifEntryWithTag(EXIF_TAG_EXPOSURE_TIME, exif_data);
    }
    if (!isSet(EXIF_INFORMATION_ATTRIBUTE_FNUMBER)) {
        LOGD("Removing f-number");
        ExifTagSaver::removeExifEntryWithTag(EXIF_TAG_FNUMBER, exif_data);
    }
    if (!isSet(EXIF_INFORMATION_ATTRIBUTE_FLASH)) {
        LOGD("Removing flash");
        ExifTagSaver::removeExifEntryWithTag(EXIF_TAG_FLASH, exif_data);
    }
    if (!isSet(EXIF_INFORMATION_ATTRIBUTE_FOCAL_LENGTH)) {
        LOGD("Removing focal length");
        ExifTagSaver::removeExifEntryWithTag(EXIF_TAG_FOCAL_LENGTH, exif_data);
    }
    if (!isSet(EXIF_INFORMATION_ATTRIBUTE_GPS_ALTITUDE)) {
        LOGD("Removing gps altitude");
        ExifTagSaver::removeExifEntryWithTag(
            static_cast<ExifTag>(EXIF_TAG_GPS_ALTITUDE), exif_data);
    }
    if (!isSet(EXIF_INFORMATION_ATTRIBUTE_GPS_ALTITUDE_REF)) {
        LOGD("Removing gps altitude ref");
        ExifTagSaver::removeExifEntryWithTag(
                static_cast<ExifTag>(EXIF_TAG_GPS_ALTITUDE_REF), exif_data);
    }
    if (!isSet(EXIF_INFORMATION_ATTRIBUTE_GPS_PROCESSING_METHOD)) {
        LOGD("Removing gps processing method");
        ExifTagSaver::removeExifEntryWithTag(
                static_cast<ExifTag>(EXIF_TAG_GPS_PROCESSING_METHOD), exif_data);
    }
    if (!isSet(EXIF_INFORMATION_ATTRIBUTE_USER_COMMENT)) {
        LOGD("Removing user comment");
        ExifTagSaver::removeExifEntryWithTag(EXIF_TAG_USER_COMMENT, exif_data);
    }

    if (!m_exif_gps_location.isSet(EXIF_GPS_LOCATION_ATTRIBUTE_LATITUDE)) {
        LOGD("Removing latitude");
        ExifTagSaver::removeExifEntryWithTag(
                static_cast<ExifTag>(EXIF_TAG_GPS_LATITUDE), exif_data);
    }
    if (!m_exif_gps_location.isSet(EXIF_GPS_LOCATION_ATTRIBUTE_LATITUDE_REF)) {
        LOGD("Removing latitude ref");
        ExifTagSaver::removeExifEntryWithTag(
                static_cast<ExifTag>(EXIF_TAG_GPS_LATITUDE_REF), exif_data);
    }
    if (!m_exif_gps_location.isSet(EXIF_GPS_LOCATION_ATTRIBUTE_LONGITUDE)) {
        LOGD("Removing longitude");
        ExifTagSaver::removeExifEntryWithTag(
                static_cast<ExifTag>(EXIF_TAG_GPS_LONGITUDE), exif_data);
    }
    if (!m_exif_gps_location.isSet(EXIF_GPS_LOCATION_ATTRIBUTE_LONGITUDE_REF)) {
        LOGD("Removing longitude ref");
        ExifTagSaver::removeExifEntryWithTag(
                static_cast<ExifTag>(EXIF_TAG_GPS_LONGITUDE_REF), exif_data);
    }

    if (!m_exif_gps_time.isTimeSet()) {
        LOGD("Removing gps time");
        ExifTagSaver::removeExifEntryWithTag(
                static_cast<ExifTag>(EXIF_TAG_GPS_TIME_STAMP), exif_data);
    }
    if (!m_exif_gps_time.isDateSet()) {
        LOGD("Removing gps date");
        ExifTagSaver::removeExifEntryWithTag(
                static_cast<ExifTag>(EXIF_TAG_GPS_DATE_STAMP), exif_data);
    }
}

void ExifInformation::updateAttributesInExifData(ExifData* exif_data)
{
    LOGD("Entered");
    if(!exif_data) {
        LOGE("exif_data is NULL");
        throw UnknownException("Invalid Exif provided");
    }

    if (isSet(EXIF_INFORMATION_ATTRIBUTE_WIDTH)) {
        LOGD("Saving width: %d", getWidth());
        ExifTagSaver::saveToExif(getWidth(),
                EXIF_TAG_IMAGE_WIDTH, exif_data);
    }
    if (isSet(EXIF_INFORMATION_ATTRIBUTE_HEIGHT)) {
        LOGD("Saving height: %d", getHeight());
        ExifTagSaver::saveToExif(getHeight(),
                EXIF_TAG_IMAGE_LENGTH, exif_data);
    }
    if (isSet(EXIF_INFORMATION_ATTRIBUTE_DEVICE_MAKER)) {
        LOGD("Saving device maker: %s", getDeviceMaker().c_str());
        ExifTagSaver::saveToExif(getDeviceMaker(),
                EXIF_TAG_MAKE, exif_data);
    }
    if (isSet(EXIF_INFORMATION_ATTRIBUTE_ORIENTATION)) {
        LOGD("Saving orientation: %d", static_cast<int>(getOrientation()));
        ExifTagSaver::saveToExif(static_cast<long int>(getOrientation()),
                EXIF_TAG_ORIENTATION, exif_data);
    }
    if (isSet(EXIF_INFORMATION_ATTRIBUTE_EXPOSURE_PROGRAM)) {
        LOGD("Saving exposure program: %d", static_cast<int>(getExposureProgram()));
        ExifTagSaver::saveToExif(getExposureProgram(),
                EXIF_TAG_EXPOSURE_PROGRAM, exif_data);
    }
    if (isSet(EXIF_INFORMATION_ATTRIBUTE_ISO_SPEED_RATINGS)) {
        std::vector<long long int> iso_ratings = getIsoSpeedRatings();
        LOGD("Saving iso speed ratings count:%d", iso_ratings.size());
        ExifTagSaver::saveToExif(iso_ratings, EXIF_FORMAT_SHORT,
                EXIF_TAG_ISO_SPEED_RATINGS, exif_data);
    }
    if (isSet(EXIF_INFORMATION_ATTRIBUTE_WHITE_BALANCE)) {
        LOGD("Saving white balance: %d", static_cast<int>(getWhiteBalanceMode()));
        ExifTagSaver::saveToExif(getWhiteBalanceMode(),
                EXIF_TAG_WHITE_BALANCE, exif_data);
    }
    if (isSet(EXIF_INFORMATION_ATTRIBUTE_DEVICE_MODEL)) {
        LOGD("Saving device model: %s", getDeviceModel().c_str());
        ExifTagSaver::saveToExif(getDeviceModel(),
                EXIF_TAG_MODEL, exif_data);
    }
    if (isSet(EXIF_INFORMATION_ATTRIBUTE_ORIGINAL_TIME)) {
        const time_t o_time = getOriginalTime();
        const std::string o_time_str = ExifUtil::timeTToExifDateTimeOriginal(o_time);
        LOGD("Saving original time time_t:%d, format:%s", static_cast<int>(o_time),
                o_time_str.c_str());

        ExifTagSaver::saveToExif(o_time_str,
                EXIF_TAG_DATE_TIME_ORIGINAL, exif_data);
    }
    if (isSet(EXIF_INFORMATION_ATTRIBUTE_EXPOSURE_TIME)) {
        Rational exposure_time = getExposureTime();
        if (exposure_time.isValid()) {
            LOGD("Saving exposure time: %s (%s)",
                    exposure_time.toString().c_str(),
                    exposure_time.toExposureTimeString().c_str());

            ExifTagSaver::saveToExif(exposure_time,
                    EXIF_TAG_EXPOSURE_TIME, exif_data);
        }
    }
    if (isSet(EXIF_INFORMATION_ATTRIBUTE_FNUMBER)) {
        auto f_number = getFNumber();
        LOGD("Saving f-number: %f (%s)", f_number.toDouble(),
                f_number.toString().c_str());
        ExifTagSaver::saveToExif(f_number,
                EXIF_TAG_FNUMBER, exif_data);
    }
    if (isSet(EXIF_INFORMATION_ATTRIBUTE_FLASH)) {
        LOGD("Saving flash: %s", getFlash() ? "ON" : "OFF");
        ExifTagSaver::saveToExif(getFlash(),
                EXIF_TAG_FLASH, exif_data);
    }
    if (isSet(EXIF_INFORMATION_ATTRIBUTE_FOCAL_LENGTH)) {
        auto f_length = getFocalLength();
        LOGD("Saving focal length:%f (%s)", f_length.toDouble(),
                f_length.toString().c_str());
        ExifTagSaver::saveToExif(f_length,
                EXIF_TAG_FOCAL_LENGTH, exif_data);
    }
    if (isSet(EXIF_INFORMATION_ATTRIBUTE_GPS_ALTITUDE)) {
        LOGD("Saving gps altitude:%f (%s)", m_gps_altitude.toDouble(),
                 m_gps_altitude.toString().c_str());
        ExifTagSaver::saveToExif(m_gps_altitude,
                static_cast<ExifTag>(EXIF_TAG_GPS_ALTITUDE), exif_data);
    }
    if (isSet(EXIF_INFORMATION_ATTRIBUTE_GPS_ALTITUDE_REF)) {
        //Exif spec:
        //0 = Sea level
        //1 = Sea level reference (negative value)
        LOGD("Saving gps altitude ref:%d (%s)", static_cast<int>(m_gps_altitude_ref),
                (m_gps_altitude_ref > 0) ? "below sea" : "above sea");
        ExifTagSaver::saveToExif(static_cast<long int>(m_gps_altitude_ref),
                static_cast<ExifTag>(EXIF_TAG_GPS_ALTITUDE_REF), exif_data);
    }
    if (isSet(EXIF_INFORMATION_ATTRIBUTE_GPS_PROCESSING_METHOD)) {
        LOGD("Saving gps processing method: [%s] type:%s",
                getGpsProcessingMethod().c_str(), getGpsProcessingMethodType().c_str());

        const std::string joined = getGpsProcessingMethodType() + getGpsProcessingMethod();
        LOGD("joined: [%s]", joined.c_str());

        ExifTagSaver::saveToExif(joined,
                static_cast<ExifTag>(EXIF_TAG_GPS_PROCESSING_METHOD), exif_data, false);
    }
    if (isSet(EXIF_INFORMATION_ATTRIBUTE_USER_COMMENT)) {
        LOGD("Saving user comment: %s (type:%s)", getUserComment().c_str(),
            getUserCommentType().c_str());

        const std::string joined = getUserCommentType() + getUserComment();
        LOGD("joined: [%s]", joined.c_str());

        ExifTagSaver::saveToExif(joined,
                EXIF_TAG_USER_COMMENT, exif_data, false);
    }

    if(m_gps_location) {
        m_exif_gps_location.set(m_gps_location);
    }
    ExifTagSaver::saveGpsLocationToExif(m_exif_gps_location, exif_data);

    if(m_gps_time) {
        m_exif_gps_time.setDateAndTime(m_gps_time);
    }
    ExifTagSaver::saveGpsTimeToExif(m_exif_gps_time, exif_data);
}

void ExifInformation::saveToFile(const std::string& file_path)
{
    LOGD("Entered");
    LOGD("Using JpegFile to read: [%s] and Exif if present", file_path.c_str());

    bool exif_data_is_new = false;
    JpegFilePtr jpg_file = JpegFile::loadFile(file_path);
    ExifData* exif_data = jpg_file->getExifData();

    //Exif is not present in file - create new ExifData
    if (!exif_data) {
        LOGD("Exif is not present in file: [%s] creating new", file_path.c_str());

        exif_data = exif_data_new();
        exif_data_set_option(exif_data, EXIF_DATA_OPTION_FOLLOW_SPECIFICATION);
        exif_data_set_data_type(exif_data, EXIF_DATA_TYPE_COMPRESSED);
        exif_data_set_byte_order(exif_data, EXIF_BYTE_ORDER_MOTOROLA);
        exif_data_is_new = true;
    }

    if (!exif_data) {
        LOGE("Couldn't allocate new ExifData");
        throw UnknownException("Memory allocation failed");
    }

    LOGD("Exif data type: %d", exif_data_get_data_type(exif_data) );
    LOGD("Exif byte order: %d", exif_data_get_byte_order(exif_data) );
    exif_data_set_option(exif_data, EXIF_DATA_OPTION_FOLLOW_SPECIFICATION);

    try {
        //If we have created new ExifData there is nothing to remove
        if(!exif_data_is_new) {
            //Remove attributes that have been nulled
            removeNulledAttributesFromExifData(exif_data);
        }

        updateAttributesInExifData(exif_data);

        LOGD("Using JpegFile to save new Exif in: [%s]", file_path.c_str());
        if(exif_data_is_new) {
            jpg_file->setNewExifData(exif_data);
        }

        jpg_file->saveToFile(file_path);
    }
    catch (...) {
        exif_data_unref(exif_data);
        exif_data = NULL;
        throw;
    }

    exif_data_unref(exif_data);
    exif_data = NULL;
}

} // Exif
} // DeviceAPI
