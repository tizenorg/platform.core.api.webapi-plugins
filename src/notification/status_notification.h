// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NOTIFICATION_STATUS_NOTIFICATION_H_
#define NOTIFICATION_STATUS_NOTIFICATION_H_

#include <notification.h>
#include <app_control.h>

#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace notification {

typedef std::map<int, notification_text_type_e> InformationEnumMap;
typedef std::map<int, notification_image_type_e> ImageEnumMap;

class StatusNotification {
 public:
  static common::PlatformResult ToJson(int id, notification_h noti_handle,
                                       app_control_h app_handle,
                                       picojson::object* out_ptr);
  static common::PlatformResult GetAppControl(notification_h noti_handle,
                                              app_control_h* app_control);

 private:
  StatusNotification();
  virtual ~StatusNotification();

  static const InformationEnumMap info_map_;
  static const InformationEnumMap info_sub_map_;
  static const ImageEnumMap thumbnails_map_;

  static common::PlatformResult StatusTypeFromPlatform(
      notification_type_e noti_type,
      notification_ly_type_e noti_layout,
      std::string* type);
  static common::PlatformResult GetImage(notification_h noti_handle,
                                         notification_image_type_e image_type,
                                         std::string* image_path);
  static common::PlatformResult GetText(notification_h noti_handle,
                                        notification_text_type_e text_type,
                                        std::string* noti_text);
  static common::PlatformResult GetNumber(notification_h noti_handle,
                                          notification_text_type_e text_type,
                                          long* number);
  static common::PlatformResult GetDetailInfos(notification_h noti_handle,
                                               picojson::array* out);
  static common::PlatformResult GetLedColor(notification_h noti_handle,
                                            std::string* led_color);
  static common::PlatformResult GetLedPeriod(notification_h noti_handle,
                                             unsigned long* on_period,
                                             unsigned long* off_period);
  static common::PlatformResult GetThumbnails(notification_h noti_handle,
                                              picojson::array* out);
  static common::PlatformResult GetSoundPath(notification_h noti_handle,
                                             std::string* sound_path);
  static common::PlatformResult GetVibration(notification_h noti_handle,
                                             bool* vibration);
  static common::PlatformResult GetApplicationControl(
      app_control_h app_handle,
      picojson::object* out_ptr);
  static common::PlatformResult GetApplicationId(app_control_h app_handle,
                                                 std::string* app_id);
  static common::PlatformResult GetProgressValue(
      notification_h noti_handle,
      const std::string& progess_type,
      double* progress_value);
  static common::PlatformResult GetPostedTime(notification_h noti_handle,
                                              time_t* posted_time);
};

}  // namespace notification
}  // namespace extension

#endif /* NOTIFICATION_STATUS_NOTIFICATION_H_ */
