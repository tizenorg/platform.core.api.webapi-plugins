// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tvchannel/tvchannel_instance.h"
#include <functional>
#include <memory>
#include "common/logger.h"
#include "tizen/tizen.h"
#include "common/picojson.h"
#include "tvchannel/tvchannel_manager.h"
#include "tvchannel/channel_info.h"
#include "tvchannel/program_info.h"

namespace extension {
namespace tvchannel {

TVChannelInstance::TVChannelInstance() {
    LOGE("Entered");
    RegisterSyncHandler("TVChannelManager_getCurrentChannel",
        std::bind(&TVChannelInstance::getCurrentChannel, this,
            std::placeholders::_1,
            std::placeholders::_2));
    RegisterSyncHandler("TVChannelManager_getCurrentProgram",
        std::bind(&TVChannelInstance::getCurrentProgram, this,
            std::placeholders::_1,
            std::placeholders::_2));
}

TVChannelInstance::~TVChannelInstance() {
    LOGE("Entered");
}

void TVChannelInstance::getCurrentChannel(picojson::value const& args,
    picojson::object& out) {
    std::unique_ptr< ChannelInfo > pChannel = TVChannelManager::getInstance()->getCurrentChannel(
        args.get("windowType").get<std::string>());

    picojson::value::object channel;
    channel.insert(
        std::make_pair("major",
            picojson::value(static_cast<double>(pChannel->getMajor()))));
    channel.insert(
        std::make_pair("minor",
            picojson::value(static_cast<double>(pChannel->getMinor()))));
    channel.insert(
        std::make_pair("channelName",
            picojson::value(pChannel->getChannelName())));
    channel.insert(
        std::make_pair("programNumber",
            picojson::value(
                static_cast<double>(pChannel->getProgramNumber()))));
    channel.insert(
        std::make_pair("ptc",
            picojson::value(static_cast<double>(pChannel->getPtc()))));
    channel.insert(
        std::make_pair("lcn",
            picojson::value(static_cast<double>(pChannel->getLcn()))));
    channel.insert(
        std::make_pair("sourceID",
            picojson::value(static_cast<double>(pChannel->getSourceID()))));
    channel.insert(
        std::make_pair("transportStreamID",
            picojson::value(
                static_cast<double>(pChannel->getTransportStreamID()))));
    channel.insert(
        std::make_pair("originalNetworkID",
            picojson::value(
                static_cast<double>(pChannel->getOriginalNetworkID()))));
    channel.insert(
        std::make_pair("serviceName",
            picojson::value(pChannel->getServiceName())));

    picojson::value v(channel);

    ReportSuccess(v, out);
}

void TVChannelInstance::getCurrentProgram(const picojson::value& args,
    picojson::object& out) {
    std::unique_ptr<ProgramInfo> pInfo(TVChannelManager::getInstance()
        ->getCurrentProgram(args.get("windowType").get<std::string>()));
    picojson::value::object program;
    program.insert(
        std::make_pair("title",
            picojson::value(pInfo->getTitle())));
    program.insert(
        std::make_pair("startTime",
            picojson::value(static_cast<double>(pInfo->getStartTimeMs()))));
    program.insert(
        std::make_pair("duration",
            picojson::value(static_cast<double>(pInfo->getDuration()))));
    program.insert(
        std::make_pair("detailedDescription",
            picojson::value(pInfo->getDetailedDescription())));
    program.insert(
        std::make_pair("language",
            picojson::value(pInfo->getLanguage())));
    program.insert(
        std::make_pair("rating",
            picojson::value(pInfo->getRating())));

    picojson::value result(program);
    ReportSuccess(result, out);
}


}  // namespace tvchannel
}  // namespace extension
