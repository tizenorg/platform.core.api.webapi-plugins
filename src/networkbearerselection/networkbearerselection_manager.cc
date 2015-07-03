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
  std::lock_guard<std::mutex> lock(m_mutex);
  m_listeners.push_back(listener);
}

void NetworkBearerSelectionManager::RemoveListener(
    NetworkBearerSelectionListener* listener) {
  LoggerD("Enter");
  std::lock_guard<std::mutex> lock(m_mutex);
  m_listeners.remove(listener);
}

NetworkBearerSelectionManager* NetworkBearerSelectionManager::GetInstance() {
  LoggerD("Enter");
  static NetworkBearerSelectionManager instance;
  return &instance;
}

NetworkBearerSelectionManager::NetworkBearerSelectionManager()
    : m_connectionHandle(nullptr),
      m_profileHandle(nullptr),
      m_connectionState(ConnectionState::Unknown),
      m_isConnectionOpen(false) {
  LoggerD("Enter");
  int ret = connection_create(&m_connectionHandle);

  if (CONNECTION_ERROR_NONE == ret) {
    LoggerD("Client registration success");
  } else {
    LoggerE("Client registration failed");
    m_connectionHandle = nullptr;
  }
}

NetworkBearerSelectionManager::~NetworkBearerSelectionManager() {
  LoggerD("Enter");
  if (m_connectionHandle != nullptr) {
    LoggerD("Client deregistration success");
    if (m_profileHandle) {
      connection_profile_destroy(m_profileHandle);
      m_profileHandle = nullptr;
    }
    connection_destroy(m_connectionHandle);
  } else {
    LoggerE("Client deregistration failed");
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
    NetworkBearerSelectionRequestEvent* event =
        static_cast<NetworkBearerSelectionRequestEvent*>(user_data);

    if (state == CONNECTION_PROFILE_STATE_DISCONNECTED) {
      std::string domain_name = event->domain_name;
      NetworkBearerSelectionManager::GetInstance()->deregistStateChangeListener(domain_name);
      delete event;

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

  NetworkBearerSelectionRequestEvent* event =
      static_cast<NetworkBearerSelectionRequestEvent*>(user_data);
  std::string domain_name = event->domain_name;
  delete event;

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

  NetworkBearerSelectionReleaseEvent* event =
      static_cast<NetworkBearerSelectionReleaseEvent*>(user_data);
  std::string domain_name = event->domain_name;
  ReleaseReplyCallback callback = event->callback;
  delete event;

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

void NetworkBearerSelectionManager::connection_closed_callback2(
    connection_error_e result,
    void* user_data) {
  LoggerD("enter");
  if (result == CONNECTION_ERROR_NONE) {
    LoggerD("Connection close Succeeded");
  }
}

void NetworkBearerSelectionManager::requestRouteToHost(
    const std::string& domain_name) {
  LoggerD("NetworkBearerSelectionManager::requestRouteToHost");
  connection_profile_h profileHandle;

  if (m_connectionState == ConnectionState::Connected) {
    LoggerD("connection is already opened.");
    for (std::list<std::string>::iterator it = m_domainNames.begin();
         it != m_domainNames.end();
         it++) {
      if (*it == domain_name) {
        LoggerD("Same domain name is exist in list.");
        makeSuccessCallback(domain_name);
        return;
      }
    }
  }

  if (m_profileHandle) {
    connection_profile_destroy(m_profileHandle);
    m_profileHandle = nullptr;
  }

  if (connection_get_default_cellular_service_profile(
          m_connectionHandle,
          CONNECTION_CELLULAR_SERVICE_TYPE_INTERNET,
          &m_profileHandle) != CONNECTION_ERROR_NONE) {
    LoggerE("Fail to get profile handle");
    makeErrorCallback(domain_name, kPlatformError);
    return;
  }

  char* defaultProfileName_c = nullptr;
  std::string defaultProfileName;

  connection_profile_get_name(m_profileHandle, &defaultProfileName_c);
  if (defaultProfileName_c == nullptr) {
    LoggerE("default profile is not exist.");
    makeErrorCallback(domain_name, kPlatformError);
    return;
  }
  defaultProfileName = defaultProfileName_c;
  free(defaultProfileName_c);
  defaultProfileName_c = nullptr;

  if (connection_get_current_profile(m_connectionHandle, &profileHandle) !=
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
    NetworkBearerSelectionRequestEvent* event =
        new NetworkBearerSelectionRequestEvent(domain_name);
    if (connection_open_profile(m_connectionHandle,
                                m_profileHandle,
                                connection_profile_opened_callback,
                                event) != CONNECTION_ERROR_NONE) {
      LoggerE("Connection open Failed");
      delete event;
      makeErrorCallback(domain_name, kPlatformError);
    } else {
      m_isConnectionOpen = true;
    }
  } else {
    registStateChangeListener(domain_name);
  }
}

bool NetworkBearerSelectionManager::releaseRouteToHost(
    const std::string& domain_name,
    const ReleaseReplyCallback& reply_cb) {
  LoggerD("enter");
  for (std::list<std::string>::iterator it = m_domainNames.begin();
       it != m_domainNames.end();
       it++) {
    if (*it == domain_name) {
      LoggerD("Same domain name is exist in list.");
      m_domainNames.remove(domain_name);
      LoggerD("list size : %i", m_domainNames.size());
      if (m_domainNames.size() == 0) {
        if (!m_profileHandle) {
          // TODO: ALREADY_IN_USE EXCEPTION
          return false;
        }

        if (connection_profile_unset_state_changed_cb(m_profileHandle) !=
            CONNECTION_ERROR_NONE) {
          LoggerE("unset callback is failed");
          if (m_profileHandle) {
            connection_profile_destroy(m_profileHandle);
            m_profileHandle = NULL;
          }
          return true;
        }

        if (m_isConnectionOpen) {
          NetworkBearerSelectionReleaseEvent* event =
              new NetworkBearerSelectionReleaseEvent(domain_name, reply_cb);
          if (connection_close_profile(m_connectionHandle,
                                       m_profileHandle,
                                       connection_closed_callback,
                                       event) != CONNECTION_ERROR_NONE) {
            LoggerE("connection close failed");
            delete event;
            reply_cb(false);
          } else {
            m_isConnectionOpen = false;
            deregistStateChangeListener(domain_name);
          }
        } else {
          reply_cb(true);
        }
      }
      return true;
    }
  }

  // TODO: INVALID_ARGUMENT_EXCEPTION
  return false;
}

void NetworkBearerSelectionManager::registStateChangeListener(
    const std::string& domain_name) {
  LoggerD("enter");
  char* interfaceName = nullptr;
  char* hostAddr = nullptr;
  std::unique_ptr<char, void(*)(void*)> host_addr_ptr(nullptr, &std::free);
  struct addrinfo* servinfo = nullptr;

  if (connection_profile_get_network_interface_name(
          m_profileHandle, &interfaceName) != CONNECTION_ERROR_NONE) {
    LoggerE("Fail to get interface name!");
    if (m_profileHandle) {
      connection_profile_destroy(m_profileHandle);
      m_profileHandle = nullptr;
    }
    makeErrorCallback(domain_name, kPlatformError);
  } else {
    LoggerD("Interface name : %s", interfaceName);
  }

  LoggerD("Domain name to be resolved: %s", domain_name.c_str());

  int ret_val = getaddrinfo(domain_name.c_str() , nullptr , nullptr , &servinfo);
  if (0 != ret_val) {
    LoggerE("Error while calling getaddrinfo(): %s", gai_strerror(ret_val));
    if (m_profileHandle) {
      connection_profile_destroy(m_profileHandle);
      m_profileHandle = nullptr;
    }
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
      if (m_profileHandle) {
        connection_profile_destroy(m_profileHandle);
        m_profileHandle = nullptr;
      }
      makeErrorCallback(domain_name, kPlatformError);
      freeaddrinfo(servinfo);
      return;
    }
    LoggerD("hostAddr : %s", hostAddr);

    freeaddrinfo(servinfo);
  }

  NetworkBearerSelectionRequestEvent* event =
      new NetworkBearerSelectionRequestEvent(domain_name);
  if (connection_profile_set_state_changed_cb(m_profileHandle,
                                              connection_state_changed_callback,
                                              event) != CONNECTION_ERROR_NONE) {
    LoggerE("Callback register is failed.");
    if (m_profileHandle) {
      connection_profile_destroy(m_profileHandle);
      m_profileHandle = nullptr;
    }
    delete event;
  } else {
    if (connection_add_route(m_connectionHandle, interfaceName, hostAddr) !=
        CONNECTION_ERROR_NONE) {
      LoggerE("add route is failed.");
      connection_profile_unset_state_changed_cb(m_profileHandle);
      makeErrorCallback(domain_name, kPlatformError);
    } else {
      LoggerD("add route is successed.");
      m_domainNames.push_back(domain_name);
      makeSuccessCallback(domain_name);
    }
  }
}

void NetworkBearerSelectionManager::deregistStateChangeListener(
    const std::string& domain_name) {
  LoggerD("enter");
  if (m_profileHandle) {
    connection_profile_unset_state_changed_cb(m_profileHandle);
    connection_profile_destroy(m_profileHandle);
    m_profileHandle = NULL;
  }
  m_domainNames.remove(domain_name);
  m_connectionState = ConnectionState::Disconnected;
}

void NetworkBearerSelectionManager::makeSuccessCallback(
    const std::string& domain_name) {
  LoggerD("enter");
  std::lock_guard<std::mutex> lock(m_mutex);
  for (NetworkBearerSelectionListener* listener : m_listeners)
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
  std::lock_guard<std::mutex> lock(m_mutex);
  for (NetworkBearerSelectionListener* listener : m_listeners)
    listener->onNBSError(domain_name, info);
}

void NetworkBearerSelectionManager::makeDisconnectCallback(
    const std::string& domain_name) {
  LoggerD("enter");
  std::lock_guard<std::mutex> lock(m_mutex);
  for (NetworkBearerSelectionListener* listener : m_listeners)
    listener->onNBSDisconnect(domain_name);
}

}  // namespace networkbearerselection
}  // namespace extension
