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
            REGISTER_SYNC("FMRadio_FrequencyGetter", FrequencyGetter);
            REGISTER_SYNC("FMRadio_SignalStrengthGetter", SignalStrengthGetter);
            REGISTER_SYNC("FMRadio_IsAntennaConnectedGetter", AntenaGetter);
            REGISTER_SYNC("FMRadio_RadioStateGetter", StateGetter);
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

    RadioInstance& RadioInstance::getInstance() {
            static RadioInstance instance;
            return instance;
    }

    void RadioInstance::AntenaGetter(const picojson::value& args,
   picojson::object& out){
   }

   void RadioInstance::StateGetter(const picojson::value& args,
   picojson::object& out){
   }

    void RadioInstance::FrequencyGetter(const picojson::value& args,
            picojson::object& out)
    {
        LoggerD(".cc FrequencyGetter()");
        double freq = FMRadioManager::GetInstance()->FrequencyGetter();
        ReportSuccess(picojson::value(freq),out);
    }

    void RadioInstance::SignalStrengthGetter(const picojson::value& args,
              picojson::object& out)
      {
          LoggerD(".cc SignalStrengthGetter()");
          double strength = FMRadioManager::GetInstance()->SignalStrengthGetter();
          ReportSuccess(picojson::value(strength),out);
      }

    void RadioInstance::InstanceReportSuccess(picojson::object& out) {
        LoggerD(".cc InstanceReportSuccess()");
    }

    void RadioInstance::SeekUp(const picojson::value& args,
            picojson::object& out) {
        LoggerD(".cc SeekUp()");
        FMRadioManager::GetInstance()->SeekUp(args);
    }

    void RadioInstance::SeekDown(const picojson::value& args,
            picojson::object& out) {
        LoggerD(".cc SeekDown()");
        FMRadioManager::GetInstance()->SeekDown(args);
    }

    void RadioInstance::Start(const picojson::value& args,
            picojson::object& out) {
        try{
            LoggerD(".cc RadioInstance::Start()");
            FMRadioManager::GetInstance()->Start(args.get("frequency").get<double>());
        }
        catch(const PlatformException& e){
            ReportError(e,out);
        }
    }

    void RadioInstance::Stop(const picojson::value& args,
            picojson::object& out) {
        try{
          LoggerD(".cc Stop()");
          FMRadioManager::GetInstance()->Stop();
        }
        catch(const PlatformException& e){
            ReportError(e,out);
        }
    }

    void RadioInstance::ScanStart(const picojson::value& args,
            picojson::object& out) {
          FMRadioManager::GetInstance()->ScanStart(args);
    }

    void RadioInstance::ScanStop(const picojson::value& args,
            picojson::object& out) {
        FMRadioManager::GetInstance()->ScanStop(args);
    }

    void RadioInstance::SetFMRadioInterruptedListener(const picojson::value& args,
            picojson::object& out) {
        LoggerD(".cc SetFMRadioInterruptedListener()");
        FMRadioManager::GetInstance()->SetFMRadioInterruptedListener();
    }

    void RadioInstance::UnsetFMRadioInterruptedListener(const picojson::value& args,
            picojson::object& out) {
        LoggerD(".cc UnsetFMRadioInterruptedListener()");
        FMRadioManager::GetInstance()->UnsetFMRadioInterruptedListener();
    }

    void RadioInstance::SetAntennaChangeListener(const picojson::value& args,
            picojson::object& out) {
        LoggerD(".cc SetAntennaChangeListener()");
        FMRadioManager::GetInstance()->SetAntennaChangeListener();
    }

    void RadioInstance::UnsetAntennaChangeListener(const picojson::value& args,
            picojson::object& out) {
        LoggerD(".cc UnsetAntennaChangeListener()");
        FMRadioManager::GetInstance()->UnsetAntennaChangeListener();
    }

}
}
