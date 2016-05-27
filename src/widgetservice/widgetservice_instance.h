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

#ifndef WIDGETSERVICE_WIDGET_INSTANCE_H_
#define WIDGETSERVICE_WIDGET_INSTANCE_H_

#include <mutex>
#include <map>

#include "common/tizen_instance.h"

namespace extension {
namespace widgetservice {

class WidgetServiceInstance : public common::TizenInstance {
 public:
  WidgetServiceInstance();
  virtual ~WidgetServiceInstance();
  void CallWidgetLifecycleListener(const std::string& widget_id, const picojson::value& response);
 private:
  //WidgetManager
  common::TizenResult GetWidget(picojson::object const& args);
  common::TizenResult GetWidgets(picojson::object const& args, const common::AsyncToken& token);
  common::TizenResult GetPrimaryWidgetId(picojson::object const& args);
  common::TizenResult GetSize(picojson::object const& args);
  //Widget
  common::TizenResult GetName(picojson::object const& args);
  common::TizenResult GetInstances(picojson::object const& args, const common::AsyncToken& token);
  common::TizenResult GetVariant(picojson::object const& args);
  common::TizenResult GetVariants(picojson::object const& args, const common::AsyncToken& token);
  common::TizenResult AddStateChangeListener(picojson::object const& args);
  common::TizenResult RemoveStateChangeListener(picojson::object const& args);
  //WidgetInstance
  common::TizenResult ChangeUpdatePeriod(picojson::object const& args);
  common::TizenResult SendContent(picojson::object const& args);
  common::TizenResult GetContent(picojson::object const& args, const common::AsyncToken& token);

  static std::mutex listener_mutex_;
  std::map<std::string, int> listener_map_;
};

} // namespace widgetservice
} // namespace extension

#endif // WIDGETSERVICE_WIDGET_INSTANCE_H_
