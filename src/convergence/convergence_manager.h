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

#ifndef CONVERGENCE_CONVERGENCE_MANAGER_H__
#define CONVERGENCE_CONVERGENCE_MANAGER_H__

#include <d2d_conv_manager.h>

#include <string>
#include <unordered_map>

#include "common/tizen_result.h"

namespace extension {
namespace convergence {

enum ServiceCommands {
 kServiceStart,
 kServiceRead,
 kServiceSend,
 kServiceStop,
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
  common::TizenResult Connect(const char *device_id, const int service_type,
   const int cur_listener_id);
  common::TizenResult Disconnect(const char *device_id, const int service_type);
  common::TizenResult Start(const char *device_id, const int service_type,
    const picojson::value &channel, const picojson::value &payload);
  common::TizenResult Read(const char *device_id, const int service_type,
    const picojson::value &channel, const picojson::value &payload);
  common::TizenResult Send(const char *device_id, const int service_type,
    const picojson::value &channel, const picojson::value &payload);
  common::TizenResult Stop(const char *device_id, const int service_type,
    const picojson::value &channel, const picojson::value &payload);
  common::TizenResult SetListener(const char *device_id, const int service_type,
   const int cur_listener_id);
  common::TizenResult UnsetListener(const char *device_id,
   const int service_type);
  common::TizenResult CreateLocalService(const int service_type,
   const picojson::value &service_data);

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
   const char *device_id,
   const int service_type,
   const picojson::value &channel,
   const picojson::value &payload);

 private:
  conv_device_h get_remote_device(const char *device_id) const;
  conv_service_h get_remote_service(const conv_device_h device, const int service_type) const;
  conv_service_h get_service(const char *device_id, const int service_type) const;
  void DestroyService(conv_service_h service);
  bool is_local_device(const char *device_id) const;

 private:
  class ConvergenceInstance *convergence_plugin_;
  conv_h convergence_manager_;
  mutable std::unordered_map<std::string, conv_device_h> remote_devices_;
  mutable std::unordered_map<int, conv_service_h> local_services_;
};


} // namespace convergence
} // namespace extension

#endif // CONVERGENCE_CONVERGENCE_MANAGER_H__
