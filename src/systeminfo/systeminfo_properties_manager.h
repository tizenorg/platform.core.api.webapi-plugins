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

#ifndef WEBAPI_PLUGINS_SYSTEMINFO_SYSTEMINFO_PROPERTIES_MANAGER_H__
#define WEBAPI_PLUGINS_SYSTEMINFO_SYSTEMINFO_PROPERTIES_MANAGER_H__

#include <string>
#include "common/picojson.h"
#include "common/platform_result.h"
#include "systeminfo/systeminfo_sim_details_manager.h"

namespace extension {
namespace systeminfo {

class SysteminfoManager;

class SysteminfoPropertiesManager {
 public:
  SysteminfoPropertiesManager(SysteminfoManager& manager);
  ~SysteminfoPropertiesManager();

  common::PlatformResult GetPropertyValue(
      const std::string& prop, bool is_array_type, picojson::value* res);

 private:
  common::PlatformResult ReportProperty(const std::string& property, int index,
                                               picojson::object* res_obj);
  common::PlatformResult ReportBattery(picojson::object* out);
  common::PlatformResult ReportCpu(picojson::object* out);
  common::PlatformResult ReportDisplay(picojson::object* out);
  common::PlatformResult ReportDeviceOrientation(picojson::object* out);
  common::PlatformResult ReportBuild(picojson::object* out);
  common::PlatformResult ReportLocale(picojson::object* out);
  common::PlatformResult ReportNetwork(picojson::object* out, unsigned long count);
  common::PlatformResult ReportWifiNetwork(picojson::object* out);
  common::PlatformResult ReportEthernetNetwork(picojson::object* out);
  common::PlatformResult ReportCellularNetwork(picojson::object* out, unsigned long count);
  common::PlatformResult ReportSim(picojson::object* out, unsigned long count);
  common::PlatformResult ReportPeripheral(picojson::object* out);
  common::PlatformResult ReportCameraFlash(picojson::object* out, unsigned long count);
  common::PlatformResult ReportMemory(picojson::object* out);
  common::PlatformResult ReportStorage(picojson::object* out);

  common::PlatformResult FetchIsAutoRotation(bool* result);
  common::PlatformResult FetchStatus(std::string* result);

  SysteminfoManager& manager_;
  SimDetailsManager sim_manager_;
};

} // namespace systeminfo
} // namespace webapi

#endif // WEBAPI_PLUGINS_SYSTEMINFO_SYSTEMINFO_PROPERTIES_MANAGER_H__
