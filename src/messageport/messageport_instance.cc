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

static void BundleJsonIterator(const char *k, const char *v, void *d) {
  LoggerD("Enter");
  picojson::value::array *array = static_cast<picojson::value::array *>(d);
  picojson::value::object o;
  o["key"] = picojson::value(k);
  o["value"] = picojson::value(v);
  array->push_back(picojson::value(o));
}

#define CHECK_EXIST(args, name, out) \
    if (!args.contains(name)) {\
      ReportError(TypeMismatchException(name" is required argument"), out);\
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

  bundle_iterate(message, BundleJsonIterator, &data);

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
        LoggerE("The input parameter contains an invalid value");
        ReportError(InvalidValuesException
          ("The input parameter contains an invalid value."), out);
        break;
      case MESSAGE_PORT_ERROR_OUT_OF_MEMORY:
        LoggerE("Out of memory");
        ReportError(UnknownException("Out of memory."), out);
        break;
      case MESSAGE_PORT_ERROR_IO_ERROR:
        LoggerE("Internal I/O error ocurred");
        ReportError(UnknownException("Internal I/O error ocurred."), out);
        break;
      default:
        LoggerE("Unknown Exception");
        ReportError(UnknownException("Unknown Exception"), out);
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
        LoggerE("The input parameter contains an invalid value");
        ReportError(InvalidValuesException
          ("The input parameter contains an invalid value."), out);
        break;
      case MESSAGE_PORT_ERROR_OUT_OF_MEMORY:
        LoggerE("Out of memory");
        ReportError(UnknownException("Out of memory."), out);
        break;
      case MESSAGE_PORT_ERROR_IO_ERROR:
        LoggerE("Internal I/O error ocurred");
        ReportError(UnknownException("Internal I/O error ocurred."), out);
        break;
      default:
        LoggerE("Unknown Exception");
        ReportError(UnknownException("Unknown Exception"), out);
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
  LoggerD("Error code: -0x%X", -ret);

  if (ret == MESSAGE_PORT_ERROR_NONE) {
    if (portCheck)  {
      ReportSuccess(out);
    } else {
      LoggerE("The port of the target application is not found");
      ReportError(
          NotFoundException("The port of the target application is not found"),
          out);
    }
  } else if (ret == MESSAGE_PORT_ERROR_INVALID_PARAMETER) {
    LoggerE("An input parameter contains an invalid value");
    ReportError(
        InvalidValuesException("An input parameter contains an invalid value."),
        out);
  } else if (ret == MESSAGE_PORT_ERROR_OUT_OF_MEMORY) {
    LoggerE("Out of memory");
    ReportError(UnknownException("Out of memory."), out);
  } else if (ret == MESSAGE_PORT_ERROR_IO_ERROR) {
    // IO error means that remote port does not exist
    LoggerE("The port of the target application is not found");
    ReportError(
        NotFoundException("The port of the target application is not found"),
        out);
  } else if (ret == MESSAGE_PORT_ERROR_PORT_NOT_FOUND) {
    ReportError(
        NotFoundException("The port of the target application is not found"),
        out);
  } else {
    LoggerE("Unknown Error");
    ReportError(UnknownException("Unknown Error"), out);
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
  LoggerD("Error code: -0x%X", -ret);

  if (ret == MESSAGE_PORT_ERROR_NONE) {
    if (portCheck) {
      ReportSuccess(out);
    } else {

      LoggerE("The port of the target application is not found");
      ReportError(
          NotFoundException("The port of the target application is not found"),
          out);
    }
  } else if (ret == MESSAGE_PORT_ERROR_INVALID_PARAMETER) {
    LoggerE("An input parameter contains an invalid value");
    ReportError(
        InvalidValuesException("An input parameter contains an invalid value."),
        out);
  } else if (ret == MESSAGE_PORT_ERROR_OUT_OF_MEMORY) {
    LoggerE("Out of memory");
    ReportError(UnknownException("Out of memory."), out);
  } else if (ret == MESSAGE_PORT_ERROR_IO_ERROR) {
    // IO error means that remote port does not exist
    LoggerE("The port of the target application is not found");
    ReportError(
        NotFoundException("The port of the target application is not found"),
        out);
  } else if (ret == MESSAGE_PORT_ERROR_PORT_NOT_FOUND) {
    ReportError(
        NotFoundException("The port of the target application is not found"),
        out);
  } else if (ret == MESSAGE_PORT_ERROR_CERTIFICATE_NOT_MATCH) {
    LoggerE("The remote application is not signed with the same certificate");
    ReportError(
        UnknownException(
            "The remote application is not signed with the same certificate"),
        out);
  } else {
    LoggerE("Unknown Error");
    ReportError(UnknownException("Unknown Error"), out);
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
      bundle_add(bundle, (*it).get("key").to_str().c_str(),
        (*it).get("value").to_str().c_str());
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

  if (result == MESSAGE_PORT_ERROR_NONE)
    ReportSuccess(out);
  else if (result == MESSAGE_PORT_ERROR_INVALID_PARAMETER)
    ReportError(InvalidValuesException
      ("An input parameter contains an invalid value.") , out);
  else if (result == MESSAGE_PORT_ERROR_PORT_NOT_FOUND)
    ReportError(NotFoundException
      ("The port of the target application is not found"), out);
  else if (result == MESSAGE_PORT_ERROR_MAX_EXCEEDED)
    ReportError(QuotaExceededException
      ("The size of message has exceeded the maximum limit."), out);
  else if (result == MESSAGE_PORT_ERROR_RESOURCE_UNAVAILABLE)
    ReportError(UnknownException("A resource is temporarily unavailable"), out);
  else if (result == MESSAGE_PORT_ERROR_OUT_OF_MEMORY)
    ReportError(UnknownException("Out of memory."), out);
  else if (result == MESSAGE_PORT_ERROR_IO_ERROR)
    ReportError(UnknownException("Internal I/O error ocurred."), out);
  else if (result == MESSAGE_PORT_ERROR_CERTIFICATE_NOT_MATCH)
    ReportError(UnknownException
      ("The remote application is not signed with the same certificate") , out);
  else
    ReportError(UnknownException("Unknown Exception"), out);
}


#undef CHECK_EXIST

}  // namespace messageport
}  // namespace extension
