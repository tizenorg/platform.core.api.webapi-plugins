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

#ifndef SRC_TVCHANNEL_TVCHANNEL_MANAGER_H_
#define SRC_TVCHANNEL_TVCHANNEL_MANAGER_H_

#include <string>
#include <memory>
#include <mutex>
#include <map>
#include <list>
#include <TVServiceAPI.h>
#include <ServiceNavigationDataType.h>
#include <NavigationModeHelper.h>
#include "tvchannel/types.h"
#include "tvchannel/tune_option.h"
#include "common/platform_exception.h"
#include "tvchannel/program_info.h"

namespace common {
class PlatformException;
}

namespace extension {
namespace tvchannel {

class ChannelInfo;
class ProgramInfo;
//  TVServiceAPI static methods returns 0 on success
static const int TV_SERVICE_API_SUCCESS = 0;
//  TVServiceAPI object methods return 1 on success
static const int TV_SERVICE_API_METHOD_SUCCESS = 1;
//  TVServiceAPI object methods return -1 on failure
static const int TV_SERVICE_API_METHOD_FAILURE = -1;
//  You need to check error return in function/method docs and use correct
//  constant

class EventListener {
 public:
    virtual void onChannelChange(double callbackId) = 0;
    virtual void onEPGReceived(double callbackId) = 0;
    virtual void onNoSignal(double callbackId) = 0;
    virtual ~EventListener() {
    }
};

class TVChannelManager {
 public:
    struct TuneData {
        TuneData(TuneOption _tuneOption, WindowType _windowType,
            double _callbackId) :
            tuneOption(_tuneOption), windowType(_windowType),
                callbackId(_callbackId) {
        }
        TuneData(NavigatorMode _navMode, WindowType _windowType,
                    double _callbackId) :
                        navMode(_navMode), windowType(_windowType),
                        callbackId(_callbackId) {
                }
        NavigatorMode navMode;
        TuneOption tuneOption;
        WindowType windowType;
        double callbackId;
        u_int64_t serviceId;
        std::shared_ptr<common::PlatformException> pError;
    };

    static TVChannelManager* getInstance();
    static void ucs2utf8(char *out, size_t out_len, char *in, size_t in_len);
    void registerListener(ISignalSubscriber* pSubscriber);
    ISignalSubscriber* createSubscriber(EventListener* pListener);
    std::unique_ptr<ChannelInfo> getCurrentChannel(WindowType _windowType);
    ProgramInfo* getCurrentProgram(WindowType _windowType);

    void tune(std::shared_ptr<TuneData> const& _pTuneData);
    void tuneUp(std::shared_ptr<TuneData> const& _pTuneData);
    void tuneDown(std::shared_ptr<TuneData> const& _pTuneData);

    EProfile getProfile(WindowType windowType);
    IServiceNavigation* getNavigation(EProfile profileId, u_int16_t screenId);
    TSTvMode getTvMode(IServiceNavigation* _pNavigation);
    std::unique_ptr<TCCriteriaHelper> getBasicCriteria(TSTvMode tvMode,
        ENavigationMode naviMode);
    TCServiceData getCurrentServiceInfo(IServiceNavigation* _pNavigation,
        TSTvMode _mode, std::unique_ptr<TCCriteriaHelper> const& _pCriteria);

    IService* getService();

    struct FindChannelData {
        std::shared_ptr<common::PlatformException> error;
        int32_t major;
        int32_t minor;
        double callbackId;
        std::list<ChannelInfo*> channels;
    };

    void findChannel(const std::shared_ptr<FindChannelData>& data);

    struct GetChannelListData {
        NavigatorMode tuneMode;
        bool nStartSet;
        bool numberSet;
        std::shared_ptr<common::PlatformException> error;
        int32_t nStart;
        int32_t number;
        double callbackId;
        std::list<ChannelInfo*> channels;

        GetChannelListData() :
            tuneMode(NOT_DEFINED),
            nStartSet(false),
            numberSet(false) {
        }
    };

    void getChannelList(const std::shared_ptr<GetChannelListData>& data);

    struct GetProgramListData {
        TCServiceId channelId;
        u_int32_t startTime;
        u_int32_t duration;
        std::shared_ptr<common::PlatformException> error;
        double callbackId;
        std::list<ProgramInfo*> programs;
    };

    void getProgramList(const std::shared_ptr<GetProgramListData>& data);

    ~TVChannelManager();

 private:
    EventListener* m_listener;
    //  Not copyable, assignable, movable
    TVChannelManager();
    TVChannelManager(TVChannelManager const&) = delete;
    void operator=(TVChannelManager const&) = delete;
    TVChannelManager(TVChannelManager &&) = delete;

    TCServiceId getCurrentChannelId(WindowType _windowType);
    static int signalListener(ESignalType type, EProfile _profile,
        u_int16_t _screenID, TSSignalData data, void*);
    IService* m_pService;

    static const int SCREENID = 0;

    std::mutex tuneMutex;
    std::map<u_int64_t, double> m_callbackTuneMap;
};

}  // namespace tvchannel
}  // namespace extension

#endif  // SRC_TVCHANNEL_TVCHANNEL_MANAGER_H_
