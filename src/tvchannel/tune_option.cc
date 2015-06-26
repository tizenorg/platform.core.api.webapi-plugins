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

#include "tvchannel/tune_option.h"
#include <stdint.h>
#include <common/logger.h>

namespace extension {
namespace tvchannel {

TuneOption::TuneOption() {
    initialize();
}

void TuneOption::initialize() {
    m_ptc = 0;
    m_major = 0;
    m_minor = 0;
    m_sourceID = 0;
    m_programNumber = 0;
    m_transportStreamID = 0;
    m_originalNetworkID = 0;
    m_ptc_is_set = false;
    m_major_is_set = false;
    m_minor_is_set = false;
    m_sourceID_is_set = false;
    m_programNumber_is_set = false;
    m_transportStreamID_is_set = false;
    m_originalNetworkID_is_set = false;
}

TuneOption::~TuneOption() {
}

TuneOption::TuneOption(picojson::object const& _tuneDict) {
    LoggerD("Enter");
    initialize();
    picojson::object tuneDict = _tuneDict;
    if (tuneDict.find("major") != tuneDict.end()) {
        setMajor(static_cast<int64_t>(tuneDict["major"].get<double>()));
    }
    if (tuneDict.find("minor") != tuneDict.end()) {
        setMinor(static_cast<int64_t>(tuneDict["minor"].get<double>()));
    }
    if (tuneDict.find("sourceID") != tuneDict.end()) {
        setSourceID(static_cast<int64_t>(tuneDict["sourceID"].get<double>()));
    }
    if (tuneDict.find("programNumber") != tuneDict.end()) {
        setProgramNumber(
            static_cast<int64_t>(tuneDict["programNumber"].get<double>()));
    }
    if (tuneDict.find("transportStreamID") != tuneDict.end()) {
        setTransportStreamID(
            static_cast<int64_t>(tuneDict["transportStreamID"].get<double>()));
    }
    if (tuneDict.find("ptc") != tuneDict.end()) {
        setPtc(static_cast<int64_t>(tuneDict["ptc"].get<double>()));
    }
    if (tuneDict.find("originalNetworkID") != tuneDict.end()) {
        setOriginalNetworkID(
            static_cast<int64_t>(tuneDict["originalNetworkID"].get<double>()));
    }
}

int64_t TuneOption::getPtc() const {
    return m_ptc;
}

bool TuneOption::isPtcSet() const {
    return m_ptc_is_set;
}

void TuneOption::setPtc(int64_t ptc) {
    m_ptc = ptc;
    m_ptc_is_set = true;
}

int64_t TuneOption::getMajor() const {
    return m_major;
}

bool TuneOption::isMajorSet() const {
    return m_major_is_set;
}

void TuneOption::setMajor(int64_t major) {
    m_major = major;
    m_major_is_set = true;
}

int64_t TuneOption::getMinor() const {
    return m_minor;
}

bool TuneOption::isMinorSet() const {
    return m_minor_is_set;
}

void TuneOption::setMinor(int64_t minor) {
    m_minor = minor;
    m_minor_is_set = true;
}

int64_t TuneOption::getSourceID() const {
    return m_sourceID;
}

bool TuneOption::isSourceIDSet() const {
    return m_sourceID_is_set;
}

void TuneOption::setSourceID(int64_t sourceID) {
    m_sourceID = sourceID;
    m_sourceID_is_set = true;
}

int64_t TuneOption::getProgramNumber() const {
    return m_programNumber;
}

bool TuneOption::isProgramNumberSet() const {
    return m_programNumber_is_set;
}

void TuneOption::setProgramNumber(int64_t programNumber) {
    m_programNumber = programNumber;
    m_programNumber_is_set = true;
}

int64_t TuneOption::getTransportStreamID() const {
    return m_transportStreamID;
}

bool TuneOption::isTransportStreamIDSet() const {
    return m_transportStreamID_is_set;
}

void TuneOption::setTransportStreamID(int64_t transportStreamID) {
    m_transportStreamID = transportStreamID;
    m_transportStreamID_is_set = true;
}

int64_t TuneOption::getOriginalNetworkID() const {
    return m_originalNetworkID;
}

bool TuneOption::isOriginalNetworkIDSet() const {
    return m_originalNetworkID_is_set;
}

void TuneOption::setOriginalNetworkID(int64_t originalNetworkID) {
    m_originalNetworkID = originalNetworkID;
    m_originalNetworkID_is_set = true;
}

}  //  namespace tvchannel
}  //  namespace extension
