// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "notification/status_notification.h"

#include <notification_internal.h>
#include <app_control_internal.h>

#include "common/converter.h"
#include "common/logger.h"
#include "common/scope_exit.h"

namespace extension {
namespace notification {

using namespace common;

const std::string kProgressTypePercentage = "PERCENTAGE";
const std::string kProgressTypeByte = "BYTE";

const InformationEnumMap StatusNotification::info_map_ = {
    {0, NOTIFICATION_TEXT_TYPE_INFO_1},
    {1, NOTIFICATION_TEXT_TYPE_INFO_2},
    {2, NOTIFICATION_TEXT_TYPE_INFO_3}};

const InformationEnumMap StatusNotification::info_sub_map_ = {
    {0, NOTIFICATION_TEXT_TYPE_INFO_SUB_1},
    {1, NOTIFICATION_TEXT_TYPE_INFO_SUB_2},
    {2, NOTIFICATION_TEXT_TYPE_INFO_SUB_3}};

const ImageEnumMap StatusNotification::thumbnails_map_ = {
    {0, NOTIFICATION_IMAGE_TYPE_LIST_1},
    {1, NOTIFICATION_IMAGE_TYPE_LIST_2},
    {2, NOTIFICATION_IMAGE_TYPE_LIST_3},
    {3, NOTIFICATION_IMAGE_TYPE_LIST_4},
    {4, NOTIFICATION_IMAGE_TYPE_LIST_5}};

StatusNotification::StatusNotification() {}

StatusNotification::~StatusNotification() {}

bool StatusNotification::IsColorFormatNumberic(const std::string& color) {
  std::string hexCode = "0123456789abcdef";
  if (color.length() != 7 || !color.compare(0, 1, "#")) {
    return false;
  }

  for (size_t i = 1; i < color.length(); i++) {
    if (std::string::npos == hexCode.find(color[i])) {
      return false;
    }
  }

  return true;
}

PlatformResult StatusNotification::SetLayout(notification_h noti_handle,
                                             const std::string& noti_type) {
  notification_ly_type_e noti_layout = NOTIFICATION_LY_NONE;

  if (noti_type == "SIMPLE") {
    long number;
    PlatformResult status =
        GetNumber(noti_handle, NOTIFICATION_TEXT_TYPE_EVENT_COUNT, &number);
    if (status.IsError()) return status;

    if (number > 0)
      noti_layout = NOTIFICATION_LY_NOTI_EVENT_MULTIPLE;
    else
      noti_layout = NOTIFICATION_LY_NOTI_EVENT_SINGLE;
  } else if (noti_type == "THUMBNAIL") {
    noti_layout = NOTIFICATION_LY_NOTI_THUMBNAIL;
  }
  if (noti_type == "ONGOING") {
    noti_layout = NOTIFICATION_LY_ONGOING_EVENT;
  } else if (noti_type == "PROGRESS") {
    noti_layout = NOTIFICATION_LY_ONGOING_PROGRESS;
  }

  int ret = notification_set_layout(noti_handle, noti_layout);
  if (ret != NOTIFICATION_ERROR_NONE) {
    LoggerE("Set notification layout error: %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Set notification layout error");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::GetId(notification_h noti_handle, int* id) {
  int ret = notification_get_id(noti_handle, NULL, id);
  if (ret != NOTIFICATION_ERROR_NONE) {
    LoggerE("Get notification id error: %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Get notification id error");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

static bool ServiceExtraDataCb(app_control_h service, const char* key,
                               void* user_data) {
  if (user_data != NULL && key != NULL) {
    LoggerE("User data or key not exist");
    return true;
  }

  picojson::array* control_data = static_cast<picojson::array*>(user_data);

  int length = 0;
  char** value = NULL;
  SCOPE_EXIT { free(value); };

  int ret = app_control_get_extra_data_array(service, key, &value, &length);
  if (ret != APP_CONTROL_ERROR_NONE) {
    LoggerE("Get app control extra data error: %d", ret);
    return true;
  }

  if (!value || !length) {
    LoggerE("Get app control extra data value error");
    return true;
  }

  picojson::array values = picojson::array();
  for (int index = 0; index < length; ++index) {
    values.push_back(picojson::value(value[index]));
  }

  picojson::object data_control_elem = picojson::object();
  data_control_elem["key"] = picojson::value(key);
  data_control_elem["value"] = picojson::value(values);

  control_data->push_back(picojson::value(data_control_elem));

  return true;
}

PlatformResult StatusNotification::Create(notification_type_e noti_type,
                                          notification_h noti_handle) {
  noti_handle = notification_create(noti_type);
  if (!noti_handle) {
    LoggerE("Cannot make new notification object");
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Cannot make new notification object");
  }

  if (NOTIFICATION_TYPE_ONGOING == noti_type) {
    int ret = notification_set_display_applist(
        noti_handle, NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY |
                         NOTIFICATION_DISPLAY_APP_INDICATOR);
    if (ret != NOTIFICATION_ERROR_NONE) {
      LoggerE("Cannot make new notification object: %d", ret);
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Cannot set notification display applist");
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::StatusTypeFromPlatform(
    notification_type_e noti_type,
    notification_ly_type_e noti_layout,
    std::string* type) {
  if (noti_type == NOTIFICATION_TYPE_NOTI) {
    if (noti_layout == NOTIFICATION_LY_NOTI_EVENT_SINGLE ||
        noti_layout == NOTIFICATION_LY_NOTI_EVENT_MULTIPLE) {
      *type = "SIMPLE";
    } else if (noti_layout == NOTIFICATION_LY_NOTI_THUMBNAIL) {
      *type = "THUMBNAIL";
    }
  } else if (noti_type == NOTIFICATION_TYPE_ONGOING) {
    if (noti_layout == NOTIFICATION_LY_ONGOING_EVENT) {
      *type = "ONGOING";
    } else if (noti_layout == NOTIFICATION_LY_ONGOING_PROGRESS) {
      *type = "PROGRESS";
    }
  } else {
    LoggerE("Notification type not found");
    return PlatformResult(ErrorCode::NOT_FOUND_ERR,
                          "Notification type not found");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::StatusTypeToPlatform(
    const std::string& type,
    notification_type_e* noti_type) {
  if (type == "SIMPLE" || type == "THUMBNAIL") {
    *noti_type = NOTIFICATION_TYPE_NOTI;
  } else if (type == "ONGOING" || type == "PROGRESS") {
    *noti_type = NOTIFICATION_TYPE_ONGOING;
  } else {
    LoggerI("Invalide noti type: %s", type.c_str());
    return PlatformResult(ErrorCode::TYPE_MISMATCH_ERR,
                          "Invalide notification type");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::GetImage(
    notification_h noti_handle, notification_image_type_e image_type,
    std::string* image_path) {
  char* path = NULL;

  if (notification_get_image(noti_handle, image_type, &path) !=
      NOTIFICATION_ERROR_NONE) {
    LoggerE("Get notification image error, image_type: %d", image_type);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Get notification image error");
  }

  *image_path = path;

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::SetImage(
    notification_h noti_handle,
    notification_image_type_e image_type,
    const std::string& image_path) {
  int ret = notification_set_image(noti_handle, image_type, image_path.c_str());
  if (ret != NOTIFICATION_ERROR_NONE) {
    LoggerE("Set notification image error, image_type: %d, error: %d",
            image_type, ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Set notification image error");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::GetText(notification_h noti_handle,
                                           notification_text_type_e text_type,
                                           std::string* noti_text) {
  char* text = NULL;

  *noti_text = "";

  if (notification_get_text(noti_handle, text_type, &text) !=
      NOTIFICATION_ERROR_NONE) {
    LoggerE("Get notification text error, text_type: %d", text_type);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Get notification text error");
  }

  if (text) *noti_text = text;

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::SetText(notification_h noti_handle,
                                           notification_text_type_e text_type,
                                           const std::string& noti_text) {
  int ret = notification_set_text(noti_handle, text_type, noti_text.c_str(),
                                  NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  if (ret != NOTIFICATION_ERROR_NONE) {
    LoggerE("Set notification text error, text_type: %d, error: %d", text_type,
            ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Set notification text error");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::GetNumber(notification_h noti_handle,
                                             notification_text_type_e text_type,
                                             long* number) {
  std::string text;
  PlatformResult status = GetText(noti_handle, text_type, &text);
  if (status.IsError()) return status;

  if (text.length())
    *number = std::stol(text);
  else
    *number = -1;

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::GetDetailInfos(notification_h noti_handle,
                                                  picojson::array* out) {
  if (info_map_.size() != info_sub_map_.size()) {
    LoggerE("Different notification information types element size");
    return PlatformResult(
        ErrorCode::VALIDATION_ERR,
        "Different notification information types element size");
  }

  picojson::value detail_info = picojson::value(picojson::object());
  picojson::object& detail_info_obj = detail_info.get<picojson::object>();

  std::string text;
  for (int idx = 0; idx < info_map_.size(); ++idx) {
    PlatformResult status = GetText(noti_handle, info_map_.at(idx), &text);
    if (status.IsError()) return status;

    if (!text.length()) break;

    detail_info_obj["mainText"] = picojson::value(text);

    status = GetText(noti_handle, info_sub_map_.at(idx), &text);
    if (status.IsError()) return status;

    if (text.length()) {
      detail_info_obj["subText"] = picojson::value(text);
    }

    out->push_back(detail_info);
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::SetDetailInfos(
    notification_h noti_handle,
    const picojson::array& value) {
  int idx = 0;

  for (auto& item : value) {
    const picojson::object& obj = JsonCast<picojson::object>(item);

    PlatformResult status =
        SetText(noti_handle, info_map_.at(idx),
                common::FromJson<std::string>(obj, "mainText"));
    if (status.IsError()) return status;

    if (!IsNull(obj, "subText")) {
      PlatformResult status =
          SetText(noti_handle, info_sub_map_.at(idx),
                  common::FromJson<std::string>(obj, "subText"));
      if (status.IsError()) return status;
    }

    ++idx;

    if (idx > info_map_.size()) {
      LoggerE("Too many values in notification detailInfo array");
      return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                            "Too many values in notification detailInfo array");
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::GetLedColor(notification_h noti_handle,
                                               std::string* led_color) {
  unsigned int color = 0;
  notification_led_op_e type = NOTIFICATION_LED_OP_ON;

  if (notification_get_led(noti_handle, &type, (int*)&color) !=
      NOTIFICATION_ERROR_NONE) {
    LoggerE("Get notification led displaying option error");
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Get notification led displaying option error");
  }

  *led_color = "";
  std::stringstream stream;

  if (NOTIFICATION_LED_OP_OFF != type) {
    color = 0x00FFFFFF & color;
    stream << std::hex << color;
    *led_color = "#" + stream.str();

    while (led_color->length() < 7) {
      (*led_color).insert(1, "0");
    }

    std::transform(led_color->begin(), led_color->end(), led_color->begin(),
                   ::tolower);
  }

  LoggerD("color:%s", led_color->c_str());

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::SetLedColor(notification_h noti_handle,
                                               const std::string& led_color) {
  std::string color_str = led_color;
  std::transform(color_str.begin(), color_str.end(), color_str.begin(),
                 ::tolower);

  if (!IsColorFormatNumberic(color_str)) {
    LoggerE("Led color is not numeric value: %s", color_str.c_str());
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                          "Led color is not numeric value");
  }

  std::stringstream stream;
  unsigned int color = 0;
  notification_led_op_e type = NOTIFICATION_LED_OP_ON;
  std::string color_code =
      color_str.substr(1, color_str.length()).insert(0, "ff");

  stream << std::hex << color_code;
  stream >> color;

  if (color != 0)
    type = NOTIFICATION_LED_OP_ON_CUSTOM_COLOR;
  else
    type = NOTIFICATION_LED_OP_OFF;

  int ret = notification_set_led(noti_handle, type, static_cast<int>(color));
  if (ret != NOTIFICATION_ERROR_NONE) {
    LoggerE("Set notification led color eror: %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Set notification led color eror");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::GetLedPeriod(notification_h noti_handle,
                                                unsigned long* on_period,
                                                unsigned long* off_period) {
  int on_time = 0;
  int off_time = 0;

  if (notification_get_led_time_period(noti_handle, &on_time, &off_time) !=
      NOTIFICATION_ERROR_NONE) {
    LoggerE("Get notification led on/off period error");
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Get notification led on/off period error");
  }

  if (on_period) *on_period = on_time;
  if (off_period) *off_period = off_time;

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::SetLedOnPeriod(notification_h noti_handle,
                                                  unsigned long on_period) {
  unsigned long off_period = 0;
  PlatformResult status = GetLedPeriod(noti_handle, nullptr, &off_period);
  if (status.IsError()) return status;

  int ret =
      notification_set_led_time_period(noti_handle, on_period, off_period);
  if (ret != NOTIFICATION_ERROR_NONE) {
    LoggerE("Set notification led on period error: %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Set notification led on period error");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::SetLedOffPeriod(notification_h noti_handle,
                                                   unsigned long off_period) {
  unsigned long on_period = 0;
  PlatformResult status = GetLedPeriod(noti_handle, &on_period, nullptr);
  if (status.IsError()) return status;

  int ret =
      notification_set_led_time_period(noti_handle, on_period, off_period);
  if (ret != NOTIFICATION_ERROR_NONE) {
    LoggerE("Set notification led off period error: %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Set notification led off period error");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::GetThumbnails(notification_h noti_handle,
                                                 picojson::array* out) {
  std::string text;
  for (int idx = 0; idx < thumbnails_map_.size(); ++idx) {
    PlatformResult status =
        GetImage(noti_handle, thumbnails_map_.at(idx), &text);
    if (status.IsError()) return status;

    if (!text.length()) break;

    out->push_back(picojson::value(text));
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::SetThumbnails(notification_h noti_handle,
                                                 const picojson::array& value) {
  int idx = 0;

  for (auto& item : value) {
    const std::string& text = JsonCast<std::string>(item);

    PlatformResult status =
        SetImage(noti_handle, thumbnails_map_.at(idx), text);
    if (status.IsError()) return status;

    ++idx;

    if (idx > thumbnails_map_.size()) {
      LoggerE("Too many values in notification thumbnail array");
      return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                            "Too many values in notification thumbnail array");
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::GetSoundPath(notification_h noti_handle,
                                                std::string* sound_path) {
  *sound_path = "";

  if (noti_handle) {
    LoggerE("Null notification handle");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Null notification handle");
  }

  const char* path = NULL;
  notification_sound_type_e type = NOTIFICATION_SOUND_TYPE_NONE;

  if (notification_get_sound(noti_handle, &type, &path) !=
      NOTIFICATION_ERROR_NONE) {
    LoggerE("Get notification sound error");
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Get notification sound error");
  }

  LoggerD("Sound type = %d", type);

  if (path && (type == NOTIFICATION_SOUND_TYPE_USER_DATA)) {
    *sound_path = path;
  }

  LoggerD("Sound path = %s", sound_path->c_str());

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::SetSoundPath(notification_h noti_handle,
                                                const std::string& sound_path) {
  int ret = notification_set_sound(
      noti_handle, NOTIFICATION_SOUND_TYPE_USER_DATA, sound_path.c_str());
  if (ret != NOTIFICATION_ERROR_NONE) {
    LoggerE("Set notification sound error: %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Set notification sound error");
  }

  LoggerD("Sound path = %s", sound_path.c_str());

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::GetVibration(notification_h noti_handle,
                                                bool* vibration) {
  notification_vibration_type_e vib_type = NOTIFICATION_VIBRATION_TYPE_NONE;

  if (notification_get_vibration(noti_handle, &vib_type, NULL) !=
      NOTIFICATION_ERROR_NONE) {
    LoggerE("Get notification vibration error");
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Get notification vibration error");
  }

  if (NOTIFICATION_VIBRATION_TYPE_DEFAULT == vib_type ||
      NOTIFICATION_VIBRATION_TYPE_USER_DATA == vib_type) {
    *vibration = true;
  } else {
    *vibration = false;
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::SetVibration(notification_h noti_handle,
                                                bool vibration) {
  bool platform_vibration;
  PlatformResult status = GetVibration(noti_handle, &platform_vibration);
  if (status.IsError()) return status;

  if (platform_vibration != vibration) {
    notification_vibration_type_e vib_type = NOTIFICATION_VIBRATION_TYPE_NONE;

    if (vibration) {
      vib_type = NOTIFICATION_VIBRATION_TYPE_DEFAULT;
    }

    int ret = notification_set_vibration(noti_handle, vib_type, NULL);
    if (ret != NOTIFICATION_ERROR_NONE) {
      LoggerE("Set notification vibration error: %d", ret);
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Set notification vibration error");
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::GetApplicationControl(
    app_control_h app_handle,
    picojson::object* out_ptr) {
  picojson::object& out = *out_ptr;

  char* operation = NULL;
  char* uri = NULL;
  char* mime = NULL;
  char* category = NULL;
  SCOPE_EXIT {
    free(operation);
    free(uri);
    free(mime);
    free(category);
  };

  if (app_control_get_operation(app_handle, &operation) !=
      APP_CONTROL_ERROR_NONE) {
    LoggerE("Get application control operation error");
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Get application control operation error");
  }
  out["operation"] = picojson::value(operation);
  LoggerD("operation = %s", operation);

  if (app_control_get_uri(app_handle, &uri) != APP_CONTROL_ERROR_NONE) {
    LoggerE("Get application control uri error");
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Get application control uri error");
  }
  if (uri) {
    out["uri"] = picojson::value(uri);
    LoggerD("uri = %s", uri);
  }

  if (app_control_get_mime(app_handle, &mime) != APP_CONTROL_ERROR_NONE) {
    LoggerE("Get application control mime error");
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Get application control mime error");
  }
  if (mime) {
    out["mime"] = picojson::value(mime);
    LoggerD("mime = %s", mime);
  }

  if (app_control_get_category(app_handle, &category) !=
      APP_CONTROL_ERROR_NONE) {
    LoggerE("Get application control category error");
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Get application control category error");
  }
  if (category) {
    out["category"] = picojson::value(category);
    LoggerD("category = %s", category);
  }

  picojson::array app_control_data = picojson::array();
  if (app_control_foreach_extra_data(app_handle, ServiceExtraDataCb,
                                     (void*)&app_control_data) !=
      APP_CONTROL_ERROR_NONE) {
    LoggerE("Get application control data error");
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Get application control data error");
  }
  out["data"] = picojson::value(app_control_data);

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::SetApplicationControl(
    app_control_h app_handle,
    const picojson::object& app_ctrl) {
  const std::string& operation =
      common::FromJson<std::string>(app_ctrl, "operation");
  int ret = app_control_set_operation(app_handle, operation.c_str());
  if (ret != APP_CONTROL_ERROR_NONE) {
    LoggerE("Set application control operation error: %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Set application control operation error");
  }

  if (!IsNull(app_ctrl, "uri")) {
    const std::string& uri = common::FromJson<std::string>(app_ctrl, "uri");
    ret = app_control_set_uri(app_handle, uri.c_str());
    if (ret != APP_CONTROL_ERROR_NONE) {
      LoggerE("Set application control uri error: %d", ret);
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Set application control uri error");
    }
  }

  if (!IsNull(app_ctrl, "mime")) {
    const std::string& mime = common::FromJson<std::string>(app_ctrl, "mime");
    ret = app_control_set_mime(app_handle, mime.c_str());
    if (ret != APP_CONTROL_ERROR_NONE) {
      LoggerE("Set application control mime error: %d", ret);
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Set application control mime error");
    }
  }

  if (!IsNull(app_ctrl, "category")) {
    const std::string& category =
        common::FromJson<std::string>(app_ctrl, "category");
    ret = app_control_set_category(app_handle, category.c_str());
    if (ret != APP_CONTROL_ERROR_NONE) {
      LoggerE("Set application control category error: %d", ret);
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Set application control category error");
    }
  }

  int idx = 0;
  const picojson::array& data =
      common::FromJson<picojson::array>(app_ctrl, "data");
  for (auto& item : data) {
    const picojson::object& obj = JsonCast<picojson::object>(item);

    const std::string key = common::FromJson<std::string>(obj, "key");
    const picojson::array values =
        common::FromJson<picojson::array>(obj, "value");

    const char** arrayValue =
        (const char**)calloc(sizeof(char*), values.size());
    SCOPE_EXIT { free(arrayValue); };
    idx = 0;
    for (auto& item : values) {
      arrayValue[idx] = JsonCast<std::string>(item).c_str();
      ++idx;
    }

    ret = app_control_add_extra_data_array(app_handle, key.c_str(), arrayValue,
                                           values.size());
    if (ret != APP_CONTROL_ERROR_NONE) {
      LoggerE("Set application control extra data error: %d", ret);
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Set application control extra data error");
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::GetApplicationId(app_control_h app_handle,
                                                    std::string* app_id) {
  char* app_id_str = NULL;
  SCOPE_EXIT { free(app_id_str); };

  if (app_control_get_app_id(app_handle, &app_id_str) !=
      APP_CONTROL_ERROR_NONE) {
    LoggerE("Get applicaiton ID failed");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Get applicaiton ID failed");
  }

  if (app_id_str != NULL) {
    *app_id = app_id_str;
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::SetApplicationId(app_control_h app_handle,
                                                    const std::string& app_id) {
  int ret = app_control_set_app_id(app_handle, app_id.c_str());
  if (ret != APP_CONTROL_ERROR_NONE) {
    LoggerE("Set applicaiton ID error: %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Set applicaiton ID error");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::GetProgressValue(
    notification_h noti_handle,
    const std::string& progess_type,
    double* progress_value) {
  *progress_value = 0.0;

  if (progess_type == kProgressTypeByte) {
    if (notification_get_size(noti_handle, progress_value) !=
        NOTIFICATION_ERROR_NONE) {
      LoggerE("Get notification size error");
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Get notification size error");
    }
    LoggerD("Size value = %f", *progress_value);
  } else if (progess_type == kProgressTypePercentage) {
    if (notification_get_progress(noti_handle, progress_value) !=
        NOTIFICATION_ERROR_NONE) {
      LoggerE("Get notification progress error");
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Get notification progress error");
    }
    LoggerD("Percentage value = %f", *progress_value);
  } else {
    LoggerE("Unknown notification progress type: %s ", progess_type.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Unknown notification progress type");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::SetProgressValue(
    notification_h noti_handle,
    const std::string& progess_type,
    double progress_value) {
  int ret;

  if (progess_type == kProgressTypeByte) {
    ret = notification_set_size(noti_handle, progress_value);
    if (ret != NOTIFICATION_ERROR_NONE) {
      LoggerE("Set notification size error: %d", ret);
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Set notification size error");
    }
  } else if (progess_type == kProgressTypePercentage) {
    ret = notification_set_progress(noti_handle, progress_value);
    if (ret != NOTIFICATION_ERROR_NONE) {
      LoggerE("Set notification progress error: %d", ret);
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Set notification progress error");
    }
  } else {
    LoggerE("Unknown notification progress type: %s ", progess_type.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Unknown notification progress type");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::GetPostedTime(notification_h noti_handle,
                                                 time_t* posted_time) {
  *posted_time = 0;

  if (notification_get_insert_time(noti_handle, posted_time) !=
      NOTIFICATION_ERROR_NONE) {
    LoggerE("Get notification posted time error");
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Get notification posted time error");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::GetAppControl(notification_h noti_handle,
                                                 app_control_h* app_control) {
  bundle* service = NULL;

  int ret = notification_get_execute_option(
      noti_handle, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, NULL, &service);
  if (ret == NOTIFICATION_ERROR_NONE) {
    LoggerE("Get notification execute option error: %d", ret);
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                          "Get notification execute option error");
  }

  if (!service) {
    LoggerE("Null service_handle Appsvc bundle data");
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Null service_handle Appsvc bundle data");
  }

  ret = app_control_create(app_control);
  if (ret != APP_CONTROL_ERROR_NONE) {
    LoggerE("Fail to create app_control");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Fail to create app_control");
  }

  ret = app_control_import_from_bundle(*app_control, service);
  if (ret != APP_CONTROL_ERROR_NONE) {
    LoggerI("Application control create event error: %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Application control create event error");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::ToJson(int id, notification_h noti_handle,
                                          app_control_h app_handle,
                                          picojson::object* out_ptr) {
  if (noti_handle) {
    LoggerE("Null notification handle");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Null notification handle");
  }

  if (app_handle) {
    LoggerE("Null application control handle");
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Null application control handle");
  }

  picojson::object& out = *out_ptr;

  out["id"] = picojson::value(std::to_string(id));
  out["type"] = picojson::value("STATUS");

  // Nitification type
  notification_type_e noti_type = NOTIFICATION_TYPE_NONE;
  int ret = notification_get_type(noti_handle, &noti_type);
  if (ret != NOTIFICATION_ERROR_NONE) {
    LoggerE("Notification get type error: %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Notification get type error");
  }

  notification_ly_type_e noti_layout = NOTIFICATION_LY_NONE;
  ret = notification_get_layout(noti_handle, &noti_layout);
  if (ret != NOTIFICATION_ERROR_NONE) {
    LoggerE("Notification get layout error: %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Notification get layout error");
  }

  std::string noti_type_str;
  PlatformResult status =
      StatusTypeFromPlatform(noti_type, noti_layout, &noti_type_str);
  if (status.IsError()) return status;
  out["statusType"] = picojson::value(noti_type_str);

  std::string value_str;
  status = GetImage(noti_handle, NOTIFICATION_IMAGE_TYPE_ICON, &value_str);
  if (status.IsError()) return status;
  if (value_str.length()) {
    out["iconPath"] = picojson::value(value_str);
  }

  status = GetImage(noti_handle, NOTIFICATION_IMAGE_TYPE_ICON_SUB, &value_str);
  if (status.IsError()) return status;
  if (value_str.length()) {
    out["subIconPath"] = picojson::value(value_str);
  }

  long number;
  status = GetNumber(noti_handle, NOTIFICATION_TEXT_TYPE_EVENT_COUNT, &number);
  if (status.IsError()) return status;
  if (number >= 0) {
    out["number"] = picojson::value(static_cast<double>(number));
  }

  picojson::array detail_infos = picojson::array();
  status = GetDetailInfos(noti_handle, &detail_infos);
  if (status.IsError()) return status;
  if (detail_infos.size()) {
    out["detailInfo"] = picojson::value(detail_infos);
  }

  status = GetLedColor(noti_handle, &value_str);
  if (status.IsError()) return status;
  if (value_str.length()) {
    out["ledColor"] = picojson::value(value_str);
  }

  unsigned long on_period;
  unsigned long off_period;
  status = GetLedPeriod(noti_handle, &on_period, &off_period);
  if (status.IsError()) return status;
  out["ledOnPeriod"] = picojson::value(static_cast<double>(on_period));
  out["ledOffPeriod"] = picojson::value(static_cast<double>(off_period));

  status =
      GetImage(noti_handle, NOTIFICATION_IMAGE_TYPE_BACKGROUND, &value_str);
  if (status.IsError()) return status;
  if (value_str.length()) {
    out["backgroundImagePath"] = picojson::value(value_str);
  }

  picojson::array thumbnails = picojson::array();
  status = GetThumbnails(noti_handle, &thumbnails);
  if (status.IsError()) return status;
  if (thumbnails.size()) {
    out["thumbnails"] = picojson::value(thumbnails);
  }

  status = GetSoundPath(noti_handle, &value_str);
  if (status.IsError()) return status;
  if (value_str.length()) {
    out["soundPath"] = picojson::value(value_str);
  }

  bool vibration;
  status = GetVibration(noti_handle, &vibration);
  if (status.IsError()) return status;
  out["vibration"] = picojson::value(vibration);

  picojson::object app_control = picojson::object();
  status = GetApplicationControl(app_handle, &app_control);
  if (status.IsError()) return status;
  if (app_control.size()) {
    out["appControl"] = picojson::value(app_control);
  }

  status = GetApplicationId(app_handle, &value_str);
  if (status.IsError()) return status;
  if (value_str.length()) {
    out["appId"] = picojson::value(value_str);
  }

  std::string progress_type;
  status =
      GetImage(noti_handle, NOTIFICATION_IMAGE_TYPE_LIST_5, &progress_type);
  if (status.IsError()) return status;
  out["progressType"] = picojson::value(progress_type);

  double progress_value;
  status = GetProgressValue(noti_handle, progress_type, &progress_value);
  if (status.IsError()) return status;
  if (value_str.length()) {
    out["progressValue"] = picojson::value(progress_value);
  }

  time_t posted_time;
  status = GetPostedTime(noti_handle, &posted_time);
  if (status.IsError()) return status;
  out["postedTime"] = picojson::value(static_cast<double>(posted_time));

  status = GetText(noti_handle, NOTIFICATION_TEXT_TYPE_TITLE, &value_str);
  if (status.IsError()) return status;
  out["title"] = picojson::value(value_str);

  status = GetText(noti_handle, NOTIFICATION_TEXT_TYPE_CONTENT, &value_str);
  if (status.IsError()) return status;
  if (value_str.length()) {
    out["content"] = picojson::value(value_str);
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult StatusNotification::FromJson(const picojson::object& args,
                                            int* id) {
  const std::string& status_type =
      common::FromJson<std::string>(args, "statusType");

  notification_type_e noti_type;
  PlatformResult status = StatusTypeToPlatform(status_type, &noti_type);
  if (status.IsError()) return status;

  notification_h noti_handle;
  status = Create(noti_type, noti_handle);
  if (status.IsError()) return status;

  status = GetId(noti_handle, id);
  if (status.IsError()) return status;

  status = SetLayout(noti_handle, status_type);
  if (status.IsError()) return status;

  if (!IsNull(args, "iconPath")) {
    const std::string& value_str =
        common::FromJson<std::string>(args, "iconPath");
    status = SetImage(noti_handle, NOTIFICATION_IMAGE_TYPE_ICON, value_str);
    if (status.IsError()) return status;
  }

  if (!IsNull(args, "subIconPath")) {
    const std::string& value_str =
        common::FromJson<std::string>(args, "subIconPath");
    status = SetImage(noti_handle, NOTIFICATION_IMAGE_TYPE_ICON_SUB, value_str);
    if (status.IsError()) return status;
  }

  if (!IsNull(args, "number")) {
    const std::string& value_str =
        std::to_string(common::FromJson<double>(args, "number"));
    status = SetImage(noti_handle, NOTIFICATION_IMAGE_TYPE_ICON_SUB, value_str);
    if (status.IsError()) return status;
  }

  if (!IsNull(args, "detailInfo")) {
    status = SetDetailInfos(
        noti_handle, common::FromJson<picojson::array>(args, "detailInfo"));
    if (status.IsError()) return status;
  }

  if (!IsNull(args, "ledColor")) {
    status = SetLedColor(noti_handle,
                         common::FromJson<std::string>(args, "ledColor"));
    if (status.IsError()) return status;
  }

  status = SetLedOnPeriod(noti_handle,
                          static_cast<unsigned long>(
                              common::FromJson<double>(args, "ledOnPeriod")));
  if (status.IsError()) return status;

  status = SetLedOffPeriod(noti_handle,
                           static_cast<unsigned long>(
                               common::FromJson<double>(args, "ledOffPeriod")));
  if (status.IsError()) return status;

  if (!IsNull(args, "thumbnails")) {
    status = SetThumbnails(
        noti_handle, common::FromJson<picojson::array>(args, "thumbnails"));
    if (status.IsError()) return status;
  }

  if (!IsNull(args, "soundPath")) {
    status = SetSoundPath(noti_handle,
                          common::FromJson<std::string>(args, "soundPath"));
    if (status.IsError()) return status;
  }

  status = SetVibration(noti_handle, common::FromJson<bool>(args, "vibration"));
  if (status.IsError()) return status;

  app_control_h app_control;
  status = GetAppControl(noti_handle, &app_control);
  if (status.IsError()) return status;
  if (!IsNull(args, "appControl")) {
    status = SetApplicationControl(
        app_control, common::FromJson<picojson::object>(args, "appControl"));
    if (status.IsError()) return status;
  }

  if (!IsNull(args, "appId")) {
    status = SetApplicationId(app_control,
                              common::FromJson<std::string>(args, "appId"));
    if (status.IsError()) return status;
  }

  const std::string& progress_type =
      common::FromJson<std::string>(args, "progressType");
  status = SetImage(noti_handle, NOTIFICATION_IMAGE_TYPE_LIST_5, progress_type);
  if (status.IsError()) return status;

  if (!IsNull(args, "progressValue")) {
    status = SetLedOnPeriod(noti_handle,
                            common::FromJson<double>(args, "progressValue"));
    if (status.IsError()) return status;
  }

  status = SetText(noti_handle, NOTIFICATION_TEXT_TYPE_TITLE,
                   common::FromJson<std::string>(args, "title"));
  if (status.IsError()) return status;

  if (!IsNull(args, "content")) {
    status = SetText(noti_handle, NOTIFICATION_TEXT_TYPE_CONTENT,
                     common::FromJson<std::string>(args, "content"));
    if (status.IsError()) return status;
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

}  // namespace notification
}  // namespace extension
