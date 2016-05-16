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

#include <glib.h>
#include <d2d_conv_internal.h>

#include "convergence/convergence_instance.h"
#include "common/logger.h"

namespace extension {
namespace convergence {

using common::TizenResult;
using common::TizenSuccess;

namespace {
// Commonly used key words
static const std::string kId = "id";
static const std::string kName = "name";
static const std::string kKey = "key";
static const std::string kValue = "value";

// Channel data keys
static const std::string kChannel = "channel";
static const std::string kChannelDataChannelId = "channel_id";
static const std::string kChannelDataChannelOption = "option";
static const std::string kChannelDataChannelOptions = "options";

// Payload data keys and some values
static const std::string kPayload = "payload";
static const std::string kPayloadDataType = "type"; // String, bytes or app control
static const std::string kTypeString = "STRING";
static const std::string kTypeRawBytes = "RAW_BYTES";
static const std::string kTypeAppControl = "APP_CONTROL";
static const std::string kPayloadDataAppControlAppId = "appI";
static const std::string kPayloadDataAppControlOperation = "operation";
static const std::string kPayloadDataAppControlUri = "uri";
static const std::string kPayloadDataAppControlMime = "mime";
static const std::string kPayloadDataAppControlLaunchMode = "launch_mode";
static const std::string kPayloadDataAppControlCategory = "category";

// Service keys
static const std::string kServiceType = "serviceType";
static const std::string kServiceConnectionState = "connectionState";
static const std::string kServiceProperties = "properties";

// Device keys
static const std::string kDevice = "device";
static const std::string kDeviceType = "type";
static const std::string kDeviceServices = "services";

// Discovery keys
static const std::string kDiscoveryStatus = "discovery_status";
static const std::string kDiscoveryStatusDeviceFound = "device_found";
static const std::string kDiscoveryStatusFinished = "discovery_finished";
static const std::string kDiscoveryStatusError = "discovery_error";
static const std::string kDiscoveryError = "discovery_error";

// Service connection status keys
static const std::string kServiceConnectionStatus = "connect_status";
static const std::string kServiceConnectionStatusConnected = "service_connected";
static const std::string kServiceConnectionStatusNotConnected = "service_not_connected";

// Service listener status keys
static const std::string kServiceListenerStatus = "listener_status";
static const std::string kServiceListenerStatusOk = "listener_ok";
static const std::string kServiceListenerStatusError = "listener_error";
static const std::string kServiceListenerError = "listener_error";
} // namespace


////////////////////////////////////////////////////////////////////////////////

static void trace_conv_error(const int error, int line_number, const char *extra_text) {
  std::string error_text;
  switch (error) {
  case CONV_ERROR_NONE:
    error_text = "CONV_ERROR_NONE";
    break;
  case CONV_ERROR_INVALID_PARAMETER:
    error_text = "CONV_ERROR_INVALID_PARAMETER";
    break;
  case CONV_ERROR_INVALID_OPERATION:
    error_text = "CONV_ERROR_INVALID_OPERATION";
    break;
  case CONV_ERROR_OUT_OF_MEMORY:
    error_text = "CONV_ERROR_OUT_OF_MEMORY";
    break;
  case CONV_ERROR_PERMISSION_DENIED:
    error_text = "CONV_ERROR_PERMISSION_DENIED";
    break;
  case CONV_ERROR_NOT_SUPPORTED:
    error_text = "CONV_ERROR_NOT_SUPPORTED";
    break;
  case CONV_ERROR_NO_DATA:
    error_text = "CONV_ERROR_NO_DATA";
    break;
  default:
    error_text = "UNSUPPORTED D2D ERROR CODE";
    break;
  }

  if (extra_text) {
    LoggerE("ERROR! D2D API error [%s], line %d: %s", error_text.c_str(), line_number, extra_text);
  } else {
    LoggerE("ERROR! D2D API error [%s], line %d", error_text.c_str(), line_number);
  }
}

static picojson::value channel_to_json(conv_channel_h channel_handle) {
  picojson::object channel_object;
  channel_object[kId] = picojson::value("");
  picojson::array channel_options;

  do {
    if (!channel_handle) {
      LoggerE("Error: trying to convert NULL channel handle to json");
      break;
    }

    picojson::value parsed_channel;
    {
      char *channel_json = nullptr;
      int error = conv_channel_internal_export_to_string(channel_handle, &channel_json);
      if ((CONV_ERROR_NONE != error) || !channel_json) {
	trace_conv_error(error, __LINE__, "converting channel handle to json");
        break;
      }

      std::string err;
      picojson::parse(parsed_channel, channel_json,
        channel_json + strlen(channel_json), &err);
      free(channel_json);
      if (!err.empty()) {
        LoggerE("Error parsing channel_handle json: %s", err.c_str());
        break;
      }
    }

    if (!parsed_channel.is<picojson::object>()) {
      LoggerE("ERROR: Channel is not an object");
      break;
    }

    const picojson::object &options = parsed_channel.get<picojson::object>();
    for(auto it = std::begin(options); it != std::end(options); ++it) {
      const std::string option_name = it->first;
      const picojson::value option_value = it->second;

      picojson::object option_object;
      option_object[kKey] = picojson::value(option_name);
      option_object[kChannelDataChannelOption] = option_value;
      channel_options.push_back(picojson::value(option_object));

      if (kChannelDataChannelId == option_name) {
        channel_object[kId] = picojson::value(option_value);
      }
    }
  } while(false);

  channel_object[kChannelDataChannelOptions] = picojson::value(channel_options);
  return picojson::value(channel_object);
}

static conv_channel_h json_to_channel(const picojson::value &channel_json_value) {
  if (channel_json_value.is<picojson::null>()) {
    LoggerE("ERROR: Channel json value is NULL");
    return nullptr;
  }

  conv_channel_h channel_handle = nullptr;
  int error = conv_channel_create(&channel_handle);

  if ((CONV_ERROR_NONE != error) || !channel_handle) {
    trace_conv_error(error, __LINE__, "creating channel handle");
    return nullptr;
  }

  if (!channel_json_value.get(kChannelDataChannelOptions).is<picojson::array>()) {
    return channel_handle;
  }

  const picojson::array &options =
    channel_json_value.get(kChannelDataChannelOptions).get<picojson::array>();
  for(size_t i = 0; i < options.size(); i++) {
    picojson::value option = options[i];
    const std::string key = option.get(kKey).to_str();
    const std::string value = option.get(kChannelDataChannelOption).to_str();
    error = conv_channel_set_string(channel_handle, key.c_str(), value.c_str());
    if (CONV_ERROR_NONE != error) {
      trace_conv_error(error, __LINE__, "setting channel string");
    }
  }

  return channel_handle;
}

static picojson::value payload_to_json(conv_payload_h payload_handle) {
  picojson::array payloads; // Array of payloads to deliver to JS layer

  do {
    if (!payload_handle) {
      LoggerE("ERROR: trying to convert NULL payload handle to json");
      break;
    }

    picojson::value parsed_payload;
    {
      char *payload_json_str = nullptr;
      int error = conv_payload_internal_export_to_string(payload_handle, &payload_json_str);
      if ((CONV_ERROR_NONE != error) || !payload_json_str) {
        trace_conv_error(error, __LINE__, "converting payload handle to json");
        break;
      }

      std::string err;
      picojson::parse(parsed_payload, payload_json_str,
        payload_json_str + strlen(payload_json_str), &err);
      free(payload_json_str);
      if (!err.empty()) {
        LoggerE("Error parsing payload json: %s", err.c_str());
        break;
      }
    }

    if (!parsed_payload.is<picojson::object>()) {
      LoggerE("ERROR: Payload is not an object");
      break;
    }

    const picojson::object &payload_items = parsed_payload.get<picojson::object>();
    for(auto it = std::begin(payload_items); it != std::end(payload_items); ++it) {
      const std::string item_name = it->first;
      const picojson::value item_value = it->second;

      picojson::object payload_object;
      payload_object[kKey] = picojson::value(item_name);

      // !!!! Recognizing type of the payload
      if (item_value.is<picojson::array>()) { // Byte array payload
        payload_object[kPayloadDataType] = picojson::value(kTypeRawBytes);
        payload_object[kValue] = item_value;
      } else if (item_value.is<picojson::object>()) { // App Control or String payload
        // Check if it is AppControl rather than arbitrary object
        // Attempting to convert it to app control. If failed, than it is an arbitrary object

        app_control_h app_control = nullptr;
        int error = conv_payload_get_app_control(payload_handle,
          item_name.c_str(), &app_control);
        if ((CONV_ERROR_NONE == error) && app_control) { // App Control payload
          payload_object[kPayloadDataType] = picojson::value(kTypeAppControl);

          char *app_id = nullptr;
          error = app_control_get_app_id(app_control, &app_id);
          if (APP_CONTROL_ERROR_NONE == error) {
            payload_object[kPayloadDataAppControlAppId] = picojson::value(app_id);
            free(app_id);
          } else {
            LoggerE("ERROR! AppControl API error [%d]", error);
          }

          //TODO extract app control fields and set it to json-style app control
          picojson::object app_control_object;
          char *operation = nullptr;
          error = app_control_get_operation(app_control, &operation);
          if (APP_CONTROL_ERROR_NONE == error) {
            app_control_object[kPayloadDataAppControlOperation] = picojson::value(operation);
            free(operation);
          } else {
            LoggerE("ERROR! AppControl API error [%d]", error);
          }

          char *uri = nullptr;
          error = app_control_get_uri(app_control, &uri);
          if (APP_CONTROL_ERROR_NONE == error) {
            app_control_object[kPayloadDataAppControlUri] = picojson::value(uri);
            free(uri);
          } else {
            LoggerE("ERROR! AppControl API error [%d]", error);
          }

          char *mime = nullptr;
          error = app_control_get_mime(app_control, &mime);
          if (APP_CONTROL_ERROR_NONE == error) {
            app_control_object[kPayloadDataAppControlMime] = picojson::value(mime);
            free(mime);
          } else {
            LoggerE("ERROR! AppControl API error [%d]", error);
          }

          char *category = nullptr;
          error = app_control_get_category(app_control, &category);
          if (APP_CONTROL_ERROR_NONE == error) {
            app_control_object[kPayloadDataAppControlCategory] = picojson::value(category);
            free(category);
          } else {
            LoggerE("ERROR! AppControl API error [%d]", error);
          }

          /* TODO
          attribute ApplicationControlData[] data;
          app_control_get_extra_data (app_control_h app_control, const char *key, char **value)
          app_control_get_extra_data_array (app_control_h app_control, const char *key, char ***value, int *length)
          app_control_is_extra_data_array (app_control_h app_control, const char *key, bool *array)
          app_control_foreach_extra_data (app_control_h app_control, app_control_extra_data_cb callback, void *user_data)
          */

          app_control_object[kPayloadDataAppControlLaunchMode] = picojson::value("SINGLE");
          payload_object[kValue] = picojson::value(app_control_object);
          error = app_control_destroy(app_control);
          if (APP_CONTROL_ERROR_NONE != error) {
            LoggerE("ERROR! AppControl API error [%d]", error);
          }
        } else { // Object, but not App Control: assuming it is a JSON String payload
          /*payload_object[kPayloadDataType] = picojson::value(kTypeString);
          //payload_object[kValue] = picojson::value(item_value.serialize());
          payload_object[kValue] = item_value;

          //if (item_value.is<picojson::object>())
          LoggerE("===Serialized: %s", item_value.serialize().c_str());
          LoggerE("===Stringifyed: %s", item_value.to_str().c_str());*/

          // !!!!!
          // This is an object type of payload and we don't support it in JS API
          // It is better to ignore this payload portion
          LoggerE("IGNORED OBJECT-TYPE PAYLOAD: %s", item_value.serialize().c_str());
          continue;
        }
      } else { // String payload
        payload_object[kPayloadDataType] = picojson::value(kTypeString);
        payload_object[kValue] = item_value;
      }

      payloads.push_back(picojson::value(payload_object));
    }
  } while(false);

  return picojson::value(payloads);
}

static conv_payload_h json_to_payload(const picojson::value &payload_value) {
  if (payload_value.is<picojson::null>()) {
    LoggerE("ERROR: trying to convert NULL payload json value");
    return nullptr;
  }

  conv_payload_h payload_handle = nullptr;
  int error = conv_payload_create(&payload_handle);
  if ((CONV_ERROR_NONE != error) || !payload_handle) {
    trace_conv_error(error, __LINE__, "creating payload handle");
    return nullptr;
  }

  if (!payload_value.is<picojson::array>()) {
    LoggerE("ERROR: Payload json is not an array");
    return payload_handle;
  }

  const picojson::array &payload_items = payload_value.get<picojson::array>();
  for(size_t i = 0; i < payload_items.size(); i++) {
    picojson::value item = payload_items[i];
    const std::string type = item.get(kPayloadDataType).to_str();
    if (kTypeString == type) {
      const std::string key = item.get(kKey).to_str();
      const std::string value = item.get(kValue).to_str(); // TODO: check if it is an object
      int error = conv_payload_set_string(payload_handle, key.c_str(), value.c_str());
      if (CONV_ERROR_NONE != error) {
        trace_conv_error(error, __LINE__, "setting payload string");
      }
    } else if (kTypeRawBytes == type) {
      // TODO
      LoggerW("IMPLEMENT BYTE PAYLOAD!!!");
    } else if (kTypeAppControl == type) {
      app_control_h app_control = nullptr;
      error = app_control_create(&app_control);
      if (APP_CONTROL_ERROR_NONE != error) {
        LoggerE("ERROR! AppControl API error [%d]", error);
      }

      const std::string app_id = item.get(kPayloadDataAppControlAppId).to_str();
      if (!app_id.empty() && app_id.c_str() && ("null" != app_id)) { // TODO check here and in other cases if JSON sub-object is empty
        LoggerW("...app_control_set_app_id(app_control, %s)", app_id.c_str());
        error = app_control_set_app_id(app_control, app_id.c_str());
        if (APP_CONTROL_ERROR_NONE != error) {
          LoggerE("ERROR! AppControl API error [%d]", error);
        }
      }

      const std::string operation = item.get(kValue).get(kPayloadDataAppControlOperation).to_str();
      if (!operation.empty() && operation.c_str() && ("null" != operation)) {
        LoggerW("...app_control_set_operation(app_control, %s)", operation.c_str());
        error = app_control_set_operation(app_control, operation.c_str());
        if (APP_CONTROL_ERROR_NONE != error) {
          LoggerE("ERROR! AppControl API error [%d]", error);
        }
      }

      const std::string uri = item.get(kValue).get(kPayloadDataAppControlUri).to_str();
      if (!uri.empty() && uri.c_str() && ("null" != uri)) {
        LoggerW("...app_control_set_uri(app_control, %s)", uri.c_str());
        error = app_control_set_uri(app_control, uri.c_str());
        if (APP_CONTROL_ERROR_NONE != error) {
          LoggerE("ERROR! AppControl API error [%d]", error);
        }
      }

      const std::string mime = item.get(kValue).get(kPayloadDataAppControlMime).to_str();
      if (!mime.empty() && mime.c_str() && ("null" != mime)) {
        LoggerW("...app_control_set_mime(app_control, %s)", mime.c_str());
        error = app_control_set_mime(app_control, mime.c_str());
        if (APP_CONTROL_ERROR_NONE != error) {
          LoggerE("ERROR! AppControl API error [%d]", error);
        }
      }

      const std::string category = item.get(kValue).get(kPayloadDataAppControlCategory).to_str();
      if (!category.empty() && category.c_str() && ("null" != category)) {
        LoggerW("...app_control_set_category(app_control, %s)", category.c_str());
        error = app_control_set_category(app_control, category.c_str());
        if (APP_CONTROL_ERROR_NONE != error) {
          LoggerE("ERROR! AppControl API error [%d]", error);
        }
      }

      // TODO implement also conversion of key-value data

      const std::string key = item.get(kKey).to_str();
      if (!key.empty() && key.c_str() && ("null" != key)) {
        LoggerW("+++conv_payload_set_app_control(payload, %s, app_control)", key.c_str());
        int error = conv_payload_set_app_control(payload_handle, key.c_str(), app_control);
        if (CONV_ERROR_NONE != error) {
          trace_conv_error(error, __LINE__, "setting app control payload");
        }
      }

      // TODO release appcontrol handle and payload handle

    } else {
      LoggerE("ERROR! Unknown type of payload [%s]", type.c_str());
    }
  }

  return payload_handle;
}

////////////////////////////////////////////////////////////////////////////////

ConvergenceManager &ConvergenceManager::GetInstance(
  ConvergenceInstance *owner) {
  static ConvergenceManager instance;
  instance.convergence_plugin_ = owner;
  return instance;
}

ConvergenceManager::ConvergenceManager()
 : convergence_plugin_(nullptr)
 , convergence_manager_(nullptr) {
  ScopeLogger();
  const int error = conv_create(&convergence_manager_);
  if (CONV_ERROR_NONE != error) {
    // Handle error
    trace_conv_error(error, __LINE__, "conv_create");
  }
}

ConvergenceManager::~ConvergenceManager() {
  ScopeLogger();

  // Releasing all registered devices
  for(auto it = remote_devices_.begin(); it != remote_devices_.end(); ++it) {
      conv_device_destroy(it->second);
  }

  // TODO unregister all listeners
  // (if it is required by API and doesn't happening automatically)

  int error = conv_destroy(convergence_manager_);
  if (CONV_ERROR_NONE != error) {
    // Handle error
    trace_conv_error(error, __LINE__, "conv_destroy");
  }
  convergence_manager_ = nullptr;

  for(auto it = local_services_.begin(); it != local_services_.end(); ++it) {
    conv_service_h service = it->second;
    error = conv_service_destroy(service);
    if (CONV_ERROR_NONE != error) {
      // Handle error
      trace_conv_error(error, __LINE__, "conv_service_destroy");
    }
  }
  local_services_.clear();
}

void ConvergenceManager::ServiceForeachCb(conv_service_h service_handle,
  void* user_data) {
  ScopeLogger();
  if (!service_handle || !user_data) {
    LoggerE("ERROR! Invalid parameters of D2D API Callback");
    return;
  }

  picojson::array *services = static_cast<picojson::array *>(user_data);

  picojson::object service;

  // Extracting service type
  conv_service_e type = CONV_SERVICE_NONE;
  int error = conv_service_get_type(service_handle, &type);
  if (CONV_ERROR_NONE != error) {
    LoggerE("ERROR! D2D API error [%d]", error);
  }
  service[kServiceType] = picojson::value(static_cast<double>(type));

  // Extracting service connection state
  conv_service_connection_state_e state = CONV_SERVICE_CONNECTION_STATE_NONE;
  error = conv_service_get_connection_state(service_handle, &state);
  if (CONV_ERROR_NONE != error) {
    LoggerE("ERROR! D2D API error [%d]", error);
    trace_conv_error(error, __LINE__, "conv_service_get_connection_state");
  }
  service[kServiceConnectionState] = picojson::value(static_cast<double>(state));

  // Extracting service properties
  picojson::array properties;
  { // Service ID
    char *id = nullptr;
    error = conv_service_get_property_string(service_handle, CONV_SERVICE_ID, &id);
    if (CONV_ERROR_NONE != error) {
      trace_conv_error(error, __LINE__, "conv_service_get_property_string");
    }
    picojson::object propId;
    propId[kKey] = picojson::value(CONV_SERVICE_ID);
    propId[kValue] = picojson::value(id);
    //properties.push_back(picojson::value(id));
    properties.push_back(picojson::value(propId));
    free(id);
  }

  { // Service version
    char *version = nullptr;
    error = conv_service_get_property_string(service_handle,
      CONV_SERVICE_VERSION, &version);
    if (CONV_ERROR_NONE != error) {
      trace_conv_error(error, __LINE__, "conv_service_get_property_string");
    }
    picojson::object propVersion;
    //properties.push_back(picojson::value(version));
    propVersion[kKey] = picojson::value(CONV_SERVICE_VERSION);
    propVersion[kValue] = picojson::value(version);
    properties.push_back(picojson::value(propVersion));
    free(version);
  }

  // TODO: add other properties, which are not named with predefined keys

  service[kServiceProperties] = picojson::value(properties);

  // Add newly discovered service to the list of available services
  services->push_back(picojson::value(service));
}

void ConvergenceManager::DiscoveryCb(conv_device_h device_handle,
  conv_discovery_result_e result, void* user_data) {
  ScopeLogger();

  if (!user_data) {
    LoggerE("ERROR! NULL user data in discovery callback");
    return;
  }

  ConvergenceManager *owner = static_cast<ConvergenceManager *>(user_data);

  switch(result) {
  case CONV_DISCOVERY_RESULT_SUCCESS: {
    LoggerI("...found a device");

    picojson::object device;

    { // Extracting device ID
      char *id = nullptr;
      int error = conv_device_get_property_string(device_handle, CONV_DEVICE_ID, &id);
      if (CONV_ERROR_NONE != error) {
        trace_conv_error(error, __LINE__, "conv_device_get_property_string");
      }
      if (id) {
        device[kId] = picojson::value(id);

        LoggerI("......device id: %s", id);

        // Store newly found device for further operations with services
        conv_device_h device_handle_clone = nullptr;
        error = conv_device_clone(device_handle, &device_handle_clone);
        if (CONV_ERROR_NONE != error) {
          trace_conv_error(error, __LINE__, "conv_device_clone");
        }
        owner->remote_devices_[id] = device_handle_clone;

        LoggerI("...registering the device %x", device_handle);

        free(id);
      } else
        device[kId] = picojson::value("null");
    }

    { // Extracting device name
      char *name = nullptr;
      const int error = conv_device_get_property_string(device_handle, CONV_DEVICE_NAME, &name);
      if (CONV_ERROR_NONE != error) {
        trace_conv_error(error, __LINE__, "conv_device_get_property_string");
      }
      if (name) {
        device[kName] = picojson::value(name);

        LoggerI("......device name: %s", name);

        free(name);
      } else
        device[kName] = picojson::value("null");
    }

    { // Extracting device type
      char *type = nullptr;
      int error = conv_device_get_property_string(device_handle, CONV_DEVICE_TYPE, &type);
      if (CONV_ERROR_NONE != error) {
        trace_conv_error(error, __LINE__, "conv_device_get_property_string CONV_DEVICE_TYPE");
      }
      if (type)  {
        LoggerI("......device type: %s", type);
        device[kDeviceType] = picojson::value(type);
        free(type);
      } else
        device[kDeviceType] = picojson::value("null");
    }

    { // Extracting services
      picojson::array services;
      const int error = conv_device_foreach_service(device_handle, ServiceForeachCb, &services);
      if (CONV_ERROR_NONE != error) {
        trace_conv_error(error, __LINE__, "conv_device_foreach_service");
      }
      device[kDeviceServices] = picojson::value(services);
    }

    picojson::object param;
    param[kDiscoveryStatus] = picojson::value(kDiscoveryStatusDeviceFound);
    param[kDevice] = picojson::value(device);
    owner->convergence_plugin_->ReplyAsync(kConvergenceManagerDiscoveryCallback,
      -1,
      true,
      param);

    break;
  }
  case CONV_DISCOVERY_RESULT_FINISHED: {
    LoggerE("...discovery finished");

    picojson::object param;
    param[kDiscoveryStatus] = picojson::value(kDiscoveryStatusFinished);
    owner->convergence_plugin_->ReplyAsync(kConvergenceManagerDiscoveryCallback,
      -1,
      true,
      param);
    break;
  }
  case CONV_DISCOVERY_RESULT_ERROR: {
    LoggerE("Discovery Error");
    picojson::object param;
    param[kDiscoveryStatus] = picojson::value(kDiscoveryStatusError);
    param[kDiscoveryError] = picojson::value(static_cast<double>(result));
    owner->convergence_plugin_->ReplyAsync(kConvergenceManagerDiscoveryCallback,
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
  ScopeLogger();
  const int error = conv_discovery_start(convergence_manager_,
    (const int)timeout, DiscoveryCb, this);
  if (CONV_ERROR_NONE != error) {
    // TODO: Handle error
    trace_conv_error(error, __LINE__, "conv_discovery_start");
  }
  return TizenSuccess();
}

TizenResult ConvergenceManager::StopDiscovery() {
  ScopeLogger();
  const int error = conv_discovery_stop(convergence_manager_);
  if (CONV_ERROR_NONE != error) {
    // TODO: Handle error
    trace_conv_error(error, __LINE__, "conv_discovery_stop");
  }
  return TizenSuccess();
}

conv_device_h ConvergenceManager::get_remote_device(const char *device_id) const {
  //ScopeLogger();
  // TODO: re-design to request device list every time
  std::string strDeviceId(device_id); // TODO: add critical section
  if (0 >= remote_devices_.count(strDeviceId)) {
    return nullptr;
  }
  return remote_devices_[strDeviceId];
}


//-----------------------------------------------------------------------------
class ServiceFindVisitor {
 public:
  conv_service_h service_;
  const int service_type_;
  ServiceFindVisitor(const int type) : service_(nullptr), service_type_(type) {}
  void visit(void *data) {
    conv_service_h s = (conv_service_h)data;
    conv_service_e type = CONV_SERVICE_NONE;
    int error = conv_service_get_type(s, &type);
    if (CONV_ERROR_NONE != error) {
      trace_conv_error(error, __LINE__, "conv_service_get_type");
    }
    if (int(type) == service_type_) {
      conv_service_h service_clone = nullptr;
      error = conv_service_clone(s, &service_clone);
      if (CONV_ERROR_NONE != error) {
        trace_conv_error(error, __LINE__, "conv_service_clone");
      } else {
        service_ = service_clone;
      }
    }
  }
  bool done() const { return (service_ != nullptr); }
};
//-----------------------------------------------------------------------------


void ConvergenceManager::FindServiceCb(conv_service_h service_handle,
  void *user_data) {
  ServiceFindVisitor *visitor = static_cast<ServiceFindVisitor *>(user_data);
  if (visitor && !visitor->done()) {
    visitor->visit(service_handle);
  }
}

conv_service_h ConvergenceManager::get_remote_service(const conv_device_h device,
  const int service_type) const {
  ServiceFindVisitor visitor(service_type);
  const int error  = conv_device_foreach_service(device, FindServiceCb, &visitor);
  if (CONV_ERROR_NONE != error) {
    trace_conv_error(error, __LINE__, "conv_device_foreach_service");
  }
  return visitor.service_;
}

conv_service_h ConvergenceManager::get_service(const char *device_id,
  const int service_type) const {
  ScopeLogger();
  if (is_local_device(device_id)) {
    if (0 >= local_services_.count(service_type)) {
      LoggerE("LOCAL Service with specified type does not exist");
      return nullptr;
    }

    return local_services_[service_type];
  } else {
    conv_device_h device = get_remote_device(device_id);
    if (!device) {
      LoggerE("Device with specified id does not exist");
      return nullptr;
    }

    conv_service_h service = get_remote_service(device, service_type);
    if (!service) {
      LoggerE("Service with specified type does not exist");
      return nullptr;
    }

    return service;
  }
}

/* TODO: Implement smart handle class for service
 * class ConvServiceHandle:
 *  private:
 *   conv_service_h __service_handle
 *  public:
 *   constructor(ConvergenceManager *owner, device_id, service_type)
 *   destructor - if the service is not local, destroy it
 *   operator conv_service_h() - get the handle from d2d-conv-manager of from the table of local services
 */


void ConvergenceManager::DestroyService(conv_service_h service) {
  ScopeLogger();
  if (!service) {
    LoggerE("ERROR! trying to destroy NULL service handle");
    return;
  }

  for(auto it = std::begin(local_services_); it != std::end(local_services_); ++it) {
    if (it->second == service) {
      return; // This is a local service, so not needed to destroy it here.
        // (it will be destroyed in the destructor of the manager)
    }
  }

  conv_service_destroy(service);
}

// TODO make function inline
bool ConvergenceManager::is_local_device(const char *device_id) const {
  ScopeLogger();
  return (g_strcmp0(device_id, "localhost") == 0);
}

void ConvergenceManager::ServiceConnectedCb(conv_service_h service_handle,
  conv_error_e error, conv_payload_h result, void* user_data) {
  ScopeLogger();

  if (!user_data) {
    LoggerE("ERROR! NULL user data in Service Connect Callback");
    return;
  }

  std::pair<ConvergenceManager *, int> *callbackParam =
    static_cast<std::pair<ConvergenceManager *, int> *>(user_data);
  ConvergenceManager *owner = callbackParam->first;
  const int cur_listener_id = callbackParam->second;
  delete callbackParam;

  picojson::object param;
  param[kPayload] = payload_to_json(result);

  if (CONV_ERROR_NONE == error) {
    param[kServiceConnectionStatus] = picojson::value(kServiceConnectionStatusConnected);
    owner->convergence_plugin_->ReplyAsync(kServiceConnectCallback,
      cur_listener_id,
      true,
      param);
  } else {
    // Error occured during connection
    param[kServiceConnectionStatus] = picojson::value(kServiceConnectionStatusNotConnected);
    owner->convergence_plugin_->ReplyAsync(kServiceConnectCallback,
      cur_listener_id,
      false,
      param);
  }
}

void ConvergenceManager::ServiceListenerCb(conv_service_h service_handle,
  conv_channel_h channel_handle,
  conv_error_e error, conv_payload_h result, void* user_data) {
  ScopeLogger();

  if (!user_data) {
    LoggerE("ERROR! NULL user data in Service Listener Callback");
    return;
  }

  std::pair<ConvergenceManager *, int> *callbackParam=
    static_cast<std::pair<ConvergenceManager *, int> *>(user_data);
  ConvergenceManager *owner = callbackParam->first;
  const int cur_listener_id = callbackParam->second;
  delete callbackParam; // This delete was forgotten

  picojson::object param;
  param[kChannel] = channel_to_json(channel_handle);
  param[kPayload] = payload_to_json(result);

  if (CONV_ERROR_NONE == error) {
    param[kServiceListenerStatus] = picojson::value(kServiceListenerStatusOk);
    owner->convergence_plugin_->ReplyAsync(kServiceListenerCallback,
      cur_listener_id,
      true,
      param);
  } else {
    // Error occured during connection
    param[kServiceListenerStatus] = picojson::value(kServiceListenerStatusError);
    param[kServiceListenerError] = picojson::value(static_cast<double>(error));
    owner->convergence_plugin_->ReplyAsync(kServiceListenerCallback,
      cur_listener_id,
      false,
      param);
  }
}

TizenResult ConvergenceManager::CreateLocalService(const int service_type,
  const picojson::value &service_data) {
  ScopeLogger();
  if ( 0 < local_services_.count(service_type)) {
    return LogAndCreateTizenError(NotFoundError,
      "LOCAL service is already created");
  }

  int error = CONV_ERROR_NONE;
  conv_service_h service = nullptr;
  do {
    error = conv_service_create(&service);
    if (CONV_ERROR_NONE != error) {
      LoggerE("ERROR! D2D API error [%d]", error);
      break;
    }

    error = conv_service_set_type(service, CONV_SERVICE_APP_TO_APP_COMMUNICATION);
    if (CONV_ERROR_NONE != error) {
      trace_conv_error(error, __LINE__, "conv_service_set_type");
      break;
    }

    const picojson::array &properties =
      service_data.get(kServiceProperties).get<picojson::array>();
    for(size_t i = 0; i < properties.size(); i++) {
      picojson::value property = properties[i];
      const std::string key = property.get(kKey).to_str();
      const std::string value = property.get(kValue).to_str();
      error = conv_service_set_property_string(service, key.c_str(), value.c_str());
      if (CONV_ERROR_NONE != error) {
        trace_conv_error(error, __LINE__, "conv_service_set_property_string");
      }
      LoggerI("Set local service property: [%s: %s]", key.c_str(), value.c_str());
    }

    local_services_[service_type] = service;

    return TizenSuccess();
  } while(false);

  error = conv_service_destroy(service);
  if (CONV_ERROR_NONE != error) {
    trace_conv_error(error, __LINE__, "conv_service_destroy");
  }

  return LogAndCreateTizenError(NotFoundError,
    "LOCAL service creation failed");
}

TizenResult ConvergenceManager::Connect(const char *device_id,
  const int service_type, const int cur_listener_id) {
  ScopeLogger();

  conv_service_h service = get_service(device_id, service_type);
  if (!service) {
    return LogAndCreateTizenError(NotFoundError,
      "Service with specified type does not exist");
  }

  // TODO: make a garbage collection in case of callback is not invoked
  std::pair<ConvergenceManager *, int> *param =
    new std::pair<ConvergenceManager *, int>(this, cur_listener_id);
  const int error = conv_service_connect(service, ServiceConnectedCb, param);
  if (CONV_ERROR_NONE != error) {
    // TODO: Handle error
    trace_conv_error(error, __LINE__, "conv_service_connect");
  }

  DestroyService(service);

  return TizenSuccess();
}

TizenResult ConvergenceManager::Disconnect(const char *device_id,
  const int service_type) {
  ScopeLogger();

  conv_service_h service = get_service(device_id, service_type);
  if (!service)
    return LogAndCreateTizenError(NotFoundError,
      "Service with specified type does not exist");

  const int error = conv_service_disconnect(service);
  if (CONV_ERROR_NONE != error) {
    // TODO: Handle error
    trace_conv_error(error, __LINE__, "conv_service_disconnect");
  }

  DestroyService(service);

  return TizenSuccess();
}

common::TizenResult ConvergenceManager::Start(const char *device_id,
  const int service_type, const picojson::value &channel,
  const picojson::value &payload) {
  ScopeLogger();
  return ExecuteServiceCommand(kServiceStart,
    device_id, service_type, channel, payload);
}

common::TizenResult ConvergenceManager::Read(const char *device_id,
  const int service_type, const picojson::value &channel,
  const picojson::value &payload) {
  ScopeLogger();
  return ExecuteServiceCommand(kServiceRead,
    device_id, service_type, channel, payload);
}

common::TizenResult ConvergenceManager::Send(const char *device_id,
  const int service_type, const picojson::value &channel,
  const picojson::value &payload) {
  ScopeLogger();
  return ExecuteServiceCommand(kServiceSend,
    device_id, service_type, channel, payload);
}

common::TizenResult ConvergenceManager::Stop(const char *device_id,
  const int service_type, const picojson::value &channel,
  const picojson::value &payload) {
  ScopeLogger();
  return ExecuteServiceCommand(kServiceStop,
    device_id, service_type, channel, payload);
}

common::TizenResult ConvergenceManager::ExecuteServiceCommand(
  const ServiceCommands command,
  const char *device_id,
  const int service_type,
  const picojson::value &channel,
  const picojson::value &payload) {
  ScopeLogger();

  conv_service_h service = get_service(device_id, service_type);
  if (!service) {
    LoggerE("AAAAAA!!! Service Forbidden: this device has no more required service");
    return LogAndCreateTizenError(NotFoundError,
     "Service with specified type does not exist");
  }

  // TODO: if channel == nullObj, use default channel
  conv_channel_h channel_handle = json_to_channel(channel);
  conv_payload_h payload_handle = json_to_payload(payload);

  int error = CONV_ERROR_NONE;
  switch(command) {
  case kServiceStart:
    error = conv_service_start(service, channel_handle, payload_handle);
    break;
  case kServiceRead:
    error = conv_service_read(service, channel_handle, payload_handle);
    break;
  case kServiceSend:
    error = conv_service_publish(service, channel_handle, payload_handle);
    break;
  case kServiceStop:
    error = conv_service_stop(service, channel_handle, payload_handle);
    break;
  default:
    LoggerE("Unknown service command [%d]", command);
    break;
  }
  if (CONV_ERROR_NONE != error) {
    // TODO: Handle error
    trace_conv_error(error, __LINE__, "conv_service_start/read/send/stop");
  }
  conv_channel_destroy(channel_handle);
  conv_payload_destroy(payload_handle);
  DestroyService(service);
  return TizenSuccess();
}

TizenResult ConvergenceManager::SetListener(const char *device_id,
  const int service_type, const int cur_listener_id) {
  ScopeLogger();

  conv_service_h service = get_service(device_id, service_type);
  if (!service) {
    return LogAndCreateTizenError(NotFoundError,
      "Service with specified type does not exist");
  }

  // TODO: make a garbage collection in case of callback is not invoked
  std::pair<ConvergenceManager *, int> *param =
    new std::pair<ConvergenceManager *, int>(this, cur_listener_id);
  const int error = conv_service_set_listener_cb(service, ServiceListenerCb, param);
  if (CONV_ERROR_NONE != error) {
    // TODO: Handle error
    trace_conv_error(error, __LINE__, "conv_service_set_listener_cb");
  }

  DestroyService(service);

  return TizenSuccess();
}

TizenResult ConvergenceManager::UnsetListener(const char *device_id,
  const int service_type) {
  ScopeLogger();

  conv_service_h service = get_service(device_id, service_type);
  if (!service)
    return LogAndCreateTizenError(NotFoundError,
      "Service with specified type does not exist");

  const int error = conv_service_unset_listener_cb(service);
  if (CONV_ERROR_NONE != error) {
    // TODO: Handle error
    trace_conv_error(error, __LINE__, "conv_service_unset_listener_cb");
  }

  DestroyService(service);

  return TizenSuccess();
}

} // namespace convergence
}  // namespace extension
