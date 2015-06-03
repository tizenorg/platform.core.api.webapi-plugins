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

#ifndef RADIO_RADIO_INSTANCE_H_
#define RADIO_RADIO_INSTANCE_H_

#include "common/extension.h"
#include "common/picojson.h"

#include "radio/radio_manager.h"

namespace extension {
namespace radio {

class RadioInstance : public common::ParsedInstance {
 public:
  RadioInstance();
  virtual ~RadioInstance();

 private:
  void MuteGetter(const picojson::value& args, picojson::object& out);
  void MuteSetter(const picojson::value& args, picojson::object& out);
  void FrequencyGetter(const picojson::value& args, picojson::object& out);
  void SignalStrengthGetter(const picojson::value& args, picojson::object& out);
  void AntennaGetter(const picojson::value& args, picojson::object& out);
  void StateGetter(const picojson::value& args, picojson::object& out);
  void SeekUp(const picojson::value& args, picojson::object& out);
  void SeekDown(const picojson::value& args, picojson::object& out);
  void ScanStart(const picojson::value& args, picojson::object& out);
  void ScanStop(const picojson::value& args, picojson::object& out);
  void Start(const picojson::value& args, picojson::object& out);
  void Stop(const picojson::value& args, picojson::object& out);
  void SetFMRadioInterruptedListener(const picojson::value& args, picojson::object& out);
  void UnsetFMRadioInterruptedListener(const picojson::value& args, picojson::object& out);
  void SetAntennaChangeListener(const picojson::value& args, picojson::object& out);
  void UnsetAntennaChangeListener(const picojson::value& args, picojson::object& out);

  FMRadioManager manager_;
};

} // namespace radio
} // namespace extension

#endif
