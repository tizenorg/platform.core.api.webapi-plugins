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

#ifndef SYSTEMINFO_INSTANCE_H_
#define SYSTEMINFO_INSTANCE_H_

#include "common/extension.h"
#include "systeminfo/systeminfo_manager.h"

namespace extension {
namespace systeminfo {

class SysteminfoInstance
    : public common::ParsedInstance
{
 public:
  SysteminfoInstance();
  virtual ~SysteminfoInstance();
 private:
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

  SysteminfoManager manager_;
};

} // namespace systeminfo
} // namespace extension

#endif // SYSTEMINFO_INSTANCE_H_
