// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
    void tuneTask(std::shared_ptr<TVChannelManager::TuneData> const& _tuneData);
    void tuneTaskAfter(
        std::shared_ptr<TVChannelManager::TuneData> const& _pTuneData);

    ISignalSubscriber* m_pSubscriber;
};

}  // namespace tvchannel
}  // namespace extension

#endif  // SRC_TVCHANNEL_TVCHANNEL_INSTANCE_H_
