// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYSTEMINFO_INSTANCE_H_
#define SYSTEMINFO_INSTANCE_H_

#include "common/extension.h"

namespace extension {
namespace systeminfo {

class SysteminfoInstance
    : public common::ParsedInstance
{
 public:
  static SysteminfoInstance& getInstance();

 private:
  SysteminfoInstance();
  virtual ~SysteminfoInstance();

  void GetCapabilities(const picojson::value& args, picojson::object& out);
  void GetCapability(const picojson::value& args, picojson::object& out);
  void GetPropertyValue(const picojson::value& args, picojson::object& out);
  void GetPropertyValueArray(const picojson::value& args, picojson::object& out);
  void AddPropertyValueChangeListener(const picojson::value& args, picojson::object& out);
  void RemovePropertyValueChangeListener(const picojson::value& args, picojson::object& out);
  void GetMaxBrightness(const picojson::value& args, picojson::object& out);
  void GetBrightness(const picojson::value& args, picojson::object& out);
  void SetBrightness(const picojson::value& args, picojson::object& out);
  void GetTotalMemory(const picojson::value& args, picojson::object& out);
  void GetAvailableMemory(const picojson::value& args, picojson::object& out);
  void GetCount(const picojson::value& args, picojson::object& out);
};

} // namespace systeminfo
} // namespace extension

#endif // SYSTEMINFO_INSTANCE_H_
