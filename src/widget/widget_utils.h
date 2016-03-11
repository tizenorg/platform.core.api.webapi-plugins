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

#ifndef WEBAPI_PLUGINS_WIDGET_WIDGET_UTILS_H__
#define WEBAPI_PLUGINS_WIDGET_WIDGET_UTILS_H__

#include <string>

#include <widget_service.h>

#include "common/tizen_result.h"

namespace extension {
namespace widget {

#define CHECK_EXIST(args, name, out) \
  if (args.end() == args.find(name)) { \
    return common::TypeMismatchError(std::string(name) + " is required argument"); \
  }

extern const std::string kWidgetId;
extern const std::string kPackageId;
extern const std::string kId;
extern const std::string kSizeType;
extern const std::string kWidth;
extern const std::string kHeight;

class WidgetUtils {
 public:
  static widget_size_type_e ToSizeType(const std::string& e);
  static std::string FromSizeType(widget_size_type_e e);
  static std::string FromEventType(widget_lifecycle_event_e e);
  static common::TizenResult ConvertErrorCode(int error);
  static common::TizenResult WidgetToJson(const char* id, picojson::object* out, const char* pkgid = nullptr);
  static common::TizenResult SizeToJson(widget_size_type_e type, picojson::object* out);
  static common::TizenResult WidgetVariantToJson(const char* id, widget_size_type_e type, picojson::object* out);

};

} // widget
} // extension

#endif // WEBAPI_PLUGINS_WIDGET_WIDGET_UTILS_H__
