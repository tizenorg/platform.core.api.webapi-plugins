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

#ifndef WEBAPI_PLUGINS_SYSTEMINFO_SYSTEMINFO_SIM_DETAILS_MANAGER_H__
#define WEBAPI_PLUGINS_SYSTEMINFO_SYSTEMINFO_SIM_DETAILS_MANAGER_H__

#include <string>
#include <mutex>

#include <ITapiModem.h>
#include <ITapiSim.h>

#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace systeminfo {

class SimDetailsManager {
 public:
  SimDetailsManager();

  common::PlatformResult GatherSimInformation(TapiHandle* handle, picojson::object* out);
  long GetSimCount(TapiHandle **tapi_handle);
  void TryReturn();

  void set_operator_name(const std::string& name);
  void set_msisdn(const std::string& msisdn);
  void set_spn(const std::string& spn);

 private:
  void ResetSimHolder(picojson::object* out);
  void FetchSimState(TapiHandle *tapi_handle);
  common::PlatformResult FetchSimSyncProps(TapiHandle *tapi_handle);
  void ReturnSimToJS();

  unsigned short mcc_;
  unsigned short mnc_;
  std::string operator_name_;
  std::string msin_;
  std::string state_;
  std::string msisdn_;
  std::string iccid_;
  std::string spn_;

  picojson::object* sim_result_obj_;
  unsigned short to_process_;
  std::mutex sim_to_process_mutex_;
  std::mutex sim_info_mutex_;
};

} // namespace systeminfo
} // namespace webapi

#endif // WEBAPI_PLUGINS_SYSTEMINFO_SYSTEMINFO_SIM_DETAILS_MANAGER_H__
