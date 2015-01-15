// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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

  static void GetPackagesInfo(picojson::object& out);
  static void GetPackageInfo(picojson::object& out);  
  static void GetPackageInfo(const char* packageId, picojson::object& out);
  static bool GetPackageInfo(const pkgmgrinfo_pkginfo_h info, picojson::object& out);
  
 private:
  static bool GetCurrentPackageId(char** packageId);
};

} // namespace package
} // namespace extension

#endif // PACKAGE_PACKAGE_INFO_PROVIDER_H_
