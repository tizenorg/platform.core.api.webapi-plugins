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

#ifndef SRC_TVCHANNEL_CHANNEL_INFO_H_
#define SRC_TVCHANNEL_CHANNEL_INFO_H_

#include <ServiceData.h>
#include <string>
#include "tvchannel/types.h"

namespace extension {
namespace tvchannel {

class ChannelInfo {
 public:
    ChannelInfo();
    virtual ~ChannelInfo();

    void fromApiData(const TCServiceData &channelData);

    int64_t getMajor() const;
    void setMajor(int64_t major);

    int64_t getMinor() const;
    void setMinor(int64_t minor);

    std::string getChannelName() const;
    void setChannelName(std::string channelName);
    void setChannelName(const TCServiceData &channelData);

    int64_t getProgramNumber() const;
    void setProgramNumber(int64_t programNumber);

    int64_t getPtc() const;
    void setPtc(int64_t ptc);

    int64_t getLcn() const;
    void setLcn(int64_t lcn);

    int64_t getSourceID() const;
    void setSourceID(int64_t sourceID);

    int64_t getTransportStreamID() const;
    void setTransportStreamID(int64_t transportStreamID);

    int64_t getOriginalNetworkID() const;
    void setOriginalNetworkID(int64_t originalNetworkID);

    std::string getServiceName() const;
    void setServiceName(std::string serviceName);
    void setServiceName(const TCServiceData &channelData);

    TCServiceId getServiceID() const;
    void setServiceID(TCServiceId serviceID);

 private:
    int64_t m_major;
    int64_t m_minor;
    std::string m_channelName;
    int64_t m_programNumber;
    int64_t m_ptc;
    int64_t m_lcn;
    int64_t m_sourceID;
    int64_t m_transportStreamID;
    int64_t m_originalNetworkID;
    std::string m_serviceName;
    TCServiceId m_serviceID;

    static const int CHANNEL_NAME_MAX_SIZE = 128;
};

}  //  namespace tvchannel
}  //  namespace extension

#endif  // SRC_TVCHANNEL_CHANNEL_INFO_H_
