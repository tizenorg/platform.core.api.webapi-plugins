// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_APPLICATION_APPLICATION_UTILS_H__
#define SRC_APPLICATION_APPLICATION_UTILS_H__

#include <pkgmgr-info.h>
#include <app_context.h>
#include <app_control.h>

#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace application {

class ApplicationUtils {
 public:
  static void CreateApplicationInformation(const pkgmgrinfo_appinfo_h handle,
                                           picojson::object* app_info);

  static bool CreateApplicationContext(const app_context_h handle,
                                       picojson::object* app_context);

  static void CreateApplicationContext(pid_t pid,
                                       const std::string& app_id,
                                       picojson::object* app_context);

  static void CreateApplicationCertificate(const char* cert_type,
                                           const char* cert_value,
                                           picojson::object* app_certificate);

  static void CreateApplicationMetaData(const char* key,
                                        const char* value,
                                        picojson::object* app_meta_data);

  static common::PlatformResult ApplicationControlToService(const picojson::object& app_control_obj,
                                                            app_control_h* app_control);

  static common::PlatformResult ApplicationControlDataToServiceExtraData(
      const picojson::object& app_control_data,
      app_control_h app_control);

  static void ServiceToApplicationControl(app_control_h app_control,
                                          picojson::object* app_control_obj);

  static void ServiceExtraDataToApplicationControlData(app_control_h app_control,
                                                       const std::string& key,
                                                       picojson::object* app_control_data);

  static bool ServiceToApplicationControlDataArray(app_control_h app_control,
                                                   picojson::array* data);
 private:
  static bool ServiceExtraDataCallback(app_control_h app_control,
                                       const char* key,
                                       void* user_data);
};

} // namespace application
} // namespace extension

#endif // SRC_APPLICATION_APPLICATION_UTILS_H__
