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

#ifndef PACKAGE_PACKAGE_INFO_PROVIDER_H_
#define PACKAGE_PACKAGE_INFO_PROVIDER_H_

#include <pkgmgr-info.h>

#include "common/extension.h"
#include "common/picojson.h"
#include "common/platform_exception.h"

namespace extension {
namespace package {

class PackageInfoProvider {
 public:
  PackageInfoProvider();
  virtual ~PackageInfoProvider();

  /* out["status"] = "success" or "error"
  * If status is "success", then the result(picojson::value)
  * will be stored in out["informationArray"].
  * If status is "error", then the error(picojson::value)
  * will be stored in out["error"].
  */
  static void GetPackagesInfo(picojson::object& out);

  /* out["status"] = "success" or "error"
  * If status is "success", then the result(picojson::value)
  * will be stored in out["result"].
  * If status is "error", then the error(picojson::value)
  * will be stored in out["error"].
  */
  static void GetPackageInfo(picojson::object& out);
  static void GetPackageInfo(const char* package_id,
      picojson::object& out);

  static bool ConvertToPackageToObject(
      const pkgmgrinfo_pkginfo_h info, picojson::object& out);

  static void GetTotalSize(const std::string& id, picojson::object* out);
  static void GetDataSize(const std::string& id, picojson::object* out);

 private:
  static bool GetCurrentPackageId(char** package_id);
};

}  // namespace package
}  // namespace extension

#endif  // PACKAGE_PACKAGE_INFO_PROVIDER_H_
