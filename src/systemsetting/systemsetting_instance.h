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

#ifndef SYSTEMSETTING_SYSTEMSETTING_INSTANCE_H_
#define SYSTEMSETTING_SYSTEMSETTING_INSTANCE_H_

#include "common/extension.h"
#include "common/platform_result.h"

namespace extension {
namespace systemsetting {

class SystemSettingInstance : public common::ParsedInstance
{
 public:
  SystemSettingInstance();
  virtual ~SystemSettingInstance();

 private:
  void getProperty(const picojson::value& args, picojson::object& out);
  common::PlatformResult getPlatformPropertyValue(
      const std::string& valueType,
      picojson::value* out);

  void setProperty(const picojson::value& args, picojson::object& out);
  common::PlatformResult setPlatformPropertyValue(
      const std::string& settingType,
      const std::string& settingValue);
};

} // namespace systemsetting
} // namespace extension

#endif // SYSTEMSETTING_SYSTEMSETTING_INSTANCE_H_
