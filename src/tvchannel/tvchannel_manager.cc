// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tvchannel/tvchannel_manager.h"
#include <iconv.h>
#include "tvchannel/channel_info.h"
#include "tvchannel/program_info.h"
#include "common/logger.h"
#include "common/platform_exception.h"

namespace extension {
namespace tvchannel {

using common::UnknownException;

TVChannelManager* TVChannelManager::getInstance() {
    static TVChannelManager manager;
    return &manager;
}

std::unique_ptr<ChannelInfo> TVChannelManager::getCurrentChannel(
        std::string const& _windowType) {
    LOGE("Entered %s", _windowType.c_str());

    TCServiceData serviceData;
    TCCriteriaHelper criteria;
    criteria.Fetch(SERVICE_ID);
    criteria.Fetch(MAJOR);
    criteria.Fetch(MINOR);
    criteria.Fetch(PROGRAM_NUMBER);
    criteria.Fetch(CHANNEL_NUMBER);
    criteria.Fetch(CHANNEL_TYPE);
    criteria.Fetch(SERVICE_NAME);
    criteria.Fetch(SOURCE_ID);
    criteria.Fetch(TRANSPORT_STREAM_ID);
    criteria.Fetch(ORIGINAL_NETWORK_ID);
    criteria.Fetch(LCN);

    //  Navigation
    IServiceNavigation* navigation;
    int ret = TVServiceAPI::CreateServiceNavigation(
            getProfile(stringToWindowType(_windowType)), 0, &navigation);
    if (TV_SERVICE_API_SUCCESS != ret) {
        LoggerE("Failed to create service navigation: %d", ret);
        throw common::UnknownException("Failed to create service navigation");
    }

    struct TSTvMode tvMode;
    ret = navigation->GetTvMode(tvMode);
    if (TV_SERVICE_API_METHOD_SUCCESS != ret) {
        LoggerE("Failed to get current tv mode: %d", ret);
        throw common::UnknownException("Failed to get current tv mode");
    }
    LOGE("tvMode : antenna - %d, service - %d", tvMode.antennaMode,
            tvMode.serviceMode);

    ret = navigation->GetCurrentServiceInfo(tvMode, criteria, serviceData);
    if (TV_SERVICE_API_METHOD_SUCCESS != ret) {
        LOGE("Failed to get current service info: %d", ret);
        throw common::UnknownException("Failed to get current service info");
    }
    LoggerE("Current channel id: %llu",
            serviceData.Get < TCServiceId > (SERVICE_ID));
    std::unique_ptr<ChannelInfo> pChannel( new ChannelInfo() );
    pChannel->fromApiData(serviceData);
    return pChannel;
}

EProfile TVChannelManager::getProfile(WindowType windowType) {
    LOGE("Enter");
    switch (windowType) {
    case MAIN:
        return PROFILE_TYPE_MAIN;
// PIP is not supported - j.h.lim
//      case PIP:
//          return PROFILE_TYPE_PIP;
    default:
        LOGE("Unsupported window type: %d", windowType);
    }
}

TCServiceId TVChannelManager::getCurrentChannelId(
    const std::string& _windowType) {
    TCServiceData serviceData;
    TCCriteriaHelper criteria;
    criteria.Fetch(SERVICE_ID);
    //  Navigation
    IServiceNavigation* navigation;
    int ret = TVServiceAPI::CreateServiceNavigation(
            getProfile(stringToWindowType(_windowType)), 0, &navigation);
    if (TV_SERVICE_API_SUCCESS != ret) {
        LOGE("Failed to create service navigation: %d", ret);
        throw UnknownException("Failed to create service navigation");
    }

    struct TSTvMode tvMode;
    ret = navigation->GetTvMode(tvMode);
    if (TV_SERVICE_API_METHOD_SUCCESS != ret) {
        LOGE("Failed to get current tv mode: %d", ret);
        throw UnknownException("Failed to get current tv mode");
    }

    ret = navigation->GetCurrentServiceInfo(tvMode, criteria, serviceData);
    if (TV_SERVICE_API_METHOD_SUCCESS != ret) {
        LOGE("Failed to get current service info: %d", ret);
        throw UnknownException("Failed to get current service info");
    }
    return serviceData.Get<TCServiceId>(SERVICE_ID);
}

ProgramInfo* TVChannelManager::getCurrentProgram(
    const std::string& _windowType) {

    IServiceGuide* guide;
    int ret = TVServiceAPI::CreateServiceGuide(&guide);
    if (TV_SERVICE_API_SUCCESS != ret) {
        LOGE("Failed to create service guide: %d", ret);
        throw UnknownException("Failed to create service guide");
    }

    TCProgramData programData;
    ret = guide->GetPresentProgram(getCurrentChannelId(_windowType),
        programData);
    if (TV_SERVICE_API_METHOD_SUCCESS != ret) {
        LOGE("Failed to get current program: %d", ret);
        throw UnknownException("Failed to get current program");
    }
    ProgramInfo* program = new ProgramInfo();
    program->fromApiData(programData);
    return program;
}

void TVChannelManager::registerListener(EventListener* listener) {
    LOGD("Enter");
    m_listener = listener;
    ISignalSubscriber* subscriber;
    int ret = TVServiceAPI::CreateSignalSubscriber(signalListener, &subscriber);
    if (TV_SERVICE_API_SUCCESS != ret) {
        LOGW("Failed to create tvs-api SignalSubscriber");
        return;
    }
    ret = subscriber->Subscribe(SIGNAL_TUNE_SUCCESS);
    if (TV_SERVICE_API_METHOD_SUCCESS != ret) {
        LOGW("Failed to add listener: SIGNAL_TUNE_SUCCESS");
    }
}

int TVChannelManager::signalListener(ESignalType type,
    TSSignalData data, void*) {
    LOGD("Enter: %d", type);
    if (!getInstance()->m_listener) {
        LOGE("Listener is empty, ignoring message");
        return 0;
    }
    switch (type) {
        case SIGNAL_TUNE_SUCCESS:
            getInstance()->m_listener->onChannelChange();
            break;
        default:
            LOGW("Unrecognized event type");
    }
    return 0;
}

void TVChannelManager::ucs2utf8(char *out, size_t out_len, char *in,
        size_t in_len) {
    iconv_t cd;
    size_t r;

    cd = iconv_open("UTF-8", "UCS-2BE");
    if (cd == (iconv_t) - 1) {
        LOGE("Failed to open iconv");
    }

    r = iconv(cd, &in, &in_len, &out, &out_len);
    if (r == (size_t) - 1) {
        LOGE("Failed convert string to utf8");
    }

    iconv_close(cd);
}

}  // namespace tvchannel
}  // namespace extension
