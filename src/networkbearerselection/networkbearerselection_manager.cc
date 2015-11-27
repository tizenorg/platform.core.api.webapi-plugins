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

#include "networkbearerselection_manager.h"
#include "common/logger.h"
#include "common/scope_exit.h"

#include <netdb.h>
#include <arpa/inet.h>
#include <memory>

namespace extension {
namespace networkbearerselection {

namespace {
const char* kPlatformError = "Platform error";
}

struct NetworkBearerSelectionRequestEvent {
  std::string domain_name;
  NetworkBearerSelectionRequestEvent(const std::string& dm) : domain_name(dm) {}
};

struct NetworkBearerSelectionReleaseEvent {
  std::string domain_name;
  NetworkBearerSelectionManager::ReleaseReplyCallback callback;
  NetworkBearerSelectionReleaseEvent(
      const std::string& dm,
      const NetworkBearerSelectionManager::ReleaseReplyCallback& cb)
      : domain_name(dm), callback(cb) {}
};

void NetworkBearerSelectionManager::AddListener(
    NetworkBearerSelectionListener* listener) {
  LoggerD("Enter");
  std::lock_guard<std::mutex> lock(m_mutex_);
  m_listeners_.push_back(listener);
}

void NetworkBearerSelectionManager::RemoveListener(
    NetworkBearerSelectionListener* listener) {
  LoggerD("Enter");
  std::lock_guard<std::mutex> lock(m_mutex_);
  m_listeners_.remove(listener);
}

NetworkBearerSelectionManager* NetworkBearerSelectionManager::GetInstance() {
  LoggerD("Enter");
  static NetworkBearerSelectionManager instance;
  return &instance;
}

NetworkBearerSelectionManager::NetworkBearerSelectionManager()
    : m_connection_handle_(nullptr),
      m_profile_handle_(nullptr),
      m_connection_state_(ConnectionState::Unknown),
      m_is_connection_open_(false) {
  LoggerD("Enter");
  int ret = connection_create(&m_connection_handle_);

  if (CONNECTION_ERROR_NONE == ret) {
    LoggerD("Client registration success");
  } else {
    LoggerE("Client registration failed");
    m_connection_handle_ = nullptr;
  }
}

NetworkBearerSelectionManager::~NetworkBearerSelectionManager() {
  LoggerD("Enter");
  if (m_connection_handle_ != nullptr) {
    LoggerD("Client deregistration success");
    destroyProfileHandle();
    connection_destroy(m_connection_handle_);
  } else {
    LoggerE("Client deregistration failed");
  }

  {
    std::lock_guard<std::mutex> lock(m_request_mutex_);
    LoggerD("Delete %d request object(s)", m_request_events_.size());
    m_request_events_.clear();
  }

  {
    std::lock_guard<std::mutex> lock(m_release_mutex_);
    LoggerD("Delete %d release object(s)", m_release_events_.size());
    m_release_events_.clear();
  }
}

void NetworkBearerSelectionManager::connection_state_changed_callback(
    connection_profile_state_e state,
    void* user_data) {
  LoggerD("enter");
  if (user_data != nullptr) {
    LoggerD("Callback registration Succeeded");
    if (state == CONNECTION_PROFILE_STATE_ASSOCIATION) {
      LoggerD("association state");
      return;
    }

    RequestEventPtr event = NetworkBearerSelectionManager::GetInstance()->getRequestEvent(
        static_cast<NetworkBearerSelectionRequestEvent*>(user_data));
    if (!event) {
      LoggerD("Event is not found.");
      return;
    }

    if (state == CONNECTION_PROFILE_STATE_DISCONNECTED) {
      std::string domain_name = event->domain_name;
      NetworkBearerSelectionManager::GetInstance()->deregistStateChangeListener(domain_name);

      NetworkBearerSelectionManager::GetInstance()->makeDisconnectCallback(domain_name);
    }
  }
}

void NetworkBearerSelectionManager::connection_profile_opened_callback(
    connection_error_e result,
    void* user_data) {
  LoggerD("enter");
  if (user_data == nullptr) {
    LoggerD("Error: null passed in profile open callback");
    return;
  }

  RequestEventPtr event = NetworkBearerSelectionManager::GetInstance()->getRequestEvent(
      static_cast<NetworkBearerSelectionRequestEvent*>(user_data));
  if (!event) {
    LoggerD("Event is not found.");
    return;
  }

  std::string domain_name = event->domain_name;

  if (result == CONNECTION_ERROR_NONE) {
    LoggerD("Connection open Succeeded");
    if (user_data != nullptr) {
      NetworkBearerSelectionManager::GetInstance()->registStateChangeListener(
          domain_name);
    }
  } else {
    LoggerD("Connection open Failed");
    NetworkBearerSelectionManager::GetInstance()->makeErrorCallback(
        domain_name, kPlatformError);
  }
}

void NetworkBearerSelectionManager::connection_closed_callback(
    connection_error_e result,
    void* user_data) {
  LoggerD("enter");
  if (user_data == nullptr) {
    LoggerD("Error: null passed in profile open callback");
    return;
  }

  ReleaseEventPtr event = NetworkBearerSelectionManager::GetInstance()->getReleaseEvent(
      static_cast<NetworkBearerSelectionReleaseEvent*>(user_data));
  if (!event) {
    LoggerD("Event is not found.");
    return;
  }

  std::string domain_name = event->domain_name;
  ReleaseReplyCallback callback = event->callback;

  if (result == CONNECTION_ERROR_NONE) {
    LoggerD("Connection close Succeeded");
    if (user_data != nullptr) {
      NetworkBearerSelectionManager::GetInstance()->deregistStateChangeListener(
          domain_name);
      callback(true);
    }
  } else {
    callback(false);
  }
}

void NetworkBearerSelectionManager::requestRouteToHost(
    const std::string& domain_name) {
  LoggerD("NetworkBearerSelectionManager::requestRouteToHost");
  connection_profile_h profileHandle;

  if (m_connection_state_ == ConnectionState::Connected) {
    LoggerD("connection is already opened.");
    for (std::list<std::string>::iterator it = m_domain_names_.begin();
         it != m_domain_names_.end();
         it++) {
      if (*it == domain_name) {
        LoggerD("Same domain name is exist in list.");
        makeSuccessCallback(domain_name);
        return;
      }
    }
  }

  destroyProfileHandle();

  if (connection_get_default_cellular_service_profile(
          m_connection_handle_,
          CONNECTION_CELLULAR_SERVICE_TYPE_INTERNET,
          &m_profile_handle_) != CONNECTION_ERROR_NONE) {
    LoggerE("Fail to get profile handle");
    makeErrorCallback(domain_name, kPlatformError);
    return;
  }

  char* defaultProfileName_c = nullptr;
  std::string defaultProfileName;

  connection_profile_get_name(m_profile_handle_, &defaultProfileName_c);
  if (defaultProfileName_c == nullptr) {
    LoggerE("default profile is not exist.");
    makeErrorCallback(domain_name, kPlatformError);
    return;
  }
  defaultProfileName = defaultProfileName_c;
  free(defaultProfileName_c);
  defaultProfileName_c = nullptr;

  if (connection_get_current_profile(m_connection_handle_, &profileHandle) !=
      CONNECTION_ERROR_NONE) {
    LoggerE("Fail to get current profile handle");
    makeErrorCallback(domain_name, kPlatformError);
    return;
  }

  char* currentProfileName_c = nullptr;
  std::string currentProfileName;
  if (connection_profile_get_name(profileHandle, &currentProfileName_c) !=
      CONNECTION_ERROR_NONE) {
    LoggerE("Fail to get current profile name");
    makeErrorCallback(domain_name, kPlatformError);
    return;
  }

  if (currentProfileName_c == nullptr) {
    LoggerE("current profile is not exist.");
    makeErrorCallback(domain_name, kPlatformError);
    return;
  }
  currentProfileName = currentProfileName_c;
  free(currentProfileName_c);
  currentProfileName_c = nullptr;

  if (defaultProfileName != currentProfileName) {
    RequestEventPtr event(new NetworkBearerSelectionRequestEvent(domain_name));

    if (connection_open_profile(m_connection_handle_,
                                m_profile_handle_,
                                connection_profile_opened_callback,
                                event.get()) != CONNECTION_ERROR_NONE) {
      LoggerE("Connection open Failed");
      makeErrorCallback(domain_name, kPlatformError);
    } else {
      m_is_connection_open_ = true;

      std::lock_guard<std::mutex> lock(m_request_mutex_);
      m_request_events_.push_back(event);
    }
  } else {
    registStateChangeListener(domain_name);
  }
}

common::PlatformResult NetworkBearerSelectionManager::releaseRouteToHost(
    const std::string& domain_name, const ReleaseReplyCallback& reply_cb) {
  LoggerD("enter");

  for (const auto& name : m_domain_names_) {
    if (name == domain_name) {
      LoggerD("Same domain name is exist in list.");
      m_domain_names_.remove(domain_name);
      LoggerD("list size : %i", m_domain_names_.size());
      if (m_domain_names_.size() == 0) {
        if (!m_profile_handle_) {
          return LogAndCreateResult(common::ErrorCode::UNKNOWN_ERR, "Already in use");
        }

        if (connection_profile_unset_state_changed_cb(m_profile_handle_) !=
            CONNECTION_ERROR_NONE) {
          LoggerE("unset callback is failed");
          destroyProfileHandle();
          return common::PlatformResult(common::ErrorCode::NO_ERROR);
        }

        if (m_is_connection_open_) {
          ReleaseEventPtr event(new NetworkBearerSelectionReleaseEvent(domain_name, reply_cb));

          if (connection_close_profile(m_connection_handle_,
                                       m_profile_handle_,
                                       connection_closed_callback,
                                       event.get()) != CONNECTION_ERROR_NONE) {
            LoggerE("connection close failed");
            reply_cb(false);
          } else {
            m_is_connection_open_ = false;
            deregistStateChangeListener(domain_name);

            std::lock_guard<std::mutex> lock(m_release_mutex_);
            m_release_events_.push_back(event);
          }
        } else {
          reply_cb(true);
        }
      }
      return common::PlatformResult(common::ErrorCode::NO_ERROR);
    }
  }

  return LogAndCreateResult(common::ErrorCode::UNKNOWN_ERR, "Invalid argument");
}

void NetworkBearerSelectionManager::registStateChangeListener(
    const std::string& domain_name) {
  LoggerD("enter");
  char* interfaceName = nullptr;
  char* hostAddr = nullptr;
  std::unique_ptr<char, void(*)(void*)> host_addr_ptr(nullptr, &std::free);
  struct addrinfo* servinfo = nullptr;

  SCOPE_EXIT {
    if (interfaceName) {
      free(interfaceName);
    }
    freeaddrinfo(servinfo);
  };

  if (connection_profile_get_network_interface_name(
          m_profile_handle_, &interfaceName) != CONNECTION_ERROR_NONE) {
    LoggerE("Fail to get interface name!");
    destroyProfileHandle();
    makeErrorCallback(domain_name, kPlatformError);
  } else {
    LoggerD("Interface name : %s", interfaceName);
  }

  LoggerD("Domain name to be resolved: %s", domain_name.c_str());

  int ret_val = getaddrinfo(domain_name.c_str() , nullptr , nullptr , &servinfo);
  if (0 != ret_val) {
    LoggerE("Error while calling getaddrinfo(): %s", gai_strerror(ret_val));
    destroyProfileHandle();
    makeErrorCallback(domain_name, kPlatformError);
    return;
  } else {
    hostAddr = new char[servinfo->ai_addrlen + 1];
    host_addr_ptr.reset(hostAddr);

    struct in_addr  *addr = nullptr;
    if (AF_INET == servinfo->ai_family) {
      struct sockaddr_in *ipv = (struct sockaddr_in *)servinfo->ai_addr;
      addr = &(ipv->sin_addr);
    } else {
      struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)servinfo->ai_addr;
      addr = (struct in_addr *) &(ipv6->sin6_addr);
    }
    if (nullptr == inet_ntop(servinfo->ai_family, addr, hostAddr, servinfo->ai_addrlen)) {
      LoggerE("Error while calling inet_ntop()");
      destroyProfileHandle();
      makeErrorCallback(domain_name, kPlatformError);
      return;
    }
    LoggerD("hostAddr : %s", hostAddr);
  }

  RequestEventPtr event(new NetworkBearerSelectionRequestEvent(domain_name));

  if (connection_profile_set_state_changed_cb(m_profile_handle_,
                                              connection_state_changed_callback,
                                              event.get()) != CONNECTION_ERROR_NONE) {
    LoggerE("Callback register is failed.");
    destroyProfileHandle();
  } else {
    if (connection_add_route(m_connection_handle_, interfaceName, hostAddr) !=
        CONNECTION_ERROR_NONE) {
      LoggerE("add route is failed.");
      connection_profile_unset_state_changed_cb(m_profile_handle_);
      makeErrorCallback(domain_name, kPlatformError);
    } else {
      LoggerD("add route is successed.");
      m_domain_names_.push_back(domain_name);
      makeSuccessCallback(domain_name);
    }

    std::lock_guard<std::mutex> lock(m_request_mutex_);
    m_request_events_.push_back(event);
  }
}

void NetworkBearerSelectionManager::deregistStateChangeListener(
    const std::string& domain_name) {
  LoggerD("enter");
  if (m_profile_handle_) {
    connection_profile_unset_state_changed_cb(m_profile_handle_);
    connection_profile_destroy(m_profile_handle_);
    m_profile_handle_ = nullptr;
  }
  m_domain_names_.remove(domain_name);
  m_connection_state_ = ConnectionState::Disconnected;
}

void NetworkBearerSelectionManager::makeSuccessCallback(
    const std::string& domain_name) {
  LoggerD("enter");
  std::lock_guard<std::mutex> lock(m_mutex_);
  for (NetworkBearerSelectionListener* listener : m_listeners_)
    listener->onNBSSuccess(domain_name);
}

void NetworkBearerSelectionManager::makeErrorCallback(
    const std::string& domain_name,
    const char* info) {
  LoggerD("enter");
  std::string l_info = info;
  makeErrorCallback(domain_name, l_info);
}

void NetworkBearerSelectionManager::makeErrorCallback(
    const std::string& domain_name,
    const std::string& info) {
  LoggerD("enter");
  std::lock_guard<std::mutex> lock(m_mutex_);
  for (NetworkBearerSelectionListener* listener : m_listeners_)
    listener->onNBSError(domain_name, info);
}

void NetworkBearerSelectionManager::makeDisconnectCallback(
    const std::string& domain_name) {
  LoggerD("enter");
  std::lock_guard<std::mutex> lock(m_mutex_);
  for (NetworkBearerSelectionListener* listener : m_listeners_)
    listener->onNBSDisconnect(domain_name);
}

void NetworkBearerSelectionManager::destroyProfileHandle() {
  LoggerD("enter");
  if (m_profile_handle_) {
    connection_profile_destroy(m_profile_handle_);
    m_profile_handle_ = nullptr;
  }
}

RequestEventPtr NetworkBearerSelectionManager::getRequestEvent(NetworkBearerSelectionRequestEvent* event) {
  LoggerD("enter");
  std::lock_guard<std::mutex> lock(m_request_mutex_);
  for (auto it = m_request_events_.begin(); it != m_request_events_.end(); it++) {
    if (it->get() == event) {
      LoggerD("Found object [%p]", it->get());
      RequestEventPtr ev = *it;
      m_request_events_.erase(it);
      return ev;
    }
  }
  return nullptr;
}

ReleaseEventPtr NetworkBearerSelectionManager::getReleaseEvent(NetworkBearerSelectionReleaseEvent* event) {
  LoggerD("enter");
  std::lock_guard<std::mutex> lock(m_release_mutex_);
  for (auto it = m_release_events_.begin(); it != m_release_events_.end(); it++) {
    if (it->get() == event) {
      LoggerD("Found object [%p]", it->get());
      ReleaseEventPtr ev = *it;
      m_release_events_.erase(it);
      return ev;
    }
  }
  return nullptr;
}

}  // namespace networkbearerselection
}  // namespace extension
