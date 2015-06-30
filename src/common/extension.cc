// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/extension.h"

#include <assert.h>
#include <iostream>
#include <vector>
#include <string>
#include <map>

#include "common/logger.h"

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

bool InitializeInterfaces(XW_GetInterface get_interface) {
  LoggerD("Enter");
  static bool initialized = false;

  if (!initialized) {
    g_core = reinterpret_cast<const XW_CoreInterface*>(
        get_interface(XW_CORE_INTERFACE));
    if (!g_core) {
      std::cerr << "Can't initialize extension: error getting Core interface.\n";
      return false;
    }

    g_messaging = reinterpret_cast<const XW_MessagingInterface*>(
        get_interface(XW_MESSAGING_INTERFACE));
    if (!g_messaging) {
      std::cerr <<
          "Can't initialize extension: error getting Messaging interface.\n";
      return false;
    }

    g_sync_messaging =
        reinterpret_cast<const XW_Internal_SyncMessagingInterface*>(
            get_interface(XW_INTERNAL_SYNC_MESSAGING_INTERFACE));
    if (!g_sync_messaging) {
      std::cerr <<
          "Can't initialize extension: error getting SyncMessaging interface.\n";
      return false;
    }

    g_entry_points = reinterpret_cast<const XW_Internal_EntryPointsInterface*>(
        get_interface(XW_INTERNAL_ENTRY_POINTS_INTERFACE));
    if (!g_entry_points) {
      std::cerr << "NOTE: Entry points interface not available in this version "
                << "of Crosswalk, ignoring entry point data for extensions.\n";
    }

    g_runtime = reinterpret_cast<const XW_Internal_RuntimeInterface*>(
        get_interface(XW_INTERNAL_RUNTIME_INTERFACE));
    if (!g_runtime) {
      std::cerr << "NOTE: runtime interface not available in this version "
                << "of Crosswalk, ignoring runtime variables for extensions.\n";
    }

    g_permission = reinterpret_cast<const XW_Internal_PermissionsInterface*>(
        get_interface(XW_INTERNAL_PERMISSIONS_INTERFACE));
    if (!g_permission) {
      std::cerr << "NOTE: permission interface not available in this version "
        << "of Crosswalk, ignoring permission for extensions.\n";
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
  // crosswalk has used the double quote for the app_id from the first.
  // the n-wrt (new wrt) is using the double quote also.
  // but that's wrt and wrt-service's bug.
  // To keep compatibilities, two case of formats should be considered in webapi-plugins.
  // removing double quote to keep compatibilities with new and old wrt
  std::string value = std::string(res.data());
  if (0 == strncmp(var_name, "app_id", 6) && value.find('"', 0) != std::string::npos
      && value.find('"', value.size() -1) != std::string::npos) {

    value = value.erase(0, 1);
    value = value.erase(value.size() - 1, 1);
  }

  return value;
}

// static
void Extension::OnInstanceCreated(XW_Instance xw_instance, Instance* instance) {
  LoggerD("Enter");
  assert(!g_core->GetInstanceData(xw_instance));
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

//static
int32_t Extension::XW_Initialize(XW_Extension extension,
                                 XW_GetInterface get_interface,
                                 XW_Initialize_Func initialize,
                                 XW_CreatedInstanceCallback created_instance,
                                 XW_ShutdownCallback shutdown) {
  LoggerD("Enter");
  assert(extension);

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
  return XW_OK;
}


Instance::Instance() :
    xw_instance_(0)
{
  LoggerD("Enter");
}

Instance::~Instance() {
  LoggerD("Enter");
  assert(xw_instance_ == 0);
}

void Instance::PostMessage(const char* msg) {
  LoggerD("Enter");
  if (!xw_instance_) {
    std::cerr << "Ignoring PostMessage() in the constructor or after the "
              << "instance was destroyed.";
    return;
  }
  g_messaging->PostMessage(xw_instance_, msg);
}

void Instance::SendSyncReply(const char* reply) {
  LoggerD("Enter");
  if (!xw_instance_) {
    std::cerr << "Ignoring SendSyncReply() in the constructor or after the "
              << "instance was destroyed.";
    return;
  }
  g_sync_messaging->SetSyncReply(xw_instance_, reply);
}


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
      std::cerr << "Ignoring message. " << err;
      return;
    }

    if (!value.is<picojson::object>()) {
      std::cerr << "Ignoring message. It is not an object.";
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
  std::cerr << "Exception: " << ex.message();
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

}  // namespace common
