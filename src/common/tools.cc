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

#include "common/tools.h"

#include <privilegemgr/privilege_manager.h>
#include <app_manager.h>
#include <pkgmgr-info.h>

#ifdef PRIVILEGE_USE_DB
#include <sqlite3.h>
#include "common/current_application.h"
#elif PRIVILEGE_USE_ACE
//#include <privilege_checker.h>
#elif PRIVILEGE_USE_CYNARA
#include <unistd.h>

#include <cynara/cynara-client.h>
#include <sys/smack.h>
#endif

#include "common/logger.h"
#include "common/scope_exit.h"

namespace common {
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
  LoggerE("Error without error code");
  out.insert(std::make_pair("status", picojson::value("error")));
}

void ReportError(const PlatformException& ex, picojson::object& out) {
  LoggerE("PlatformException: %s, message: %s", ex.name().c_str(), ex.message().c_str());
  out.insert(std::make_pair("status", picojson::value("error")));
  out.insert(std::make_pair("error", ex.ToJSON()));
}

void ReportError(const PlatformResult& error, picojson::object* out) {
  LoggerE("PlatformResult: %d, message: %s", error.error_code(), error.message().c_str());
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
    const std::string app_id = common::CurrentApplication::GetInstance().GetApplicationId();
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

    return true;
};

#elif PRIVILEGE_USE_CYNARA

class AccessControlImpl {
 public:
  AccessControlImpl() : cynara_(nullptr) {
    LoggerD("Privilege access checked using Cynara.");

    char* smack_label = nullptr;
    int len= smack_new_label_from_self(&smack_label);

    if (0 < len && nullptr != smack_label) {
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

    int ret = cynara_initialize(&cynara_, nullptr);
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

  std::string api_version;
  PlatformResult res = common::tools::GetPkgApiVersion(&api_version);
  if (res.IsError()) {
    return res;
  }
  LoggerD("Application api version: %s", api_version.c_str());

  for (auto input_priv : privileges) {
    LoggerD("Input privilege: %s", input_priv.c_str());
    GList *input_glist = nullptr;
    GList *mapped_glist = nullptr;

    SCOPE_EXIT {
      g_list_free(input_glist);
      g_list_free(mapped_glist);
    };

    input_glist = g_list_append(input_glist, (void*)input_priv.c_str());
    int ret = privilege_manager_get_mapped_privilege_list(api_version.c_str(),
                                                          PRVMGR_PACKAGE_TYPE_WRT,
                                                          input_glist,
                                                          &mapped_glist);
    if (ret != PRVMGR_ERR_NONE) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "Fail to get mapped privilege list");
    }

    LoggerD("Mapped privileges:");
    std::vector<std::string> mapped_vector;
    auto push_elem = [](gpointer data, gpointer user_data) -> void {
      if (data && user_data) {
        std::vector<std::string>* mapped_vector =
            static_cast<std::vector<std::string>*>(user_data);
        char* char_data = static_cast<char*>(data);
        mapped_vector->push_back(char_data);
        LoggerD("mapped to: %s", char_data);
      }
    };
    g_list_foreach (mapped_glist, push_elem, &mapped_vector);

    if (!AccessControl::GetInstance().CheckAccess(mapped_vector)){
      for (const auto& mapped_priv : mapped_vector) {
        LoggerD("Access to privilege: %s has been denied.", mapped_priv.c_str());
      }
      return PlatformResult(ErrorCode::SECURITY_ERR, "Permission denied");
    }
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult GetPkgApiVersion(std::string* api_version) {
  LoggerD("Entered");

  char* app_id = nullptr;
  char* pkgid = nullptr;
  char* api_ver = nullptr;
  app_info_h app_handle = nullptr;
  pkgmgrinfo_pkginfo_h pkginfo_handle = nullptr;

  SCOPE_EXIT {
    if (app_id) {
      free(app_id);
    }
    if (pkgid) {
      free(pkgid);
    }
    if (app_handle) {
      app_info_destroy(app_handle);
    }
    if (pkginfo_handle) {
      pkgmgrinfo_pkginfo_destroy_pkginfo(pkginfo_handle);
    }
  };

  pid_t pid = getpid();
  int ret = app_manager_get_app_id(pid, &app_id);
  if (ret != APP_MANAGER_ERROR_NONE) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "Fail to get app id");
  }

  ret = app_info_create(app_id, &app_handle);
  if (ret != APP_MANAGER_ERROR_NONE) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "Fail to get app info");
  }

  ret = app_info_get_package(app_handle, &pkgid);
  if ((ret != APP_MANAGER_ERROR_NONE) || (pkgid == nullptr)) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "Fail to get pkg id");
  }

  ret = pkgmgrinfo_pkginfo_get_usr_pkginfo(pkgid, getuid(), &pkginfo_handle);
  if (ret != PMINFO_R_OK) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "Fail to get pkginfo_h");
  }

  ret = pkgmgrinfo_pkginfo_get_api_version(pkginfo_handle, &api_ver);
  if (ret != PMINFO_R_OK) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "Fail to get api version");
  }

  *api_version = api_ver;
  return PlatformResult(ErrorCode::NO_ERROR);
}

std::string GetErrorString(int error_code) {
  static const size_t kSize = 1024;
  char msg[kSize] = {0};
  strerror_r(error_code, msg, kSize);
  return msg;
}


int HexToInt(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'A' && c <= 'Z') {
    return c - 'A' + 10;
  } else {
    return c - 'a' + 10;
  }
}

unsigned char* HexToBin(const char* hex, int size, unsigned char* bin, int bin_size) {
  for (int i = 0; i < size - 1 && i / 2 < bin_size; i += 2) {
    bin[i / 2] = HexToInt(hex[i]) << 4;
    bin[i / 2] |= HexToInt(hex[i + 1]);
  }
  return bin;
}

char* BinToHex(const unsigned char* bin, int size, char* hex, int hex_size) {
  static const char * const digits = "0123456789ABCDEF";
  for (int i = 0; i < size && i < hex_size / 2; i++) {
    hex[i * 2] = digits[bin[i] >> 4];
    hex[i * 2 + 1] = digits[bin[i] & 0xF];
  }
  return hex;
}

}  // namespace tools
}  // namespace common
