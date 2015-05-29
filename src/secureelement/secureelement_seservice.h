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

#ifndef SECUREELEMENT_SESERVICE_H_
#define SECUREELEMENT_SESERVICE_H_

#include <SEService.h>
#include <SEServiceHelper.h>

namespace extension {
namespace secureelement {

class SecureElementInstance;

class SEService {
 public:
  explicit SEService(SecureElementInstance& instance);
  ~SEService();

  void GetReaders(double callback_id);
  void RegisterSEListener();
  void UnregisterSEListener();
  void Shutdown();

  void ServiceConnected();
  void EventHandler(char *se_name, int event);
  void ErrorHandler(int error);
 private:
  SEService(const SEService&) = delete;
  SEService& operator=(const SEService&) = delete;

  smartcard_service_api::SEService *se_service_;
  bool is_initialized_;
  bool is_listener_set_;
  bool is_error_;
  std::vector<double> get_readers_callbacks_;
  SecureElementInstance& instance_;
};

} // secureelement
} // extension

#endif // SECUREELEMENT_SESERVICE_H_
