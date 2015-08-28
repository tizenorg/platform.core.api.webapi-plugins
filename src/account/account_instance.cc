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

#include "account/account_instance.h"

#include "string.h"

#include <functional>

#include "common/picojson.h"
#include "common/logger.h"
#include "common/task-queue.h"
#include "common/platform_exception.h"

namespace extension {
namespace account {

using common::TaskQueue;
using common::TypeMismatchException;
using common::UnknownException;
using common::SecurityException;

#define CHECK_EXIST(args, name, out) \
    if (!args.contains(name)) {\
      ReportError(TypeMismatchException(name" is required argument"), out);\
      return;\
    }

AccountInstance::AccountInstance() {
  LoggerD("Enter");

  manager_ = new AccountManager;
  subscribe_ = NULL;

  using std::placeholders::_1;
  using std::placeholders::_2;

  #define REGISTER_ASYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&AccountInstance::x, this, _1, _2));
  REGISTER_ASYNC("AccountManager_getAccounts", AccountManagerGetAccounts);
  REGISTER_ASYNC("AccountManager_getProviders", AccountManagerGetProviders);
  REGISTER_ASYNC("Account_getExtendedData", AccountGetExtendedData);
  #undef REGISTER_ASYNC
  #define REGISTER_SYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&AccountInstance::x, this, _1, _2));
  REGISTER_SYNC("AccountManager_removeAccountListener", AccountManagerRemoveAccountListener);
  REGISTER_SYNC("AccountManager_update", AccountManagerUpdate);
  REGISTER_SYNC("AccountManager_remove", AccountManagerRemove);
  REGISTER_SYNC("AccountManager_getAccount", AccountManagerGetAccount);
  REGISTER_SYNC("AccountManager_getProvider", AccountManagerGetProvider);
  REGISTER_SYNC("AccountManager_addAccountListener", AccountManagerAddAccountListener);
  REGISTER_SYNC("AccountManager_add", AccountManagerAdd);
  REGISTER_SYNC("Account_setExtendedData", AccountSetExtendedData);
  REGISTER_SYNC("Account_getExtendedDataSync", AccountGetExtendedDataSync);
  #undef REGISTER_SYNC
}

AccountInstance::~AccountInstance() {
  LoggerD("Enter");
  delete manager_;
  if (subscribe_) {
    account_unsubscribe_notification(subscribe_);
  }
}

AccountManager* AccountInstance::GetAccountManager() {
  LoggerD("Enter");
  return manager_;
}

void AccountInstance::AccountSetExtendedData(const picojson::value& args,
                                             picojson::object& out) {
  LoggerD("Enter");

  CHECK_EXIST(args, "key", out)
  CHECK_EXIST(args, "value", out)
  CHECK_EXIST(args, "accountId", out)

  const std::string& key = args.get("key").get<std::string>();
  const std::string& value = args.get("value").get<std::string>();
  int account_id = static_cast<int>(args.get("accountId").get<double>());

  this->manager_->SetExtendedData(account_id, key, value, out);
}

void AccountInstance::AccountGetExtendedData(const picojson::value& args,
                                             picojson::object& out) {
  LoggerD("Enter");

  CHECK_EXIST(args, "accountId", out)
  CHECK_EXIST(args, "callbackId", out)

  int account_id = static_cast<int>(args.get("accountId").get<double>());
  int callback_id = static_cast<int>(args.get("callbackId").get<double>());

  auto get_extended_data = [this, account_id](const std::shared_ptr<picojson::value>& result) {
    this->manager_->GetExtendedData(account_id, result->get<picojson::object>());
  };

  auto get_extended_data_result = [this, callback_id](const std::shared_ptr<picojson::value>& result) {
    result->get<picojson::object>()["callbackId"] = picojson::value{static_cast<double>(callback_id)};
    Instance::PostMessage(this, result->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<picojson::value>(
      get_extended_data,
      get_extended_data_result,
      std::shared_ptr<picojson::value>{new picojson::value{picojson::object()}});

  ReportSuccess(out);
}

void AccountInstance::AccountGetExtendedDataSync(const picojson::value& args,
                                                 picojson::object& out) {
  LoggerD("Enter");

  CHECK_EXIST(args, "key", out)
  CHECK_EXIST(args, "accountId", out)

  const std::string& key = args.get("key").get<std::string>();
  int account_id = static_cast<int>(args.get("accountId").get<double>());

  this->manager_->GetExtendedData(account_id, key, out);
}

void AccountInstance::AccountManagerAdd(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  manager_->AddAccount(args, out);
}

void AccountInstance::AccountManagerRemove(const picojson::value& args,
                                           picojson::object& out) {
  LoggerD("Enter");
  manager_->RemoveAccount(args, out);
}

void AccountInstance::AccountManagerUpdate(const picojson::value& args,
                                           picojson::object& out) {
  LoggerD("Enter");
  manager_->UpdateAccount(args, out);
}

void AccountInstance::AccountManagerGetAccount(const picojson::value& args,
                                               picojson::object& out) {
  LoggerD("Enter");

  CHECK_EXIST(args, "accountId", out)

  int account_id = static_cast<int>(args.get("accountId").get<double>());
  LoggerD("account_id [%d]", account_id);
  manager_->GetAccountInfo(account_id, out);
}

void AccountInstance::AccountManagerGetAccounts(const picojson::value& args,
                                                picojson::object& out) {
  LoggerD("Enter");

  CHECK_EXIST(args, "callbackId", out)
  int callback_id = static_cast<int>(args.get("callbackId").get<double>());

  const std::string application_id = args.contains("applicationId") ? args.get("applicationId").get<std::string>() : "";
  LoggerD("application ID: [%s]", application_id.c_str());

  auto get_accounts = [this, application_id](const std::shared_ptr<picojson::value>& result) {
    this->manager_->GetAccountsInfo(application_id, result->get<picojson::object>());
  };

  auto get_accounts_result = [this, callback_id](const std::shared_ptr<picojson::value>& result) {
    result->get<picojson::object>()["callbackId"] = picojson::value{static_cast<double>(callback_id)};
    Instance::PostMessage(this, result->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<picojson::value>(
      get_accounts,
      get_accounts_result,
      std::shared_ptr<picojson::value>{new picojson::value{picojson::object()}});

  ReportSuccess(out);
}

void AccountInstance::AccountManagerGetProvider(const picojson::value& args,
                                                picojson::object& out) {
  LoggerD("Enter");

  std::string application_id = args.get("applicationId").get<std::string>();
  LoggerD("application_id [%s]", application_id.c_str());

  manager_->GetProviderInfo(application_id, out);
}

void AccountInstance::AccountManagerGetProviders(const picojson::value& args,
                                                 picojson::object& out) {
  LoggerD("Enter");

  CHECK_EXIST(args, "callbackId", out)
  int callback_id = static_cast<int>(args.get("callbackId").get<double>());

  const auto& cap = args.get("capability");
  const std::string& capability = cap.is<picojson::null>() ? "" : cap.get<std::string>();

  LoggerD("capability [%s]", capability.c_str());

  auto get_providers = [this, capability](const std::shared_ptr<picojson::value>& result) {
    this->manager_->GetProvidersInfo(capability, result->get<picojson::object>());
  };

  auto get_providers_result = [this, callback_id](const std::shared_ptr<picojson::value>& result) {
    result->get<picojson::object>()["callbackId"] = picojson::value{static_cast<double>(callback_id)};
    Instance::PostMessage(this, result->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<picojson::value>(
      get_providers,
      get_providers_result,
      std::shared_ptr<picojson::value>{new picojson::value{picojson::object()}});

  ReportSuccess(out);
}

void AccountInstance::InvokeListener(picojson::object& param) {
  LoggerD("Enter");
  picojson::value result = picojson::value(param);
  Instance::PostMessage(this, result.serialize().c_str());
}

static bool AccountEventCb(const char *event_type, int account_id,
                           void *user_data) {
  LoggerD("Enter");

  AccountInstance* instance = static_cast<AccountInstance*>(user_data);
  if (!instance) {
    LoggerE("instance is NULL");
    return false;
  }

  picojson::object result;
  result["listenerId"] = picojson::value("ACCOUNT_CHANGED");
  if (!strcmp(event_type, ACCOUNT_NOTI_NAME_INSERT)) {
    LoggerD("Added");
    result["action"] = picojson::value("onadded");
    picojson::object info;
    instance->GetAccountManager()->GetAccountInfo(account_id, info);
    result["result"] = picojson::value(info["result"]);
    instance->InvokeListener(result);
  } else if (!strcmp(event_type, ACCOUNT_NOTI_NAME_UPDATE)) {
    LoggerD("Updated");
    result["action"] = picojson::value("onupdated");
    picojson::object info;
    instance->GetAccountManager()->GetAccountInfo(account_id, info);
    result["result"] = picojson::value(info["result"]);
    instance->InvokeListener(result);
  } else if (!strcmp(event_type, ACCOUNT_NOTI_NAME_DELETE)) {
    LoggerD("Deleted");
    result["action"] = picojson::value("onremoved");
    result["result"] = picojson::value(static_cast<double>(account_id));
    instance->InvokeListener(result);
  }

  return true;
}

void AccountInstance::AccountManagerAddAccountListener(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");

  if (!subscribe_) {
    LoggerD("Creating subscription");
    int ret = account_subscribe_create(&subscribe_);
    if (ret != ACCOUNT_ERROR_NONE) {
      LoggerE("Failed to create account subscribe");
      ReportError(UnknownException(manager_->GetErrorMsg(ret)), out);
      return;
    }

    LoggerD("Subscribing for notification");
    ret = account_subscribe_notification(subscribe_, AccountEventCb, this);
    if (ret != ACCOUNT_ERROR_NONE) {
      LoggerE("Failed to subscribe notification");
      ReportError(UnknownException(manager_->GetErrorMsg(ret)), out);
      return;
    }

    LoggerD("Success");
  }

  ReportSuccess(out);
}

void AccountInstance::AccountManagerRemoveAccountListener(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");

  if (subscribe_) {
    LoggerD("Removing subscription");

    if (account_unsubscribe_notification(subscribe_) != ACCOUNT_ERROR_NONE) {
      LoggerE("Failed to unsubscribe notification");
    }

    subscribe_ = nullptr;
  }

  ReportSuccess(out);
}

#undef CHECK_EXIST

} // namespace account
} // namespace extension
