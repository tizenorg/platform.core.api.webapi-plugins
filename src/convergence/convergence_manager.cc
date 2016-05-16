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

#include "convergence/convergence_manager.h"
#include "convergence/convergence_instance.h"
#include "common/logger.h"
#include <d2d_conv_internal.h>


namespace extension {
namespace convergence {

using common::TizenResult;
using common::TizenSuccess;


////////////////////////////////////////////////////////////////////////////////

static picojson::value __channel2json(conv_channel_h channel) {
  if(!channel)
    return picojson::value();

  picojson::value c;

  char *channel_json = NULL;
  conv_channel_internal_export_to_string(channel, &channel_json);
  if(!channel_json)
    return c;

  std::string err;
  picojson::parse(c, channel_json, channel_json + strlen(channel_json), &err);
  if (! err.empty()) {
    LoggerE("Error parsing channel json: %s", err.c_str());
  }

  free(channel_json);

  return c;
}

static conv_channel_h __json2channel(const picojson::value &c) {
  if(c.is<picojson::null>())
    return NULL;

  conv_channel_h channel = NULL;
  conv_channel_create(&channel);

  if(!channel)
    return NULL;

  if(!c.get("options").is<picojson::array>())
    return channel;

  const picojson::array &options = c.get("options").get<picojson::array>();
  for(size_t i = 0; i < options.size(); i ++) {
    picojson::value opt = options[i];
    const std::string key = opt.get("key").to_str();
    const std::string value = opt.get("option").to_str();
    conv_channel_set_string(channel, key.c_str(), value.c_str());
  }

  char *chan_json = NULL;
  conv_channel_internal_export_to_string(channel, &chan_json);
  free(chan_json);

  return channel;
}

static picojson::value __payload2json(conv_payload_h payload) {
  if(!payload)
    return picojson::value();

  picojson::value p;

  char *payload_json = NULL;
  conv_payload_internal_export_to_string(payload, &payload_json);
  if(!payload_json)
    return p;

  std::string err;
  picojson::parse(p, payload_json, payload_json + strlen(payload_json), &err);
  if (! err.empty()) {
    LoggerE("Error parsing payload json: %s", err.c_str());
  }

  free(payload_json);

  return p;
}

static conv_payload_h __json2payload(const picojson::value &p) {
  if(p.is<picojson::null>())
    return NULL;

  conv_payload_h payload = NULL;
  conv_payload_create(&payload);
  if(!payload)
    return NULL;

  if(!p.is<picojson::array>())
    return payload;

  const picojson::array &items = p.get<picojson::array>();
  for(size_t i = 0; i < items.size(); i ++) {
    picojson::value item = items[i];
    const std::string key = item.get("key").to_str();
    const std::string value = item.get("value").to_str(); // TODO: check if it is an object
    conv_payload_set_string(payload, key.c_str(), value.c_str());
  }

  char *pld_json = NULL;
  conv_payload_internal_export_to_string(payload, &pld_json);
  free(pld_json);

  return payload;
}

////////////////////////////////////////////////////////////////////////////////

ConvergenceManager &ConvergenceManager::GetInstance(
  ConvergenceInstance *owner) {
  static ConvergenceManager instance;
  instance.plg = owner;
  return instance;
}

ConvergenceManager::ConvergenceManager() : plg(NULL), conv_(NULL) {
  LoggerD("Enter");
  const int error = conv_create(&conv_);
  if (error != CONV_ERROR_NONE) {
    // Handle error
  }
}

ConvergenceManager::~ConvergenceManager() {
  LoggerD("Enter");

  // Releasing all registered devices
  for(std::unordered_map<std::string, conv_device_h>::iterator it = devices.begin();
    it != devices.end(); ++it)
      conv_device_destroy(it->second);

  const int error = conv_destroy(conv_);
  conv_ = NULL;
  if (error != CONV_ERROR_NONE) {
    // Handle error
  }
}

void ConvergenceManager::ServiceForeachCb(conv_service_h service_handle,
  void* user_data) {
  LoggerD("Enter");
  if(!service_handle || !user_data)
    return;

  picojson::array *services = (picojson::array *)user_data;

  picojson::object service;

  // Extracting service type
  conv_service_e type = CONV_SERVICE_NONE;
  conv_service_get_type(service_handle, &type);
  service["serviceType"] = picojson::value(static_cast<double>(type));

  // Extracting service connection state
  conv_service_connection_state_e state = CONV_SERVICE_CONNECTION_STATE_NONE;
  conv_service_get_connection_state(service_handle, &state);
  service["connectionState"] = picojson::value(static_cast<double>(state));


  // Extracting service properties
  picojson::array properties;
  { // Service ID
    char *id = NULL;
    conv_service_get_property_string(service_handle, CONV_SERVICE_ID, &id);
    properties.push_back(picojson::value(id));
    free(id);
  }

  { // Service version
    char *version = NULL;
    conv_service_get_property_string(service_handle,
      CONV_SERVICE_VERSION, &version);
    properties.push_back(picojson::value(version));
    free(version);
  }
  service["properties"] = picojson::value(properties);

  // Add newly discovered service to the list of available services
  services->push_back(picojson::value(service));
}

void ConvergenceManager::DiscoveryCb(conv_device_h device_handle,
  conv_discovery_result_e result, void* user_data) {
  LoggerD("Enter");

  if(!user_data)
    return;

  ConvergenceManager *owner = (ConvergenceManager *)user_data;

  switch(result) {
  case CONV_DISCOVERY_RESULT_SUCCESS: {
    LoggerI("...found a device");

    picojson::object device;

    { // Extracting device ID
      char *id = NULL;
      conv_device_get_property_string(device_handle, CONV_DEVICE_ID, &id);
      if(id) {
        device["id"] = picojson::value(id);

        LoggerI("......device id: %s", id);

        // Store newly found device for further operations with services
        conv_device_h device_handle_clone = NULL;
        conv_device_clone(device_handle, &device_handle_clone);
        owner->devices[id] = device_handle_clone;

        LoggerI("...registering the device %x", device_handle);

        free(id);
      } else
        device["id"] = picojson::value("null");
    }

    { // Extracting device name
      char *name = NULL;
      conv_device_get_property_string(device_handle, CONV_DEVICE_NAME, &name);
      if(name) {
        device["name"] = picojson::value(name);

        LoggerI("......device name: %s", name);

        free(name);
      } else
        device["name"] = picojson::value("null");
    }

    { // Extracting device type
      char *type = NULL;
      conv_device_get_property_string(device_handle, CONV_DEVICE_TYPE, &type);
      if(type)  {
        LoggerI("......device type: %s", type);
        device["type"] = picojson::value(type);
        free(type);
      } else
        device["type"] = picojson::value("null");
    }

    { // Extracting services
      picojson::array services;
      conv_device_foreach_service(device_handle, ServiceForeachCb, &services);
      device["services"] = picojson::value(services);
    }

    picojson::object param;
    param["discovery_status"] = picojson::value("device_found");
    param["device"] = picojson::value(device);
    owner->plg->ReplyAsync(ConvergenceManagerDiscoveryCallback,
      -1,
      true,
      param);

    break;
  }
  case CONV_DISCOVERY_RESULT_FINISHED: {
    LoggerE("...discovery finished");

    picojson::object param;
    param["discovery_status"] = picojson::value("discovery_finished");
    owner->plg->ReplyAsync(ConvergenceManagerDiscoveryCallback,
      -1,
      true,
      param);
    break;
  }
  case CONV_DISCOVERY_RESULT_ERROR: {
    LoggerE("Discovery Error");
    picojson::object param;
    param["discovery_status"] = picojson::value("discovery_error");
    param["discovery_error"] = picojson::value(static_cast<double>(result));
    owner->plg->ReplyAsync(ConvergenceManagerDiscoveryCallback,
      -1,
      false,
      param);
    break;
  }
  default: {
    LoggerE("Unknown discovery result");
    break;
  }
  }
}

TizenResult ConvergenceManager::StartDiscovery(long timeout) {
  LoggerD("Enter");
  const int error = conv_discovery_start(conv_, (const int)timeout, DiscoveryCb,
    this);
  if (error != CONV_ERROR_NONE) {
    // TODO: Handle error
  }
  return TizenSuccess();
}

TizenResult ConvergenceManager::StopDiscovery() {
  LoggerD("Enter");
  const int error = conv_discovery_stop(conv_);
  if (error != CONV_ERROR_NONE) {
    // TODO: Handle error
  }
  return TizenSuccess();
}

conv_device_h ConvergenceManager::FindDevice(const char *deviceId) {
  // TODO: re-design to request device list every time
  std::string strDeviceId(deviceId); // TODO: add critical section
  if(devices.count(strDeviceId) <= 0)
    return NULL;
  return devices[strDeviceId];
}

class ServiceFindVisitor {
 public:
  conv_service_h service;
  const int serviceType;
  ServiceFindVisitor(const int type) : service(NULL), serviceType(type) {}
  void visit(void *data) {
    conv_service_h s = (conv_service_h)data;
    conv_service_e type = CONV_SERVICE_NONE;
    conv_service_get_type(s, &type);
    if(serviceType == int(type)) {
      conv_service_h service_clone = NULL;
      conv_service_clone(s, &service_clone);
      service = service_clone;
    }
  }
  bool done() const { return (service != NULL); }
};


void ConvergenceManager::FindServiceCb(conv_service_h service_handle,
  void *user_data) {
  ServiceFindVisitor *visitor = (ServiceFindVisitor *)user_data;
  if(visitor && !visitor->done())
    visitor->visit(service_handle);
}

conv_service_h ConvergenceManager::FindService(const conv_device_h device,
  const int serviceType) {
  ServiceFindVisitor visitor(serviceType);
  conv_device_foreach_service(device, FindServiceCb, &visitor);
  return visitor.service;
}

void ConvergenceManager::ServiceConnectedCb(conv_service_h service_handle,
  conv_error_e error, conv_payload_h result, void* user_data) {
  LoggerD("Enter");

  if(!user_data)
    return;

  std::pair<ConvergenceManager *, int> *cbParam =
    (std::pair<ConvergenceManager *, int> *)user_data;
  ConvergenceManager *owner = cbParam->first;
  const int curListenerId = cbParam->second;
  delete cbParam;

  picojson::object param;
  param["payload"] = __payload2json(result);

  if(error == CONV_ERROR_NONE) {
    param["connect_status"] = picojson::value("service_connected");
    owner->plg->ReplyAsync(ServiceConnectCallback,
      curListenerId,
      true,
      param);
  } else {
    // Error occured during connection
    param["connect_status"] = picojson::value("service_not_connected");
    owner->plg->ReplyAsync(ServiceConnectCallback,
      curListenerId,
      false,
      param);
  }
}

void ConvergenceManager::ServiceListenerCb(conv_service_h service_handle,
  conv_channel_h channel_handle,
  conv_error_e error, conv_payload_h result, void* user_data) {
  LoggerD("Enter");

  if(!user_data)
    return;

  std::pair<ConvergenceManager *, int> *cbParam =
    (std::pair<ConvergenceManager *, int> *)user_data;
  ConvergenceManager *owner = cbParam->first;
  const int curListenerId = cbParam->second;

  picojson::object param;
  param["channel"] = __channel2json(channel_handle);
  param["payload"] = __payload2json(result);

  if(error == CONV_ERROR_NONE) {
    param["listener_status"] = picojson::value("listener_ok");
    owner->plg->ReplyAsync(ServiceListenerCallback,
      curListenerId,
      true,
      param);
  } else {
    // Error occured during connection
    param["listener_status"] = picojson::value("listener_error");
    param["listener_error"] = picojson::value(static_cast<double>(error));
    owner->plg->ReplyAsync(ServiceListenerCallback,
      curListenerId,
      false,
      param);
  }
}

TizenResult ConvergenceManager::Connect(const char *deviceId,
  const int serviceType, const int curListenerId) {
  LoggerD("Enter");

  conv_device_h device = FindDevice(deviceId);
  if(!device)
    return LogAndCreateTizenError(NotFoundError,
      "Device with specified id does not exist");

  conv_service_h service = FindService(device, serviceType);
  if(!service)
    return LogAndCreateTizenError(NotFoundError,
      "Service with specified type does not exist");


  // TODO: make a garbage collection in case of callback is not invoked
  std::pair<ConvergenceManager *, int> *cbParam =
    new std::pair<ConvergenceManager *, int>(this, curListenerId);
  const int error = conv_service_connect(service, ServiceConnectedCb, cbParam);
  if (error != CONV_ERROR_NONE) {
    // TODO: Handle error
  }

  return TizenSuccess();
}

TizenResult ConvergenceManager::Disconnect(const char *deviceId,
  const int serviceType) {
  LoggerD("Enter");

  conv_device_h device = FindDevice(deviceId);
  if(!device)
    return LogAndCreateTizenError(NotFoundError,
      "Device with specified id does not exist");

  conv_service_h service = FindService(device, serviceType);
  if(!service)
    return LogAndCreateTizenError(NotFoundError,
      "Service with specified type does not exist");

  const int error = conv_service_disconnect(service);
  if (error != CONV_ERROR_NONE) {
    // TODO: Handle error
  }
  return TizenSuccess();
}

common::TizenResult ConvergenceManager::Start(const char *deviceId,
  const int serviceType, const picojson::value &channel,
  const picojson::value &payload) {
  LoggerD("Enter");
  return ExecuteServiceCommand(SERVICE_START,
    deviceId,
    serviceType,
    channel,
    payload);
}

common::TizenResult ConvergenceManager::Read(const char *deviceId,
  const int serviceType, const picojson::value &channel,
  const picojson::value &payload) {
  LoggerD("Enter");
  return ExecuteServiceCommand(SERVICE_START,
    deviceId,
    serviceType,
    channel,
    payload);
}

common::TizenResult ConvergenceManager::Send(const char *deviceId,
  const int serviceType, const picojson::value &channel,
  const picojson::value &payload) {
  LoggerD("Enter");
  return ExecuteServiceCommand(SERVICE_SEND,
    deviceId,
    serviceType,
    channel,
    payload);
}

common::TizenResult ConvergenceManager::Stop(const char *deviceId,
  const int serviceType, const picojson::value &channel,
  const picojson::value &payload) {
  LoggerD("Enter");
  return ExecuteServiceCommand(SERVICE_STOP,
    deviceId,
    serviceType,
    channel,
    payload);
}

common::TizenResult ConvergenceManager::ExecuteServiceCommand(
  const ServiceCommands command,
  const char *deviceId,
  const int serviceType,
  const picojson::value &channel,
  const picojson::value &payload) {
  LoggerD("Enter");

  conv_device_h device = FindDevice(deviceId);
  if(!device)
    return LogAndCreateTizenError(NotFoundError,
      "Device with specified id does not exist");

  conv_service_h service = FindService(device, serviceType);
  if(!service)
    return LogAndCreateTizenError(NotFoundError,
     "Service with specified type does not exist");

  // TODO: if channel == nullObj, use default channel
  conv_channel_h c = __json2channel(channel);
  conv_payload_h p = __json2payload(payload);

  int error = CONV_ERROR_NONE;
  switch(command) {
  case SERVICE_START:
    error = conv_service_start(service, c, p);
    break;
  case SERVICE_READ:
    error = conv_service_read(service, c, p);
    break;
  case SERVICE_SEND:
    error = conv_service_publish(service, c, p);
    break;
  case SERVICE_STOP:
    error = conv_service_stop(service, c, p);
    break;
  default:
    LoggerE("Unknown service command [%d]", command);
    break;
  }
  if (error != CONV_ERROR_NONE) {
    // TODO: Handle error
  }
  conv_channel_destroy(c);
  conv_payload_destroy(p);
  return TizenSuccess();
}

TizenResult ConvergenceManager::SetListener(const char *deviceId,
  const int serviceType, const int curListenerId) {
  LoggerD("Enter");
  //LoggerD("deviceId: %s, serviceType: %d, curListenerId: %d", deviceId, serviceType, curListenerId);

  conv_device_h device = FindDevice(deviceId);
  if(!device)
    return LogAndCreateTizenError(NotFoundError,
      "Device with specified id does not exist");

  conv_service_h service = FindService(device, serviceType);
  if(!service)
    return LogAndCreateTizenError(NotFoundError,
      "Service with specified type does not exist");

  // TODO: make a garbage collection in case of callback is not invoked
  std::pair<ConvergenceManager *, int> *cbParam =
    new std::pair<ConvergenceManager *, int>(this, curListenerId);
  const int error = conv_service_set_listener_cb(service, ServiceListenerCb, cbParam);
  if (error != CONV_ERROR_NONE) {
    // TODO: Handle error
  }

  conv_service_destroy(service);

  return TizenSuccess();
}

TizenResult ConvergenceManager::UnsetListener(const char *deviceId,
  const int serviceType) {
  LoggerD("Enter");

  conv_device_h device = FindDevice(deviceId);
  if(!device)
    return LogAndCreateTizenError(NotFoundError,
      "Device with specified id does not exist");

  conv_service_h service = FindService(device, serviceType);
  if(!service)
    return LogAndCreateTizenError(NotFoundError,
      "Service with specified type does not exist");

  const int error = conv_service_unset_listener_cb(service);
  if (error != CONV_ERROR_NONE) {
    // TODO: Handle error
  }

  return TizenSuccess();
}


} // namespace convergence
}  // namespace extension
