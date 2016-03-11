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

#include "messageport/messageport_instance.h"

#include <functional>
#include <vector>
#include <string>
#include <bundle_internal.h>

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"

namespace extension {
namespace messageport {

using common::TypeMismatchException;
using common::InvalidValuesException;
using common::UnknownException;
using common::NotFoundException;
using common::QuotaExceededException;

MessageportInstance::MessageportInstance() {
  LoggerD("Enter");
  using std::placeholders::_1;
  using std::placeholders::_2;
  #define REGISTER_SYNC(c, x) \
    RegisterSyncHandler(c, std::bind(&MessageportInstance::x, this, _1, _2));
  REGISTER_SYNC("MessagePortManager_requestTrustedRemoteMessagePort",
                MessagePortManagerRequesttrustedremotemessageport);
  REGISTER_SYNC("MessagePortManager_requestLocalMessagePort",
                MessagePortManagerRequestlocalmessageport);
  REGISTER_SYNC("MessagePortManager_requestTrustedLocalMessagePort",
                MessagePortManagerRequesttrustedlocalmessageport);
  REGISTER_SYNC("MessagePortManager_requestRemoteMessagePort",
                MessagePortManagerRequestremotemessageport);
  REGISTER_SYNC("RemoteMessagePort_sendMessage", RemoteMessagePortSendmessage);
  #undef REGISTER_SYNC
}

MessageportInstance::~MessageportInstance() {
  LoggerD("Enter");
}


enum MessageportCallbacks {
  MessagePortManagerRequesttrustedremotemessageportCallback,
  MessagePortManagerRequestlocalmessageportCallback,
  MessagePortManagerRequesttrustedlocalmessageportCallback,
  MessagePortManagerRequestremotemessageportCallback,
  LocalMessagePortRemovemessageportlistenerCallback,
  RemoteMessagePortSendmessageCallback,
  LocalMessagePortAddmessageportlistenerCallback
};

static void BundleJsonIterator(const char *key, const int type, const bundle_keyval_t *kv, void *d) {
  LoggerD("Enter");

  void *basic_val = nullptr;
  size_t basic_size = 0;

  picojson::value::array *array = static_cast<picojson::value::array *>(d);
  picojson::value::object o;

  switch (bundle_keyval_get_type(const_cast<bundle_keyval_t*>(kv))) {
    case BUNDLE_TYPE_STR:
      bundle_keyval_get_basic_val(const_cast<bundle_keyval_t*>(kv), &basic_val, &basic_size);
      o["key"] = picojson::value(key);
      o["value"] = picojson::value(static_cast<char*>(basic_val));
      break;

    case BUNDLE_TYPE_STR_ARRAY: {
      picojson::value::array tab;
      void **array_val = nullptr;
      unsigned int array_len = 0;
      size_t *array_elem_size = nullptr;

      bundle_keyval_get_array_val(const_cast<bundle_keyval_t*>(kv),
                                  &array_val,
                                  &array_len,
                                  &array_elem_size);

      for (unsigned int i = 0; i < array_len; i++) {
        tab.push_back(picojson::value(((char**) array_val)[i]));
      }

      o["key"] = picojson::value(key);
      o["value"] = picojson::value(picojson::value(tab));

      break;
    }

    case BUNDLE_TYPE_BYTE: {
      picojson::value::array tab;

      unsigned char *basic_val = nullptr;
      size_t basic_size = 0;

      bundle_keyval_get_basic_val(const_cast<bundle_keyval_t*>(kv),
          (void **)&basic_val, &basic_size);

      for (unsigned int i = 0; i < basic_size; i++) {
        tab.push_back(picojson::value(static_cast<double> (basic_val[i])));
      }

      o["key"] = picojson::value(key);
      o["value"] = picojson::value(picojson::value(tab));
      break;
    }
    case BUNDLE_TYPE_BYTE_ARRAY: {
      picojson::value::array tab;

      unsigned char **array_value=nullptr;
      size_t *array_ele_size=nullptr;
      unsigned int ele_nos=0;

      bundle_keyval_get_array_val(const_cast<bundle_keyval_t*>(kv),
          (void ***)&array_value, &ele_nos, &array_ele_size);

      for (unsigned int i=0;i<ele_nos;i++)
      {
        picojson::value::array tab2;
        for (unsigned int j = 0; j < array_ele_size[i]; j++) {
          tab2.push_back(picojson::value(static_cast<double> (array_value[i][j])));
        }
        tab.push_back(picojson::value(tab2));
      }

      o["key"] = picojson::value(key);
      o["value"] = picojson::value(picojson::value(tab));
      break;
    }
    default:
      o["key"] = picojson::value(key);
      o["value"] = picojson::value();
      break;
  }
  array->push_back(picojson::value(o));
}

#define CHECK_EXIST(args, name, out) \
    if (!args.contains(name)) {\
      LogAndReportError(TypeMismatchException(name" is required argument"), out);\
      return;\
    }

static void OnReceiveLocalMessage(int local_port_id,
  const char* remote_app_id, const char* remote_port,
  bool trusted_remote_port, bundle* message, void* user_data) {

  LoggerD("Enter");
  MessageportInstance* object = static_cast<MessageportInstance*>(user_data);
  picojson::value::object o;
  picojson::value::array data;

  o["local_port_id"] = picojson::value(static_cast<double>(local_port_id));

  if (remote_port) {
    o["remoteAppId"] = picojson::value(remote_app_id);
    o["remotePort"] = picojson::value(remote_port);
    o["trusted"] = picojson::value(trusted_remote_port);
    LoggerD("Msg received from: %s", remote_app_id);
  }

  LoggerD("Msg received");

  bundle_foreach(message, BundleJsonIterator, &data);

  o["message"] = picojson::value(data);

  common::Instance::PostMessage(object, picojson::value(o).serialize().c_str());
}


void MessageportInstance::MessagePortManagerRequestlocalmessageport
  (const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "localMessagePortName", out)

  int portId;
  const std::string& localMessagePortName =
    args.get("localMessagePortName").get<std::string>();

  portId = message_port_register_local_port
    (localMessagePortName.c_str(), OnReceiveLocalMessage, this);

  LoggerD("Registering local port %s : %s", localMessagePortName.c_str(),
    portId < 0 ? "false" : "true");


  if (portId < 0) {
    switch (portId) {
      case MESSAGE_PORT_ERROR_INVALID_PARAMETER:
        LogAndReportError(
            InvalidValuesException("The input parameter contains an invalid value."), out,
            ("message_port_register_local_port error: %d (%s)", portId, get_error_message(portId)));
        break;
      case MESSAGE_PORT_ERROR_OUT_OF_MEMORY:
        LogAndReportError(
            UnknownException("Out of memory."), out,
            ("message_port_register_local_port error: %d (%s)", portId, get_error_message(portId)));
        break;
      case MESSAGE_PORT_ERROR_IO_ERROR:
        LogAndReportError(
            UnknownException("Internal I/O error ocurred."), out,
            ("message_port_register_local_port error: %d (%s)", portId, get_error_message(portId)));
        break;
      default:
        LogAndReportError(
            UnknownException("Unknown Exception"), out,
            ("message_port_register_local_port error: %d (%s)", portId, get_error_message(portId)));
        break;
      }
  } else {
    ReportSuccess(picojson::value(static_cast<double>(portId)), out);
  }
}

void MessageportInstance::
  MessagePortManagerRequesttrustedlocalmessageport
    (const picojson::value& args, picojson::object& out) {

  LoggerD("Enter");
  CHECK_EXIST(args, "localMessagePortName", out)

  int portId;
  const std::string& localMessagePortName =
    args.get("localMessagePortName").get<std::string>();

  portId = message_port_register_trusted_local_port
    (localMessagePortName.c_str(), OnReceiveLocalMessage, this);

  LoggerD("Registering trusted local port %s:%s", localMessagePortName.c_str(),
    portId < 0 ? "false" : "true");

  if (portId < 0) {
    switch (portId) {
      case MESSAGE_PORT_ERROR_INVALID_PARAMETER:
        LogAndReportError(
            InvalidValuesException("The input parameter contains an invalid value."), out,
            ("message_port_register_trusted_local_port error: %d (%s)", portId, get_error_message(portId)));
        break;
      case MESSAGE_PORT_ERROR_OUT_OF_MEMORY:
        LogAndReportError(
            UnknownException("Out of memory."), out,
            ("message_port_register_trusted_local_port error: %d (%s)", portId, get_error_message(portId)));
        break;
      case MESSAGE_PORT_ERROR_IO_ERROR:
        LogAndReportError(
            UnknownException("Internal I/O error ocurred."), out,
            ("message_port_register_trusted_local_port error: %d (%s)", portId, get_error_message(portId)));
        break;
      default:
        LogAndReportError(
            UnknownException("Unknown Exception"), out,
            ("message_port_register_trusted_local_port error: %d (%s)", portId, get_error_message(portId)));
        break;
      }
  } else {
    ReportSuccess(picojson::value(static_cast<double>(portId)), out);
  }
}

void MessageportInstance::
  MessagePortManagerRequestremotemessageport
    (const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "remoteMessagePortName", out)

  const std::string& remoteMessagePortName =
    args.get("remoteMessagePortName").get<std::string>();
  const std::string& appId = args.get("appId").get<std::string>();

  int ret;
  bool portCheck;

  ret = message_port_check_remote_port(appId.c_str(),
    remoteMessagePortName.c_str(), &portCheck);

  LoggerD("Checking remote port of %s: %s", remoteMessagePortName.c_str(),
    portCheck ? "true" : "false");
  LoggerD("Error code: %d (%s)", ret, get_error_message(ret));

  if (ret == MESSAGE_PORT_ERROR_NONE) {
    if (portCheck)  {
      ReportSuccess(out);
    } else {
      LogAndReportError(NotFoundException("The port of the target application is not found"), out);
    }
  } else if (ret == MESSAGE_PORT_ERROR_INVALID_PARAMETER) {
    LogAndReportError(
          InvalidValuesException("An input parameter contains an invalid value."), out,
          ("message_port_check_remote_port error: %d (%s)", ret, get_error_message(ret)));
  } else if (ret == MESSAGE_PORT_ERROR_OUT_OF_MEMORY) {
    LogAndReportError(
          UnknownException("Out of memory."), out,
          ("message_port_check_remote_port error: %d (%s)", ret, get_error_message(ret)));
  } else if (ret == MESSAGE_PORT_ERROR_IO_ERROR) {
    // IO error means that remote port does not exist
    LogAndReportError(
          NotFoundException("The port of the target application is not found"), out,
          ("message_port_check_remote_port error: %d (%s)", ret, get_error_message(ret)));
  } else if (ret == MESSAGE_PORT_ERROR_PORT_NOT_FOUND) {
    LogAndReportError(
          NotFoundException("The port of the target application is not found"), out,
          ("message_port_check_remote_port error: %d (%s)", ret, get_error_message(ret)));
  } else {
    LogAndReportError(
          UnknownException("Unknown Error"), out,
          ("message_port_check_remote_port error: %d (%s)", ret, get_error_message(ret)));
  }
}

void MessageportInstance::
  MessagePortManagerRequesttrustedremotemessageport
    (const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "remoteMessagePortName", out)

  const std::string& remoteMessagePortName =
    args.get("remoteMessagePortName").get<std::string>();
  const std::string& appId = args.get("appId").get<std::string>();

  bool portCheck;
  int ret;

  ret = message_port_check_trusted_remote_port
        (appId.c_str(), remoteMessagePortName.c_str(), &portCheck);

  LoggerD("Checking trusted remote port of %s: %s",
    remoteMessagePortName.c_str(), portCheck ? "true":"false");
  LoggerD("Error code: %d (%s)", ret, get_error_message(ret));

  if (ret == MESSAGE_PORT_ERROR_NONE) {
    if (portCheck) {
      ReportSuccess(out);
    } else {
      LogAndReportError(
          NotFoundException("The port of the target application is not found"), out);
    }
  } else if (ret == MESSAGE_PORT_ERROR_INVALID_PARAMETER) {
    LogAndReportError(
        InvalidValuesException("An input parameter contains an invalid value."), out,
        ("message_port_check_trusted_remote_port error: %d (%s)", ret, get_error_message(ret)));
  } else if (ret == MESSAGE_PORT_ERROR_OUT_OF_MEMORY) {
    LogAndReportError(
        UnknownException("Out of memory."), out,
        ("message_port_check_trusted_remote_port error: %d (%s)", ret, get_error_message(ret)));
  } else if (ret == MESSAGE_PORT_ERROR_IO_ERROR) {
    // IO error means that remote port does not exist
    LogAndReportError(
        NotFoundException("The port of the target application is not found"), out,
        ("message_port_check_trusted_remote_port error: %d (%s)", ret, get_error_message(ret)));
  } else if (ret == MESSAGE_PORT_ERROR_PORT_NOT_FOUND) {
    LogAndReportError(
        NotFoundException("The port of the target application is not found"), out,
        ("message_port_check_trusted_remote_port error: %d (%s)", ret, get_error_message(ret)));
  } else if (ret == MESSAGE_PORT_ERROR_CERTIFICATE_NOT_MATCH) {
    LogAndReportError(
        UnknownException("The remote application is not signed with the same certificate"), out,
        ("message_port_check_trusted_remote_port error: %d (%s)", ret, get_error_message(ret)));
  } else {
    LogAndReportError(
        UnknownException("Unknown Error"), out,
        ("message_port_check_trusted_remote_port error: %d (%s)", ret, get_error_message(ret)));
  }
}

void MessageportInstance::RemoteMessagePortSendmessage
  (const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  const std::string& appId = args.get("appId").get<std::string>();
  const std::string& message_port_name =
    args.get("messagePortName").get<std::string>();
  std::vector<picojson::value> data = args.get("data").get<picojson::array>();

  long local_port_id =
    static_cast<long>(args.get("local_port_id").get<double>());
  bool trusted = args.get("trusted").get<bool>();

  int result;

  bundle* bundle = bundle_create();
  for (picojson::value::array::iterator it = data.begin();
       it != data.end(); ++it) {
    const std::string& valueType = (*it).get("valueType").get<std::string>();
    if ("stringValueType" == valueType) {
      LoggerD("value is string");
      bundle_add(bundle, (*it).get("key").to_str().c_str(),
        (*it).get("value").to_str().c_str());
    } else if ("stringArrayValueType" == valueType) {
      LoggerD("value is string array");
      std::vector<picojson::value> value_array = (*it).get("value").get<picojson::array>();
      const size_t size = value_array.size();
      const char** arr = new const char*[size];
      size_t i = 0;

      for (auto iter = value_array.begin(); iter != value_array.end(); ++iter, ++i) {
        arr[i] = iter->to_str().c_str();
      }

      bundle_add_str_array(bundle, (*it).get("key").to_str().c_str(), arr, size);
      delete[] arr;
    } else if ("byteStreamValueType" == valueType) {
      LoggerD("value is byte stream");
      std::vector<picojson::value> value_array = (*it).get("value").get<picojson::array>();
      const size_t size = value_array.size();
      unsigned char* arr = new unsigned char[size];
      size_t i = 0;

      for (auto iter = value_array.begin(); iter != value_array.end(); ++iter, ++i) {
        arr[i] = static_cast<unsigned char>((*iter).get<double>());
      }
      bundle_add_byte(bundle, (*it).get("key").to_str().c_str(), arr, size);
      delete[] arr;
    } else if ("byteStreamArrayValueType" == valueType) {
      LoggerD("value is byte stream array");
      std::vector<picojson::value> byteStreamArray = (*it).get("value").get<picojson::array>();
      const size_t size = byteStreamArray.size();
      bundle_add_byte_array(bundle, (*it).get("key").to_str().c_str(), nullptr, size);
      size_t i = 0;

      for (auto iter = byteStreamArray.begin(); iter != byteStreamArray.end(); ++iter, ++i) {
        std::vector<picojson::value> byteStream = (*iter).get<picojson::array>();
        const size_t streamSize = byteStream.size();
        unsigned char* arr = new unsigned char[streamSize];
        size_t j = 0;

        for (auto byteIter = byteStream.begin(); byteIter != byteStream.end(); ++byteIter, ++j) {
          arr[j] = static_cast<unsigned char>((*byteIter).get<double>());
        }

        bundle_set_byte_array_element(bundle, (*it).get("key").to_str().c_str(), i, arr, streamSize);
        delete[] arr;
      }
    }
  }

  LoggerD("%s to %s", trusted ?
    "Sending trusted messages" : "Sending normal messages",
    message_port_name.c_str());

  if (trusted) {
    if (local_port_id < 0) {
          result = message_port_send_trusted_message
            (appId.c_str(), message_port_name.c_str(), bundle);
        } else {
          result = message_port_send_trusted_message_with_local_port
            (appId.c_str(), message_port_name.c_str(), bundle, local_port_id);
        }
  } else {
    if (local_port_id < 0) {
      result = message_port_send_message
        (appId.c_str(), message_port_name.c_str(), bundle);
      LoggerD("-----------%s  & %s---------------",
        appId.c_str(), message_port_name.c_str());
    } else {
      result = message_port_send_message_with_local_port
        (appId.c_str(), message_port_name.c_str(), bundle, local_port_id);
    }
  }

  bundle_free(bundle);

  if (result == MESSAGE_PORT_ERROR_NONE) {
    ReportSuccess(out);
  } else if (result == MESSAGE_PORT_ERROR_INVALID_PARAMETER) {
    LogAndReportError(
        InvalidValuesException("An input parameter contains an invalid value."), out,
        ("An input parameter contains an invalid value %d (%s)",
            result, get_error_message(result)));
  } else if (result == MESSAGE_PORT_ERROR_PORT_NOT_FOUND) {
    LogAndReportError(
        NotFoundException("The port of the target application is not found"), out,
        ("The port of the target application is not found: %d (%s)",
            result, get_error_message(result)));
  } else if (result == MESSAGE_PORT_ERROR_MAX_EXCEEDED) {
    LogAndReportError(
        QuotaExceededException("The size of message has exceeded the maximum limit."), out,
        ("The size of message has exceeded the maximum limit: %d (%s)",
            result, get_error_message(result)));
  } else if (result == MESSAGE_PORT_ERROR_RESOURCE_UNAVAILABLE) {
    LogAndReportError(
        UnknownException("A resource is temporarily unavailable"), out,
        ("A resource is temporarily unavailable: %d (%s)",
            result, get_error_message(result)));
  } else if (result == MESSAGE_PORT_ERROR_OUT_OF_MEMORY) {
    LogAndReportError(
        UnknownException("Out of memory."), out,
        ("Out of memory: %d (%s)", result, get_error_message(result)));
  } else if (result == MESSAGE_PORT_ERROR_IO_ERROR) {
    LogAndReportError(
        UnknownException("Internal I/O error ocurred."), out,
        ("Internal I/O error ocurred: %d (%s)",
            result, get_error_message(result)));
  } else if (result == MESSAGE_PORT_ERROR_CERTIFICATE_NOT_MATCH) {
    LogAndReportError(
        UnknownException("The remote application is not signed with the same certificate") , out,
        ("The remote application is not signed with the same certificate: %d (%s)",
            result, get_error_message(result)));
  } else {
    LogAndReportError(
        UnknownException("Unknown Exception"), out,
        ("Unknown Exception: %d (%s)", result, get_error_message(result)));
  }
}


#undef CHECK_EXIST

}  // namespace messageport
}  // namespace extension
