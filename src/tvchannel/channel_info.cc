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
 
#include "tvchannel/channel_info.h"
#include <stdint.h>
#include <string>
#include "tvchannel/tvchannel_manager.h"
#include "common/logger.h"

namespace extension {
namespace tvchannel {

ChannelInfo::ChannelInfo() :
    m_major(0), m_minor(0), m_channelName(""), m_programNumber(0), m_ptc(0),
        m_lcn(0), m_sourceID(0), m_transportStreamID(0), m_originalNetworkID(0),
        m_serviceName(""), m_serviceID(0) {
}

ChannelInfo::~ChannelInfo() {
}

void ChannelInfo::fromApiData(const TCServiceData &channelData) {
    LoggerD("Enter");
    m_ptc = channelData.Get<u_int16_t>(CHANNEL_NUMBER);
    m_major = channelData.Get<u_int16_t>(MAJOR);
    m_minor = channelData.Get<u_int16_t>(MINOR);
    m_lcn = channelData.Get<u_int16_t>(LCN);
    m_sourceID = channelData.Get<u_int16_t>(SOURCE_ID);
    m_programNumber = channelData.Get<u_int16_t>(PROGRAM_NUMBER);
    m_transportStreamID = channelData.Get<u_int16_t>(TRANSPORT_STREAM_ID);
    m_originalNetworkID = channelData.Get<u_int16_t>(ORIGINAL_NETWORK_ID);
    m_serviceID = channelData.Get < TCServiceId > (SERVICE_ID);

    setChannelName(channelData);
    setServiceName(channelData);
}

int64_t ChannelInfo::getMajor() const {
    return m_major;
}

void ChannelInfo::setMajor(int64_t major) {
    m_major = major;
}

int64_t ChannelInfo::getMinor() const {
    return m_minor;
}

void ChannelInfo::setMinor(int64_t minor) {
    m_minor = minor;
}

std::string ChannelInfo::getChannelName() const {
    return m_channelName;
}

void ChannelInfo::setChannelName(std::string channelName) {
    m_channelName = channelName;
}

void ChannelInfo::setChannelName(const TCServiceData &channelData) {
    LoggerD("Enter");
    size_t len = channelData.GetLength(SERVICE_NAME);
    if (len) {
        LoggerD("ServiceName length %d", len);
        size_t c_len = len * sizeof(t_wchar_t) / sizeof(char);
        t_wchar_t svc_name[len + 1];
        channelData.Get(SERVICE_NAME, svc_name);
        char name[CHANNEL_NAME_MAX_SIZE];
        TVChannelManager::ucs2utf8(name, sizeof(name),
            reinterpret_cast<char *>(svc_name), c_len);
        m_channelName = name;
    } else {
        m_channelName = "";
    }
}

int64_t ChannelInfo::getProgramNumber() const {
    return m_programNumber;
}

void ChannelInfo::setProgramNumber(int64_t programNumber) {
    m_programNumber = programNumber;
}

int64_t ChannelInfo::getPtc() const {
    return m_ptc;
}

void ChannelInfo::setPtc(int64_t ptc) {
    m_ptc = ptc;
}

int64_t ChannelInfo::getLcn() const {
    return m_lcn;
}

void ChannelInfo::setLcn(int64_t lcn) {
    m_lcn = lcn;
}

int64_t ChannelInfo::getSourceID() const {
    return m_sourceID;
}

void ChannelInfo::setSourceID(int64_t sourceID) {
    m_sourceID = sourceID;
}

int64_t ChannelInfo::getTransportStreamID() const {
    return m_transportStreamID;
}

void ChannelInfo::setTransportStreamID(int64_t transportStreamID) {
    m_transportStreamID = transportStreamID;
}

int64_t ChannelInfo::getOriginalNetworkID() const {
    return m_originalNetworkID;
}

void ChannelInfo::setOriginalNetworkID(int64_t originalNetworkID) {
    m_originalNetworkID = originalNetworkID;
}

std::string ChannelInfo::getServiceName() const {
    return getChannelName();
}

void ChannelInfo::setServiceName(std::string serviceName) {
    setChannelName(serviceName);
}

void ChannelInfo::setServiceName(const TCServiceData &data) {
    setChannelName(data);
}

TCServiceId ChannelInfo::getServiceID() const {
    return m_serviceID;
}

void ChannelInfo::setServiceID(TCServiceId serviceID) {
    m_serviceID = serviceID;
}

}  //  namespace tvchannel
}  //  namespace extension
