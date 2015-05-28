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
#include <privilege_checker.h>
#elif PRIVILEGE_USE_CYNARA
#include <unistd.h>

#include <cynara/cynara-client.h>
#include <sys/smack.h>
#endif

#include "common/logger.h"
#include "common/scope_exit.h"

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
  return std::string(res.begin(), res.end());
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

namespace tools {
void ReportSuccess(picojson::object& out) {
  LoggerD("Enter");
  out.insert(std::make_pair("status", picojson::value("success")));
}

void ReportSuccess(const picojson::value& result, picojson::object& out) {
  LoggerD("Enter");
  out.insert(std::make_pair("status", picojson::value("success")));
  out.insert(std::make_pair("result", result));
}

void ReportError(picojson::object& out) {
  LoggerD("Enter");
  out.insert(std::make_pair("status", picojson::value("error")));
}

void ReportError(const PlatformException& ex, picojson::object& out) {
  LoggerD("Enter");
  out.insert(std::make_pair("status", picojson::value("error")));
  out.insert(std::make_pair("error", ex.ToJSON()));
}

void ReportError(const PlatformResult& error, picojson::object* out) {
  LoggerD("Enter");
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
    const std::string app_id = common::GetCurrentExtension()->GetRuntimeVariable("app_id", 64);
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
    LoggerD("Enter");
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
  AccessControlImpl() {
    LoggerD("Privilege access checked using ACE.");
  }

  ~AccessControlImpl() {
  }

  bool CheckAccess(const std::vector<std::string>& privileges) {
    LoggerD("Enter");
    int ret = 0;
    for (size_t i = 0; i < privileges.size(); ++i) {
      ret = privilege_checker_check_privilege(privileges[i].c_str());
      if (PRIVILEGE_CHECKER_ERR_NONE != ret) {
        return false;
      }
    }
    return true;
  }
};

#elif PRIVILEGE_USE_CYNARA

class AccessControlImpl {
 public:
  AccessControlImpl() : cynara_(nullptr) {
    LoggerD("Privilege access checked using Cynara.");

    char* smack_label = nullptr;
    int ret = smack_new_label_from_self(&smack_label);

    if (0 == ret && nullptr != smack_label) {
      auto uid = getuid();

      SLoggerD("uid: [%u]", uid);
      SLoggerD("smack label: [%s]", smack_label);

      uid_ = std::to_string(uid);
      smack_label_ = smack_label;

      free(smack_label);
    } else {
      LoggerE("Failed to get smack label");
      return;
    }

    ret = cynara_initialize(&cynara_, nullptr);
    if (CYNARA_API_SUCCESS != ret) {
      LoggerE("Failed to initialize Cynara");
      cynara_ = nullptr;
    }
  }

  ~AccessControlImpl() {
    if (cynara_) {
      auto ret = cynara_finish(cynara_);
      if (CYNARA_API_SUCCESS != ret) {
        LoggerE("Failed to finalize Cynara");
      }
      cynara_ = nullptr;
    }
  }

  bool CheckAccess(const std::vector<std::string>& privileges) {
    if (cynara_) {
      for (const auto& privilege : privileges) {
        if (CYNARA_API_ACCESS_ALLOWED != cynara_simple_check(cynara_,  // p_cynara
                                                             smack_label_.c_str(),  // client
                                                             "", // client_session
                                                             uid_.c_str(),  // user
                                                             privilege.c_str()  // privilege
                                                             )) {
          return false;
        }
      }
      return true;
    } else {
      return false;
    }
  }

 private:
  cynara* cynara_;
  std::string uid_;
  std::string smack_label_;
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
  LoggerD("Enter");
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
