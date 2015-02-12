// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
