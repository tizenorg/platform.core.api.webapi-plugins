/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd All Rights Reserved
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

#include "widget_utils.h"

#include <widget_service.h>
#include <widget_errno.h>

namespace extension {
namespace widget {

const std::string kWidgetId = "widgetId";
const std::string kPackageId = "packageId";
const std::string kId = "id";
const std::string kApplicationId = "applicationId";
const std::string kSetupApplicationId = "setupApplicationId";
const std::string kNoDisplay = "noDisplay";

using common::TizenResult;
using common::TizenSuccess;

TizenResult WidgetUtils::ConvertErrorCode(int error) {
  switch (error) {
    case WIDGET_ERROR_NONE:
      return TizenSuccess();
    case WIDGET_ERROR_IO_ERROR:
      return common::IoError(error);
    case WIDGET_ERROR_INVALID_PARAMETER:
      return common::InvalidValuesError(error);
    case WIDGET_ERROR_RESOURCE_BUSY:
      return common::ServiceNotAvailableError(error);
    case WIDGET_ERROR_PERMISSION_DENIED:
      return common::PermissionDeniedError(error);
    case WIDGET_ERROR_TIMED_OUT:
      return common::TimeoutError(error);
    case WIDGET_ERROR_NOT_SUPPORTED:
    case WIDGET_ERROR_DISABLED:
      return common::NotSupportedError(error);
    case WIDGET_ERROR_CANCELED:
      return common::OperationCanceledError(error);
    case WIDGET_ERROR_OUT_OF_MEMORY:
    case WIDGET_ERROR_FILE_NO_SPACE_ON_DEVICE:
    case WIDGET_ERROR_FAULT:
    case WIDGET_ERROR_ALREADY_EXIST:
    case WIDGET_ERROR_ALREADY_STARTED:
    case WIDGET_ERROR_NOT_EXIST:
    default:
      return common::AbortError(error);
  }
}

TizenResult WidgetUtils::WidgetToJson(const char* id, picojson::object* out, const char* pkgid) {
  ScopeLogger();

  //applicationId
  char* tmp_str = widget_service_get_main_app_id(id);
  if (!tmp_str) {
    LogAndReturnTizenError(
        ConvertErrorCode(get_last_result()), ("widget_service_get_main_app_id() failed"));
  }
  out->insert(std::make_pair(kApplicationId, picojson::value(tmp_str)));
  free(tmp_str);

  //setupApplicationId
  tmp_str = widget_service_get_app_id_of_setup_app(id);
  if (!tmp_str) {
    if (WIDGET_ERROR_NONE != get_last_result()) {
      LogAndReturnTizenError(
          ConvertErrorCode(get_last_result()), ("widget_service_get_app_id_of_setup_app() failed"));
    }
  } else {
    out->insert(std::make_pair(kSetupApplicationId, picojson::value(tmp_str)));
    free(tmp_str);
  }

  //packageId
  if (!pkgid) {
    tmp_str = widget_service_get_package_id(id);
    if (!tmp_str) {
      LogAndReturnTizenError(
          ConvertErrorCode(get_last_result()), ("widget_service_get_package_id() failed"));
    }
    out->insert(std::make_pair(kPackageId, picojson::value(tmp_str)));
    free(tmp_str);
  }

  //noDisplay
  bool tmp_bool = widget_service_get_nodisplay(id);
  if (WIDGET_ERROR_NONE != get_last_result()) {
    LogAndReturnTizenError(
        ConvertErrorCode(get_last_result()), ("widget_service_get_nodisplay() failed"));
  }
  out->insert(std::make_pair(kNoDisplay, picojson::value(tmp_bool)));

  //id
  out->insert(std::make_pair(kId, picojson::value(id)));

  return TizenSuccess();
}

} // widget
} // extension
