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

static bool ServiceExtraDataCb(app_control_h service, const char* key,
                                  void* user_data) {
  if (user_data != NULL && key != NULL) {
    LoggerE("User data or key not exist");
    return true;
  }

  picojson::array* control_data = static_cast<picojson::array*>(user_data);

  int length = 0;
  char** value = NULL;
  SCOPE_EXIT {
      free(value);
  };

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

PlatformResult StatusNotification::StatusTypeFromPlatform(
    notification_type_e noti_type, notification_ly_type_e noti_layout,
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

PlatformResult StatusNotification::GetApplicationControl(
    app_control_h app_handle, picojson::object* out_ptr) {
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

PlatformResult StatusNotification::GetApplicationId(app_control_h app_handle,
                                                    std::string* app_id) {
  char* app_id_str = NULL;
  SCOPE_EXIT {
      free(app_id_str);
  };

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

PlatformResult StatusNotification::GetProgressValue(
    notification_h noti_handle, const std::string& progess_type,
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

}  // namespace notification
}  // namespace extension
