// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIO_RADIO_INSTANCE_H_
#define RADIO_RADIO_INSTANCE_H_

#include "common/extension.h"


namespace extension {
namespace radio {

class RadioInstance
    : public common::ParsedInstance
      {
 public:
  RadioInstance();
  virtual ~RadioInstance();



 private:
  void Start(const picojson::value& args, picojson::object& out);
  void Stop(const picojson::value& args, picojson::object& out);
  void SeekUp(const picojson::value& args, picojson::object& out);
  void SeekDown(const picojson::value& args, picojson::object& out);
  void ScanStart(const picojson::value& args, picojson::object& out);
  void ScanStop(const picojson::value& args, picojson::object& out);
  void SetFMRadioInterruptedListener(const picojson::value& args, picojson::object& out);
  void UnsetFMRadioInterruptedListener(const picojson::value& args, picojson::object& out);
  void SetAntennaChangeListener(const picojson::value& args, picojson::object& out);
  void UnsetAntennaChangeListener(const picojson::value& args, picojson::object& out);
};

}
}

#endif
