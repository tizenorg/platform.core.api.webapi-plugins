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

#ifndef SRC_TVCHANNEL_PROGRAM_INFO_H_
#define SRC_TVCHANNEL_PROGRAM_INFO_H_

#include <ProgramData.h>
#include <string>

#include "tvchannel/types.h"

namespace extension {
namespace tvchannel {

class ProgramInfo {
 public:
    ProgramInfo();
    virtual ~ProgramInfo();
    void fromApiData(const TCProgramData &data);

    std::string getTitle() const;
    void setTitle(std::string title);
    void setTitle(const TCProgramData &data);

    int64_t getStartTime() const;
    double getStartTimeMs() const;
    void setStartTime(int64_t startTime);

    int64_t getDuration() const;
    void setDuration(int64_t duration);

    std::string getDetailedDescription() const;
    void setDetailedDescription(std::string detailedDescription);
    void setDetailedDescription(const TCProgramData &data);

    std::string getLanguage() const;
    void setLanguage(std::string language);
    void setLanguage(const TCProgramData &data);

    std::string getRating() const;
    void setRating(std::string rating);

 private:
    std::string m_title;
    int64_t m_startTime;
    int64_t m_duration;
    std::string m_detailedDescription;
    std::string m_language;
    std::string m_rating;
};

}  // namespace tvchannel
}  // namespace extension

#endif  // SRC_TVCHANNEL_PROGRAM_INFO_H_
