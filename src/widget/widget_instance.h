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

#ifndef WIDGET_WIDGET_INSTANCE_H_
#define WIDGET_WIDGET_INSTANCE_H_

#include "common/tizen_instance.h"

namespace extension {
namespace widget {

class WidgetInstance : public common::TizenInstance {
 public:
  WidgetInstance();
  virtual ~WidgetInstance();

 private:
  //WidgetManager
  common::TizenResult GetWidget(picojson::object const& args);
  common::TizenResult GetWidgets(picojson::object const& args, const common::AsyncToken& token);
  common::TizenResult GetPrimaryWidgetId(picojson::object const& args);
  common::TizenResult GetSize(picojson::object const& args);

};

} // namespace widget
} // namespace extension

#endif // WIDGET_WIDGET_INSTANCE_H_
