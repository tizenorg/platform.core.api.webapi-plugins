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

#include "keymanager/keymanager_instance.h"

#include <ckmc/ckmc-manager.h>
#include <glib.h>
#include <pkgmgr-info.h>

#include "common/logger.h"
#include "common/optional.h"
#include "common/platform_result.h"
#include "common/scope_exit.h"
#include "common/task-queue.h"
#include "common/tools.h"
#include "common/current_application.h"

namespace extension {
namespace keymanager {

using common::ErrorCode;
using common::optional;
using common::PlatformResult;
using common::TaskQueue;

namespace {

// Privileges required in KeyManager API
const std::string kPrivilegeKeyManager = "http://tizen.org/privilege/keymanager";

typedef std::vector<unsigned char> RawBuffer;

typedef int (*AliasListFunction)(ckmc_alias_list_s**);

void GetGenericAliasList(AliasListFunction func, picojson::object* out) {
  LoggerD("Enter");

  ckmc_alias_list_s* alias_list = nullptr;
  int ret = func(&alias_list);

  picojson::value result{picojson::array{}};

  if (CKMC_ERROR_NONE == ret) {
    auto& aliases = result.get<picojson::array>();

    picojson::value resultElem = picojson::value(picojson::object());
    picojson::object& obj = resultElem.get<picojson::object>();
    ckmc_alias_list_s* head = alias_list;

    while (head) {
      //aliases.push_back(picojson::value(head->alias ? head->alias : ""));
      if(head->alias) {
        char* tokenized = strtok(head->alias," ");
        obj["packageId"] = picojson::value(tokenized);
        tokenized = strtok(NULL," ");
        obj["name"] = picojson::value(tokenized);

        aliases.push_back(resultElem);
      }

      head = head->next;
    }

    if (alias_list) {
      ckmc_alias_list_all_free(alias_list);
    }
  }

  common::tools::ReportSuccess(result, *out);
}
}  // namespace

KeyManagerInstance::KeyManagerInstance() {
  LoggerD("Enter");
  using std::placeholders::_1;
  using std::placeholders::_2;

  RegisterSyncHandler("KeyManager_getDataAliasList",
      std::bind(&KeyManagerInstance::GetDataAliasList, this, _1, _2));

  RegisterSyncHandler("KeyManager_saveData",
      std::bind(&KeyManagerInstance::SaveData, this, _1, _2));

  RegisterSyncHandler("KeyManager_getData",
      std::bind(&KeyManagerInstance::GetData, this, _1, _2));

  RegisterSyncHandler("KeyManager_removeAlias",
      std::bind(&KeyManagerInstance::RemoveAlias, this, _1, _2));

  RegisterSyncHandler("KeyManager_setPermissions",
      std::bind(&KeyManagerInstance::SetPermission, this, _1, _2));
}

void KeyManagerInstance::GetDataAliasList(const picojson::value& args,
                                          picojson::object& out) {
  LoggerD("Enter");
  CHECK_PRIVILEGE_ACCESS(kPrivilegeKeyManager, out);

  GetGenericAliasList(ckmc_get_data_alias_list, &out);
}

PlatformResult KeyManagerInstance::GetError(int ret) {
  char* error = get_error_message(ret);
  if(CKMC_ERROR_NONE == ret) {
    return PlatformResult(ErrorCode::NO_ERROR);
  }
  else if(CKMC_ERROR_INVALID_PARAMETER == ret) {
    LoggerD("%s", error);
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR, error);
  }
  else if(CKMC_ERROR_DB_ALIAS_UNKNOWN == ret) {
    LoggerD("%s",error);
    return PlatformResult(ErrorCode::NOT_FOUND_ERR, error);
  }
  else if(CKMC_ERROR_AUTHENTICATION_FAILED == ret) {
    LoggerD("%s",error);
    return PlatformResult(ErrorCode::VERIFICATION_ERR, error);
  }
  else {
    LoggerD("%s", error);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, error);
  }
}

void KeyManagerInstance::SaveData(const picojson::value& args,
                                  picojson::object& out) {
  LoggerD("Enter");
  CHECK_PRIVILEGE_ACCESS(kPrivilegeKeyManager, out);

  std::string data_raw = args.get("rawData").get<std::string>();
  std::string alias = args.get("aliasName").get<std::string>();
  const auto& password_value = args.get("password");

  double callback_id = args.get("callbackId").get<double>();

  const char* password = nullptr;

  if (password_value.is<std::string>()) {
    password = (password_value.get<std::string>()).c_str();
    LoggerE("password %s ", password);
  }

  auto save_data = [data_raw, password, alias](const std::shared_ptr<picojson::value>& result) {

    unsigned char* data = new unsigned char[data_raw.size()];
    std::copy(data_raw.begin(), data_raw.end(), data);

    ckmc_raw_buffer_s raw_data { data, data_raw.size() };
    ckmc_policy_s policy { const_cast<char*>(password), true };

    int ret = ckmc_save_data(alias.c_str(), raw_data, policy);

    PlatformResult success = GetError(ret);

    if (success) {
      common::tools::ReportSuccess(result->get<picojson::object>());
    } else {
      LoggerE("Failed to save data: %d", ret);
      common::tools::ReportError(success, &result->get<picojson::object>());
    }

    delete[] data;
  };

  auto save_data_result = [this, callback_id](const std::shared_ptr<picojson::value>& result) {
    result->get<picojson::object>()["callbackId"] = picojson::value{callback_id};
    Instance::PostMessage(this, result->serialize().c_str());
  };

  auto queue_data = std::shared_ptr<picojson::value>{new picojson::value{picojson::object()}};

  TaskQueue::GetInstance().Queue<picojson::value>(
      save_data,
      save_data_result,
      queue_data);

  ReportSuccess(out);
}

void KeyManagerInstance::GetData(const picojson::value& args,
                                 picojson::object& out) {
  LoggerD("Enter");
  CHECK_PRIVILEGE_ACCESS(kPrivilegeKeyManager, out);

  const auto& data_alias = args.get("name").get<std::string>();
  const auto& password_value = args.get("password");

  const char* password = nullptr;

  if (password_value.is<std::string>()) {
    password = (password_value.get<std::string>()).c_str();
  }

  LoggerD("data_alias: %s", data_alias.c_str());

  ckmc_raw_buffer_s* data = nullptr;
  int ret = ckmc_get_data(data_alias.c_str(), password, &data);

  if (CKMC_ERROR_NONE == ret) {
    picojson::object result;

    result["rawData"] = picojson::value(std::string (data->data, data->data + data->size));

    ckmc_buffer_free(data);
    ReportSuccess(picojson::value{result}, out);
  } else {
    LoggerE("Failed to get data: %d", ret);

    PlatformResult error = GetError(ret);

    ReportError(error, &out);
  }
}

KeyManagerInstance::~KeyManagerInstance() {
  LoggerD("Enter");
}

void KeyManagerInstance::RemoveAlias(const picojson::value& args,
                                   picojson::object& out) {
  LoggerD("Enter");
  CHECK_PRIVILEGE_ACCESS(kPrivilegeKeyManager, out);

  const std::string& alias = args.get("aliasName").get<std::string>();
  int ret = ckmc_remove_alias(alias.c_str());

  if (CKMC_ERROR_NONE != ret) {
    LoggerE("Failed to remove alias [%d]", ret);
    PlatformResult result = GetError(ret);
    ReportError(result, &out);
  } else {
    ReportSuccess(out);
  }
}

void KeyManagerInstance::SetPermission(const picojson::value& args,
                                            picojson::object& out) {
  LoggerD("Enter");
  CHECK_PRIVILEGE_ACCESS(kPrivilegeKeyManager, out);

  const std::string& data_name = args.get("aliasName").get<std::string>();
  const std::string& id = args.get("packageId").get<std::string>();
  const double callback_id = args.get("callbackId").get<double>();
  const std::string& access = args.get("permissionType").get<std::string>();

  int permissions = CKMC_PERMISSION_NONE;
  if( "NONE" == access) {
    permissions = CKMC_PERMISSION_NONE;
  }
  else if ("READ" == access) {
    permissions = CKMC_PERMISSION_READ;
  }
  else if ("REMOVE" == access) {
    permissions = CKMC_PERMISSION_REMOVE;
  }
  else if("READ_REMOVE" == access) {
    permissions = CKMC_PERMISSION_READ | CKMC_PERMISSION_REMOVE;
  }

  auto set_permissions = [data_name, id, permissions](const std::shared_ptr<picojson::value>& response) -> void {
    int ret = ckmc_set_permission(data_name.c_str(), id.c_str(), permissions);

    if (CKMC_ERROR_NONE != ret) {
      PlatformResult result = GetError(ret);
      common::tools::ReportError(result, &response->get<picojson::object>());
    } else {
      common::tools::ReportSuccess(response->get<picojson::object>());
    }
  };

  auto set_permissions_response = [this, callback_id](const std::shared_ptr<picojson::value>& response) -> void {
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair("callbackId", picojson::value(callback_id)));
    Instance::PostMessage(this, response->serialize().c_str());
  };

  auto data = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));

  TaskQueue::GetInstance().Queue<picojson::value>(
      set_permissions,
      set_permissions_response,
      std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));
}

} // namespace keymanager
} // namespace extension
