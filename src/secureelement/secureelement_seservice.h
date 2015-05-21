// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
