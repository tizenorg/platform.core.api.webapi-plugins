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

#include "tvchannel/program_info.h"
#include "tvchannel/tvchannel_manager.h"
#include "common/logger.h"

namespace extension {
namespace tvchannel {

ProgramInfo::ProgramInfo() :
m_title(""),
m_startTime(0),
m_duration(0),
m_detailedDescription(""),
m_language(""),
m_rating("") {
}

ProgramInfo::~ProgramInfo() {
}

void ProgramInfo::fromApiData(const TCProgramData &data) {
    LoggerD("Enter");
    setTitle(data);
    m_startTime = data.StartTime();
    m_duration = data.EndTime() - m_startTime;
    setDetailedDescription(data);
    setLanguage(data);
}

std::string ProgramInfo::getTitle() const {
    return m_title;
}

void ProgramInfo::setTitle(const TCProgramData &data) {
    LoggerD("Enter");
    try {
        unsigned int len;
        size_t c_len;
        t_wchar_t *title;

        len = data.TitleLength();
        if (len) {
            LoggerD("Title length %d", len);
            title = new t_wchar_t[len + 1];
            TCProgramData copy = data;
            copy.Title(title, &len);
            c_len = len * sizeof (t_wchar_t) / sizeof (char);
            char name[128];
            TVChannelManager::ucs2utf8(name, (size_t)sizeof (name),
                reinterpret_cast<char*>(title), c_len);
            delete [] title;
            m_title = name;
        } else {
            m_title = "";
        }
    } catch (...) {
        LoggerD("There is some problem on text encoding.");
        m_title = "";
    }
}

void ProgramInfo::setTitle(std::string title) {
    m_title = title;
}

int64_t ProgramInfo::getStartTime() const {
    return m_startTime;
}

double ProgramInfo::getStartTimeMs() const {
    return static_cast<double> (m_startTime * 1000.0);
}

void ProgramInfo::setStartTime(int64_t startTime) {
    m_startTime = startTime;
}

int64_t ProgramInfo::getDuration() const {
    return m_duration;
}

void ProgramInfo::setDuration(int64_t duration) {
    m_duration = duration;
}

std::string ProgramInfo::getDetailedDescription() const {
    return m_detailedDescription;
}

void ProgramInfo::setDetailedDescription(std::string detailedDescription) {
    m_detailedDescription = detailedDescription;
}

void ProgramInfo::setDetailedDescription(const TCProgramData &data) {
    LoggerD("Enter");
    try {
        unsigned int len;
        size_t c_len;
        t_wchar_t *description;

        len = data.ExtendedTextLength();
        LoggerD("Description length %d", len);
        if (len) {
            description = new t_wchar_t[len + 1];
            TCProgramData copy = data;
            copy.ExtendedText(description, &len);

            c_len = len * sizeof (t_wchar_t) / sizeof (char);
            char name[TCProgramData::EExtendChannelTextLength
                ::EXTEND_CHANNEL_TEXT_LENGTH];
            TVChannelManager::ucs2utf8(name, (size_t)sizeof (name),
                reinterpret_cast<char*>(description), c_len);
            delete [] description;
            m_detailedDescription = name;
        } else {
            m_detailedDescription = "";
        }
    } catch (...) {
        LoggerD("There is some problem on text encoding.");
        m_detailedDescription = "";
    }
}

std::string ProgramInfo::getLanguage() const {
    return m_language;
}

void ProgramInfo::setLanguage(std::string language) {
    m_language = language;
}

void ProgramInfo::setLanguage(const TCProgramData &data) {
    // todo: convert language number to code?
    m_language = std::to_string(data.Language());
}

std::string ProgramInfo::getRating() const {
    return m_rating;
}

void ProgramInfo::setRating(std::string rating) {
    m_rating = rating;
}


}  // namespace tvchannel
}  // namespace extension
