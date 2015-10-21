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

#ifndef FMRADIO_FMRADIO_MANAGER_H_
#define FMRADIO_FMRADIO_MANAGER_H_

#include <list>
#include <string>
#include <vector>
#include <mutex>

#include <radio.h>
#include <runtime_info.h>

#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace radio {

class RadioInstance;

class FMRadioManager {
 public:
  explicit FMRadioManager(RadioInstance& instance);
  // FMRadioManager destroys radio_h in destructor, so it cannot be copyable
  FMRadioManager(const FMRadioManager& other) = delete;
  ~FMRadioManager();

  common::PlatformResult Start(double freq);
  common::PlatformResult Stop();
  void SeekUp(double callback_id);
  void SeekDown(double callback_id);
  void ScanStart(double callback_id);
  void ScanStop(double callback_id);
  common::PlatformResult SetFMRadioInterruptedListener();
  common::PlatformResult UnsetFMRadioInterruptedListener();
  common::PlatformResult SetAntennaChangeListener();
  common::PlatformResult UnsetAntennaChangeListener();

  bool IsMuted();
  void SetMute(bool mute);
  common::PlatformResult SetFrequency(double frequency);
  double GetFrequency();
  double GetSignalStrength();
  bool HasAntenna();
  const char* GetState();

  void PostMessage(const std::string& msg) const;
  void PostResultSuccess(double callbackId, picojson::value* event) const;
  void PostResultCallbackSuccess(double callbackId) const;
  void PostResultFailure(double callbackId, const common::PlatformResult& result) const;

 private:
  RadioInstance& instance_;
  radio_h radio_instance_;
};

} // namespace radio
} // namespace extension

#endif // RADIO_RADIO_MANAGER_H_
