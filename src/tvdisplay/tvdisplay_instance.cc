// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tvdisplay/tvdisplay_instance.h"

#include <vconf/vconf.h>
#include <system_info.h>

#include <common/logger.h>
#include <common/platform_exception.h>

#include <cstdio>
#include <string>
#include <functional>


namespace {
  const char* kCmd = "cmd";
  const char* kArg = "arg";
  const char* kError = "error";
  const char* kValue = "value";
  const char* kNotImplemented = "Not implemented";

  const int kVCONF_SUCCESS = 0;
  const char* kVCONF_3D_MODE_KEY = "db/menu/picture/3d/3dmode";
}  // namespace

namespace extension {
namespace tvdisplay {

TVDisplayInstance::TVDisplayInstance() {
    using std::placeholders::_1;
    using std::placeholders::_2;
    RegisterSyncHandler(
        "TVDisplay_is3DModeEnabled",
        std::bind(&TVDisplayInstance::Is3DModeEnabled,
                this, _1, _2));
    RegisterSyncHandler(
        "TVDisplay_get3DEffectMode",
        std::bind(&TVDisplayInstance::Get3DEffectMode,
                this, _1, _2));
    RegisterSyncHandler(
        "TVDisplay_getSupported3DEffectModeList",
        std::bind(&TVDisplayInstance::GetSupported3DEffectModeList,
                this, _1, _2));
}

TVDisplayInstance::~TVDisplayInstance() {}

void TVDisplayInstance::Is3DModeEnabled(
        const picojson::value& value,
        picojson::object& out) {
    LoggerD("Enter");
    bool is_supported = true;
    picojson::value::object o;

    int ret = system_info_get_value_bool(
            SYSTEM_INFO_KEY_3D_EFFECT_SUPPORTED,
            &is_supported);

    if (SYSTEM_INFO_ERROR_NONE != ret) {
        std::string err =
            "'system_info' error while getting 3d mode details: "
            + std::to_string(ret);
        LoggerE("%s", err.c_str());
        ReportError(common::UnknownException(err), out);
    }
    std::string mode = is_supported ? "READY" : "NOT_SUPPORTED";
    LoggerD("3D Mode is: %s", mode.c_str());
    ReportSuccess(picojson::value(mode), out);
}

void TVDisplayInstance::Get3DEffectMode(
        const picojson::value& value,
        picojson::object& out) {
  LoggerD("Enter");

  int mode = 0;
  int ret = vconf_get_int(kVCONF_3D_MODE_KEY, &mode);

  if (kVCONF_SUCCESS != ret) {
      std::string err
          = "Platform error while getting 3d mode details: "
          + std::to_string(ret);
      LoggerE("%s", err.c_str());
      ReportError(common::UnknownException(err), out);
  }
  ReportSuccess(picojson::value(mode * 1.0), out);
}

void TVDisplayInstance::GetSupported3DEffectModeList(
        const picojson::value& value,
        picojson::object& out) {
    LoggerD("Enter");
    picojson::value::object o;

    // TODO(m.wasowski2): temporary, should be made async
    ReportSuccess(picojson::value(kNotImplemented), out);
}

}  // namespace tvdisplay
}  // namespace extension
