// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIO_RADIO_INSTANCE_H_
#define RADIO_RADIO_INSTANCE_H_

#include "common/extension.h"
#include "radio_manager.h"
#include "common/picojson.h"

namespace extension {
namespace radio {

class RadioInstance
    : public common::ParsedInstance
      {
 public:
  RadioInstance();
  virtual ~RadioInstance();

  static RadioInstance& getInstance();

  void InstanceReportSuccess(picojson::object& out);

 private:
  void FrequencyGetter(const picojson::value& args,picojson::object& out);
  void SignalStrengthGetter(const picojson::value& args,picojson::object& out);
  void AntenaGetter(const picojson::value& args,picojson::object& out);
  void StateGetter(const picojson::value& args,picojson::object& out);
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
};

}
}

#endif
