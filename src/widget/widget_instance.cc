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

#include "widget/widget_instance.h"

#include <thread>

#include <widget_service.h>
#include <widget_errno.h>

#include "widget/widget_utils.h"

namespace extension {
namespace widget {

using common::TizenResult;
using common::TizenSuccess;

namespace {
const std::string kPrivilegeWidget = "http://tizen.org/privilege/widget.viewer";

int WidgetListCb(const char* pkgid, const char* widget_id, int is_primary, void* data) {
  ScopeLogger();

  //is_primary is not supported by native api
  picojson::array* array = static_cast<picojson::array*>(data);

  if (!array) {
    LoggerW("User data is null");
    return WIDGET_ERROR_NONE;
  }

  picojson::value val = picojson::value(picojson::object());

  auto result = WidgetUtils::WidgetToJson(widget_id, &val.get<picojson::object>(), pkgid);
  if (result) {
    array->push_back(val);
  }

  return WIDGET_ERROR_NONE;
}

int WidgetListByPkgIdCb(const char* widget_id, int is_primary, void* data) {
  ScopeLogger();

  //is_primary is not supported by native api
  picojson::array* array = static_cast<picojson::array*>(data);

  if (!array) {
    LoggerW("User data is null");
    return WIDGET_ERROR_NONE;
  }

  picojson::value val = picojson::value(picojson::object());

  auto result = WidgetUtils::WidgetToJson(widget_id, &val.get<picojson::object>());
  if (result) {
    array->push_back(val);
  }

  return WIDGET_ERROR_NONE;
}
}  // namespace

WidgetInstance::WidgetInstance() {
  ScopeLogger();
  using std::placeholders::_1;
  using std::placeholders::_2;

#define REGISTER_SYNC(c, x) \
  RegisterSyncHandler(c, std::bind(&WidgetInstance::x, this, _1));

  REGISTER_SYNC("WidgetManager_getWidget", GetWidget);
#undef REGISTER_SYNC

#define REGISTER_ASYNC(c, x) \
  RegisterHandler(c, std::bind(&WidgetInstance::x, this, _1, _2));

  REGISTER_ASYNC("WidgetManager_getWidgets", GetWidgets);
#undef REGISTER_ASYNC
}

WidgetInstance::~WidgetInstance() {
  ScopeLogger();
}

TizenResult WidgetInstance::GetWidget(const picojson::object& args) {
  ScopeLogger();

  //CHECK_PRIVILEGE_ACCESS(kPrivilegeWidget, &out);
  CHECK_EXIST(args, kWidgetId, out)

  std::string widget_id = args.find(kWidgetId)->second.get<std::string>();

  picojson::value value {picojson::object{}};
  auto* obj = &value.get<picojson::object>();

  auto result = WidgetUtils::WidgetToJson(widget_id.c_str(), obj);
  if (!result) {
    LogAndReturnTizenError(result, ("GetWidget() failed"));
  }

  return TizenSuccess(value);
}

TizenResult WidgetInstance::GetWidgets(const picojson::object& args,
                                               const common::AsyncToken& token) {
  ScopeLogger();

  //CHECK_PRIVILEGE_ACCESS(kPrivilegeWidget, &out);

  std::string pkgid;
  const auto id = args.find(kPackageId);
  if (args.end() != id) {
    pkgid = id->second.get<std::string>();
  }

  auto get_widgets = [this, pkgid](const common::AsyncToken& token) -> void {
    int ret = WIDGET_ERROR_NONE;
    picojson::value response{picojson::array{}};
    auto* array = &response.get<picojson::array>();

    if (pkgid.empty()) {
      ret = widget_service_get_widget_list(WidgetListCb, array);
    } else {
      ret = widget_service_get_widget_list_by_pkgid(pkgid.c_str(), WidgetListByPkgIdCb, array);
    }

    TizenResult result = TizenSuccess();

    if (WIDGET_ERROR_NONE != ret) {
      LoggerE("widget_service_get_widget_list() failed");
      result = WidgetUtils::ConvertErrorCode(ret);
    } else {
      result = TizenSuccess{response};
    }

    this->Post(token, result);
  };

  std::thread(get_widgets, token).detach();

  return TizenSuccess();
}

} // namespace widget
} // namespace extension
