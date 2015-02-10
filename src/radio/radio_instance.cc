// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radio/radio_instance.h"

#include <functional>

#include <radio.h>

#include "common/logger.h"
#include "common/platform_exception.h"

#include "radio/radio_manager.h"

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
  REGISTER_SYNC("FMRadio_IsAntennaConnectedGetter", AntennaGetter);
  REGISTER_SYNC("FMRadio_RadioStateGetter", StateGetter);
  REGISTER_SYNC("FMRadio_MuteSetter", MuteSetter);
  REGISTER_SYNC("FMRadio_MuteGetter", MuteGetter);
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

void RadioInstance::MuteGetter(const picojson::value& args,
                               picojson::object& out) {
  LoggerD("Enter");
  ReportSuccess(picojson::value(FMRadioManager::GetInstance()->IsMuted()), out);
}

void RadioInstance::MuteSetter(const picojson::value& args,
                               picojson::object& out) {
  LoggerD("Enter");
  FMRadioManager::GetInstance()->SetMute(args.get("mute").get<bool>());
  ReportSuccess(out);
}

void RadioInstance::AntennaGetter(const picojson::value& args,
                                  picojson::object& out) {
  LoggerD("Enter");
  ReportSuccess(picojson::value(FMRadioManager::GetInstance()->HasAntenna()),
                out);
}

void RadioInstance::StateGetter(const picojson::value& args,
                                picojson::object& out) {
  LoggerD("Enter");
  ReportSuccess(picojson::value(FMRadioManager::GetInstance()->GetState()),
                out);
}

void RadioInstance::FrequencyGetter(const picojson::value& args,
                                    picojson::object& out) {
  LoggerD("Enter");
  ReportSuccess(picojson::value(FMRadioManager::GetInstance()->GetFrequency()),
                out);
}

void RadioInstance::SignalStrengthGetter(const picojson::value& args,
                                         picojson::object& out) {
  LoggerD("Enter");
  ReportSuccess(picojson::value(FMRadioManager::GetInstance()->GetSignalStrength()),
                out);
}

void RadioInstance::SeekUp(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  FMRadioManager::GetInstance()->SeekUp(args.get("callbackId").get<double>());
  ReportSuccess(out);
}

void RadioInstance::SeekDown(const picojson::value& args,
                             picojson::object& out) {
  LoggerD("Enter");
  FMRadioManager::GetInstance()->SeekDown(args.get("callbackId").get<double>());
  ReportSuccess(out);
}

void RadioInstance::Start(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  try {
    FMRadioManager::GetInstance()->Start(args.get("frequency").get<double>());
    ReportSuccess(out);
  } catch (const PlatformException& e) {
    ReportError(e, out);
  }
}

void RadioInstance::Stop(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  try {
    FMRadioManager::GetInstance()->Stop();
    ReportSuccess(out);
  } catch (const PlatformException& e) {
    ReportError(e, out);
  }
}

void RadioInstance::ScanStart(const picojson::value& args,
                              picojson::object& out) {
  LoggerD("Enter");
  FMRadioManager::GetInstance()->ScanStart(args.get("callbackId").get<double>());
  ReportSuccess(out);
}

void RadioInstance::ScanStop(const picojson::value& args,
                             picojson::object& out) {
  LoggerD("Enter");
  FMRadioManager::GetInstance()->ScanStop(args.get("callbackId").get<double>());
  ReportSuccess(out);
}

void RadioInstance::SetFMRadioInterruptedListener(const picojson::value& args,
                                                  picojson::object& out) {
  LoggerD("Enter");
  FMRadioManager::GetInstance()->SetFMRadioInterruptedListener();
  ReportSuccess(out);
}

void RadioInstance::UnsetFMRadioInterruptedListener(const picojson::value& args,
                                                    picojson::object& out) {
  LoggerD("Enter");
  FMRadioManager::GetInstance()->UnsetFMRadioInterruptedListener();
  ReportSuccess(out);
}

void RadioInstance::SetAntennaChangeListener(const picojson::value& args,
                                             picojson::object& out) {
  LoggerD("Enter");
  FMRadioManager::GetInstance()->SetAntennaChangeListener();
  ReportSuccess(out);
}

void RadioInstance::UnsetAntennaChangeListener(const picojson::value& args,
                                               picojson::object& out) {
  LoggerD("Enter");
  FMRadioManager::GetInstance()->UnsetAntennaChangeListener();
  ReportSuccess(out);
}

} // namespace radio
} // namespace extension
