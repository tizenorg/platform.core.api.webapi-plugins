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

#ifndef SRC_TVCHANNEL_TVCHANNEL_INSTANCE_H_
#define SRC_TVCHANNEL_TVCHANNEL_INSTANCE_H_

#include <string>
#include <memory>

#include "common/extension.h"
#include "common/picojson.h"
#include "tvchannel/tvchannel_extension.h"
#include "tvchannel/tvchannel_manager.h"

namespace extension {
namespace tvchannel {

class TVChannelInstance:
        public common::ParsedInstance,
        public EventListener {
 public:
    TVChannelInstance();
    virtual ~TVChannelInstance();

 private:
    void getCurrentChannel(const picojson::value& args, picojson::object& out);
    void getCurrentProgram(const picojson::value& args, picojson::object& out);
    virtual void onChannelChange(double callbackId);
    virtual void onEPGReceived(double callbackId);
    virtual void onNoSignal(double callbackId);
    void findChannel(const picojson::value& args, picojson::object& out);
    void findChannelResult(
        const std::shared_ptr<TVChannelManager::FindChannelData>& data);
    void getChannelList(const picojson::value& args, picojson::object& out);
    void getChannelListResult(
        const std::shared_ptr<TVChannelManager::GetChannelListData>& data);
    picojson::value channelInfoToJson(
        const std::unique_ptr<ChannelInfo> &pChannel);
    picojson::value programInfoToJson(
        const std::unique_ptr<ProgramInfo> &pProgram);
    void getProgramList(const picojson::value& args, picojson::object& out);
    void getProgramListResult(
        const std::shared_ptr<TVChannelManager::GetProgramListData>& data);
    void tune(picojson::value const& args,
        picojson::object& out);
    void tuneUp(picojson::value const& args,
        picojson::object& out);
    void tuneDown(picojson::value const& args,
        picojson::object& out);
    void tuneTask(std::shared_ptr<
        TVChannelManager::TuneData> const& _tuneData);
    void tuneTaskAfter(
        std::shared_ptr<TVChannelManager::TuneData> const& _pTuneData);
    void tuneUpTask(std::shared_ptr<
        TVChannelManager::TuneData> const& _tuneData);
    void tuneDownTask(
        std::shared_ptr<TVChannelManager::TuneData> const& _tuneData);

    ISignalSubscriber* m_pSubscriber;
};

}  // namespace tvchannel
}  // namespace extension

#endif  // SRC_TVCHANNEL_TVCHANNEL_INSTANCE_H_
