// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radio/radio_instance.h"

#include <functional>

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"

#include <radio.h>

namespace extension {
namespace radio {

    using namespace common;
    using namespace extension::radio;

    RadioInstance::RadioInstance() {
        using namespace std::placeholders;
        #define REGISTER_SYNC(c,x) \
            RegisterSyncHandler(c, std::bind(&RadioInstance::x, this, _1, _2));
            REGISTER_SYNC("FMRadio_Start", Start);
            REGISTER_SYNC("FMRadio_Stop", Stop);
            REGISTER_SYNC("FMRadio_SetFMRadioInterruptedListener", SetFMRadioInterruptedListener);
            REGISTER_SYNC("FMRadio_UnsetFMRadioInterruptedListener", UnsetFMRadioInterruptedListener);
            REGISTER_SYNC("FMRadio_SetAntennaChangeListener", SetAntennaChangeListener);
            REGISTER_SYNC("FMRadio_UnsetAntennaChangeListener", UnsetAntennaChangeListener);
        #undef REGISTER_SYNC
        #define REGISTER_ASYNC(c,x) \
            RegisterHandler(c, std::bind(&RadioInstance::x, this, _1, _2));
            REGISTER_ASYNC("FMRadio_SeekUp", SeekUp);
            REGISTER_ASYNC("FMRadio_SeekDown", SeekDown);
            REGISTER_ASYNC("FMRadio_ScanStart", ScanStart);
            REGISTER_ASYNC("FMRadio_ScanStop", ScanStop);
        #undef REGISTER_ASYNC

        LoggerD("RadioInstance()");
    }

    RadioInstance::~RadioInstance() {
    }

    void RadioInstance::Start(const picojson::value& args,
            picojson::object& out){
        LoggerD(".cc Start()");
    }

    void RadioInstance::Stop(const picojson::value& args,
            picojson::object& out){
        LoggerD(".cc Stop()");
    }

    void RadioInstance::SeekUp(const picojson::value& args,
            picojson::object& out) {
        LoggerD(".cc SeekUp()");
    }
    void RadioInstance::SeekDown(const picojson::value& args,
            picojson::object& out) {
        LoggerD(".cc SeekDown()");
    }
    void RadioInstance::ScanStart(const picojson::value& args,
            picojson::object& out) {
        LoggerD(".cc ScanStart()");
    }
    void RadioInstance::ScanStop(const picojson::value& args,
            picojson::object& out) {
        LoggerD(".cc ScanStop()");
    }
    void RadioInstance::SetFMRadioInterruptedListener(const picojson::value& args,
            picojson::object& out) {
        LoggerD(".cc SetFMRadioInterruptedListener()");
    }
    void RadioInstance::UnsetFMRadioInterruptedListener(const picojson::value& args,
            picojson::object& out) {
        LoggerD(".cc UnsetFMRadioInterruptedListener()");
    }
    void RadioInstance::SetAntennaChangeListener(const picojson::value& args,
            picojson::object& out) {
        LoggerD(".cc SetAntennaChangeListener()");
    }
    void RadioInstance::UnsetAntennaChangeListener(const picojson::value& args,
            picojson::object& out) {
        LoggerD(".cc UnsetAntennaChangeListener()");
    }

}
}
