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

#include <widget_errno.h>

namespace extension {
namespace widget {

namespace {

#define WIDGET_SIZE_TYPE_E \
  X(WIDGET_SIZE_TYPE_1x1, "1x1") \
  X(WIDGET_SIZE_TYPE_2x1, "2x1") \
  X(WIDGET_SIZE_TYPE_2x2, "2x2") \
  X(WIDGET_SIZE_TYPE_4x1, "4x1") \
  X(WIDGET_SIZE_TYPE_4x2, "4x2") \
  X(WIDGET_SIZE_TYPE_4x3, "4x3") \
  X(WIDGET_SIZE_TYPE_4x4, "4x4") \
  X(WIDGET_SIZE_TYPE_4x5, "4x5") \
  X(WIDGET_SIZE_TYPE_4x6, "4x6") \
  X(WIDGET_SIZE_TYPE_EASY_1x1, "EASY_1x1") \
  X(WIDGET_SIZE_TYPE_EASY_3x1, "EASY_3x1") \
  X(WIDGET_SIZE_TYPE_EASY_3x3, "EASY_3x3") \
  X(WIDGET_SIZE_TYPE_FULL, "FULL") \
  XD(WIDGET_SIZE_TYPE_UNKNOWN, "unknown")

#define WIDGET_LIFECYCLE_EVENT_E \
  X(WIDGET_LIFE_CYCLE_EVENT_CREATE, "CREATE") \
  X(WIDGET_LIFE_CYCLE_EVENT_DESTROY, "DESTROY") \
  X(WIDGET_LIFE_CYCLE_EVENT_PAUSE, "PAUSE") \
  X(WIDGET_LIFE_CYCLE_EVENT_RESUME, "RESUME") \
  XD(WIDGET_LIFE_CYCLE_EVENT_MAX, "unknown")

} // namespace

const std::string kWidgetId = "widgetId";
const std::string kPackageId = "packageId";
const std::string kId = "id";
const std::string kApplicationId = "applicationId";
const std::string kSetupApplicationId = "setupApplicationId";
const std::string kNoDisplay = "noDisplay";
const std::string kSizeType = "sizeType";
const std::string kWidth = "width";
const std::string kHeight = "height";
const std::string kNeedsMouseEvents = "needsMouseEvents";
const std::string kNeedsTouchEffect = "needsTouchEffect";
const std::string kNeedsFrame = "needsFrame";
const std::string kPreviewImagePath = "previewImagePath";

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

TizenResult WidgetUtils::SizeToJson(widget_size_type_e type, picojson::object* out) {
  ScopeLogger();

  int width = 0;
  int height = 0;

  int ret = widget_service_get_size(type, &width, &height);
  if (WIDGET_ERROR_NONE != ret) {
    LogAndReturnTizenError(ConvertErrorCode(ret), ("widget_service_get_size() failed"));
  }

  out->insert(std::make_pair(kWidth, picojson::value(static_cast<double>(width))));
  out->insert(std::make_pair(kHeight, picojson::value(static_cast<double>(height))));

  return TizenSuccess();
}

TizenResult WidgetUtils::WidgetVariantToJson(
    const char* id, widget_size_type_e type, picojson::object* out) {
  ScopeLogger();

  bool tmp = false;

  //needsMouseEvents
  int ret = widget_service_get_need_of_mouse_event(id, type, &tmp);
  if (WIDGET_ERROR_NONE != ret) {
    LogAndReturnTizenError(
        ConvertErrorCode(ret), ("widget_service_get_need_of_mouse_event() failed"));
  }
  out->insert(std::make_pair(kNeedsMouseEvents, picojson::value(tmp)));

  //needsTouchEffect
  ret = widget_service_get_need_of_touch_effect(id, type, &tmp);
  if (WIDGET_ERROR_NONE != ret) {
    LogAndReturnTizenError(
        ConvertErrorCode(ret), ("widget_service_get_need_of_touch_effect() failed"));
  }
  out->insert(std::make_pair(kNeedsTouchEffect, picojson::value(tmp)));

  //needsFrame
  ret = widget_service_get_need_of_frame(id, type, &tmp);
  if (WIDGET_ERROR_NONE != ret) {
    LogAndReturnTizenError(
        ConvertErrorCode(ret), ("widget_service_get_need_of_frame() failed"));
  }
  out->insert(std::make_pair(kNeedsFrame, picojson::value(tmp)));

  //previewImagePath
  char* path = widget_service_get_preview_image_path(id, type);
  if (!path) {
    LogAndReturnTizenError(
        ConvertErrorCode(get_last_result()), ("widget_service_get_preview_image_path() failed"));
  }
  out->insert(std::make_pair(kPreviewImagePath, picojson::value(path)));
  free(path);

  return TizenSuccess();
}

#define X(v, s) case v: return s;
#define XD(v, s) \
  default: \
    LoggerE("Unknown value: %d, returning default: %s", e, s); \
    return s;

std::string WidgetUtils::FromSizeType(widget_size_type_e e) {
  ScopeLogger();

  switch (e) {
    WIDGET_SIZE_TYPE_E
  }
}

std::string WidgetUtils::FromEventType(widget_lifecycle_event_e e) {
  ScopeLogger();

  switch (e) {
    WIDGET_LIFECYCLE_EVENT_E
  }
}

#undef X
#undef XD

#define X(v, s) if (e == s) return v;
#define XD(v, s) \
  LoggerE("Unknown value: %s, returning default: %d", e.c_str(), v); \
  return v;

widget_size_type_e WidgetUtils::ToSizeType(const std::string& e) {
  ScopeLogger();

  WIDGET_SIZE_TYPE_E
}

#undef X
#undef XD

} // widget
} // extension
