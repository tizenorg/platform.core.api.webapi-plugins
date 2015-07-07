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

#include "tvchannel/tvchannel_manager.h"
#include <iconv.h>
#include <stdint.h>
#include <functional>
#include <map>
#include "tvchannel/channel_info.h"
#include "tvchannel/program_info.h"
#include "common/logger.h"
#include "common/task-queue.h"
#include <NavigationModeHelper.h>
#include "tvchannel/criteria_filter.h"

namespace extension {
namespace tvchannel {

TVChannelManager* TVChannelManager::getInstance() {
    static TVChannelManager manager;
    return &manager;
}

TVChannelManager::TVChannelManager() :
    m_listener(NULL) {
    LoggerD("Enter");
    int ret = TVServiceAPI::CreateService(&m_pService);
    if (TV_SERVICE_API_SUCCESS != ret) {
        LoggerE("Failed to create tvs-api service: %d", ret);
        throw common::UnknownException("Failed to create tvs-api service");
    }
}

IService* TVChannelManager::getService() {
    return m_pService;
}

void TVChannelManager::tune(std::shared_ptr<TuneData> const& _pTuneData) {
    LoggerD("Enter");
    try {
        std::unique_lock<std::mutex> lock(tuneMutex);

        WindowType windowType = _pTuneData->windowType;
        TuneOption tuneOption = _pTuneData->tuneOption;

        TCServiceId currentServiceId =
            getCurrentChannel(windowType)->getServiceID();

        TSTvMode tvMode = getTvMode(
            getNavigation(getProfile(windowType), SCREENID));

        ENavigationMode naviMode = NAVIGATION_MODE_ALL;
        std::unique_ptr < TCCriteriaHelper > pCriteria = getBasicCriteria(
            tvMode, naviMode);
        pCriteria->Fetch(SERVICE_ID);
        pCriteria->Fetch(CHANNEL_TYPE);
        pCriteria->Fetch(CHANNEL_NUMBER);

        CriteriaFilter filter = CriteriaFilter(tvMode.serviceMode);

        if (tuneOption.isMajorSet()) {
            LoggerD("MAJOR: %d", tuneOption.getMajor());
            filter.filterWhere(MAJOR, static_cast<int>(tuneOption.getMajor()));
        }
        if (tuneOption.isMinorSet()) {
            LoggerD("MINOR: %d", tuneOption.getMinor());
            filter.filterWhere(MINOR, static_cast<int>(tuneOption.getMinor()));
        }
        if (tuneOption.isPtcSet()) {
            LoggerD("PTC: %d", tuneOption.getPtc());
            filter.filterWhere(CHANNEL_NUMBER,
                static_cast<int>(tuneOption.getPtc()));
        }
        if (tuneOption.isOriginalNetworkIDSet()) {
            LoggerD("ORIGINAL_NETWORK_ID: %d", tuneOption.getOriginalNetworkID());
            filter.filterWhere(ORIGINAL_NETWORK_ID,
                static_cast<int>(tuneOption.getOriginalNetworkID()));
        }
        if (tuneOption.isProgramNumberSet()) {
            LoggerD("PROGRAM_NUMBER: %d", tuneOption.getProgramNumber());
            filter.filterWhere(PROGRAM_NUMBER,
                static_cast<int>(tuneOption.getProgramNumber()));
        }
        if (tuneOption.isSourceIDSet()) {
            LoggerD("SOURCE_ID: %d", tuneOption.getSourceID());
            filter.filterWhere(SOURCE_ID,
                static_cast<int>(tuneOption.getSourceID()));
        }
        if (tuneOption.isTransportStreamIDSet()) {
            LoggerD("TRANSPORT_STREAM_ID: %d", tuneOption.getTransportStreamID());
            filter.filterWhere(TRANSPORT_STREAM_ID,
                static_cast<int>(tuneOption.getTransportStreamID()));
        }

        filter.getFilteredCriteria(pCriteria);

        TCServiceData foundService;
        int ret = getService()->FindService(*pCriteria, foundService);
        if (TV_SERVICE_API_METHOD_SUCCESS != ret) {
            LoggerE("Failed to find channel: %d", ret);
            throw common::NotFoundException("Failed to find channel");
        }

        TCServiceId serviceId = foundService.Get < TCServiceId > (SERVICE_ID);
        u_int16_t channelNumber = foundService.Get < u_int16_t
            > (CHANNEL_NUMBER);
        EChannelType channelType = foundService.Get < EChannelType
            > (CHANNEL_TYPE);

        ret = getNavigation(getProfile(windowType), 0)->SetService(serviceId);
        if (TV_SERVICE_API_METHOD_SUCCESS != ret) {
            LoggerE("Failed to set selected channel: %d", ret);
            throw common::UnknownException(
                "Failed to set selected channel");
        }
        _pTuneData->serviceId = serviceId;
        m_callbackTuneMap[serviceId] = _pTuneData->callbackId;
    } catch (common::PlatformException const& _error) {
        _pTuneData->pError.reset(
            new common::PlatformException(_error.name(), _error.message()));
        LoggerE("Some exception caught");
    }
}

ENavigationMode navigatorModeToENavigationMode(NavigatorMode mode) {
    switch (mode) {
        case DIGITAL:
            return NAVIGATION_MODE_DIGITAL;
        case ANALOG:
            return NAVIGATION_MODE_ANALOG;
        case FAVORITE:
            return NAVIGATION_MODE_FAVORITES;
        default:
            return NAVIGATION_MODE_ALL;
    }
}

void TVChannelManager::tuneUp(std::shared_ptr<TuneData> const& _pTuneData) {
    LoggerD("Enter");
    try {
        std::unique_lock<std::mutex> lock(tuneMutex);

        WindowType windowType = _pTuneData->windowType;

        TCServiceId currentServiceId =
            getCurrentChannel(windowType)->getServiceID();

        TSTvMode tvMode = getTvMode(
            getNavigation(getProfile(windowType), SCREENID));

        ENavigationMode naviMode = navigatorModeToENavigationMode(
            _pTuneData->navMode);
        std::unique_ptr < TCCriteriaHelper > pCriteria = getBasicCriteria(
            tvMode, naviMode);
        pCriteria->Fetch(SERVICE_ID);
        pCriteria->Fetch(SERVICE_NAME);
        pCriteria->Fetch(CHANNEL_TYPE);
        pCriteria->Fetch(CHANNEL_NUMBER);
        TCServiceData previousService;
        int ret = getNavigation(
            getProfile(windowType), SCREENID)->GetPreviousService(*pCriteria,
                currentServiceId,
                previousService);
        if (TV_SERVICE_API_METHOD_SUCCESS != ret) {
            LoggerE("Failed to find previous channel: %d", ret);
            throw common::NotFoundException("Failed to find previous channel");
        }
        TCServiceId serviceId = previousService.Get<TCServiceId>(SERVICE_ID);

        // if GetNextService's result is same with current service id,
        // it means failure to find previous channel.
        if (currentServiceId == serviceId) {
            LoggerE("Failed to find previous channel: %d", ret);
            throw common::NotFoundException("Failed to find next channel");
        }
        ret = getNavigation(getProfile(windowType), 0)->SetService(serviceId);
        if (TV_SERVICE_API_METHOD_SUCCESS != ret) {
            LoggerE("Failed to set selected channel: %d", ret);
            throw common::UnknownException(
                "Failed to set selected channel");
        }
        m_callbackTuneMap[serviceId] = _pTuneData->callbackId;
    } catch (common::PlatformException const& _error) {
        _pTuneData->pError.reset(
            new common::PlatformException(_error.name(), _error.message()));
        LoggerE("Some exception caught");
    }
}

void TVChannelManager::tuneDown(std::shared_ptr<TuneData> const& _pTuneData) {
    LoggerD("Enter");
    try {
        std::unique_lock<std::mutex> lock(tuneMutex);

        WindowType windowType = _pTuneData->windowType;

        TCServiceId currentServiceId =
            getCurrentChannel(windowType)->getServiceID();

        TSTvMode tvMode = getTvMode(
            getNavigation(getProfile(windowType), SCREENID));

        ENavigationMode naviMode = navigatorModeToENavigationMode(
            _pTuneData->navMode);
        std::unique_ptr < TCCriteriaHelper > pCriteria = getBasicCriteria(
            tvMode, naviMode);
        pCriteria->Fetch(SERVICE_ID);
        pCriteria->Fetch(SERVICE_NAME);
        pCriteria->Fetch(CHANNEL_TYPE);
        pCriteria->Fetch(CHANNEL_NUMBER);
        TCServiceData nextService;
        int ret = getNavigation(
            getProfile(windowType), SCREENID)->GetNextService(*pCriteria,
                currentServiceId,
                nextService);
        if (TV_SERVICE_API_METHOD_SUCCESS != ret) {
            LoggerE("Failed to find previous channel: %d", ret);
            throw common::NotFoundException("Failed to find previous channel");
        }
        TCServiceId serviceId = nextService.Get<TCServiceId>(SERVICE_ID);

        // if GetNextService's result is same with current service id,
        // it means failure to find previous channel.
        if (currentServiceId == serviceId) {
            LoggerE("Failed to find previous channel: %d", ret);
            throw common::NotFoundException("Failed to find previous channel");
        }
        ret = getNavigation(getProfile(windowType), 0)->SetService(serviceId);
        if (TV_SERVICE_API_METHOD_SUCCESS != ret) {
            LoggerE("Failed to set selected channel: %d", ret);
            throw common::UnknownException(
                "Failed to set selected channel");
        }
        m_callbackTuneMap[serviceId] = _pTuneData->callbackId;
    } catch (common::PlatformException const& _error) {
        _pTuneData->pError.reset(
            new common::PlatformException(_error.name(), _error.message()));
        LoggerE("Some exception caught");
    }
}

IServiceNavigation* TVChannelManager::getNavigation(EProfile profileId,
    u_int16_t screenId) {
    LoggerD("Enter");
    IServiceNavigation* navigation;
    int ret = TVServiceAPI::CreateServiceNavigation(profileId, screenId,
        &navigation);
    if (TV_SERVICE_API_SUCCESS != ret) {
        LoggerE("Failed to create service navigation: %d", ret);
        throw common::UnknownException("Failed to create service navigation");
    }
    return navigation;
}

TSTvMode TVChannelManager::getTvMode(IServiceNavigation* pNavigation) {
    LoggerD("Enter");
    TSTvMode tvMode;
    int ret = pNavigation->GetTvMode(tvMode);
    if (TV_SERVICE_API_METHOD_SUCCESS != ret) {
        LoggerE("Failed to get current tv mode: %d", ret);
        throw common::UnknownException("Failed to get current tv mode");
    }
    LoggerD("tvMode : antenna - %d, service - %d", tvMode.antennaMode,
        tvMode.serviceMode);
    return tvMode;
}

std::unique_ptr<TCCriteriaHelper> TVChannelManager::getBasicCriteria(
    TSTvMode tvMode, ENavigationMode naviMode) {
    LoggerD("Enter");
    std::unique_ptr < TCCriteriaHelper > pCriteria(new TCCriteriaHelper());
    bool found = TCNavigationModeHelper::GetNavigationCriteria(tvMode, naviMode,
        *pCriteria);
    if (!found) {
        LoggerE("Failed to create navigation criteria");
        throw common::UnknownException("Failed to create navigation criteria");
    }
    return pCriteria;
}

TCServiceData TVChannelManager::getCurrentServiceInfo(
    IServiceNavigation* _pNavigation, TSTvMode _mode,
    std::unique_ptr<TCCriteriaHelper> const& _pCriteria) {
    LoggerD("Enter");
    TCServiceData serviceData;
    int ret = _pNavigation->GetCurrentServiceInfo(_mode, *_pCriteria,
        serviceData);
    if (TV_SERVICE_API_METHOD_SUCCESS != ret) {
        LoggerE("Failed to get current service info: %d", ret);
        throw common::UnknownException("Failed to get current service info");
    }
    return serviceData;
}

std::unique_ptr<ChannelInfo> TVChannelManager::getCurrentChannel(
    WindowType _windowType) {
    LoggerD("Entered %d", _windowType);

    std::unique_ptr < TCCriteriaHelper > pCriteria(new TCCriteriaHelper());
    pCriteria->Fetch(SERVICE_ID);
    pCriteria->Fetch(MAJOR);
    pCriteria->Fetch(MINOR);
    pCriteria->Fetch(PROGRAM_NUMBER);
    pCriteria->Fetch(CHANNEL_NUMBER);
    pCriteria->Fetch(CHANNEL_TYPE);
    pCriteria->Fetch(SERVICE_NAME);
    pCriteria->Fetch(SOURCE_ID);
    pCriteria->Fetch(TRANSPORT_STREAM_ID);
    pCriteria->Fetch(ORIGINAL_NETWORK_ID);
    pCriteria->Fetch(LCN);

    //  Navigation
    IServiceNavigation* navigation = getNavigation(getProfile(_windowType),
        SCREENID);

    TSTvMode tvMode = getTvMode(navigation);

    TCServiceData serviceData = getCurrentServiceInfo(navigation, tvMode,
        pCriteria);
    LoggerD("Current channel id: %llu",
        serviceData.Get < TCServiceId > (SERVICE_ID));
    std::unique_ptr<ChannelInfo> pChannel(new ChannelInfo());
    pChannel->fromApiData(serviceData);
    return pChannel;
}

EProfile TVChannelManager::getProfile(WindowType windowType) {
    LoggerD("Enter");
    switch (windowType) {
    case MAIN:
        return PROFILE_TYPE_MAIN;
// PIP is not supported - j.h.lim
//      case PIP:
//          return PROFILE_TYPE_PIP;
    default:
        LoggerE("Unsupported window type: %d", windowType);
    }
}

TCServiceId TVChannelManager::getCurrentChannelId(WindowType _windowType) {
    LoggerD("Enter");
    //  Navigation
    IServiceNavigation* navigation = getNavigation(getProfile(_windowType),
        SCREENID);
    TSTvMode tvMode = getTvMode(navigation);
    std::unique_ptr < TCCriteriaHelper > pCriteria(new TCCriteriaHelper());
    pCriteria->Fetch(SERVICE_ID);
    TCServiceData serviceData = getCurrentServiceInfo(navigation, tvMode,
        pCriteria);
    return serviceData.Get < TCServiceId > (SERVICE_ID);
}

ProgramInfo* TVChannelManager::getCurrentProgram(WindowType _windowType) {
    LoggerD("Enter");
    IServiceGuide* guide;
    int ret = TVServiceAPI::CreateServiceGuide(&guide);
    if (TV_SERVICE_API_SUCCESS != ret) {
        LoggerE("Failed to create service guide: %d", ret);
        throw common::UnknownException("Failed to create service guide");
    }

    TCProgramData programData;
    ret = guide->GetPresentProgram(getCurrentChannelId(_windowType),
        programData);
    if (TV_SERVICE_API_METHOD_SUCCESS != ret) {
        LoggerE("Failed to get current program: %d", ret);
        throw common::UnknownException("Failed to get current program");
    }
    ProgramInfo* program = new ProgramInfo();
    program->fromApiData(programData);
    return program;
}

ISignalSubscriber* TVChannelManager::createSubscriber(
    EventListener* pListener) {
    LoggerD("Enter");
    m_listener = pListener;
    ISignalSubscriber* pSubscriber;
    int ret = TVServiceAPI::CreateSignalSubscriber(signalListener,
        &pSubscriber);
    if (TV_SERVICE_API_SUCCESS != ret) {
        LoggerW("Failed to create tvs-api SignalSubscriber");
    }
    return pSubscriber;
}

void TVChannelManager::registerListener(ISignalSubscriber* pSubscriber) {
    LoggerD("Enter");
    pSubscriber->Unsubscribe(SIGNAL_TUNE_SUCCESS);
    int ret = pSubscriber->Subscribe(SIGNAL_TUNE_SUCCESS);
    if (TV_SERVICE_API_METHOD_SUCCESS != ret) {
        LoggerW("Failed to add listener: SIGNAL_TUNE_SUCCESS");
    }
    pSubscriber->Unsubscribe(SIGNAL_TUNER_LOCK_FAIL);
    ret = pSubscriber->Subscribe(SIGNAL_TUNER_LOCK_FAIL);
    if (TV_SERVICE_API_METHOD_SUCCESS != ret) {
        LoggerW("Failed to add listener: SIGNAL_TUNER_LOCK_FAIL");
    }
    pSubscriber->Unsubscribe(SIGNAL_EPG_COMPLETED);
    ret = pSubscriber->Subscribe(SIGNAL_EPG_COMPLETED);
    if (TV_SERVICE_API_METHOD_SUCCESS != ret) {
        LoggerW("Failed to add listener: SIGNAL_EPG_COMPLETED");
    }
}

int TVChannelManager::signalListener(ESignalType type, EProfile _profile,
    u_int16_t _screenID, TSSignalData data, void*) {
    LoggerD("Enter: %d", type);
    if (!getInstance()->m_listener) {
        LoggerE("Listener is empty, ignoring message");
        return 0;
    }

    TCServiceId pChannelId =
        getInstance()->getCurrentChannelId(stringToWindowType("MAIN"));
    double callbackID = -1;
    auto it = getInstance()->m_callbackTuneMap.find(pChannelId);
    if (it != getInstance()->m_callbackTuneMap.end()) {
        callbackID = it->second;
    }
    LoggerD("CallbackID %f", callbackID);

    switch (type) {
    case SIGNAL_TUNE_SUCCESS:
        getInstance()->m_listener->onChannelChange(callbackID);
        break;
    case SIGNAL_TUNER_LOCK_FAIL:
        getInstance()->m_listener->onNoSignal(callbackID);
        break;
    case SIGNAL_EPG_COMPLETED:
        getInstance()->m_listener->onEPGReceived(callbackID);
        break;
    default:
        LoggerW("Unrecognized event type");
    }
    return 0;
}

void TVChannelManager::ucs2utf8(char *out, size_t out_len, char *in,
    size_t in_len) {
    iconv_t cd;
    size_t r;

    cd = iconv_open("UTF-8", "UCS-2BE");
    if (cd == (iconv_t) - 1) {
        LoggerE("Failed to open iconv");
    }

    r = iconv(cd, &in, &in_len, &out, &out_len);
    if (r == (size_t) - 1) {
        LoggerE("Failed convert string to utf8");
    }

    iconv_close(cd);
}

void TVChannelManager::findChannel(
    const std::shared_ptr<FindChannelData>& data) {
    LoggerD("Enter");
    try {
        IServiceNavigation* navigation =
            getNavigation(getProfile(WindowType::MAIN), 0);

        std::unique_ptr<TCCriteriaHelper> criteria = getBasicCriteria(
            getTvMode(navigation), NAVIGATION_MODE_ALL);
        criteria->Fetch(SERVICE_ID);
        criteria->Fetch(MAJOR);
        criteria->Fetch(MINOR);
        criteria->Fetch(PROGRAM_NUMBER);
        criteria->Fetch(CHANNEL_NUMBER);
        criteria->Fetch(CHANNEL_TYPE);
        criteria->Fetch(SERVICE_NAME);
        criteria->Fetch(SOURCE_ID);
        criteria->Fetch(TRANSPORT_STREAM_ID);
        criteria->Fetch(ORIGINAL_NETWORK_ID);
        criteria->Fetch(LCN);

        CriteriaFilter filter = CriteriaFilter(
            getTvMode(navigation).serviceMode);
        filter.filterWhere(MAJOR, static_cast<int>(data->major));
        filter.filterWhere(MINOR, static_cast<int>(data->minor));
        filter.getFilteredCriteria(criteria);

        std::list<TCServiceData*> resultServices;
        int ret = m_pService->FindServiceList(*criteria, resultServices);
        if (TV_SERVICE_API_METHOD_FAILURE == ret) {
            LoggerE("Failed to find channel: %d", ret);
            throw common::NotFoundException("Failed to find channel");
        }
        LoggerD("Found channels: %d", resultServices.size());
        auto it = resultServices.begin();
        for (; it != resultServices.end(); ++it) {
            ChannelInfo *channelInfo = new ChannelInfo();
            channelInfo->fromApiData(*(*it));
            data->channels.push_back(channelInfo);
            delete (*it);
        }
        resultServices.clear();
    } catch (common::PlatformException& e) {
        data->error.reset(new common::PlatformException(e.name(), e.message()));
    } catch (...) {
        data->error.reset(
            new common::UnknownException("Couldn't find channels"));
    }
}

void TVChannelManager::getChannelList(
    const std::shared_ptr<GetChannelListData>& data) {
    LoggerD("Enter");
    try {
        IServiceNavigation* navigation =
            getNavigation(getProfile(WindowType::MAIN), 0);

        ENavigationMode naviMode = NAVIGATION_MODE_ALL;
        switch (data->tuneMode) {
            case DIGITAL:
                naviMode = NAVIGATION_MODE_DIGITAL;
                break;
            case ANALOG:
                naviMode = NAVIGATION_MODE_ANALOG;
                break;
            case FAVORITE:
                naviMode = NAVIGATION_MODE_FAVORITES;
                break;
            default:
                naviMode = NAVIGATION_MODE_ALL;
        }
        std::unique_ptr<TCCriteriaHelper> criteria = getBasicCriteria(
            getTvMode(navigation), naviMode);
        criteria->Fetch(SERVICE_ID);
        criteria->Fetch(MAJOR);
        criteria->Fetch(MINOR);
        criteria->Fetch(PROGRAM_NUMBER);
        criteria->Fetch(CHANNEL_NUMBER);
        criteria->Fetch(CHANNEL_TYPE);
        criteria->Fetch(SERVICE_NAME);
        criteria->Fetch(SOURCE_ID);
        criteria->Fetch(TRANSPORT_STREAM_ID);
        criteria->Fetch(ORIGINAL_NETWORK_ID);
        criteria->Fetch(LCN);
        if (NAVIGATION_MODE_FAVORITES == naviMode) {
            criteria->WhereNot(FAVORITE_ID, 0);
        }

        std::list<TCServiceData*> resultServices;
        int ret = m_pService->FindServiceList(*criteria, resultServices);
        if (TV_SERVICE_API_METHOD_FAILURE == ret) {
            LoggerE("Failed to find channels: %d", ret);
            throw common::NotFoundException("Failed to find channels");
        }
        LoggerD("Found channels: %d", resultServices.size());
        auto it = resultServices.begin();
        for (; it != resultServices.end(); ++it) {
            ChannelInfo *channelInfo = new ChannelInfo();
            channelInfo->fromApiData(*(*it));
            data->channels.push_back(channelInfo);
            delete (*it);
        }
        resultServices.clear();
    } catch (common::PlatformException& e) {
        data->error.reset(new common::PlatformException(e.name(), e.message()));
    } catch (...) {
        data->error.reset(
            new common::UnknownException("Couldn't find channels"));
    }
}

void TVChannelManager::getProgramList(
    const std::shared_ptr<GetProgramListData>& data) {
    LoggerD("Enter");
    try {
        IServiceGuide* guide;
        int ret = TVServiceAPI::CreateServiceGuide(&guide);
        if (TV_SERVICE_API_SUCCESS != ret) {
            LoggerE("Failed to create service guide: %d", ret);
            throw common::UnknownException("Failed to create service guide");
        }

        std::map<unsigned int, TCProgramData*> programList;
        ret = guide->GetProgramList(data->channelId,
            data->startTime, data->duration, programList);
        if (TV_SERVICE_API_METHOD_FAILURE == ret) {
            LoggerE("Failed to get program list.");
            throw common::NotFoundException("Failed to get program list.");
        }
        LoggerD("Found programs: %d", programList.size());

        auto it = programList.begin();
        for (; it != programList.end(); ++it) {
            ProgramInfo *programInfo = new ProgramInfo();
            programInfo->fromApiData(*(it->second));
            data->programs.push_back(programInfo);
            delete it->second;
        }
        programList.clear();
    } catch (common::PlatformException& e) {
        data->error.reset(new common::PlatformException(e.name(), e.message()));
    } catch (...) {
        data->error.reset(
            new common::UnknownException("Couldn't find channels"));
    }
}

TVChannelManager::~TVChannelManager() {
    TVServiceAPI::Destroy();
}


}  // namespace tvchannel
}  // namespace extension
