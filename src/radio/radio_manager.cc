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

#include "radio_manager.h"

#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <glib.h>
#include <runtime_info.h>
#include <vconf.h>

#include "common/logger.h"
#include "common/extension.h"
#include "common/task-queue.h"

#include "radio/radio_instance.h"

using namespace common;
using namespace std;

namespace extension {
namespace radio {

namespace {

const int kLowestFrequency = 87500;
const int kHighestFrequency = 108000;

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

struct RadioData {
  explicit RadioData(FMRadioManager& manager)
      : manager_(manager),
        callback_id_(0.0) {
  }

  FMRadioManager& manager_;
  double callback_id_;
};

struct RadioScanData : public RadioData {
  using RadioData::RadioData;
  std::vector<int> frequencies_;
};

void RadioSeekCallback(int frequency, void* user_data) {
  LoggerD("Enter, freq: %d", frequency);

  RadioData* data = static_cast<RadioData*>(user_data);

  if (frequency >= kLowestFrequency && frequency <= kHighestFrequency) {
    common::TaskQueue::GetInstance().Async(std::bind(
      &FMRadioManager::PostResultCallbackSuccess, &data->manager_,
      data->callback_id_));
  } else {
    common::TaskQueue::GetInstance().Async(std::bind(
      &FMRadioManager::PostResultFailure, &data->manager_,
      data->callback_id_, PlatformResult(ErrorCode::UNKNOWN_ERR,
        "Unsupported frequency")));
  }
  delete data;
}

void ScanStartCallback(int frequency, void* user_data) {
  LoggerD("Enter");

  RadioScanData* data = static_cast<RadioScanData*>(user_data);
  data->frequencies_.push_back(frequency);

  picojson::value event{picojson::object()};
  auto& obj = event.get<picojson::object>();
  obj.insert(std::make_pair("frequency", picojson::value(ToMHz(frequency))));
  obj.insert(std::make_pair("listenerId", picojson::value("FMRadio_Onfrequencyfound")));
  common::TaskQueue::GetInstance().Async(std::bind(
    &FMRadioManager::PostMessage, &data->manager_, event.serialize()));
}

void PostAsyncSuccess(FMRadioManager* manager, double callbackId, picojson::value* event) {
  manager->PostResultSuccess(callbackId, event);
  delete event;
}

void ScanCompleteCallback(void* user_data) {
  LoggerD("Enter");

  RadioScanData* data = static_cast<RadioScanData*>(user_data);

  picojson::value* event = new picojson::value(picojson::object());
  auto& obj = event->get<picojson::object>();
  obj.insert(std::make_pair("name", picojson::value("onfinished")));

  picojson::array frequencies;
  for (auto frequency : data->frequencies_) {
    frequencies.push_back(picojson::value(ToMHz(frequency)));
  }

  obj.insert(std::make_pair("frequencies", picojson::value(frequencies)));
  common::TaskQueue::GetInstance().Async(std::bind(&PostAsyncSuccess,
    &data->manager_, data->callback_id_, event));

  delete data;
}

void ScanStopCallback(void *user_data) {
  LoggerD("Enter");
  RadioData* data = static_cast<RadioData*>(user_data);

  common::TaskQueue::GetInstance().Async(std::bind(
    &FMRadioManager::PostResultCallbackSuccess, &data->manager_, data->callback_id_));
  delete data;
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

  FMRadioManager* manager = static_cast<FMRadioManager*>(user_data);
  common::TaskQueue::GetInstance().Async(std::bind(
    &FMRadioManager::PostMessage, manager, event.serialize()));
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

  FMRadioManager* manager = static_cast<FMRadioManager*>(user_data);
  common::TaskQueue::GetInstance().Async(std::bind(
    &FMRadioManager::PostMessage, manager, event.serialize()));
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
    LoggerD("Frequency: %d", frequency);
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

FMRadioManager::FMRadioManager(RadioInstance& instance)
    : instance_(instance),
      radio_instance_(nullptr) {
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

PlatformResult FMRadioManager::Start(double frequency) {
  LoggerD("Enter, frequency: %f", frequency);

  radio_state_e state;
  const auto err = radio_get_state(radio_instance_, &state);

  if (RADIO_ERROR_NONE != err) {
    LoggerE("radio_get_state() failed: %d", err);
    return GetPlatformResult("radio_get_state() failed.", err);
  }

  if (RADIO_STATE_READY != state) {
    if (RADIO_STATE_PLAYING == state) {
      return PlatformResult(ErrorCode::NO_ERROR);
    }
    return PlatformResult(ErrorCode::INVALID_STATE_ERR, "Invalid radio state.");
  }

  PlatformResult result = SetFrequency(frequency);

  if (!result) {
    return result;
  }

  return CheckError("radio_start", radio_start(radio_instance_));
}

PlatformResult FMRadioManager::Stop() {
  LoggerD("Enter");

  radio_state_e state;
  const auto err = radio_get_state(radio_instance_, &state);

  if (RADIO_ERROR_NONE != err) {
    LoggerE("radio_get_state() failed: %d", err);
    return GetPlatformResult("radio_get_state() failed.", err);
  }

  if (RADIO_STATE_PLAYING != state) {
    return PlatformResult(ErrorCode::INVALID_STATE_ERR, "Invalid radio state.");
  }

  return CheckError("radio_stop", radio_stop(radio_instance_));
}

void FMRadioManager::SeekUp(double callback_id) {
  LoggerD("Enter");

  RadioData* user_data = new RadioData(*this);
  user_data->callback_id_ = callback_id;

  const auto err = radio_seek_up(radio_instance_, RadioSeekCallback, user_data);

  if (RADIO_ERROR_NONE != err) {
    LoggerE("Failed");
    PostResultFailure(callback_id, GetPlatformResult("radio_seek_up", err));
    delete user_data;
  }
}

void FMRadioManager::SeekDown(double callback_id) {
  LoggerD("Enter");

  RadioData* user_data = new RadioData(*this);
  user_data->callback_id_ = callback_id;

  const auto err = radio_seek_down(radio_instance_, RadioSeekCallback, user_data);

  if (RADIO_ERROR_NONE != err) {
    LoggerE("Failed");
    PostResultFailure(callback_id, GetPlatformResult("radio_seek_down", err));
    delete user_data;
  }
}

void FMRadioManager::ScanStart(double callback_id) {
  LoggerD("Enter");

  RadioScanData* user_data = new RadioScanData(*this);
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

  RadioScanData* user_data = new RadioScanData(*this);
  user_data->callback_id_ = callback_id;

  auto err = radio_unset_scan_completed_cb(radio_instance_);
  if (RADIO_ERROR_NONE != err) {
    LoggerE("Failed");
    PostResultFailure(callback_id,
                      GetPlatformResult("radio_unset_scan_completed_cb", err));
    delete user_data;
    return;
  }

  err = radio_scan_stop(radio_instance_, ScanStopCallback, user_data);
  if (RADIO_ERROR_NONE != err) {
    LoggerE("Failed");
    PostResultFailure(callback_id, GetPlatformResult("radio_scan_stop", err));
    delete user_data;
  }
}

common::PlatformResult FMRadioManager::SetFMRadioInterruptedListener() {
  LoggerD("Enter");

  const auto err = radio_set_interrupted_cb(radio_instance_,
                                            RadioInterruptedCallback,
                                            this);
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
                            this);
  return CheckError("runtime_info_set_changed_cb", err);
}

common::PlatformResult FMRadioManager::UnsetAntennaChangeListener() {
  LoggerD("Enter");

  const auto err = runtime_info_unset_changed_cb(RUNTIME_INFO_KEY_AUDIO_JACK_CONNECTED);
  return CheckError("runtime_info_unset_changed_cb", err);
}

void FMRadioManager::PostMessage(const std::string& msg) const {
  LoggerD("Enter");

  Instance::PostMessage(&instance_, msg.c_str());
}
void FMRadioManager::PostResultSuccess(double callbackId, picojson::value* event) const {
  auto& obj = event->get<picojson::object>();

  tools::ReportSuccess(obj);
  AddCallbackID(callbackId, &obj);

  PostMessage(event->serialize());
}

void FMRadioManager::PostResultCallbackSuccess(double callbackId) const {
  picojson::value event{picojson::object()};

  PostResultSuccess(callbackId, &event);
}

void FMRadioManager::PostResultFailure(double callbackId, const PlatformResult& result) const {
  picojson::value event{picojson::object()};
  auto& obj = event.get<picojson::object>();

  tools::ReportError(result, &obj);
  AddCallbackID(callbackId, &obj);

  PostMessage(event.serialize());
}

} // namespace radio
} // namespace extension

