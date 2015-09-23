// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/extension.h"

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include "common/logger.h"
#include "common/assert.h"

// This function is hidden, because each plugin needs to have own implementation.
__attribute__ ((visibility ("hidden"))) common::Extension* CreateExtension() {
  common::Extension* e = new common::Extension();
  e->SetExtensionName("common");
  return e;
}

namespace {

// this variable is valid only during Extension::XW_Initialize() call
// do not use !!!
// it's here, so we don't have to modify the interface of CreateExtension(), Extension(), etc.
XW_Extension g_xw_extension_ = 0;

const XW_CoreInterface* g_core = NULL;
const XW_MessagingInterface* g_messaging = NULL;
const XW_Internal_SyncMessagingInterface* g_sync_messaging = NULL;
const XW_Internal_EntryPointsInterface* g_entry_points = NULL;
const XW_Internal_RuntimeInterface* g_runtime = NULL;
const XW_Internal_PermissionsInterface* g_permission = NULL;
const XW_Internal_DataInterface* g_data = NULL;

bool InitializeInterfaces(XW_GetInterface get_interface) {
  LoggerD("Enter");
  static bool initialized = false;

  if (!initialized) {
    g_core = reinterpret_cast<const XW_CoreInterface*>(
        get_interface(XW_CORE_INTERFACE));
    if (!g_core) {
      LoggerE("Can't initialize extension: error getting Core interface.");
      return false;
    }

    g_messaging = reinterpret_cast<const XW_MessagingInterface*>(
        get_interface(XW_MESSAGING_INTERFACE));
    if (!g_messaging) {
      LoggerE("Can't initialize extension: error getting Messaging interface.");
      return false;
    }

    g_sync_messaging =
        reinterpret_cast<const XW_Internal_SyncMessagingInterface*>(
            get_interface(XW_INTERNAL_SYNC_MESSAGING_INTERFACE));
    if (!g_sync_messaging) {
      LoggerE("Can't initialize extension: error getting SyncMessaging interface.");
      return false;
    }

    g_entry_points = reinterpret_cast<const XW_Internal_EntryPointsInterface*>(
        get_interface(XW_INTERNAL_ENTRY_POINTS_INTERFACE));
    if (!g_entry_points) {
      LoggerE("NOTE: Entry points interface not available in this version "
              "of runtime, ignoring entry point data for extensions.");
    }

    g_runtime = reinterpret_cast<const XW_Internal_RuntimeInterface*>(
        get_interface(XW_INTERNAL_RUNTIME_INTERFACE));
    if (!g_runtime) {
      LoggerE("NOTE: runtime interface not available in this version "
              "of runtime, ignoring runtime variables for extensions.");
    }

    g_permission = reinterpret_cast<const XW_Internal_PermissionsInterface*>(
        get_interface(XW_INTERNAL_PERMISSIONS_INTERFACE));
    if (!g_permission) {
      LoggerE("NOTE: permission interface not available in this version "
              "of runtime, ignoring permission for extensions.");
    }

    g_data = reinterpret_cast<const XW_Internal_DataInterface*>(
        get_interface(XW_INTERNAL_DATA_INTERFACE));
    if (!g_data) {
      LoggerE("NOTE: data interface not available in this version of "
              "runtime, ignoring data for extensions.");
    }

    initialized = true;
  }

  return true;
}

}  // namespace

namespace common {

Extension::Extension() : xw_extension_(g_xw_extension_) {
  LoggerD("Enter");
}

Extension::~Extension() {
  LoggerD("Enter");
}

void Extension::SetExtensionName(const char* name) {
  LoggerD("Enter");
  g_core->SetExtensionName(xw_extension_, name);
}

void Extension::SetJavaScriptAPI(const char* api) {
  LoggerD("Enter");
  g_core->SetJavaScriptAPI(xw_extension_, api);
}

void Extension::SetExtraJSEntryPoints(const char** entry_points) {
  LoggerD("Enter");
  if (g_entry_points)
    g_entry_points->SetExtraJSEntryPoints(xw_extension_, entry_points);
}

bool Extension::RegisterPermissions(const char* perm_table) {
  LoggerD("Enter");
  if (g_permission)
    return g_permission->RegisterPermissions(xw_extension_, perm_table);
  return false;
}

bool Extension::CheckAPIAccessControl(const char* api_name) {
  LoggerD("Enter");
  if (g_permission)
    return g_permission->CheckAPIAccessControl(xw_extension_, api_name);
  return false;
}

Instance* Extension::CreateInstance() {
  LoggerD("Enter");
  return NULL;
}

std::string Extension::GetRuntimeVariable(const char* var_name, unsigned len) {
  LoggerD("Enter");
  if (!g_runtime)
    return "";

  std::vector<char> res(len + 1, 0);
  g_runtime->GetRuntimeVariableString(xw_extension_, var_name, &res[0], len);
  return std::string(res.begin(), res.end());
}

// static
void Extension::OnInstanceCreated(XW_Instance xw_instance, Instance* instance) {
  LoggerD("Enter");
  Assert(!g_core->GetInstanceData(xw_instance));
  if (!instance)
    return;
  instance->xw_instance_ = xw_instance;
  g_core->SetInstanceData(xw_instance, instance);
  instance->Initialize();
}

// static
void Extension::OnInstanceDestroyed(XW_Instance xw_instance) {
  LoggerD("Enter");
  Instance* instance =
      reinterpret_cast<Instance*>(g_core->GetInstanceData(xw_instance));
  if (!instance)
    return;
  instance->xw_instance_ = 0;
  delete instance;
}

// static
void Extension::HandleMessage(XW_Instance xw_instance, const char* msg) {
  LoggerD("Enter");
  Instance* instance =
      reinterpret_cast<Instance*>(g_core->GetInstanceData(xw_instance));
  if (!instance)
    return;
  instance->HandleMessage(msg);
}

// static
void Extension::HandleSyncMessage(XW_Instance xw_instance, const char* msg) {
  LoggerD("Enter");
  Instance* instance =
      reinterpret_cast<Instance*>(g_core->GetInstanceData(xw_instance));
  if (!instance)
    return;
  instance->HandleSyncMessage(msg);
}

// static
void Extension::HandleData(XW_Instance xw_instance, const char* msg,
                           uint8_t* buffer, size_t len) {
  LoggerD("Enter");
  Instance* instance =
      reinterpret_cast<Instance*>(g_core->GetInstanceData(xw_instance));
  if (!instance)
    return;
  instance->HandleData(msg, buffer, len);
}

// static
void Extension::HandleSyncData(XW_Instance xw_instance, const char* msg,
                               uint8_t* buffer, size_t len) {
  LoggerD("Enter");
  Instance* instance =
      reinterpret_cast<Instance*>(g_core->GetInstanceData(xw_instance));
  if (!instance)
    return;
  instance->HandleSyncData(msg, buffer, len);
}

//static
int32_t Extension::XW_Initialize(XW_Extension extension,
                                 XW_GetInterface get_interface,
                                 XW_Initialize_Func initialize,
                                 XW_CreatedInstanceCallback created_instance,
                                 XW_ShutdownCallback shutdown) {
  LoggerD("Enter");
  Assert(extension);

  if (!InitializeInterfaces(get_interface)) {
    return XW_ERROR;
  }

  g_xw_extension_ = extension;

  if (XW_ERROR == initialize(extension, get_interface)) {
    return XW_ERROR;
  }

  g_xw_extension_ = 0;

  using common::Extension;
  g_core->RegisterShutdownCallback(extension, shutdown);
  g_core->RegisterInstanceCallbacks(extension, created_instance,
                                    Extension::OnInstanceDestroyed);
  g_messaging->Register(extension, Extension::HandleMessage);
  g_sync_messaging->Register(extension, Extension::HandleSyncMessage);

  if (g_data) {
    g_data->RegisterSync(extension, Extension::HandleSyncData);
    g_data->RegisterAsync(extension, Extension::HandleData);
  }

  return XW_OK;
}

std::unordered_set<Instance*> Instance::all_instances_;

Instance::Instance() :
    xw_instance_(0)
{
  LoggerD("Enter");
  {
    all_instances_.insert(this);
  }
}

Instance::~Instance() {
  LoggerD("Enter");
  {
    all_instances_.erase(this);
  }
  Assert(xw_instance_ == 0);
}

void Instance::PostMessage(Instance* that, const char* msg) {
  LoggerD("Enter");
  if (that && all_instances_.end() != all_instances_.find(that)) {
    that->PostMessage(msg);
  } else {
    LoggerE("Trying to post message to non-existing instance: [%p], ignoring",
            that);
  }
}

void Instance::PostMessage(const char* msg) {
  LoggerD("Enter");
  if (!xw_instance_) {
    LoggerE("Ignoring PostMessage() in the constructor or after the "
            "instance was destroyed.");
    return;
  }
  g_messaging->PostMessage(xw_instance_, msg);
}


void Instance::PostData(const char* msg, uint8_t* buffer, size_t len) {
  LoggerD("Enter");
  if (!xw_instance_) {
    LoggerE("Ignoring PostData() in the constructor or after the "
            "instance was destroyed.");
    return;
  }
  g_data->PostData(xw_instance_, msg, buffer, len);
}


void Instance::SendSyncReply(const char* reply) {
  LoggerD("Enter");
  if (!xw_instance_) {
    LoggerE("Ignoring SendSyncReply() in the constructor or after the "
            "instance was destroyed.");
    return;
  }
  g_sync_messaging->SetSyncReply(xw_instance_, reply);
}

void Instance::SendSyncReply(const char* reply, uint8_t* buffer, size_t len) {
  LoggerD("Enter");
  if (!xw_instance_) {
    LoggerE("Ignoring SendSyncReply() in the constructor or after the "
            "instance was destroyed.");
    return;
  }
  g_data->SetSyncReply(xw_instance_, reply, buffer, len);
}


// ParsedInstance //////////////////////////////////////////////////////////////

ParsedInstance::ParsedInstance() {
  LoggerD("Enter");
}

ParsedInstance::~ParsedInstance() {
  LoggerD("Enter");
}

void ParsedInstance::RegisterHandler(const std::string& name, const NativeHandler& func) {
  LoggerD("Enter");
  handler_map_.insert(std::make_pair(name, func));
}

void ParsedInstance::RegisterSyncHandler(const std::string& name, const NativeHandler& func) {
  LoggerD("Enter");
  handler_map_.insert(std::make_pair("#SYNC#" + name, func));
}

void ParsedInstance::ReportSuccess(picojson::object& out) {
  LoggerD("Enter");
  tools::ReportSuccess(out);
}

void ParsedInstance::ReportSuccess(const picojson::value& result, picojson::object& out) {
  LoggerD("Enter");
  tools::ReportSuccess(result, out);
}

void ParsedInstance::ReportError(picojson::object& out) {
  LoggerD("Enter");
  tools::ReportError(out);
}

void ParsedInstance::ReportError(const PlatformException& ex, picojson::object& out) {
  LoggerD("Enter");
  tools::ReportError(ex, out);
}

void ParsedInstance::ReportError(const PlatformResult& error, picojson::object* out) {
  LoggerD("Enter");
  tools::ReportError(error, out);
}

void ParsedInstance::HandleMessage(const char* msg) {
  LoggerD("Enter");
  HandleMessage(msg, false);
}

void ParsedInstance::HandleSyncMessage(const char* msg) {
  LoggerD("Enter");
  HandleMessage(msg, true);
}

void ParsedInstance::HandleMessage(const char* msg, bool is_sync) {
  LoggerD("Enter");
  try {
    picojson::value value;
    std::string err;
    picojson::parse(value, msg, msg + strlen(msg), &err);
    if (!err.empty()) {
      LoggerE("Ignoring message, error: %s", err.c_str());
      return;
    }

    if (!value.is<picojson::object>()) {
      LoggerE("Ignoring message. It is not an object.");
      return;
    }

    std::string cmd = (is_sync ? "#SYNC#" : "") + value.get("cmd").to_str();

    auto it = handler_map_.find(cmd);
    if (handler_map_.end() == it) {
      throw UnknownException("Unknown command.");
    }

    NativeHandler func = it->second;

    // check for args in JSON message
    const picojson::value& args = value.get("args");
    if (!args.is<picojson::object>()) {
      throw InvalidValuesException("No \"args\" field in message");
    }

    picojson::value result = picojson::value(picojson::object());
    func(args, result.get<picojson::object>());

    if (is_sync)
      SendSyncReply(result.serialize().c_str());

  } catch (const PlatformException& e) {
    return HandleException(e);
  } catch (const PlatformException* e) {
    return HandleException(*e);
  } catch (const std::exception& e) {
    return HandleException(UnknownException(e.what()));
  } catch (const std::exception* e) {
    return HandleException(UnknownException(e->what()));
  } catch (...) {
    return HandleException(UnknownException("Unknown exception"));
  }
}

void ParsedInstance::HandleException(const PlatformException& ex) {
  LoggerD("Enter");
  LoggerE("Exception: %s", ex.message().c_str());
  picojson::value result = picojson::value(picojson::object());
  ReportError(ex, result.get<picojson::object>());
  SendSyncReply(result.serialize().c_str());
}

void ParsedInstance::HandleError(const PlatformResult& e) {
  LoggerE("Error: %s", static_cast<int>(e.error_code()));
  picojson::value result = picojson::value(picojson::object());
  ReportError(e, &result.get<picojson::object>());
  SendSyncReply(result.serialize().c_str());
}

// ParsedDataStruct ////////////////////////////////////////////////////////////

ParsedDataStruct::ParsedDataStruct()
    : buffer_(NULL), buffer_len_(0) {
}

ParsedDataStruct::ParsedDataStruct(uint8_t* buffer, size_t len)
    : buffer_(buffer), buffer_len_(len) {
}

void ParsedDataStruct::SetBuffer(uint8_t* buffer, size_t len) {
  buffer_ = buffer;
  buffer_len_ = len;
}

ParsedDataRequest::ParsedDataRequest(uint8_t* buffer, size_t len)
    : ParsedDataStruct(buffer, len) {
}

bool ParsedDataRequest::Parse(const char* msg) {
  std::string err;
  picojson::parse(value_, msg, msg + strlen(msg), &err);
  if (!err.empty()) {
    LoggerE("Ignoring message, error: %s", err.c_str());
    return false;
  }

  if (!value_.is<picojson::object>()) {
    LoggerE("Ignoring message. It is not an object.");
    return false;
  }

  return true;
}

std::string ParsedDataRequest::cmd() const {
  return value_.get("cmd").to_str();
}

const picojson::value& ParsedDataRequest::args() const {
  return value_.get("args");
}

ParsedDataResponse::ParsedDataResponse()
    : value_(picojson::value(picojson::object())) {
}

picojson::object& ParsedDataResponse::object() {
  return value_.get<picojson::object>();
}

// ParsedDataInstance //////////////////////////////////////////////////////////

void ParsedDataInstance::RegisterHandler(const std::string& name,
                                         const NativeDataHandler& func) {
  LoggerD("Enter");
  data_handler_map_.insert(std::make_pair(name, func));
}

void ParsedDataInstance::RegisterSyncHandler(const std::string& name,
                                             const NativeDataHandler& func) {
  LoggerD("Enter");
  data_handler_map_.insert(std::make_pair("#SYNC#" + name, func));
}

void ParsedDataInstance::HandleData(const char* msg,
                                    uint8_t* buffer, size_t len) {
  LoggerD("Enter");
  HandleData(msg, buffer, len, false);
}

void ParsedDataInstance::HandleSyncData(const char* msg,
                                        uint8_t* buffer, size_t len) {
  LoggerD("Enter");
  HandleData(msg, buffer, len, true);
}

void ParsedDataInstance::HandleData(const char* msg,
                                    uint8_t* buffer, size_t len, bool is_sync) {
  LoggerD("Enter");
  try {
    ParsedDataRequest request(buffer, len);
    if (!request.Parse(msg)) {
      return;
    }

    // check for args in JSON message
    const picojson::value& args = request.args();
    if (!args.is<picojson::object>()) {
      throw InvalidValuesException("No \"args\" field in message");
    }

    std::string cmd = (is_sync ? "#SYNC#" : "") + request.cmd();
    auto it = data_handler_map_.find(cmd);
    if (data_handler_map_.end() == it) {
      throw UnknownException("Unknown command.");
    }
    NativeDataHandler func = it->second;

    ParsedDataResponse response;
    func(request, response);
    if (is_sync) {
      SendSyncReply(response.value().serialize().c_str(),
                    response.buffer(), response.buffer_length());
    }
  } catch (const PlatformException& e) {
    return HandleException(e);
  } catch (const PlatformException* e) {
    return HandleException(*e);
  } catch (const std::exception& e) {
    return HandleException(UnknownException(e.what()));
  } catch (const std::exception* e) {
    return HandleException(UnknownException(e->what()));
  } catch (...) {
    return HandleException(UnknownException("Unknown exception"));
  }
}



}  // namespace common
