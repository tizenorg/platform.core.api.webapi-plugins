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

#ifndef NETWORKBEARERSELECTION_NETWORKBEARERSELECTION_MANAGER_H_
#define NETWORKBEARERSELECTION_NETWORKBEARERSELECTION_MANAGER_H_

#include <string>
#include <list>
#include <vector>
#include <mutex>
#include <memory>
#include <functional>
#include <device/callback.h>
#include <net_connection.h>

#include "common/platform_result.h"

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

struct NetworkBearerSelectionRequestEvent;
struct NetworkBearerSelectionReleaseEvent;

typedef std::shared_ptr<NetworkBearerSelectionRequestEvent> RequestEventPtr;
typedef std::shared_ptr<NetworkBearerSelectionReleaseEvent> ReleaseEventPtr;

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
  void RemoveListener(NetworkBearerSelectionListener* listener);

  void requestRouteToHost(const std::string& domain_name);
  common::PlatformResult releaseRouteToHost(const std::string& domain_name,
                                            const ReleaseReplyCallback& reply_cb);

  common::PlatformResult getCellularState();

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

  void registStateChangeListener(const std::string& domain_name);
  void deregistStateChangeListener(const std::string& domain_name);

  void makeSuccessCallback(const std::string& domain_name);
  void makeErrorCallback(const std::string& domain_name, const char* info);
  void makeErrorCallback(const std::string& domain_name,
                         const std::string& info);
  common::ErrorCode GetNBSErrorCode(int error_code);
  void makeDisconnectCallback(const std::string& domain_name);
  void destroyProfileHandle();
  RequestEventPtr getRequestEvent(NetworkBearerSelectionRequestEvent* event);
  ReleaseEventPtr getReleaseEvent(NetworkBearerSelectionReleaseEvent* event);

  NetworkBearerSelectionManager();
  ~NetworkBearerSelectionManager();

  std::list<NetworkBearerSelectionListener*> m_listeners_;

  connection_h m_connection_handle_;
  connection_profile_h m_profile_handle_;
  std::list<std::string> m_domain_names_;
  std::vector<RequestEventPtr> m_request_events_;
  std::vector<ReleaseEventPtr> m_release_events_;
  ConnectionState m_connection_state_;
  bool m_is_connection_open_;

  std::mutex m_mutex_;
  std::mutex m_request_mutex_;
  std::mutex m_release_mutex_;
};

}  // namespace networkbearerselection
}  // namespace extension

#endif  // NETWORKBEARERSELECTION_NETWORKBEARERSELECTION_MANAGER_H_
