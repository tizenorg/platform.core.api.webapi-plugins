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
  static common::PlatformResult ToJson(int id,
                                       notification_h noti_handle,
                                       app_control_h app_handle,
                                       picojson::object* out_ptr);
  static common::PlatformResult FromJson(const picojson::object& args,
                                         bool is_update,
                                         picojson::object* out_ptr);
  static common::PlatformResult GetAppControl(notification_h noti_handle,
                                              app_control_h* app_control);
  static common::PlatformResult GetNotiHandle(int id,
                                              notification_h* noti_handle);

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
  static common::PlatformResult StatusTypeToPlatform(
      const std::string& type,
      notification_type_e* noti_type);
  static common::PlatformResult Create(notification_type_e noti_type,
                                       notification_h* noti_handle);
  static common::PlatformResult GetImage(notification_h noti_handle,
                                         notification_image_type_e image_type,
                                         std::string* image_path);
  static common::PlatformResult SetImage(notification_h noti_handle,
                                         notification_image_type_e image_type,
                                         const std::string& image_path);
  static common::PlatformResult GetText(notification_h noti_handle,
                                        notification_text_type_e text_type,
                                        std::string* noti_text);
  static common::PlatformResult SetText(notification_h noti_handle,
                                        notification_text_type_e text_type,
                                        const std::string& noti_text);
  static common::PlatformResult GetNumber(notification_h noti_handle,
                                          notification_text_type_e text_type,
                                          long* number);
  static common::PlatformResult GetDetailInfos(notification_h noti_handle,
                                               picojson::array* out);
  static common::PlatformResult SetDetailInfos(notification_h noti_handle,
                                               const picojson::array& value);
  static common::PlatformResult GetLedColor(notification_h noti_handle,
                                            std::string* led_color);
  static common::PlatformResult SetLedColor(notification_h noti_handle,
                                            const std::string& led_color);
  static common::PlatformResult GetLedPeriod(notification_h noti_handle,
                                             unsigned long* on_period,
                                             unsigned long* off_period);
  static common::PlatformResult SetLedOnPeriod(notification_h noti_handle,
                                               unsigned long on_period);
  static common::PlatformResult SetLedOffPeriod(notification_h noti_handle,
                                                unsigned long off_period);
  static common::PlatformResult GetThumbnails(notification_h noti_handle,
                                              picojson::array* out);
  static common::PlatformResult SetThumbnails(notification_h noti_handle,
                                              const picojson::array& value);
  static common::PlatformResult GetSoundPath(notification_h noti_handle,
                                             std::string* sound_path);
  static common::PlatformResult SetSoundPath(notification_h noti_handle,
                                             const std::string& sound_path);
  static common::PlatformResult GetVibration(notification_h noti_handle,
                                             bool* vibration);
  static common::PlatformResult SetVibration(notification_h noti_handle,
                                             bool vibration);
  static common::PlatformResult GetApplicationControl(
      app_control_h app_handle,
      picojson::object* out_ptr);
  static common::PlatformResult SetApplicationControl(
      app_control_h app_handle,
      const picojson::object& app_ctrl);
  static common::PlatformResult GetApplicationId(app_control_h app_handle,
                                                 std::string* app_id);
  static common::PlatformResult SetApplicationId(app_control_h app_handle,
                                                 const std::string& app_id);
  static common::PlatformResult GetProgressValue(
      notification_h noti_handle,
      const std::string& progess_type,
      double* progress_value);
  static common::PlatformResult SetProgressValue(
      notification_h noti_handle,
      const std::string& progress_type,
      double progress_value,
      bool is_update);
  static common::PlatformResult GetPostedTime(notification_h noti_handle,
                                              time_t* posted_time);
  static common::PlatformResult SetLayout(notification_h noti_handle,
                                          const std::string& noti_type);
  static common::PlatformResult SetAppControl(notification_h noti_handle,
                                              app_control_h app_control);
  static common::PlatformResult CreateAppControl(app_control_h* app_control);

  static bool IsColorFormatNumberic(const std::string& color);
};

}  // namespace notification
}  // namespace extension

#endif /* NOTIFICATION_STATUS_NOTIFICATION_H_ */
