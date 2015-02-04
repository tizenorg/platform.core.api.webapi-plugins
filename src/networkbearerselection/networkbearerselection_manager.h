// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NETWORKBEARERSELECTION_NETWORKBEARERSELECTION_MANAGER_H_
#define NETWORKBEARERSELECTION_NETWORKBEARERSELECTION_MANAGER_H_

#include <string>
#include <list>
#include <functional>
#include <device/callback.h>
#include <net_connection.h>

namespace extension {
namespace networkbearerselection {

enum class ConnectionState {
  Unknown,
  Connected,
  Disconnected,
  ConnectionFailed
};

enum class NetworkType {
  Cellular,
  Unknown
};

class NetworkBearerSelectionListener {
 public:
  virtual void onNBSSuccess(const std::string& domain_name) = 0;
  virtual void onNBSError(const std::string& domain_name,
                          const std::string& info) = 0;
  virtual void onNBSDisconnect(const std::string& domain_name) = 0;
};

class NetworkBearerSelectionManager {
 public:
  typedef std::function<void(bool)> ReleaseReplyCallback;
  void AddListener(NetworkBearerSelectionListener* listener);

  void requestRouteToHost(const std::string& domain_name);
  bool releaseRouteToHost(const std::string& domain_name,
                          const ReleaseReplyCallback& reply_cb);

  static NetworkBearerSelectionManager* GetInstance();

  NetworkBearerSelectionManager(const NetworkBearerSelectionManager&) = delete;
  NetworkBearerSelectionManager& operator=(
      const NetworkBearerSelectionManager&) = delete;

 private:
  static void connection_state_changed_callback(
      connection_profile_state_e state,
      void* user_data);
  static void connection_profile_opened_callback(connection_error_e result,
                                                 void* user_data);
  static void connection_closed_callback(connection_error_e result,
                                         void* user_data);
  static void connection_closed_callback2(connection_error_e result,
                                          void* user_data);

  void registStateChangeListener(const std::string& domain_name);
  void deregistStateChangeListener(const std::string& domain_name);

  void makeSuccessCallback(const std::string& domain_name);
  void makeErrorCallback(const std::string& domain_name, const char* info);
  void makeErrorCallback(const std::string& domain_name,
                         const std::string& info);
  void makeDisconnectCallback(const std::string& domain_name);

  NetworkBearerSelectionManager();
  ~NetworkBearerSelectionManager();

  std::list<NetworkBearerSelectionListener*> m_listeners;

  connection_h m_connectionHandle;
  connection_profile_h m_profileHandle;
  std::list<std::string> m_domainNames;
  ConnectionState m_connectionState;
  bool m_isConnectionOpen;
};

}  // namespace networkbearerselection
}  // namespace extension

#endif  // NETWORKBEARERSELECTION_NETWORKBEARERSELECTION_MANAGER_H_
