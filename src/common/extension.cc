// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/extension.h"

#include <assert.h>
#include <iostream>
#include <vector>
#include <string>
#include <map>

#include "common/logger.h"

namespace {

common::Extension* g_extension = NULL;
XW_Extension g_xw_extension = 0;

const XW_CoreInterface* g_core = NULL;
const XW_MessagingInterface* g_messaging = NULL;
const XW_Internal_SyncMessagingInterface* g_sync_messaging = NULL;
const XW_Internal_EntryPointsInterface* g_entry_points = NULL;
const XW_Internal_RuntimeInterface* g_runtime = NULL;
const XW_Internal_PermissionsInterface* g_permission = NULL;

bool InitializeInterfaces(XW_GetInterface get_interface) {
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

  return true;
}

}  // namespace

int32_t XW_Initialize(XW_Extension extension, XW_GetInterface get_interface) {
  assert(extension);
  g_xw_extension = extension;

  if (!InitializeInterfaces(get_interface))
    return XW_ERROR;

  g_extension = CreateExtension();
  if (!g_extension) {
    std::cerr << "Can't initialize extension: "
              << "create extension returned NULL.\n";
    return XW_ERROR;
  }

  using common::Extension;
  g_core->RegisterShutdownCallback(g_xw_extension, Extension::OnShutdown);
  g_core->RegisterInstanceCallbacks(
      g_xw_extension, Extension::OnInstanceCreated,
      Extension::OnInstanceDestroyed);
  g_messaging->Register(g_xw_extension, Extension::HandleMessage);
  g_sync_messaging->Register(g_xw_extension, Extension::HandleSyncMessage);
  return XW_OK;
}

namespace common {

Extension::Extension() {}

Extension::~Extension() {}

void Extension::SetExtensionName(const char* name) {
  g_core->SetExtensionName(g_xw_extension, name);
}

void Extension::SetJavaScriptAPI(const char* api) {
  g_core->SetJavaScriptAPI(g_xw_extension, api);
}

void Extension::SetExtraJSEntryPoints(const char** entry_points) {
  if (g_entry_points)
    g_entry_points->SetExtraJSEntryPoints(g_xw_extension, entry_points);
}

bool Extension::RegisterPermissions(const char* perm_table) {
  if (g_permission)
    return g_permission->RegisterPermissions(g_xw_extension, perm_table);
  return false;
}

bool Extension::CheckAPIAccessControl(const char* api_name) {
  if (g_permission)
    return g_permission->CheckAPIAccessControl(g_xw_extension, api_name);
  return false;
}

Instance* Extension::CreateInstance() {
  return NULL;
}

std::string Extension::GetRuntimeVariable(const char* var_name, unsigned len) {
  if (!g_runtime)
    return "";

  std::vector<char> res(len + 1, 0);
  g_runtime->GetRuntimeVariableString(g_xw_extension, var_name, &res[0], len);
  return std::string(res.begin(), res.end());
}

// static
void Extension::OnShutdown(XW_Extension) {
  delete g_extension;
  g_extension = NULL;
}

// static
void Extension::OnInstanceCreated(XW_Instance xw_instance) {
  assert(!g_core->GetInstanceData(xw_instance));
  Instance* instance = g_extension->CreateInstance();
  if (!instance)
    return;
  instance->xw_instance_ = xw_instance;
  g_core->SetInstanceData(xw_instance, instance);
  instance->Initialize();
}

// static
void Extension::OnInstanceDestroyed(XW_Instance xw_instance) {
  Instance* instance =
      reinterpret_cast<Instance*>(g_core->GetInstanceData(xw_instance));
  if (!instance)
    return;
  instance->xw_instance_ = 0;
  delete instance;
}

// static
void Extension::HandleMessage(XW_Instance xw_instance, const char* msg) {
  Instance* instance =
      reinterpret_cast<Instance*>(g_core->GetInstanceData(xw_instance));
  if (!instance)
    return;
  instance->HandleMessage(msg);
}

// static
void Extension::HandleSyncMessage(XW_Instance xw_instance, const char* msg) {
  Instance* instance =
      reinterpret_cast<Instance*>(g_core->GetInstanceData(xw_instance));
  if (!instance)
    return;
  instance->HandleSyncMessage(msg);
}

Instance::Instance()
    : xw_instance_(0) {}

Instance::~Instance() {
  assert(xw_instance_ == 0);
}

void Instance::PostMessage(const char* msg) {
  if (!xw_instance_) {
    std::cerr << "Ignoring PostMessage() in the constructor or after the "
              << "instance was destroyed.";
    return;
  }
  g_messaging->PostMessage(xw_instance_, msg);
}

void Instance::SendSyncReply(const char* reply) {
  if (!xw_instance_) {
    std::cerr << "Ignoring SendSyncReply() in the constructor or after the "
              << "instance was destroyed.";
    return;
  }
  g_sync_messaging->SetSyncReply(xw_instance_, reply);
}


ParsedInstance::ParsedInstance() {
}

ParsedInstance::~ParsedInstance() {
}

void ParsedInstance::RegisterHandler(const std::string& name, const NativeHandler& func) {
  handler_map_.insert(std::make_pair(name, func));
}

void ParsedInstance::RegisterSyncHandler(const std::string& name, const NativeHandler& func) {
  handler_map_.insert(std::make_pair("#SYNC#" + name, func));
}

void ParsedInstance::ReportSuccess(picojson::object& out) {
  tools::ReportSuccess(out);
}

void ParsedInstance::ReportSuccess(const picojson::value& result, picojson::object& out) {
  tools::ReportSuccess(result, out);
}

void ParsedInstance::ReportError(picojson::object& out) {
  tools::ReportError(out);
}

void ParsedInstance::ReportError(const PlatformException& ex, picojson::object& out) {
  tools::ReportError(ex, out);
}

void ParsedInstance::ReportError(const PlatformResult& error, picojson::object* out) {
  tools::ReportError(error, out);
}

void ParsedInstance::HandleMessage(const char* msg) {
  HandleMessage(msg, false);
}

void ParsedInstance::HandleSyncMessage(const char* msg) {
  HandleMessage(msg, true);
}

void ParsedInstance::HandleMessage(const char* msg, bool is_sync) {
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

namespace tools {
void ReportSuccess(picojson::object& out) {
  out.insert(std::make_pair("status", picojson::value("success")));
}

void ReportSuccess(const picojson::value& result, picojson::object& out) {
  out.insert(std::make_pair("status", picojson::value("success")));
  out.insert(std::make_pair("result", result));
}

void ReportError(picojson::object& out) {
  out.insert(std::make_pair("status", picojson::value("error")));
}

void ReportError(const PlatformException& ex, picojson::object& out) {
  out.insert(std::make_pair("status", picojson::value("error")));
  out.insert(std::make_pair("error", ex.ToJSON()));
}

void ReportError(const PlatformResult& error, picojson::object* out) {
  out->insert(std::make_pair("status", picojson::value("error")));
  out->insert(std::make_pair("error", error.ToJSON()));
}
}  // namespace tools

}  // namespace common
