// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radio/radio_instance.h"

#include <functional>

#include <radio.h>

#include "common/logger.h"

namespace extension {
namespace radio {

using namespace common;
using namespace extension::radio;

RadioInstance::RadioInstance()
    : manager_(*this) {
  using std::placeholders::_1;
  using std::placeholders::_2;

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
    RegisterSyncHandler(c, std::bind(&RadioInstance::x, this, _1, _2));
  REGISTER_ASYNC("FMRadio_SeekUp", SeekUp);
  REGISTER_ASYNC("FMRadio_SeekDown", SeekDown);
  REGISTER_ASYNC("FMRadio_ScanStart", ScanStart);
  REGISTER_ASYNC("FMRadio_ScanStop", ScanStop);
  #undef REGISTER_ASYNC

  LoggerD("RadioInstance()");
}

RadioInstance::~RadioInstance() {
}

void RadioInstance::MuteGetter(const picojson::value& args,
                               picojson::object& out) {
  LoggerD("Enter");
  ReportSuccess(picojson::value(manager_.IsMuted()), out);
}

void RadioInstance::MuteSetter(const picojson::value& args,
                               picojson::object& out) {
  LoggerD("Enter");
  manager_.SetMute(args.get("mute").get<bool>());
  ReportSuccess(out);
}

void RadioInstance::AntennaGetter(const picojson::value& args,
                                  picojson::object& out) {
  LoggerD("Enter");
  ReportSuccess(picojson::value(manager_.HasAntenna()), out);
}

void RadioInstance::StateGetter(const picojson::value& args,
                                picojson::object& out) {
  LoggerD("Enter");
  ReportSuccess(picojson::value(manager_.GetState()), out);
}

void RadioInstance::FrequencyGetter(const picojson::value& args,
                                    picojson::object& out) {
  LoggerD("Enter");
  ReportSuccess(picojson::value(manager_.GetFrequency()), out);
}

void RadioInstance::SignalStrengthGetter(const picojson::value& args,
                                         picojson::object& out) {
  LoggerD("Enter");
  ReportSuccess(picojson::value(manager_.GetSignalStrength()), out);
}

void RadioInstance::SeekUp(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  manager_.SeekUp(args.get("callbackId").get<double>());
  ReportSuccess(out);
}

void RadioInstance::SeekDown(const picojson::value& args,
                             picojson::object& out) {
  LoggerD("Enter");
  manager_.SeekDown(args.get("callbackId").get<double>());
  ReportSuccess(out);
}

void RadioInstance::Start(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");

  PlatformResult result = manager_.Start(args.get("frequency").get<double>());

  if (result) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

void RadioInstance::Stop(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");

  PlatformResult result = manager_.Stop();

  if (result) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

void RadioInstance::ScanStart(const picojson::value& args,
                              picojson::object& out) {
  LoggerD("Enter");
  manager_.ScanStart(args.get("callbackId").get<double>());
  ReportSuccess(out);
}

void RadioInstance::ScanStop(const picojson::value& args,
                             picojson::object& out) {
  LoggerD("Enter");
  manager_.ScanStop(args.get("callbackId").get<double>());
  ReportSuccess(out);
}

void RadioInstance::SetFMRadioInterruptedListener(const picojson::value& args,
                                                  picojson::object& out) {
  LoggerD("Enter");

  PlatformResult result = manager_.SetFMRadioInterruptedListener();

  if (result) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

void RadioInstance::UnsetFMRadioInterruptedListener(const picojson::value& args,
                                                    picojson::object& out) {
  LoggerD("Enter");

  PlatformResult result = manager_.UnsetFMRadioInterruptedListener();

  if (result) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

void RadioInstance::SetAntennaChangeListener(const picojson::value& args,
                                             picojson::object& out) {
  LoggerD("Enter");

  PlatformResult result = manager_.SetAntennaChangeListener();

  if (result) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

void RadioInstance::UnsetAntennaChangeListener(const picojson::value& args,
                                               picojson::object& out) {
  LoggerD("Enter");

  PlatformResult result = manager_.UnsetAntennaChangeListener();

  if (result) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

} // namespace radio
} // namespace extension
