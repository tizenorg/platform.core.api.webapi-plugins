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

#ifndef SRC_TVCHANNEL_TUNE_OPTION_H_
#define SRC_TVCHANNEL_TUNE_OPTION_H_

#include <memory>
#include "common/picojson.h"
#include "tvchannel/types.h"

namespace extension {
namespace tvchannel {

class TuneOption {
 public:
    TuneOption();
    virtual ~TuneOption();

    explicit TuneOption(picojson::object const& _tuneDict);

    int64_t getPtc() const;
    bool isPtcSet() const;
    void setPtc(int64_t ptc);

    int64_t getMajor() const;
    bool isMajorSet() const;
    void setMajor(int64_t major);

    int64_t getMinor() const;
    bool isMinorSet() const;
    void setMinor(int64_t minor);

    int64_t getSourceID() const;
    bool isSourceIDSet() const;
    void setSourceID(int64_t sourceID);

    int64_t getProgramNumber() const;
    bool isProgramNumberSet() const;
    void setProgramNumber(int64_t programNumber);

    int64_t getTransportStreamID() const;
    bool isTransportStreamIDSet() const;
    void setTransportStreamID(int64_t transportStreamID);

    int64_t getOriginalNetworkID() const;
    bool isOriginalNetworkIDSet() const;
    void setOriginalNetworkID(int64_t originalNetworkID);

 private:
    int64_t m_ptc;
    int64_t m_major;
    int64_t m_minor;
    int64_t m_sourceID;
    int64_t m_programNumber;
    int64_t m_transportStreamID;
    int64_t m_originalNetworkID;
    bool m_ptc_is_set;
    bool m_major_is_set;
    bool m_minor_is_set;
    bool m_sourceID_is_set;
    bool m_programNumber_is_set;
    bool m_transportStreamID_is_set;
    bool m_originalNetworkID_is_set;
    void initialize();
};

typedef std::shared_ptr<TuneOption> TuneOptionPtr;

}  //  namespace tvchannel
}  //  namespace extension

#endif  //  SRC_TVCHANNEL_TUNE_OPTION_H_
