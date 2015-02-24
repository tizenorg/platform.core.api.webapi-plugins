// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PACKAGE_PACKAGE_INSTANCE_H_
#define PACKAGE_PACKAGE_INSTANCE_H_

#include <package_manager.h>

#include <map>

#include "common/extension.h"
#include "common/platform_exception.h"

namespace extension {
namespace package {

class PackageInstance : public common::ParsedInstance {
 public:
  PackageInstance();
  virtual ~PackageInstance();

  void InvokeCallback(int request_id, picojson::object& param);
  void InvokeListener(picojson::object& param);
  void DeregisterCallback(int request_id);

 private:
  package_manager_request_h request_;
  package_manager_h manager_;
  bool is_package_info_listener_set_;
  std::map<int, int> callbacks_map_;  // <request_id, callbackId>

  void RegisterCallback(int request_id, int callback_id);
  void InvokeErrorCallbackAsync
    (int callback_id, const common::PlatformException& ex);

  void PackageManagerInstall
    (const picojson::value& args, picojson::object& out);
  void PackageManagerUninstall(
    const picojson::value& args, picojson::object& out);
  void PackageManagerGetpackagesinfo
    (const picojson::value& args, picojson::object& out);
  void PackageManagerGetpackageinfo
    (const picojson::value& args, picojson::object& out);
  void PackageManagerSetpackageinfoeventlistener
    (const picojson::value& args, picojson::object& out);
  void PackageManagerUnsetpackageinfoeventlistener
    (const picojson::value& args, picojson::object& out);
};

}  // namespace package
}  // namespace extension

#endif  // PACKAGE_PACKAGE_INSTANCE_H_
