// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tvchannel/tvchannel_instance.h"
#include <functional>
#include "common/logger.h"
#include "tizen/tizen.h"
#include "common/picojson.h"
#include "tvchannel/channel_info.h"
#include "tvchannel/program_info.h"
#include "tvchannel/types.h"
#include "common/task-queue.h"

namespace extension {
namespace tvchannel {

TVChannelInstance::TVChannelInstance() {
    LOGD("Entered");
    RegisterSyncHandler("TVChannelManager_getCurrentChannel",
        std::bind(&TVChannelInstance::getCurrentChannel, this,
            std::placeholders::_1, std::placeholders::_2));
    RegisterSyncHandler("TVChannelManager_getCurrentProgram",
        std::bind(&TVChannelInstance::getCurrentProgram, this,
            std::placeholders::_1, std::placeholders::_2));
    RegisterHandler("TVChannelManager_findChannel",
        std::bind(&TVChannelInstance::findChannel, this,
            std::placeholders::_1,
            std::placeholders::_2));
    RegisterHandler("TVChannelManager_tune",
        std::bind(&TVChannelInstance::tune, this, std::placeholders::_1,
            std::placeholders::_2));

    m_pSubscriber = TVChannelManager::getInstance()->createSubscriber(this);
    TVChannelManager::getInstance()->registerListener(m_pSubscriber);
}

TVChannelInstance::~TVChannelInstance() {
    LOGD("Entered");
}

void TVChannelInstance::tune(picojson::value const& args,
    picojson::object& out) {
    LOGD("Enter");
    picojson::object tuneOption =
        args.get("tuneOption").get<picojson::object>();
    double callbackId = args.get("callbackId").get<double>();
    std::string windowType;
    if (args.contains("windowType")) {
        windowType = args.get("windowType").get<std::string>();
    } else {
        windowType = "MAIN";
    }

    LOGD("CallbackID %f", callbackId);
    std::shared_ptr<TVChannelManager::TuneData> pTuneData(
        new TVChannelManager::TuneData(TuneOption(tuneOption),
            stringToWindowType(windowType), callbackId));

    std::function<void(std::shared_ptr<
        TVChannelManager::TuneData> const&)> task = std::bind(
            &TVChannelInstance::tuneTask, this, std::placeholders::_1);
    std::function<void(std::shared_ptr<
            TVChannelManager::TuneData> const&)> taskAfter = std::bind(
                &TVChannelInstance::tuneTaskAfter, this, std::placeholders::_1);

    common::TaskQueue::GetInstance().Queue<TVChannelManager::TuneData>(task,
        taskAfter, pTuneData);

    picojson::value v;
    ReportSuccess(v, out);
}

void TVChannelInstance::tuneTaskAfter(
    std::shared_ptr<TVChannelManager::TuneData> const& _tuneData) {
    LOGD("Enter");
    if (_tuneData->pError) {
        picojson::value event = picojson::value(picojson::object());
        picojson::object& obj = event.get<picojson::object>();
        obj.insert(std::make_pair("callbackId", picojson::value(
            _tuneData->callbackId)));
        obj.insert(std::make_pair("error", _tuneData->pError->ToJSON()));
        PostMessage(event.serialize().c_str());
    }
}

void TVChannelInstance::tuneTask(
    std::shared_ptr<TVChannelManager::TuneData> const& _tuneData) {
    LOGD("Enter");
    TVChannelManager::getInstance()->tune(_tuneData);
}

void TVChannelInstance::getCurrentChannel(picojson::value const& args,
    picojson::object& out) {

    std::unique_ptr<ChannelInfo> pChannel =
        TVChannelManager::getInstance()->getCurrentChannel(
            stringToWindowType(args.get("windowType").get<std::string>()));

    picojson::value v = channelInfoToJson(pChannel);
    ReportSuccess(v, out);
}

picojson::value TVChannelInstance::channelInfoToJson(
    const std::unique_ptr<ChannelInfo> &pChannel) {
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

    return picojson::value(channel);
}

picojson::value TVChannelInstance::programInfoToJson(
    const std::unique_ptr<ProgramInfo>& pInfo) {
    picojson::value::object program;
    program.insert(std::make_pair("title", picojson::value(pInfo->getTitle())));
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
        std::make_pair("language", picojson::value(pInfo->getLanguage())));
    program.insert(
        std::make_pair("rating", picojson::value(pInfo->getRating())));

    picojson::value result(program);
    return result;
}

void TVChannelInstance::getCurrentProgram(const picojson::value& args,
    picojson::object& out) {
    std::unique_ptr<ProgramInfo> pInfo(
        TVChannelManager::getInstance()->getCurrentProgram(
            stringToWindowType(args.get("windowType").get<std::string>())));
    ReportSuccess(programInfoToJson(pInfo), out);
}

void TVChannelInstance::onChannelChange(double callbackId) {
    LOGD("Enter");
    try {
        WindowType windowType = stringToWindowType("MAIN");

        picojson::value::object dict;
        std::unique_ptr<ChannelInfo> pChannel =
            TVChannelManager::getInstance()->getCurrentChannel(windowType);
        dict["listenerId"] = picojson::value("ChannelChanged");
        dict["channel"] = channelInfoToJson(pChannel);
        dict["windowType"] = picojson::value("MAIN");
        dict["success"] = picojson::value(true);
        picojson::value resultListener(dict);
        PostMessage(resultListener.serialize().c_str());
        if (callbackId !=- 1) {
            dict.erase("listenerId");
            dict["callbackId"] = picojson::value(callbackId);
            picojson::value resultCallback(dict);
            PostMessage(resultCallback.serialize().c_str());
        }
    } catch (common::PlatformException& e) {
        LOGW("Failed to post message: %s", e.message().c_str());
    } catch (...) {
        LOGW("Failed to post message, unknown error");
    }
}

void TVChannelInstance::onEPGReceived(double callbackId) {
    try {
        picojson::value::object dict;
        dict["listenerId"] = picojson::value("ProgramInfoReceived");
        dict["windowType"] = picojson::value("MAIN");
        std::unique_ptr<ProgramInfo> pInfo(
            TVChannelManager::getInstance()->getCurrentProgram(MAIN));
        dict["program"] = programInfoToJson(pInfo);
        picojson::value result(dict);
        PostMessage(result.serialize().c_str());
    } catch (common::PlatformException& e) {
        LOGW("Failed to post message: %s", e.message().c_str());
    } catch (...) {
        LOGW("Failed to post message, unknown error");
    }
}
void TVChannelInstance::onNoSignal(double callbackId) {
    try {
        picojson::value::object dict;
        dict["windowType"] = picojson::value("MAIN");
        dict["callbackId"] = picojson::value(callbackId);
        dict["nosignal"] = picojson::value(true);
        picojson::value result(dict);
        PostMessage(result.serialize().c_str());
    } catch (common::PlatformException& e) {
        LOGW("Failed to post message: %s", e.message().c_str());
    } catch (...) {
        LOGW("Failed to post message, unknown error");
    }
}

void TVChannelInstance::findChannel(const picojson::value& args,
    picojson::object& out) {
    LOGD("Enter");
    std::function<void(std::shared_ptr<TVChannelManager::FindChannelData>)>
        asyncWork = std::bind(
            &TVChannelManager::findChannel,
            TVChannelManager::getInstance(),
            std::placeholders::_1);
    std::function<void(std::shared_ptr<TVChannelManager::FindChannelData>)>
        afterWork = std::bind(
            &TVChannelInstance::findChannelResult,
            this,
            std::placeholders::_1);
    std::shared_ptr<TVChannelManager::FindChannelData> data(
        new TVChannelManager::FindChannelData());
    data->major = static_cast<int32_t>(args.get("major").get<double>());
    data->minor = static_cast<int32_t>(args.get("minor").get<double>());
    data->callbackId = args.get("callbackId").get<double>();
    common::TaskQueue::GetInstance().Queue<TVChannelManager::FindChannelData>(
        asyncWork,
        afterWork,
        data);
    picojson::value result;
    ReportSuccess(result, out);
}

void TVChannelInstance::findChannelResult(
    const std::shared_ptr<TVChannelManager::FindChannelData>& data) {
    LOGD("Enter");
    picojson::value::object dict;
    dict["callbackId"] = picojson::value(data->callbackId);
    if (data->error) {
        dict["error"] = data->error->ToJSON();
    } else {
        picojson::value::array channels;
        auto it = data->channels.begin();
        for (; it != data->channels.end(); ++it) {
            channels.push_back(channelInfoToJson(
                std::unique_ptr<ChannelInfo>(*it)));
        }
        data->channels.clear();
        dict["channelInfos"] = picojson::value(channels);
    }
    picojson::value result(dict);
    PostMessage(result.serialize().c_str());
}

}  // namespace tvchannel
}  // namespace extension
