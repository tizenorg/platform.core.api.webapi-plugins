// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/extension.h"

#include <assert.h>
#include <iostream>
#include <vector>
#include <string>
#include <map>

#ifdef PRIVILEGE_USE_DB
#include <sqlite3.h>
#elif PRIVILEGE_USE_ACE
#include <sqlite3.h>
#include <dpl/wrt-dao-ro/WrtDatabase.h>
#include <ace_api_client.h>
#elif PRIVILEGE_USE_CYNARA
// TODO
#endif

#include "common/logger.h"
#include "common/scope_exit.h"

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

namespace {

#ifdef PRIVILEGE_USE_DB

class AccessControlImpl {
 public:
  AccessControlImpl()
      : initialized_(false) {
    LoggerD("Privilege access checked using DB.");

    const char* kWrtDBPath = "/opt/dbspace/.wrt.db";
    sqlite3* db = nullptr;

    int ret = sqlite3_open(kWrtDBPath, &db);
    if (SQLITE_OK != ret) {
      LoggerE("Failed to access WRT database");
      return;
    }

    const char* kQuery = "select name from WidgetFeature where app_id = "
                         "(select app_id from WidgetInfo where tizen_appid = ?)"
                         " and rejected = 0";
    const std::string app_id = common::Extension::GetRuntimeVariable("app_id", 64);
    sqlite3_stmt* stmt = nullptr;

    ret = sqlite3_prepare_v2(db, kQuery, -1, &stmt, nullptr);
    ret |= sqlite3_bind_text(stmt, 1, app_id.c_str(), -1, SQLITE_TRANSIENT);

    SCOPE_EXIT {
      sqlite3_finalize(stmt);
      sqlite3_close(db);
    };

    if (SQLITE_OK != ret) {
      LoggerE("Failed to query WRT database");
      return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
      const char* privilege = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
      SLoggerD("Granted: %s", privilege);
      granted_privileges_.push_back(privilege);
    }

    initialized_ = true;
  }

  ~AccessControlImpl() {}

  bool CheckAccess(const std::vector<std::string>& privileges) {
    if (!initialized_) {
      return false;
    }

    for (const auto& privilege : privileges) {
      if (std::find(granted_privileges_.begin(), granted_privileges_.end(), privilege) == granted_privileges_.end()) {
        return false;
      }
    }

    return true;
  }

 private:
  bool initialized_;
  std::vector<std::string> granted_privileges_;
};

#elif PRIVILEGE_USE_ACE

class AccessControlImpl {
 public:
  AccessControlImpl()
      : initialized_(false),
        widget_id_(-1) {
    LoggerD("Privilege access checked using ACE.");
    // get widget ID
    const char* kWrtDBPath = "/opt/dbspace/.wrt.db";
    sqlite3* db = nullptr;

    int ret = sqlite3_open(kWrtDBPath, &db);
    if (SQLITE_OK != ret) {
      LoggerE("Failed to access WRT database");
      return;
    }

    const char* kQuery = "select app_id from WidgetInfo where tizen_appid = ?";
    const std::string app_id = common::Extension::GetRuntimeVariable("app_id", 64);
    sqlite3_stmt* stmt = nullptr;

    ret = sqlite3_prepare_v2(db, kQuery, -1, &stmt, nullptr);
    ret |= sqlite3_bind_text(stmt, 1, app_id.c_str(), -1, SQLITE_TRANSIENT);

    SCOPE_EXIT {
      sqlite3_finalize(stmt);
      sqlite3_close(db);
    };

    if (SQLITE_OK != ret) {
      LoggerE("Failed to query WRT database");
      return;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
      widget_id_ = sqlite3_column_int(stmt, 0);
    } else {
      LoggerE("Application %s not found.", app_id.c_str());
      return;
    }

    // start ACE
    if (ACE_OK != ace_client_initialize(AlwaysDeny)) {
      LoggerE("Failed to initialize ACE.");
    } else {
      // in order to work, WrtDatabase needs to be bound to thread
      // which is going to check the WrtAccess
      WrtDB::WrtDatabase::attachToThreadRO();
      // set the session ID
      session_id_ = app_id + std::to_string(widget_id_);
      // we're ready
      initialized_ = true;
    }
  }

  ~AccessControlImpl() {
    if (initialized_) {
      WrtDB::WrtDatabase::detachFromThread();
      ace_client_shutdown();
    }
  }

  bool CheckAccess(const std::vector<std::string>& privileges) {
    if (!initialized_) {
      return false;
    }

    ace_check_result_t result = ACE_PRIVILEGE_DENIED;
    ace_request_t request;
    const auto count = privileges.size();

    request.session_id = const_cast<char*>(session_id_.c_str());
    request.widget_handle = widget_id_;
    request.feature_list = { 0, 0 };
    request.dev_cap_list = { 0, 0 };

    request.feature_list.count = count;
    request.feature_list.items = new char*[count];

    SCOPE_EXIT {
      delete [] request.feature_list.items;
    };

    for (size_t i = 0; i < count; ++i) {
      request.feature_list.items[i] = const_cast<char*>(privileges[i].c_str());
    }

    if (ACE_OK != ace_check_access_ex(&request, &result)) {
      LoggerE("Failed to check privilege.");
      return false;
    } else {
      return (result == ACE_ACCESS_GRANTED);
    }
  }

 private:
  static ace_return_t AlwaysDeny(ace_popup_t popup_type,
                                 const ace_resource_t resource_name,
                                 const ace_session_id_t session_id,
                                 const ace_param_list_t* param_list,
                                 ace_widget_handle_t handle,
                                 ace_bool_t* validation_result) {
    if (validation_result) {
      *validation_result = ACE_TRUE;
    }
    return ACE_OK;
  }

  bool initialized_;
  int widget_id_;
  std::string session_id_;
};

#elif PRIVILEGE_USE_CYNARA

class AccessControlImpl {
 public:
  AccessControlImpl() {
    LoggerD("Privilege access checked using Cynara.");
    // TODO
  }

  ~AccessControlImpl() {
    // TODO
  }

  bool CheckAccess(const std::vector<std::string>& privileges) {
    // TODO
    return false;
  }
};

#else

class AccessControlImpl {
 public:
  AccessControlImpl() {
    LoggerD("Privilege access - deny all.");
  }

  bool CheckAccess(const std::vector<std::string>& privileges) {
    return false;
  }
};

#endif

class AccessControl {
 public:
  static AccessControl& GetInstance() {
    static AccessControl instance;
    return instance;
  }

  bool CheckAccess(const std::string& privilege) {
    return CheckAccess(std::vector<std::string>{privilege});
  }

  bool CheckAccess(const std::vector<std::string>& privileges) {
    return impl_.CheckAccess(privileges);
  }

 private:
  AccessControl() {}
  ~AccessControl() {}
  AccessControlImpl impl_;
};

} // namespace

PlatformResult CheckAccess(const std::string& privilege) {
  return CheckAccess(std::vector<std::string>{privilege});
}

PlatformResult CheckAccess(const std::vector<std::string>& privileges) {
  if (AccessControl::GetInstance().CheckAccess(privileges)) {
    return PlatformResult(ErrorCode::NO_ERROR);
  } else {
    for (const auto& privilege : privileges) {
      LoggerD("Access to privilege: %s has been denied.", privilege.c_str());
    }
    return PlatformResult(ErrorCode::SECURITY_ERR, "Permission denied");
  }
}

}  // namespace tools

}  // namespace common
