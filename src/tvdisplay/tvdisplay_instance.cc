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
 
#include "tvdisplay/tvdisplay_instance.h"

#include <vconf/vconf.h>
#include <system_info.h>

#include <common/logger.h>
#include <common/platform_exception.h>
#include <common/task-queue.h>

#include <cstdio>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>


namespace {
    const char* kCmd = "cmd";
    const char* kArg = "arg";
    const char* kError = "error";
    const char* kValue = "value";
    const char* kResult = "result";
    const char* kSuccess = "success";
    const char* kNotImplemented = "Not implemented";
    const char *kID = "callbackId";

    const int kVCONF_SUCCESS = 0;
    const char* kVCONF_3D_MODE_KEY = "db/menu/picture/3d/3dmode";

    const std::map<const int, const std::string> supported_3D_modes {
        {SYSTEM_INFO_3D_MODE_OFF, "OFF"},
        {SYSTEM_INFO_3D_MODE_TOP_BOTTOM, "TOP_BOTTOM"},
        {SYSTEM_INFO_3D_MODE_SIDE_BY_SIDE, "SIDE_BY_SIDE"},
        {SYSTEM_INFO_3D_MODE_CHECKER_BOARD, "CHECKER_BD"},
        {SYSTEM_INFO_3D_MODE_LINE_BY_LINE, "LINE_BY_LINE"},
        {SYSTEM_INFO_3D_MODE_VERTICAL_STRIPE, "VERTICAL_STRIPE"},
        {SYSTEM_INFO_3D_MODE_FRAME_SEQUENTIAL, "FRAME_SEQUENCE"},
        {SYSTEM_INFO_3D_MODE_2D_3D_CONVERSION, "FROM_2D_TO_3D"}
    };

    common::PlatformResult is_3D_enabled(bool &is_3D_supported) {
        LOGD("Enter");
        int is_supported = -1;
        int ret = system_info_get_value_int(
                SYSTEM_INFO_KEY_3D_SUPPORT,
                &is_supported);

        if (SYSTEM_INFO_ERROR_NONE != ret) {
            std::string message = "'system_info' error while "
                    "getting 3d mode details: " + std::to_string(ret);
            LOGE("%s", message.c_str());
            return common::PlatformResult(common::ErrorCode::UNKNOWN_ERR,
            "Unknown error. " + message);
        }

        is_3D_supported = (is_supported == 1);
        return common::PlatformResult(common::ErrorCode::NO_ERROR);
    }
}  // namespace

namespace extension {
namespace tvdisplay {

TVDisplayInstance::TVDisplayInstance() {
    using std::placeholders::_1;
    using std::placeholders::_2;

    #define REGISTER_ASYNC(c, func) \
        RegisterSyncHandler(c, func);
    #define REGISTER_SYNC(c, func) \
        RegisterSyncHandler(c, func);

    REGISTER_SYNC("TVDisplay_is3DModeEnabled",
        std::bind(&TVDisplayInstance::Is3DModeEnabled, this, _1, _2));
    REGISTER_SYNC("TVDisplay_get3DEffectMode",
        std::bind(&TVDisplayInstance::Get3DEffectMode,this, _1, _2));
    REGISTER_ASYNC("TVDisplay_getSupported3DEffectModeList",
        std::bind(&TVDisplayInstance::GetSupported3DEffectModeList,
                  this, _1, _2));

    #undef REGISTER_ASYNC
    #undef REGISTER_SYNC
}

TVDisplayInstance::~TVDisplayInstance() {}


void TVDisplayInstance::Is3DModeEnabled(
        const picojson::value& value,
        picojson::object& out) {
    LOGD("Enter");
    picojson::value::object o;
    std::string mode = "NOT_SUPPORTED";
    bool is_3D_supported = false;
    common::PlatformResult result = is_3D_enabled(is_3D_supported);
    if (result.IsError()) {
        LOGD("Error occured");
        ReportError(result, &out);
    } else {
        if (is_3D_supported) {
            mode = "READY";
        }
        LOGD("3D Mode is: %s", mode.c_str());
        picojson::value result(mode);
        ReportSuccess(result, out);
    }
}

void TVDisplayInstance::Get3DEffectMode(
        const picojson::value& value,
        picojson::object& out) {
    LOGD("Enter");

    int mode = 0;
    int ret = vconf_get_int(kVCONF_3D_MODE_KEY, &mode);

    if (kVCONF_SUCCESS != ret) {
        std::string err
            = "Platform error while getting 3d mode details: "
            + std::to_string(ret);
        LOGE("%s", err.c_str());
        ReportError(common::UnknownException(err), out);
    }
    ReportSuccess(picojson::value(mode * 1.0), out);
}

void TVDisplayInstance::GetSupported3DEffectModeList(
        const picojson::value& value,
        picojson::object& out) {
    LOGD("Enter");

    std::shared_ptr <picojson::value::object> reply
            = std::shared_ptr <picojson::value::object>(
                    new picojson::value::object());

    (*reply)[kID] = value.get(kID);

    std::function <void(std::shared_ptr <picojson::object> const&)> task =
            std::bind(
                    &TVDisplayInstance::GetSupported3DEffectModeListTask,
                    this,
                    std::placeholders::_1);
    std::function <void(std::shared_ptr <picojson::object> const&) > taskAfter =
            std::bind(
                    &TVDisplayInstance::GetSupported3DEffectModeListTaskAfter,
                    this,
                    std::placeholders::_1);
    common::TaskQueue::GetInstance()
            .Queue <picojson::object>(
                    task,
                    taskAfter,
                    reply);
    ReportSuccess(out);
}

void TVDisplayInstance::GetSupported3DEffectModeListTask(
        std::shared_ptr<picojson::object> const& data) {
    LOGD("Enter");

    picojson::object & reply = (*data);
    std::vector <picojson::value> modes;
    bool is_3D_supported = false;
    common::PlatformResult res = is_3D_enabled(is_3D_supported);
    if (res.IsError()) {
        LOGD("Error occured");
        reply[kError] = res.ToJSON();
        return;
    }
    if (!is_3D_supported) {
        LOGD("3D is disabled");
        reply[kResult] = picojson::value(modes);
        reply[kSuccess] = picojson::value(true);
        return;
    }
    int flags = -1;
    int result =  system_info_get_value_int(
            SYSTEM_INFO_KEY_3D_EFFECT_MODE,
            &flags);
    if (SYSTEM_INFO_ERROR_NONE != result) {
        const char * kMessage =
                "Fetching SYSTEM_INFO_KEY_3D_EFFECT_MODE failed";
        LOGE("%s: %d", kMessage, result);
        LOGD("Error occured");
        res = common::PlatformResult(common::ErrorCode::UNKNOWN_ERR,
                "Unknown error. " + std::string(kMessage));
        reply[kError] = res.ToJSON();
        return;
    }

    auto it = supported_3D_modes.begin();
    for (it; it != supported_3D_modes.end(); ++it) {
        if (it->first & flags) {
            modes.push_back(picojson::value(it->second));
        }
    }

    if (flags & SYSTEM_INFO_3D_MODE_FRAME_PACKING) {
        LOGD("There is no FRAME_PACKING mode in TIZEN");
    }
    if (flags & SYSTEM_INFO_3D_MODE_FRAME_DUAL) {
        LOGD("There is no FRAME_DUAL mode in TIZEN");
    }

    reply[kResult] = picojson::value(modes);
    reply[kSuccess] = picojson::value(true);
}

void TVDisplayInstance::GetSupported3DEffectModeListTaskAfter(
        std::shared_ptr<picojson::object> const& data) {
    LOGD("Enter");
    picojson::value out(*data);
    std::string serialized(out.serialize());
    Instance::PostMessage(this, serialized.c_str());
}

}  // namespace tvdisplay
}  // namespace extension
