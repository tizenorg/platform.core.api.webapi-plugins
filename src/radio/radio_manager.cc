// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radio_manager.h"

#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <glib.h>
#include <runtime_info.h>
#include <vconf.h>

#include "common/logger.h"
#include "common/extension.h"

using namespace common;
using namespace std;

namespace extension {
namespace radio {

namespace {

const char* RADIO_STATE_ERROR = "ERROR";
std::map<radio_state_e, const char*> radio_state = {
  { RADIO_STATE_READY, "READY" },
  { RADIO_STATE_PLAYING, "PLAYING" },
  { RADIO_STATE_SCANNING, "SCANNING" },
};
static const double FREQ_LOWER = 87.5;

static void AddCallbackID(double callbackId, picojson::object* obj) {
  obj->insert(std::make_pair("callbackId", picojson::value(callbackId)));
}

static void PostResultSuccess(double callbackId, picojson::value* event) {
  auto& obj = event->get<picojson::object>();

  tools::ReportSuccess(obj);
  AddCallbackID(callbackId, &obj);

  RadioInstance::getInstance().PostMessage(event->serialize().c_str());
}

static void PostResultSuccess(double callbackId) {
  picojson::value event{picojson::object()};

  PostResultSuccess(callbackId, &event);
}

static void PostResultFailure(double callbackId, const PlatformResult& result) {
  picojson::value event{picojson::object()};
  auto& obj = event.get<picojson::object>();

  tools::ReportError(result, &obj);
  AddCallbackID(callbackId, &obj);

  RadioInstance::getInstance().PostMessage(event.serialize().c_str());
}

PlatformResult GetPlatformResult(const std::string& str, int err) {
  LoggerD("Enter");

  string message = str + " : " + to_string(err);

  switch (err) {
    case RADIO_ERROR_INVALID_PARAMETER:
      return PlatformResult(ErrorCode::INVALID_VALUES_ERR, message);

    case RADIO_ERROR_INVALID_STATE:
      return PlatformResult(ErrorCode::INVALID_STATE_ERR, message);

    case RADIO_ERROR_NOT_SUPPORTED:
      return PlatformResult(ErrorCode::SERVICE_NOT_AVAILABLE_ERR, message);

    default:
      return PlatformResult(ErrorCode::UNKNOWN_ERR, message);
  }
}

PlatformResult CheckError(const std::string& str, int err) {
  LoggerE("%s() error %d", str.c_str(), err);

  switch (err) {
    case RADIO_ERROR_NONE:
      return PlatformResult(ErrorCode::NO_ERROR);

    default:
      return GetPlatformResult(str, err);
  }
}

string TranslateInterruptedCode(int code) {
  LoggerD("Enter");
#define STRINGIFY(c) case c: return #c
  switch (code) {
    STRINGIFY(RADIO_INTERRUPTED_BY_MEDIA);
    STRINGIFY(RADIO_INTERRUPTED_BY_CALL);
    STRINGIFY(RADIO_INTERRUPTED_BY_EARJACK_UNPLUG);
    STRINGIFY(RADIO_INTERRUPTED_BY_RESOURCE_CONFLICT);
    STRINGIFY(RADIO_INTERRUPTED_BY_ALARM);
    STRINGIFY(RADIO_INTERRUPTED_BY_EMERGENCY);
    STRINGIFY(RADIO_INTERRUPTED_BY_RESUMABLE_MEDIA);
    STRINGIFY(RADIO_INTERRUPTED_BY_NOTIFICATION);
    default: return "UNKNOWN_INTERRUPTED_ERROR_CODE";
  }
#undef STRINGIFY
}

int TokHz(double frequency) {
  return static_cast<int>(frequency * 1000.0);
}

double ToMHz(int frequency) {
  return static_cast<double>(frequency) / 1000.0;
}

void RadioSeekCallback(int frequency, void* user_data) {
  LoggerD("Enter");

  double* id = static_cast<double*>(user_data);

  PlatformResult result = FMRadioManager::GetInstance()->SetFrequency(ToMHz(frequency));
  if (result) {
    PostResultSuccess(*id);
  } else {
    PostResultFailure(*id, result);
  }

  delete id;
}

struct ScanData {
  double callback_id_;
  std::vector<int> frequencies_;
};

void ScanStartCallback(int frequency, void* user_data) {
  LoggerD("Enter");

  ScanData* data = static_cast<ScanData*>(user_data);
  data->frequencies_.push_back(frequency);

  picojson::value event{picojson::object()};
  auto& obj = event.get<picojson::object>();
  obj.insert(std::make_pair("frequency", picojson::value(ToMHz(frequency))));
  obj.insert(std::make_pair("listenerId", picojson::value("FMRadio_Onfrequencyfound")));
  RadioInstance::getInstance().PostMessage(event.serialize().c_str());
}

void ScanCompleteCallback(void* user_data) {
  LoggerD("Enter");

  ScanData* data = static_cast<ScanData*>(user_data);

  picojson::value event{picojson::object()};
  auto& obj = event.get<picojson::object>();
  obj.insert(std::make_pair("name", picojson::value("onfinished")));

  picojson::array frequencies;
  for (auto frequency : data->frequencies_) {
    frequencies.push_back(picojson::value(ToMHz(frequency)));
  }

  obj.insert(std::make_pair("frequencies", picojson::value(frequencies)));
  PostResultSuccess(data->callback_id_, &event);

  delete data;
}

void ScanStopCallback(void *user_data) {
  LoggerD("Enter");
  double* callback_id = static_cast<double*>(user_data);

  PostResultSuccess(*callback_id);
  delete callback_id;
}

void RadioInterruptedCallback(radio_interrupted_code_e code, void *user_data) {
  LoggerD("Enter");

  picojson::value event{picojson::object()};
  auto& obj = event.get<picojson::object>();

  obj.insert(std::make_pair("listenerId", picojson::value("FMRadio_Interrupted")));

  if (code == RADIO_INTERRUPTED_COMPLETED) {
    obj.insert(
        std::make_pair("action", picojson::value("oninterruptfinished")));
  } else {
    obj.insert(std::make_pair("action", picojson::value("oninterrupted")));
    obj.insert(std::make_pair("reason", picojson::value(TranslateInterruptedCode(code))));
  }

  RadioInstance::getInstance().PostMessage(event.serialize().c_str());
}


void RadioAntennaCallback(runtime_info_key_e key, void* user_data) {
  LoggerD("Enter");

  bool connected = false;
  const auto err = runtime_info_get_value_bool(key, &connected);
  if (RADIO_ERROR_NONE != err) {
    LoggerE("runtime_info_get_value_bool() failed: %d", err);
  }

  picojson::value event{picojson::object()};
  auto& obj = event.get<picojson::object>();

  obj.insert(std::make_pair("connected", picojson::value(connected)));
  obj.insert(std::make_pair("listenerId", picojson::value("FMRadio_Antenna")));

  RadioInstance::getInstance().PostMessage(event.serialize().c_str());
}

} // namespace

bool FMRadioManager::IsMuted() {
  LoggerD("Enter");

  bool muted = false;
  const auto err = radio_is_muted(radio_instance_, &muted);

  if (RADIO_ERROR_NONE != err) {
    LoggerE("radio_is_muted() failed: %d", err);
  }

  return muted;
}

void FMRadioManager::SetMute(bool mute) {
  LoggerD("Enter");

  const auto err = radio_set_mute(radio_instance_, mute);

  if (RADIO_ERROR_NONE != err) {
    LoggerE("radio_set_mute() failed: %d", err);
  }
}

bool FMRadioManager::HasAntenna() {
  LoggerD("Enter");

  bool connected = false;
  const auto err = runtime_info_get_value_bool(RUNTIME_INFO_KEY_AUDIO_JACK_CONNECTED,
                                               &connected);

  if (RUNTIME_INFO_ERROR_NONE != err) {
    LoggerE("runtime_info_get_value_bool() failed: %d", err);
  }

  return connected;
}

const char* FMRadioManager::GetState() {
  LoggerD("Enter");

  radio_state_e state;

  const auto err = radio_get_state(radio_instance_, &state);

  if (RADIO_ERROR_NONE != err) {
    LoggerE("radio_get_state() failed: %d", err);
    return RADIO_STATE_ERROR;
  }

  const auto s = radio_state.find(state);

  if (radio_state.end() != s) {
    return s->second;
  } else {
    LoggerE("Unknown state: %d", state);
    return RADIO_STATE_ERROR;
  }
}

PlatformResult FMRadioManager::SetFrequency(double frequency) {
  LoggerD("Enter");
  return CheckError("radio_set_frequency", radio_set_frequency(radio_instance_, TokHz(frequency)));
}

double FMRadioManager::GetFrequency() {
  LoggerD("Enter");

  int frequency = 0;
  const auto err = radio_get_frequency(radio_instance_, &frequency);

  if (RADIO_ERROR_NONE != err) {
    LoggerE("radio_get_frequency() failed: %d", err);
    return FREQ_LOWER;
  } else {
    return ToMHz(frequency);
  }
}

double FMRadioManager::GetSignalStrength() {
  LoggerD("Enter");

  int strength = 0;
  const auto err = radio_get_signal_strength(radio_instance_, &strength);

  if (RADIO_ERROR_NONE != err) {
    LoggerE("radio_get_signal_strength() failed: %d", err);
    return 0;
  } else {
    return strength;
  }
}

FMRadioManager::FMRadioManager()
    : radio_instance_(nullptr) {
  LoggerD("Enter");

  const auto err = radio_create(&radio_instance_);

  if (RADIO_ERROR_NONE != err) {
    LoggerE("radio_create() failed: %d", err);
    radio_instance_ = nullptr;
  }
}

FMRadioManager::~FMRadioManager() {
  LoggerD("Enter");

  if (radio_instance_) {
    const auto err = radio_destroy(radio_instance_);

    if (RADIO_ERROR_NONE != err) {
      LoggerE("radio_destroy() failed: %d", err);
    }

    radio_instance_ = nullptr;
  }
}

FMRadioManager* FMRadioManager::GetInstance() {
  LoggerD("Enter");

  static FMRadioManager instance;
  return &instance;
}

PlatformResult FMRadioManager::Start(double frequency) {
  LoggerD("Enter, frequency: %f", frequency);

  PlatformResult result = SetFrequency(frequency);

  if (!result) {
    return result;
  }

  return CheckError("radio_start", radio_start(radio_instance_));
}

PlatformResult FMRadioManager::Stop() {
  LoggerD("Enter");

  return CheckError("radio_stop", radio_stop(radio_instance_));
}

void FMRadioManager::SeekUp(double callback_id) {
  LoggerD("Enter");

  double* user_data = new double(callback_id);

  const auto err = radio_seek_up(radio_instance_, RadioSeekCallback, user_data);

  if (RADIO_ERROR_NONE != err) {
    PostResultFailure(callback_id, GetPlatformResult("radio_seek_up", err));
    delete user_data;
  }
}

void FMRadioManager::SeekDown(double callback_id) {
  LoggerD("Enter");

  double* user_data = new double(callback_id);

  const auto err = radio_seek_down(radio_instance_, RadioSeekCallback, user_data);

  if (RADIO_ERROR_NONE != err) {
    PostResultFailure(callback_id, GetPlatformResult("radio_seek_down", err));
    delete user_data;
  }
}

void FMRadioManager::ScanStart(double callback_id) {
  LoggerD("Enter");

  ScanData* user_data = new ScanData();
  user_data->callback_id_ = callback_id;

  auto err = radio_set_scan_completed_cb(radio_instance_, ScanCompleteCallback,
                                         user_data);
  if (RADIO_ERROR_NONE != err) {
    PostResultFailure(callback_id,
                      GetPlatformResult("radio_set_scan_completed_cb", err));
    delete user_data;
    return;
  }

  err = radio_scan_start(radio_instance_, ScanStartCallback, user_data);
  if (RADIO_ERROR_NONE != err) {
    radio_unset_scan_completed_cb(radio_instance_);
    PostResultFailure(callback_id, GetPlatformResult("radio_scan_start", err));
    delete user_data;
  }
}

void FMRadioManager::ScanStop(double callback_id) {
  LoggerD("Enter");

  double* user_data = new double(callback_id);

  auto err = radio_unset_scan_completed_cb(radio_instance_);
  if (RADIO_ERROR_NONE != err) {
    PostResultFailure(callback_id,
                      GetPlatformResult("radio_unset_scan_completed_cb", err));
    delete user_data;
    return;
  }

  err = radio_scan_stop(radio_instance_, ScanStopCallback, user_data);
  if (RADIO_ERROR_NONE != err) {
    PostResultFailure(callback_id, GetPlatformResult("radio_scan_stop", err));
    delete user_data;
  }
}

common::PlatformResult FMRadioManager::SetFMRadioInterruptedListener() {
  LoggerD("Enter");

  const auto err = radio_set_interrupted_cb(radio_instance_,
                                            RadioInterruptedCallback,
                                            nullptr);
  return CheckError("radio_set_interrupted_cb", err);
}

common::PlatformResult FMRadioManager::UnsetFMRadioInterruptedListener() {
  LoggerD("Enter");

  const auto err = radio_unset_interrupted_cb(radio_instance_);
  return CheckError("radio_unset_interrupted_cb", err);
}

common::PlatformResult FMRadioManager::SetAntennaChangeListener() {
  LoggerD("Enter");

  const auto err = runtime_info_set_changed_cb(
                            RUNTIME_INFO_KEY_AUDIO_JACK_CONNECTED,
                            RadioAntennaCallback,
                            nullptr);
  return CheckError("runtime_info_set_changed_cb", err);
}

common::PlatformResult FMRadioManager::UnsetAntennaChangeListener() {
  LoggerD("Enter");

  const auto err = runtime_info_unset_changed_cb(RUNTIME_INFO_KEY_AUDIO_JACK_CONNECTED);
  return CheckError("runtime_info_unset_changed_cb", err);
}

} // namespace radio
} // namespace extension

