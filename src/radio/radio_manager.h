// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FMRADIO_FMRADIO_MANAGER_H_
#define FMRADIO_FMRADIO_MANAGER_H_

#include <list>
#include <string>
#include <vector>

#include <radio.h>
#include <runtime_info.h>

#include "common/picojson.h"
#include "common/platform_result.h"

#include "radio_instance.h"

namespace extension {
namespace radio {

class FMRadioManager {
 public:
  static FMRadioManager* GetInstance();

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

 private:
  FMRadioManager();
  ~FMRadioManager();

  radio_h radio_instance_;
};

} // namespace radio
} // namespace extension

#endif // RADIO_RADIO_MANAGER_H_
