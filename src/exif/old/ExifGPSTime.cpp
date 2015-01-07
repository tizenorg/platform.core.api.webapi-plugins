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

#include "ExifGPSTime.h"

#include <iomanip>
#include <sstream>
#include <time.h>

#include <JSWebAPIErrorFactory.h>
#include <Logger.h>
#include <PlatformException.h>

namespace extension {
namespace exif {

bool isValidDateFormat(const std::string& date)
{
    //Exif 2.2 spec: "YYYY:MM:DD."
    //There is no trailig '.' at the end it is just null terminate sign
    //Example: tag name:GPSDateStamp type:ASCII size:11 components:11 value:2014:06:25
    //         RAW DATA:[32 30 31 34 3a 30 36 3a 32 35 00]

    if (date.length() != 10) {
        return false;
    }

    //"YYYY:MM:DD"
    // 0123456789
    //     :  :
    for(size_t i = 0; i < date.length(); ++i) {
        const char& cur = date[i];

        if (4 == i || 7 == i) {
            if (cur != ':') {
                return false;
            }
        }
        else {
            if (cur < '0' || cur > '9') {
                return false;
            }
        }
    }
    return true;
}

bool extractDateFromString(const std::string& date, int& out_year, int& out_month,
        int& out_day)
{
    if (!isValidDateFormat(date)) {
        return false;
    }

    if (3 != sscanf(date.c_str(), "%d:%d:%d", &out_year, &out_month, &out_day)) {
        LOGE("Couldn't parse date string: [%s]", date.c_str());
        return false;
    }

    return true;
}

bool isValidTime(const Rationals& time)
{
    return (time.size() >= 3) &&
            time[0].isValid() &&
            time[1].isValid() &&
            time[2].isValid() &&
            (time[0].nominator % time[0].denominator == 0) &&
            (time[1].nominator % time[1].denominator == 0) &&
            (time[2].nominator % time[2].denominator == 0);
}

ExifGPSTime::ExifGPSTime() :
        m_time_is_set(false),
        m_date_is_set(false),
        m_time_and_date(0)
{
    m_time.push_back(Rational());
    m_time.push_back(Rational());
    m_time.push_back(Rational());
}

bool ExifGPSTime::isValid() const
{
    return isComplete() && isValidDateFormat(m_date) && isValidTime(m_time);
}

bool ExifGPSTime::isComplete() const
{
    return m_time_is_set && m_date_is_set;
}

void ExifGPSTime::unsetAll()
{
    m_date_is_set = false;
    m_time_is_set = false;
}

void ExifGPSTime::setTime(const Rationals& time)
{
    if (time.size() < 3) {
        LOGE("time is not having enought members: %d needed 3!", time.size());
        return;
    }

    if (time.size() > 3) {
        LOGW("time is having more members: %d then needed 3!", time.size());
    }

    if (!isValidTime(time)) {
        LOGE("time is invalid: [%s]h [%s]m [%s]s", time[0].toString().c_str(),
                time[1].toString().c_str(),
                time[2].toString().c_str());
        return;
    }

    //Copy just 3 Rationals
    m_time_is_set = true;
    for(size_t i = 0; i < 3 && i < time.size(); ++i) {
        m_time[i] = time[i];
    }
}

const Rationals& ExifGPSTime::getTime() const
{
    return m_time;
}

bool ExifGPSTime::isTimeSet() const
{
    return m_time_is_set;
}

void ExifGPSTime::setDate(const std::string& date)
{
    if (!isValidDateFormat(date)) {
        LOGW("Trying to set incorrect date: [%s]", date.c_str());
        return;
    }

    m_date_is_set = true;
    m_date = date;
}

const std::string& ExifGPSTime::getDate() const
{
    return m_date;
}

bool ExifGPSTime::isDateSet() const
{
    return m_date_is_set;
}

time_t ExifGPSTime::getDateAndTime() const
{
    if (!isValid()) {
        LOGE("ExifGPSTime object is not valid!");
        return 0;
    }

    int year, month, day;
    if (!extractDateFromString(m_date, year, month, day)) {
        LOGE("Couldn't extract date from m_date string");
        return 0;
    }

    time_t out_time = ExifUtil::convertToTimeT(year, month, day,
        m_time[0].nominator / m_time[0].denominator,
        m_time[1].nominator / m_time[1].denominator,
        m_time[2].nominator / m_time[2].denominator);

    LOGD("[%s] [%s]h [%s]m [%s]s ---> time_t: %d", m_date.c_str(),
            m_time[0].toString().c_str(),
            m_time[1].toString().c_str(),
            m_time[2].toString().c_str(),
            out_time);

    return out_time;
}

void ExifGPSTime::setDateAndTime(time_t new_time)
{
    struct tm* utc = gmtime(&new_time);

    int year, month, day, hour, min, sec;
    ExifUtil::extractFromTimeT(new_time, year, month, day, hour, min, sec);

    std::stringstream new_date_ss;
    new_date_ss << std::setfill('0') << std::setw(4) << year << ':' ;
    new_date_ss << std::setfill('0') << std::setw(2) << month << ':';
    new_date_ss << std::setfill('0') << std::setw(2) << day;
    m_date = new_date_ss.str();
    m_date_is_set = true;

    m_time[0] = Rational(static_cast<ExifLong>(hour), 1);
    m_time[1] = Rational(static_cast<ExifLong>(min), 1);
    m_time[2] = Rational(static_cast<ExifLong>(sec), 1);
    m_time_is_set = true;

    LOGD("time_t: %d ---> [%s] [%s]h [%s]m [%s]s",
            new_time,
            m_date.c_str(),
            m_time[0].toString().c_str(),
            m_time[1].toString().c_str(),
            m_time[2].toString().c_str());
}

Time::TZDatePtr ExifGPSTime::getTZDate() const
{
    if(!isValid()) {
        LOGW("ExifGPSTime is not valid");
        return Time::TZDatePtr();
    }

    int year, month, day;
    extractDateFromString(m_date, year, month, day);

    const int hour = static_cast<int>(m_time[0].nominator / m_time[0].denominator);
    const int min = static_cast<int>(m_time[1].nominator / m_time[1].denominator);
    const int sec = static_cast<int>(m_time[2].nominator / m_time[2].denominator);

    Time::TZDate* new_time = new (std::nothrow) Time::TZDate(year, month-1, day,
        hour, min, sec, 0, "UTC");
    if(!new_time) {
        LOGE("Couldn't create TZDate!");
        return Time::TZDatePtr();
    }

    return Time::TZDatePtr(new_time);
}

void ExifGPSTime::setDateAndTime(Time::TZDatePtr new_time)
{
    if(!new_time) {
        LOGW("TZDate new_time is NULL");
    }

    std::stringstream new_date_ss;
    new_date_ss << std::setfill('0') << std::setw(4) << new_time->getUTCFullYear() << ':';
    new_date_ss << std::setfill('0') << std::setw(2) << new_time->getUTCMonth()+1 << ':';
    new_date_ss << std::setfill('0') << std::setw(2) << new_time->getUTCDate();
    m_date = new_date_ss.str();
    m_date_is_set = true;

    m_time[0] = Rational(static_cast<ExifLong>(new_time->getUTCHours()), 1);
    m_time[1] = Rational(static_cast<ExifLong>(new_time->getUTCMinutes()), 1);
    m_time[2] = Rational(static_cast<ExifLong>(new_time->getUTCSeconds()), 1);
    m_time_is_set = true;

    LOGD("TZDatePtr: %d ---> [%s] [%s]h [%s]m [%s]s",
            new_time->toString().c_str(),
            m_date.c_str(),
            m_time[0].toString().c_str(),
            m_time[1].toString().c_str(),
            m_time[2].toString().c_str());
}

} // exif
} // extension
