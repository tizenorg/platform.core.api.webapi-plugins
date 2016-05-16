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

#ifndef WEBAPI_PLUGINS_CONVERGENCE_MANAGER_H__
#define WEBAPI_PLUGINS_CONVERGENCE_MANAGER_H__

//#include <memory>
//#include <map>
//#include "common/tizen_result.h"
#include <d2d_conv_manager.h>
#include "common/tizen_result.h"
#include <string>
#include <unordered_map>

namespace extension {
namespace convergence {

enum ServiceCommands {
 SERVICE_START,
 SERVICE_READ,
 SERVICE_SEND,
 SERVICE_STOP,
};

class ConvergenceManager {
 public:
   static ConvergenceManager &GetInstance(class ConvergenceInstance *owner);

 public:
  ConvergenceManager();
  ~ConvergenceManager();
  ConvergenceManager(const ConvergenceManager&) = delete;
  ConvergenceManager(ConvergenceManager&&) = delete;
  ConvergenceManager& operator=(const ConvergenceManager&) = delete;
  ConvergenceManager& operator=(ConvergenceManager&&) = delete;

 public:
  common::TizenResult StartDiscovery(long timeout);
  common::TizenResult StopDiscovery();
  common::TizenResult Connect(const char *deviceId, const int serviceType,
   const int curListenerId);
  common::TizenResult Disconnect(const char *deviceId, const int serviceType);
  common::TizenResult Start(const char *deviceId, const int serviceType,
    const picojson::value &channel, const picojson::value &payload);
  common::TizenResult Read(const char *deviceId, const int serviceType,
    const picojson::value &channel, const picojson::value &payload);
  common::TizenResult Send(const char *deviceId, const int serviceType,
    const picojson::value &channel, const picojson::value &payload);
  common::TizenResult Stop(const char *deviceId, const int serviceType,
    const picojson::value &channel, const picojson::value &payload);
  common::TizenResult SetListener(const char *deviceId, const int serviceType,
   const int curListenerId);
  common::TizenResult UnsetListener(const char *deviceId,
   const int serviceType);

 private:
  static void DiscoveryCb(conv_device_h device_handle,
   conv_discovery_result_e result, void* user_data);
  static void ServiceForeachCb(conv_service_h service_handle, void* user_data);
  static void ServiceConnectedCb(conv_service_h service_handle,
   conv_error_e error, conv_payload_h result, void* user_data);
  static void FindServiceCb(conv_service_h service_handle, void* user_data);
  static void ServiceListenerCb(conv_service_h service_handle,
   conv_channel_h channel_handle,
   conv_error_e error, conv_payload_h result, void* user_data);

  common::TizenResult ExecuteServiceCommand(const ServiceCommands command,
   const char *deviceId,
   const int serviceType,
   const picojson::value &channel,
   const picojson::value &payload);

 private:
  conv_device_h FindDevice(const char *deviceId);
  conv_service_h FindService(const conv_device_h device, const int serviceType);

 private:
  class ConvergenceInstance *plg;
  conv_h conv_;
  std::unordered_map<std::string, conv_device_h> devices;
};


} // namespace convergence
} // namespace extension

#endif // WEBAPI_PLUGINS_CONVERGENCE_MANAGER_H__
