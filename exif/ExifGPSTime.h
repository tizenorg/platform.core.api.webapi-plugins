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

#ifndef __TIZEN_EXIF_EXIF_GPS_TIME_H_
#define __TIZEN_EXIF_EXIF_GPS_TIME_H_

#include <string>

#include <TZDate.h>

#include "ExifUtil.h"

namespace DeviceAPI {
namespace Exif {

class ExifGPSTime
{
public:
    ExifGPSTime();

    /**
     * Returns true if both and time is set and valid
     */
    bool isValid() const;

    /**
     * Returns true if both date and time has been set
     */
    bool isComplete() const;

    /**
     * Unset both date and time
     */
    void unsetAll();

    void setTime(const Rationals& date);
    const Rationals& getTime() const;
    bool isTimeSet() const;

    void setDate(const std::string& date);
    const std::string& getDate() const;
    bool isDateSet() const;

    time_t getDateAndTime() const;
    void setDateAndTime(time_t new_time);

    /**
     * Returns NULL pointer if this ExifGPSTime is not valid
     */
    Time::TZDatePtr getTZDate() const;

    /**
     * If new_time is NULL nothing is done
     */
    void setDateAndTime(Time::TZDatePtr new_time);

private:
    Rationals m_time;
    bool m_time_is_set;

    std::string m_date;
    bool m_date_is_set;

    time_t m_time_and_date;
};

} // Exif
} // DeviceAPI

#endif // __TIZEN_EXIF_EXIF_GPS_TIME_H_
