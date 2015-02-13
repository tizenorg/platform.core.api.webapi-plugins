// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ACCOUNT_ACCOUNT_MANAGER_H_
#define ACCOUNT_ACCOUNT_MANAGER_H_

#include <account.h>

#include "common/extension.h"
#include "common/picojson.h"
#include "common/platform_exception.h"

namespace extension {
namespace account {

class AccountManager {
 public:
  AccountManager();
  virtual ~AccountManager();

  /* out["status"] = "success" or "error"
  * If status is "success", then the result(picojson::value) will be stored in out["result"].
  * If status is "error", then the error(picojson::value) will be stored in out["error"].
  */
  void GetAccountsInfo(const std::string& application_id, picojson::object& out);

 /* out["status"] = "success" or "error"
 * If status is "success", then the result(picojson::value) will be stored in out["result"].
 * If status is "error", then the error(picojson::value) will be stored in out["error"].
 */
  bool GetAccountInfo(int account_id, picojson::object& out);

 /* out["status"] = "success" or "error"
 * If status is "success", then the result(picojson::value) will be stored in out["result"].
 * If status is "error", then the error(picojson::value) will be stored in out["error"].
 */
  void GetProvidersInfo(const std::string& capability, picojson::object& out);

 /* out["status"] = "success" or "error"
 * If status is "success", then the result(picojson::value) will be stored in out["result"].
 * If status is "error", then the error(picojson::value) will be stored in out["error"].
 */
  static bool GetProviderInfo(const std::string& provider_id, picojson::object& out);

  /* out["status"] = "success" or "error"
   * If status is "success", then the result(picojson::value) will be stored in out["result"].
   * If status is "error", then the error(picojson::value) will be stored in out["error"].
   */
  void GetExtendedData(int account_id, const std::string& key, picojson::object& out);

  /* out["status"] = "success" or "error"
   * If status is "success", then the result(picojson::value) will be stored in out["result"].
   * If status is "error", then the error(picojson::value) will be stored in out["error"].
   */
  void GetExtendedData(int account_id, picojson::object& out);

  /* out["status"] = "success" or "error"
   * If status is "success", then the result(picojson::value) will be stored in out["result"].
   * If status is "error", then the error(picojson::value) will be stored in out["error"].
   */
  void SetExtendedData(int account_id, const std::string& key, const std::string& value, picojson::object& out);

  static bool ConvertAccountToObject(account_h account, picojson::object& out);

  static bool ConvertProviderToObject(account_type_h provider, picojson::object& out);

  static std::string GetErrorMsg(int error);

  void AddAccount(const picojson::value& data, picojson::object& obj);
  void RemoveAccount(const picojson::value& data, picojson::object& obj);
  void UpdateAccount(const picojson::value& data, picojson::object& obj);
  //bool IsValidPackage(const std::string& packageName);
};

} // namespace account
} // namespace extension

#endif // ACCOUNT_ACCOUNT_MANAGER_H_
