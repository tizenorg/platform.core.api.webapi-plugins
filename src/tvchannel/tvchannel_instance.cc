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

using common::TaskQueue;

TVChannelInstance::TVChannelInstance() {
    using std::placeholders::_1;
    using std::placeholders::_2;

    #define REGISTER_ASYNC(c, func) \
        RegisterSyncHandler(c, func);
    #define REGISTER_SYNC(c, func) \
        RegisterSyncHandler(c, func);

    REGISTER_SYNC("TVChannelManager_getCurrentChannel",
        std::bind(&TVChannelInstance::getCurrentChannel, this, _1, _2));
    REGISTER_SYNC("TVChannelManager_getCurrentProgram",
        std::bind(&TVChannelInstance::getCurrentProgram, this, _1, _2));
    REGISTER_ASYNC("TVChannelManager_findChannel",
        std::bind(&TVChannelInstance::findChannel, this, _1, _2));
    REGISTER_ASYNC("TVChannelManager_getChannelList",
        std::bind(&TVChannelInstance::getChannelList, this, _1, _2));
    REGISTER_ASYNC("TVChannelManager_getProgramList",
        std::bind(&TVChannelInstance::getProgramList, this, _1, _2));
    REGISTER_ASYNC("TVChannelManager_tune",
        std::bind(&TVChannelInstance::tune, this, _1, _2));
    REGISTER_ASYNC("TVChannelManager_tuneUp",
        std::bind(&TVChannelInstance::tuneUp, this, _1, _2));
    REGISTER_ASYNC("TVChannelManager_tuneDown",
        std::bind(&TVChannelInstance::tuneDown, this, _1, _2));

    #undef REGISTER_ASYNC
    #undef REGISTER_SYNC

    m_pSubscriber = TVChannelManager::getInstance()->createSubscriber(this);
    TVChannelManager::getInstance()->registerListener(m_pSubscriber);
}

TVChannelInstance::~TVChannelInstance() {
    LoggerD("Entered");
}

void TVChannelInstance::tune(picojson::value const& args,
    picojson::object& out) {
    LoggerD("Enter");
    picojson::object tuneOption =
        args.get("tuneOption").get<picojson::object>();
    double callbackId = args.get("callbackId").get<double>();
    std::string windowType;
    if (args.contains("windowType")) {
        windowType = args.get("windowType").get<std::string>();
    } else {
        windowType = "MAIN";
    }

    LoggerD("CallbackID %f", callbackId);
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
    LoggerD("Enter");
    if (_tuneData->pError) {
        picojson::value event = picojson::value(picojson::object());
        picojson::object& obj = event.get<picojson::object>();
        obj.insert(std::make_pair("callbackId", picojson::value(
            _tuneData->callbackId)));
        obj.insert(std::make_pair("error", _tuneData->pError->ToJSON()));
        Instance::PostMessage(this, event.serialize().c_str());
    }
}

void TVChannelInstance::tuneTask(
    std::shared_ptr<TVChannelManager::TuneData> const& _tuneData) {
    LoggerD("Enter");
    TVChannelManager::getInstance()->tune(_tuneData);
}

void TVChannelInstance::tuneUp(picojson::value const& args,
    picojson::object& out) {
    LoggerD("Enter");

    NavigatorMode navMode;
    if (args.contains("tuneMode")) {
        navMode = stringToNavigatorMode(
                args.get("tuneMode").get<std::string>());
    } else {
        navMode = NavigatorMode::ALL;
    }

    double callbackId = args.get("callbackId").get<double>();
    std::string windowType;
    if (args.contains("windowType")) {
        windowType = args.get("windowType").get<std::string>();
    } else {
        windowType = "MAIN";
    }

    LoggerD("CallbackID %f", callbackId);
    std::shared_ptr<TVChannelManager::TuneData> pTuneData(
        new TVChannelManager::TuneData(navMode,
            stringToWindowType(windowType), callbackId));

    std::function<void(std::shared_ptr<
        TVChannelManager::TuneData> const&)> task = std::bind(
            &TVChannelInstance::tuneUpTask, this, std::placeholders::_1);
    std::function<void(std::shared_ptr<
            TVChannelManager::TuneData> const&)> taskAfter = std::bind(
                &TVChannelInstance::tuneTaskAfter, this, std::placeholders::_1);

    common::TaskQueue::GetInstance().Queue<TVChannelManager::TuneData>(task,
        taskAfter, pTuneData);

    picojson::value v;
    ReportSuccess(v, out);
}

void TVChannelInstance::tuneUpTask(
    std::shared_ptr<TVChannelManager::TuneData> const& _tuneData) {
    TVChannelManager::getInstance()->tuneUp(_tuneData);
}

void TVChannelInstance::tuneDown(picojson::value const& args,
    picojson::object& out) {
    LoggerD("Enter");

    NavigatorMode navMode;
    if (args.contains("tuneMode")) {
        navMode = stringToNavigatorMode(
                args.get("tuneMode").get<std::string>());
    } else {
        navMode = NavigatorMode::ALL;
    }

    double callbackId = args.get("callbackId").get<double>();
    std::string windowType;
    if (args.contains("windowType")) {
        windowType = args.get("windowType").get<std::string>();
    } else {
        windowType = "MAIN";
    }

    LoggerD("CallbackID %f", callbackId);
    std::shared_ptr<TVChannelManager::TuneData> pTuneData(
        new TVChannelManager::TuneData(navMode,
            stringToWindowType(windowType), callbackId));

    std::function<void(std::shared_ptr<
        TVChannelManager::TuneData> const&)> task = std::bind(
            &TVChannelInstance::tuneUpTask, this, std::placeholders::_1);
    std::function<void(std::shared_ptr<
            TVChannelManager::TuneData> const&)> taskAfter = std::bind(
                &TVChannelInstance::tuneTaskAfter, this, std::placeholders::_1);

    common::TaskQueue::GetInstance().Queue<TVChannelManager::TuneData>(task,
        taskAfter, pTuneData);

    picojson::value v;
    ReportSuccess(v, out);
}

void TVChannelInstance::tuneDownTask(
    std::shared_ptr<TVChannelManager::TuneData> const& _tuneData) {
    TVChannelManager::getInstance()->tuneDown(_tuneData);
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
    // internal property
    channel.insert(
        std::make_pair("_serviceId",
            picojson::value(
                static_cast<double>(pChannel->getServiceID()))));
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
    LoggerD("Enter");
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
        Instance::PostMessage(this, resultListener.serialize().c_str());
        if (callbackId !=- 1) {
            dict.erase("listenerId");
            dict["callbackId"] = picojson::value(callbackId);
            picojson::value resultCallback(dict);
            Instance::PostMessage(this, resultCallback.serialize().c_str());
        }
    } catch (common::PlatformException& e) {
        LoggerW("Failed to post message: %s", e.message().c_str());
    } catch (...) {
        LoggerW("Failed to post message, unknown error");
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
        Instance::PostMessage(this, result.serialize().c_str());
    } catch (common::PlatformException& e) {
        LoggerW("Failed to post message: %s", e.message().c_str());
    } catch (...) {
        LoggerW("Failed to post message, unknown error");
    }
}
void TVChannelInstance::onNoSignal(double callbackId) {
    try {
        picojson::value::object dict;
        dict["windowType"] = picojson::value("MAIN");
        dict["callbackId"] = picojson::value(callbackId);
        dict["nosignal"] = picojson::value(true);
        picojson::value result(dict);
        Instance::PostMessage(this, result.serialize().c_str());
    } catch (common::PlatformException& e) {
        LoggerW("Failed to post message: %s", e.message().c_str());
    } catch (...) {
        LoggerW("Failed to post message, unknown error");
    }
}

void TVChannelInstance::findChannel(const picojson::value& args,
    picojson::object& out) {
    LoggerD("Enter");
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
    TaskQueue::GetInstance().Queue<TVChannelManager::FindChannelData>(
        asyncWork,
        afterWork,
        data);
    picojson::value result;
    ReportSuccess(result, out);
}

void TVChannelInstance::findChannelResult(
    const std::shared_ptr<TVChannelManager::FindChannelData>& data) {
    LoggerD("Enter");
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
    Instance::PostMessage(this, result.serialize().c_str());
}

void TVChannelInstance::getChannelList(const picojson::value& args,
    picojson::object& out) {
    LoggerD("Enter");
    std::function<void(std::shared_ptr<TVChannelManager::GetChannelListData>)>
        asyncWork = std::bind(
            &TVChannelManager::getChannelList,
            TVChannelManager::getInstance(),
            std::placeholders::_1);
    std::function<void(std::shared_ptr<TVChannelManager::GetChannelListData>)>
        afterWork = std::bind(
            &TVChannelInstance::getChannelListResult,
            this,
            std::placeholders::_1);
    std::shared_ptr<TVChannelManager::GetChannelListData> data(
        new TVChannelManager::GetChannelListData());
    if (args.contains("tuneMode")) {
        data->tuneMode = stringToNavigatorMode(
            args.get("tuneMode").get<std::string>());
    }
    if (args.contains("nStart")) {
        data->nStart = static_cast<int32_t>(args.get("nStart").get<double>());
        data->nStartSet = true;
    }
    if (args.contains("number")) {
        data->number = static_cast<int32_t>(args.get("number").get<double>());
        data->numberSet = true;
    }
    data->callbackId = args.get("callbackId").get<double>();
    TaskQueue::GetInstance().Queue<TVChannelManager::GetChannelListData>(
        asyncWork,
        afterWork,
        data);
    picojson::value result;
    ReportSuccess(result, out);
}

void TVChannelInstance::getChannelListResult(
    const std::shared_ptr<TVChannelManager::GetChannelListData>& data) {
    picojson::value::object dict;
    dict["callbackId"] = picojson::value(data->callbackId);
    if (data->error) {
        dict["error"] = data->error->ToJSON();
    } else {
        picojson::value::array channels;
        int32_t start = data->nStartSet ? data->nStart : 0;
        int32_t number = data->numberSet ? data->number : data->channels.size();
        int32_t i = 0;
        auto it = data->channels.begin();
        for (; it != data->channels.end(); ++it) {
            // add only limited rows by start and number
            if (i >= start && i < start + number) {
                channels.push_back(channelInfoToJson(
                    std::unique_ptr<ChannelInfo>(*it)));
            } else {
                delete *it;
            }
            ++i;
        }
        data->channels.clear();
        dict["channelInfos"] = picojson::value(channels);
    }
    picojson::value result(dict);
    Instance::PostMessage(this, result.serialize().c_str());
}

void TVChannelInstance::getProgramList(
    const picojson::value& args, picojson::object& out) {
    LoggerD("Enter");
    std::function<void(std::shared_ptr<TVChannelManager::GetProgramListData>)>
        asyncWork = std::bind(
            &TVChannelManager::getProgramList,
            TVChannelManager::getInstance(),
            std::placeholders::_1);
    std::function<void(std::shared_ptr<TVChannelManager::GetProgramListData>)>
        afterWork = std::bind(
            &TVChannelInstance::getProgramListResult,
            this,
            std::placeholders::_1);
    std::shared_ptr<TVChannelManager::GetProgramListData> data(
        new TVChannelManager::GetProgramListData());
    data->channelId = static_cast<u_int64_t>(
        args.get("channelId").get<double>());
    data->startTime = static_cast<u_int32_t>(
        args.get("startTime").get<double>());
    if (args.contains("duration")) {
        data->duration =
            static_cast<u_int32_t>(args.get("duration").get<double>());
    } else {
         data->duration = UINT32_MAX;
    }
    data->callbackId = args.get("callbackId").get<double>();
    TaskQueue::GetInstance().Queue<TVChannelManager::GetProgramListData>(
        asyncWork,
        afterWork,
        data);
    picojson::value result;
    ReportSuccess(result, out);
}

void TVChannelInstance::getProgramListResult(
    const std::shared_ptr<TVChannelManager::GetProgramListData>& data) {
    picojson::value::object dict;
    dict["callbackId"] = picojson::value(data->callbackId);
    if (data->error) {
        dict["error"] = data->error->ToJSON();
    } else {
        picojson::value::array programs;
        auto it = data->programs.begin();
        for (; it != data->programs.end(); ++it) {
            programs.push_back(programInfoToJson(
                std::unique_ptr<ProgramInfo>(*it)));
        }
        data->programs.clear();
        dict["programInfos"] = picojson::value(programs);
    }
    picojson::value result(dict);
    Instance::PostMessage(this, result.serialize().c_str());
}

}  // namespace tvchannel
}  // namespace extension
