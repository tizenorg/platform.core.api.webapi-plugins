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
 
#include "tvwindow/tvwindow_instance.h"
#include "common/logger.h"
#include "tizen/tizen.h"
#include "common/picojson.h"
#include "common/platform_result.h"
#include <system_info.h>

namespace extension {
namespace tvwindow {

TVWindowInstance::TVWindowInstance(TVWindowExtension const& extension) {
  LoggerD("Entered");
  using std::placeholders::_1;
  using std::placeholders::_2;

  RegisterSyncHandler("TVWindow_GetScreenDimension",
      std::bind(&TVWindowInstance::GetScreenDimension, this, _1, _2));
}

TVWindowInstance::~TVWindowInstance() {
  LoggerD("Entered");
}

void TVWindowInstance::GetScreenDimension(const picojson::value& args,
    picojson::object& out) {
  LoggerD("Entered");

  int max_width = 0;
  if (system_info_get_value_int(SYSTEM_INFO_KEY_OSD_RESOLUTION_WIDTH,
      &max_width) != SYSTEM_INFO_ERROR_NONE) {
    common::PlatformResult res(common::ErrorCode::UNKNOWN_ERR,
        "Failed to retrieve screen max width");
    ReportError(res, &out);
    return;
  }
  int max_height = 0;
  if (system_info_get_value_int(SYSTEM_INFO_KEY_OSD_RESOLUTION_HEIGHT,
      &max_height) != SYSTEM_INFO_ERROR_NONE) {
    common::PlatformResult res(common::ErrorCode::UNKNOWN_ERR,
        "Failed to retrieve screen max height");
    ReportError(res, &out);
    return;
  }
  picojson::value::object dict;
  dict["width"] = picojson::value(static_cast<double>(max_width));
  dict["height"] = picojson::value(static_cast<double>(max_height));
  picojson::value result(dict);
  ReportSuccess(result, out);
}

}  // namespace tvwindow
}  // namespace extension
