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

#ifndef ACCOUNT_ACCOUNT_INSTANCE_H_
#define ACCOUNT_ACCOUNT_INSTANCE_H_

#include <account.h>

#include "account/account_manager.h"
#include "common/extension.h"

namespace extension {
namespace account {

class AccountInstance : public common::ParsedInstance {
 public:
  AccountInstance();
  virtual ~AccountInstance();

  AccountManager* GetAccountManager();
  void InvokeListener(picojson::object& param);

 private:
  AccountInstance(const AccountInstance&) = delete;
  AccountInstance(const AccountInstance&&) = delete;
  AccountInstance& operator=(const AccountInstance&) = delete;
  AccountInstance& operator=(const AccountInstance&&) = delete;

  AccountManager* manager_;
  account_subscribe_h subscribe_;

  void AccountManagerRemoveAccountListener(
      const picojson::value& args, picojson::object& out);
  void AccountManagerUpdate(
      const picojson::value& args, picojson::object& out);
  void AccountManagerRemove(
      const picojson::value& args, picojson::object& out);
  void AccountConstructor(
      const picojson::value& args, picojson::object& out);
  void AccountManagerGetAccount(
      const picojson::value& args, picojson::object& out);
  void AccountManagerGetProvider(
      const picojson::value& args, picojson::object& out);
  void AccountSetExtendedData(
      const picojson::value& args, picojson::object& out);
  void AccountManagerAddAccountListener(
      const picojson::value& args, picojson::object& out);
  void AccountManagerAdd(
      const picojson::value& args, picojson::object& out);
  void AccountManagerGetAccounts(
      const picojson::value& args, picojson::object& out);
  void AccountGetExtendedData(
      const picojson::value& args, picojson::object& out);
  void AccountGetExtendedDataSync(
      const picojson::value& args, picojson::object& out);
  void AccountManagerGetProviders(
      const picojson::value& args, picojson::object& out);
};

} // namespace account
} // namespace extension

#endif // ACCOUNT_ACCOUNT_INSTANCE_H_
