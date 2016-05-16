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
#include <glib.h>


namespace extension {
namespace convergence {

using common::TizenResult;
using common::TizenSuccess;


////////////////////////////////////////////////////////////////////////////////

static picojson::value __channel2json(conv_channel_h channel) {
  picojson::object chanObj;
  chanObj["id"] = picojson::value("");
  picojson::array chanOptions;

  do {
    if(!channel)
      break;

    picojson::value parsedChan;
    {
      char *channel_json = NULL;
      conv_channel_internal_export_to_string(channel, &channel_json);
      if(!channel_json)
        break;

      std::string err;
      picojson::parse(parsedChan, channel_json, channel_json + strlen(channel_json), &err);
      free(channel_json);
      if (!err.empty()) {
        LoggerE("Error parsing channel json: %s", err.c_str());
        break;
      }
    }

    if(!parsedChan.is<picojson::object>()) {
      LoggerE("ERROR: Channel is not an object");
      break;
    }

    const picojson::object &options = parsedChan.get<picojson::object>();
    for(auto it = std::begin(options); it != std::end(options); ++it) {
      const std::string optName = it->first;
      const picojson::value optValue = it->second;

      picojson::object opt;
      opt["key"] = picojson::value(optName);
      opt["option"] = optValue;
      chanOptions.push_back(picojson::value(opt));

      if(optName == "channel_id")
        chanObj["id"] = picojson::value(optValue);
    }
  } while(false);

  chanObj["options"] = picojson::value(chanOptions);
  return picojson::value(chanObj);
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

  return channel;
}

static picojson::value __payload2json(conv_payload_h payload) {
  picojson::array payloads;

  do {
    if(!payload)
      break;

    picojson::value parsedPl;
    {
      char *payload_json = NULL;
      conv_payload_internal_export_to_string(payload, &payload_json);
      if(!payload_json)
        break;

      std::string err;
      picojson::parse(parsedPl, payload_json, payload_json + strlen(payload_json), &err);
      free(payload_json);
      if (!err.empty()) {
        LoggerE("Error parsing channel json: %s", err.c_str());
        break;
      }
    }

    if(!parsedPl.is<picojson::object>()) {
      LoggerE("ERROR: Channel is not an object");
      break;
    }

    const picojson::object &payloadItems = parsedPl.get<picojson::object>();
    for(auto it = std::begin(payloadItems); it != std::end(payloadItems); ++it) {
      const std::string itemName = it->first;
      const picojson::value itemValue = it->second;

      picojson::object p;
      p["key"] = picojson::value(itemName);

      // !!!! Recognizing type of the payload
      if(itemValue.is<picojson::array>()) {
        p["type"] = picojson::value("RAW_BYTES");
        p["value"] = itemValue;
      } else if(itemValue.is<picojson::object>()) {
        // Check if it is AppControl rather than arbitrary object
        // Attempting to convert it to app control. If failed, than it is an arbitrary object

        app_control_h app_control = NULL;
        int error = conv_payload_get_app_control(payload, itemName.c_str(), &app_control);
        if((error == CONV_ERROR_NONE) && app_control) {
          p["type"] = picojson::value("APP_CONTROL");

          char *appId = NULL;
          if(app_control_get_app_id(app_control, &appId) == APP_CONTROL_ERROR_NONE) {
            p["appId"] = picojson::value(appId);
            free(appId);
          }

          //TODO extract app control fields and set it to json-style app control
          picojson::object ac;
          char *operation = NULL;
          if(app_control_get_operation(app_control, &operation) == APP_CONTROL_ERROR_NONE) {
            ac["operation"] = picojson::value(operation);
            free(operation);
          }

          char *uri = NULL;
          if(app_control_get_uri(app_control, &uri) == APP_CONTROL_ERROR_NONE) {
            ac["uri"] = picojson::value(uri);
            free(uri);
          }

          char *mime = NULL;
          if(app_control_get_mime(app_control, &mime) == APP_CONTROL_ERROR_NONE) {
            ac["mime"] = picojson::value(mime);
            free(mime);
          }

          char *category = NULL;
          if(app_control_get_category(app_control, &category) == APP_CONTROL_ERROR_NONE) {
            ac["category"] = picojson::value(category);
            free(category);
          }

          /* TODO
          attribute ApplicationControlData[] data;
          app_control_get_extra_data (app_control_h app_control, const char *key, char **value)
          app_control_get_extra_data_array (app_control_h app_control, const char *key, char ***value, int *length)
          app_control_is_extra_data_array (app_control_h app_control, const char *key, bool *array)
          app_control_foreach_extra_data (app_control_h app_control, app_control_extra_data_cb callback, void *user_data)
          */

          ac["launch_mode"] = picojson::value("SINGLE");
          p["value"] = picojson::value(ac);
          app_control_destroy(app_control);
        } else {
          p["type"] = picojson::value("STRING");
          p["value"] = picojson::value(itemValue.serialize());
          //p["value"] = picojson::value(itemValue.to_str());
        }
      } else {
        p["type"] = picojson::value("STRING");
        p["value"] = itemValue;
      }

      payloads.push_back(picojson::value(p));
    }
  } while(false);

  return picojson::value(payloads);
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
    const std::string type = item.get("type").to_str();
    if(type == "STRING") {
      const std::string key = item.get("key").to_str();
      const std::string value = item.get("value").to_str(); // TODO: check if it is an object
      conv_payload_set_string(payload, key.c_str(), value.c_str());
    } else if(type == "RAW_BYTES") {
      // TODO
      LoggerW("IMPLEMENT BYTE PAYLOAD!!!");
    } else if(type == "APP_CONTROL") {
      app_control_h app_control = NULL;
      app_control_create(&app_control);

      const std::string appId = item.get("appId").to_str();
      if(!appId.empty() && appId.c_str() && (appId != "null")) { // TODO check here and in other cases if JSON sub-object is empty
        LoggerW("...app_control_set_app_id(app_control, %s)", appId.c_str());
        app_control_set_app_id(app_control, appId.c_str());
      }

      const std::string operation = item.get("value").get("operation").to_str();
      if(!operation.empty() && operation.c_str() && (operation != "null")) {
        LoggerW("...app_control_set_operation(app_control, %s)", operation.c_str());
        app_control_set_operation(app_control, operation.c_str());
      }

      const std::string uri = item.get("value").get("uri").to_str();
      if(!uri.empty() && uri.c_str() && (uri != "null")) {
        LoggerW("...app_control_set_uri(app_control, %s)", uri.c_str());
        app_control_set_uri(app_control, uri.c_str());
      }

      const std::string mime = item.get("value").get("mime").to_str();
      if(!mime.empty() && mime.c_str() && (mime != "null")) {
        LoggerW("...app_control_set_mime(app_control, %s)", mime.c_str());
        app_control_set_mime(app_control, mime.c_str());
      }

      const std::string category = item.get("value").get("category").to_str();
      if(!category.empty() && category.c_str() && (category != "null")) {
        LoggerW("...app_control_set_category(app_control, %s)", category.c_str());
        app_control_set_category(app_control, category.c_str());
      }

      // TODO implement also conversion of key-value data

      const std::string key = item.get("key").to_str();
      if(!key.empty() && key.c_str() && (key != "null")) {
        LoggerW("+++conv_payload_set_app_control(payload, %s, app_control)", key.c_str());
        conv_payload_set_app_control(payload, key.c_str(), app_control);
      }

      // TODO release appcontrol handle and payload handle

    } else {
      LoggerE("ERROR! Unknown type of payload [%s]", type.c_str());
    }
  }

  char *pld_json = NULL;
  conv_payload_internal_export_to_string(payload, &pld_json);
  LoggerI("...Converted payload: %s", pld_json);
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

  for(std::unordered_map<int, conv_service_h>::iterator it = __localServices.begin();
    it != __localServices.end(); ++it) {
    conv_service_h service = it->second;
    conv_service_destroy(service);
  }
  __localServices.clear();
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
    picojson::object propId;
    propId["key"] = picojson::value(CONV_SERVICE_ID);
    propId["value"] = picojson::value(id);
    //properties.push_back(picojson::value(id));
    properties.push_back(picojson::value(propId));
    free(id);
  }

  { // Service version
    char *version = NULL;
    conv_service_get_property_string(service_handle,
      CONV_SERVICE_VERSION, &version);
    picojson::object propVersion;
    //properties.push_back(picojson::value(version));
    propVersion["key"] = picojson::value(CONV_SERVICE_VERSION);
    propVersion["value"] = picojson::value(version);
    properties.push_back(picojson::value(propVersion));
    free(version);
  }

  // TODO: add other properties, which are not named with predefined keys

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

conv_device_h ConvergenceManager::getRemoteDevice(const char *deviceId) const {
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

conv_service_h ConvergenceManager::getRemoteService(const conv_device_h device,
  const int serviceType) const {
  ServiceFindVisitor visitor(serviceType);
  conv_device_foreach_service(device, FindServiceCb, &visitor);
  return visitor.service;
}

conv_service_h ConvergenceManager::getService(const char *deviceId,
  const int serviceType) const {
  LoggerD("Enter");
  if(isLocalDevice(deviceId)) {
    if(__localServices.count(serviceType) <= 0) {
      LoggerE("LOCAL Service with specified type does not exist");
      return NULL;
    }

    return __localServices[serviceType];
  } else {
    conv_device_h device = getRemoteDevice(deviceId);
    if(!device) {
      LoggerE("Device with specified id does not exist");
      return NULL;
    }

    conv_service_h service = getRemoteService(device, serviceType);
    if(!service) {
      LoggerE("Service with specified type does not exist");
      return NULL;
    }

    return service;
  }
}

/* TODO: Implement smart handle class for service
 * class ConvServiceHandle:
 *  private:
 *   conv_service_h __service_handle
 *  public:
 *   constructor(ConvergenceManager *owner, deviceId, serviceType)
 *   destructor - if the service is not local, destroy it
 *   operator conv_service_h() - get the handle from d2d-conv-manager of from the table of local services
 */


void ConvergenceManager::destroyService(conv_service_h service) {
  if(!service)
    return;

  for(auto it = std::begin(__localServices); it != std::end(__localServices); ++it)
    if(it->second == service)
      return; // This is a local service, so not needed to destroy it here.
        // (it will be destroyed in the destructor of the manager)

  conv_service_destroy(service);
}

bool ConvergenceManager::isLocalDevice(const char *deviceId) const {
  LoggerD("Enter");
  return (g_strcmp0(deviceId, "localhost") == 0);
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

TizenResult ConvergenceManager::CreateLocalService(const int serviceType,
  const picojson::value &service_data) {
  LoggerD("Enter");
  if(__localServices.count(serviceType) > 0)
    return LogAndCreateTizenError(NotFoundError,
      "LOCAL service is already created");

  int error = CONV_ERROR_NONE;
  conv_service_h service = NULL;
  do {
    error = conv_service_create(&service);
    if(error != CONV_ERROR_NONE)
      break;

    error = conv_service_set_type(service, CONV_SERVICE_APP_TO_APP_COMMUNICATION);
    if(error != CONV_ERROR_NONE)
      break;

    const picojson::array &properties =
      service_data.get("properties").get<picojson::array>();
    for(size_t i = 0; i < properties.size(); i ++) {
      picojson::value prop = properties[i];
      const std::string key = prop.get("key").to_str();
      const std::string value = prop.get("value").to_str();
      error =
        conv_service_set_property_string(service, key.c_str(), value.c_str());
      LoggerI("Set local service property: [%s: %s]", key.c_str(), value.c_str());
    }

    __localServices[serviceType] = service;

    return TizenSuccess();
  } while(false);

  conv_service_destroy(service);

  return LogAndCreateTizenError(NotFoundError,
    "LOCAL service creation failed");
}

TizenResult ConvergenceManager::Connect(const char *deviceId,
  const int serviceType, const int curListenerId) {
  LoggerD("Enter");

  conv_service_h service = getService(deviceId, serviceType);
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

  destroyService(service);

  return TizenSuccess();
}

TizenResult ConvergenceManager::Disconnect(const char *deviceId,
  const int serviceType) {
  LoggerD("Enter");

  conv_service_h service = getService(deviceId, serviceType);
  if(!service)
    return LogAndCreateTizenError(NotFoundError,
      "Service with specified type does not exist");

  const int error = conv_service_disconnect(service);
  if (error != CONV_ERROR_NONE) {
    // TODO: Handle error
  }

  destroyService(service);

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

  conv_service_h service = getService(deviceId, serviceType);
  if(!service) {
    LoggerE("AAAAAA!!!");
    return LogAndCreateTizenError(NotFoundError,
     "Service with specified type does not exist");
  }

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
  destroyService(service);
  return TizenSuccess();
}

TizenResult ConvergenceManager::SetListener(const char *deviceId,
  const int serviceType, const int curListenerId) {
  LoggerD("Enter");
  //LoggerD("deviceId: %s, serviceType: %d, curListenerId: %d", deviceId, serviceType, curListenerId);

  conv_service_h service = getService(deviceId, serviceType);
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

  destroyService(service);

  return TizenSuccess();
}

TizenResult ConvergenceManager::UnsetListener(const char *deviceId,
  const int serviceType) {
  LoggerD("Enter");

  conv_service_h service = getService(deviceId, serviceType);
  if(!service)
    return LogAndCreateTizenError(NotFoundError,
      "Service with specified type does not exist");

  const int error = conv_service_unset_listener_cb(service);
  if (error != CONV_ERROR_NONE) {
    // TODO: Handle error
  }

  destroyService(service);

  return TizenSuccess();
}

} // namespace convergence
}  // namespace extension
