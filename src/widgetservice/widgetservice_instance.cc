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

#include "widgetservice/widgetservice_instance.h"

#include <thread>

#include <widget_service.h>
#include <widget_errno.h>
#include <bundle.h>
#include <bundle_internal.h>

#include "widgetservice/widgetservice_utils.h"
#include "common/scope_exit.h"
#include "common/tools.h"

namespace extension {
namespace widgetservice {

using common::TizenResult;
using common::TizenSuccess;

std::mutex WidgetServiceInstance::listener_mutex_;

namespace {
const common::ListenerToken kWidgetChangeCallbackToken{"WidgetStateChangeCallback"};

const std::string kPrivilegeWidgetService = "http://tizen.org/privilege/widget.viewer";

const std::string kLocale = "locale";
const std::string kInstanceId = "instanceId";
const std::string kSeconds = "seconds";
const std::string kUpdateIfPaused = "updateIfPaused";
const std::string kData = "data";
const std::string kEvent = "event";

int WidgetListCb(const char* pkgid, const char* widget_id, int is_primary, void* data) {
  ScopeLogger();

  //is_primary is not supported by native api
  picojson::array* array = static_cast<picojson::array*>(data);

  if (!array) {
    LoggerW("User data is null");
    return WIDGET_ERROR_NONE;
  }

  picojson::value val = picojson::value(picojson::object());

  auto result = WidgetServiceUtils::WidgetToJson(widget_id, &val.get<picojson::object>(), pkgid);
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

  auto result = WidgetServiceUtils::WidgetToJson(widget_id, &val.get<picojson::object>());
  if (result) {
    array->push_back(val);
  }

  return WIDGET_ERROR_NONE;
}

int WidgetInstanceCb(const char* widget_id, const char* instance_id, void* data) {
  ScopeLogger();

  picojson::array* array = static_cast<picojson::array*>(data);

  if (!array) {
    LoggerW("User data is null");
    return WIDGET_ERROR_NONE;
  }

  array->push_back(picojson::value(instance_id));

  return WIDGET_ERROR_NONE;
}

int WidgetLifecycleCb(const char* widget_id, widget_lifecycle_event_e lifecycle_event,
                      const char* widget_instance_id, void* data) {
  ScopeLogger();

  //WIDGET_LIFE_CYCLE_EVENT_MAX event is not supported
  if (WIDGET_LIFE_CYCLE_EVENT_RESUME < lifecycle_event) {
    LoggerW("Unknown event type");
    return WIDGET_ERROR_NONE;
  }

  WidgetServiceInstance* instance = static_cast<WidgetServiceInstance*>(data);

  if (!instance) {
    LoggerW("User data is null");
    return WIDGET_ERROR_NONE;
  }

  picojson::value response = picojson::value(picojson::object());
  auto& obj = response.get<picojson::object>();

  obj.insert(std::make_pair(kWidgetId, picojson::value(widget_id)));
  obj.insert(std::make_pair(kInstanceId, picojson::value(widget_instance_id)));
  obj.insert(std::make_pair(kEvent, picojson::value(WidgetServiceUtils::FromEventType(lifecycle_event))));

  instance->CallWidgetLifecycleListener(widget_id, response);

  return WIDGET_ERROR_NONE;
}

}  // namespace

WidgetServiceInstance::WidgetServiceInstance() {
  ScopeLogger();
  using std::placeholders::_1;
  using std::placeholders::_2;

#define REGISTER_SYNC(c, x) \
  RegisterSyncHandler(c, std::bind(&WidgetServiceInstance::x, this, _1));

  REGISTER_SYNC("WidgetServiceManager_getWidget", GetWidget);
  REGISTER_SYNC("WidgetServiceManager_getPrimaryWidgetId", GetPrimaryWidgetId);
  REGISTER_SYNC("WidgetServiceManager_getSize", GetSize);
  REGISTER_SYNC("Widget_getName", GetName);
  REGISTER_SYNC("Widget_getVariant", GetVariant);
  REGISTER_SYNC("Widget_addStateChangeListener", AddStateChangeListener);
  REGISTER_SYNC("Widget_removeStateChangeListener", RemoveStateChangeListener);
  REGISTER_SYNC("WidgetInstance_changeUpdatePeriod", ChangeUpdatePeriod);
  REGISTER_SYNC("WidgetInstance_sendContent", SendContent);

#undef REGISTER_SYNC

#define REGISTER_ASYNC(c, x) \
  RegisterHandler(c, std::bind(&WidgetServiceInstance::x, this, _1, _2));

  REGISTER_ASYNC("WidgetServiceManager_getWidgets", GetWidgets);
  REGISTER_ASYNC("Widget_getInstances", GetInstances);
  REGISTER_ASYNC("Widget_getVariants", GetVariants);
  REGISTER_ASYNC("WidgetInstance_getContent", GetContent);
#undef REGISTER_ASYNC
}

WidgetServiceInstance::~WidgetServiceInstance() {
  ScopeLogger();

  std::lock_guard<std::mutex> lock(listener_mutex_);
  for (auto& it : listener_map_) {
    int ret = widget_service_unset_lifecycle_event_cb(it.first.c_str(), nullptr);
    if (WIDGET_ERROR_NONE != ret) {
      LoggerE("widget_service_unset_lifecycle_event_cb() failed");
    }
  }

  listener_map_.clear();
}

TizenResult WidgetServiceInstance::GetWidget(const picojson::object& args) {
  ScopeLogger();

  CHECK_PRIVILEGE(kPrivilegeWidgetService);

  CHECK_EXIST(args, kWidgetId, out)

  const auto& widget_id = args.find(kWidgetId)->second.get<std::string>();

  picojson::value value {picojson::object{}};
  auto* obj = &value.get<picojson::object>();

  auto result = WidgetServiceUtils::WidgetToJson(widget_id.c_str(), obj);
  if (!result) {
    LogAndReturnTizenError(result, ("GetWidget() failed"));
  }

  return TizenSuccess(value);
}

TizenResult WidgetServiceInstance::GetWidgets(const picojson::object& args,
                                               const common::AsyncToken& token) {
  ScopeLogger();

  CHECK_PRIVILEGE(kPrivilegeWidgetService);

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
      result = WidgetServiceUtils::ConvertErrorCode(ret);
    } else {
      result = TizenSuccess{response};
    }

    this->Post(token, result);
  };

  std::thread(get_widgets, token).detach();

  return TizenSuccess();
}

TizenResult WidgetServiceInstance::GetPrimaryWidgetId(const picojson::object& args) {
  ScopeLogger();

  CHECK_PRIVILEGE(kPrivilegeWidgetService);

  CHECK_EXIST(args, kId, out)

  const auto& id = args.find(kId)->second.get<std::string>();

  char* widget_id = widget_service_get_widget_id(id.c_str());
  if (!widget_id) {
    LogAndReturnTizenError(
        WidgetServiceUtils::ConvertErrorCode(get_last_result()), ("widget_service_get_widget_id() failed"));
  }

  SCOPE_EXIT {
    free(widget_id);
  };

  return TizenSuccess(picojson::value(widget_id));
}

TizenResult WidgetServiceInstance::GetSize(const picojson::object& args) {
  ScopeLogger();

  CHECK_EXIST(args, kSizeType, out)

  widget_size_type_e type = WidgetServiceUtils::ToSizeType(args.find(kSizeType)->second.get<std::string>());
  if (WIDGET_SIZE_TYPE_UNKNOWN == type) {
    LogAndReturnTizenError(common::InvalidValuesError(), ("incorrect size type"));
  }

  picojson::value value{picojson::object{}};
  auto* obj = &value.get<picojson::object>();

  auto result = WidgetServiceUtils::SizeToJson(type, obj);
  if (!result) {
    LogAndReturnTizenError(result, ("GetSize() failed"));
  }

  return TizenSuccess(value);
}

TizenResult WidgetServiceInstance::GetName(picojson::object const& args) {
  ScopeLogger();

  CHECK_PRIVILEGE(kPrivilegeWidgetService);

  CHECK_EXIST(args, kWidgetId, out)

  const auto& widget_id = args.find(kWidgetId)->second.get<std::string>();
  char* locale = nullptr;

  const auto locale_it = args.find(kLocale);
  if (args.end() != locale_it) {
    locale = const_cast<char*>(locale_it->second.get<std::string>().c_str());
  }

  char* name = widget_service_get_name(widget_id.c_str(), locale);
  if (!name) {
    LogAndReturnTizenError(
        WidgetServiceUtils::ConvertErrorCode(get_last_result()), ("widget_service_get_name() failed"));
  }

  SCOPE_EXIT {
    free(name);
  };

  return TizenSuccess(picojson::value(name));
}

TizenResult WidgetServiceInstance::GetInstances(picojson::object const& args, const common::AsyncToken& token) {
  ScopeLogger();

  CHECK_EXIST(args, kWidgetId, out)

  const auto& widget_id = args.find(kWidgetId)->second.get<std::string>();

  auto get_instances = [this, widget_id](const common::AsyncToken& token) -> void {
    picojson::value response{picojson::array{}};
    auto* array = &response.get<picojson::array>();

    int ret = widget_service_get_widget_instance_list(widget_id.c_str(), WidgetInstanceCb, array);

    TizenResult result = TizenSuccess();

    if (WIDGET_ERROR_NONE != ret) {
      LoggerE("widget_service_get_widget_instance_list() failed");
      result = WidgetServiceUtils::ConvertErrorCode(ret);
    } else {
      result = TizenSuccess{response};
    }

    this->Post(token, result);
  };

  std::thread(get_instances, token).detach();

  return TizenSuccess();
}

TizenResult WidgetServiceInstance::GetVariant(picojson::object const& args) {
  ScopeLogger();

  CHECK_EXIST(args, kWidgetId, out)
  CHECK_EXIST(args, kSizeType, out)

  const auto& widget_id = args.find(kWidgetId)->second.get<std::string>();
  const auto& type = args.find(kSizeType)->second.get<std::string>();

  widget_size_type_e size_type = WidgetServiceUtils::ToSizeType(type);
  if (WIDGET_SIZE_TYPE_UNKNOWN == size_type) {
    LogAndReturnTizenError(common::InvalidValuesError(), ("incorrect size type"));
  }

  picojson::value value{picojson::object{}};
  auto* obj = &value.get<picojson::object>();

  auto result = WidgetServiceUtils::SizeToJson(size_type, obj);
  if (!result) {
    LogAndReturnTizenError(result, ("GetVariant() failed"));
  }

  result = WidgetServiceUtils::WidgetVariantToJson(widget_id.c_str(), size_type, obj);
  if (!result) {
    LogAndReturnTizenError(result, ("GetVariant() failed"));
  }

  //sizeType
  obj->insert(std::make_pair(kSizeType, picojson::value(type)));

  return TizenSuccess(value);
}

TizenResult WidgetServiceInstance::GetVariants(picojson::object const& args, const common::AsyncToken& token) {
  ScopeLogger();

  CHECK_PRIVILEGE(kPrivilegeWidgetService);

  CHECK_EXIST(args, kWidgetId, out)

  const auto& widget_id = args.find(kWidgetId)->second.get<std::string>();

  auto get_variants = [this, widget_id](const common::AsyncToken& token) -> void {
    int count = 0;
    int* type_array = nullptr;
    int ret = widget_service_get_supported_size_types(widget_id.c_str(), &count, &type_array);

    if (WIDGET_ERROR_NONE != ret) {
      LoggerE("widget_service_get_supported_size_types() failed");
      this->Post(token, WidgetServiceUtils::ConvertErrorCode(ret));
      return;
    }

    //it is not mentioned in header file if array should be freed by caller
    //but in widget_service_get_supported_size_types definition it is allocated
    //so it should be released when it is not needed anymore
    SCOPE_EXIT {
      free(type_array);
    };

    TizenResult result = TizenSuccess();
    picojson::value response{picojson::array{}};
    auto& array = response.get<picojson::array>();

    for (int i = 0; i < count; i++) {
      picojson::value val = picojson::value(picojson::object());
      picojson::object* obj = &val.get<picojson::object>();

      widget_size_type_e size_type = static_cast<widget_size_type_e>(type_array[i]);
      result = WidgetServiceUtils::SizeToJson(size_type, obj);
      if (!result) {
        break;
      }

      result = WidgetServiceUtils::WidgetVariantToJson(widget_id.c_str(), size_type, obj);
      if (!result) {
        break;
      }

      obj->insert(std::make_pair(kSizeType, picojson::value(WidgetServiceUtils::FromSizeType(size_type))));
      array.push_back(val);
    }

    if (!result) {
      this->Post(token, result);
    } else {
      this->Post(token, TizenSuccess{response});
    }
  };

  std::thread(get_variants, token).detach();

  return TizenSuccess();
}

void WidgetServiceInstance::CallWidgetLifecycleListener(const std::string& widget_id,
                                                 const picojson::value& response) {
  ScopeLogger();

  std::lock_guard<std::mutex> lock(listener_mutex_);
  const auto it = listener_map_.find(widget_id);
  if (listener_map_.end() != it) {
    Post(kWidgetChangeCallbackToken, TizenSuccess{response});
    return;
  }

  LoggerW("widget id was not found.");
}

TizenResult WidgetServiceInstance::AddStateChangeListener(picojson::object const& args) {
  ScopeLogger();

  CHECK_PRIVILEGE(kPrivilegeWidgetService);

  CHECK_EXIST(args, kWidgetId, out)

  const auto& widget_id = args.find(kWidgetId)->second.get<std::string>();

  std::lock_guard<std::mutex> lock(listener_mutex_);
  auto it = listener_map_.find(widget_id);
  if (listener_map_.end() != it) {
    it->second++;
    return TizenSuccess();
  }

  int ret = widget_service_set_lifecycle_event_cb(widget_id.c_str(), WidgetLifecycleCb , this);
  if (WIDGET_ERROR_NONE != ret) {
    LogAndReturnTizenError(
        WidgetServiceUtils::ConvertErrorCode(ret), ("widget_service_set_lifecycle_event_cb() failed"));
  }

  listener_map_[widget_id]++;

  return TizenSuccess();
}

TizenResult WidgetServiceInstance::RemoveStateChangeListener(picojson::object const& args) {
  ScopeLogger();

  CHECK_EXIST(args, kWidgetId, out)

  const auto& widget_id = args.find(kWidgetId)->second.get<std::string>();

  std::lock_guard<std::mutex> lock(listener_mutex_);
  auto it = listener_map_.find(widget_id);
  if (listener_map_.end() == it) {
    LoggerW("Listener id not found");
    return TizenSuccess();
  }

  if (!(--it->second)) {
    int ret = widget_service_unset_lifecycle_event_cb(widget_id.c_str(), nullptr);
    if (WIDGET_ERROR_NONE != ret) {
      LogAndReturnTizenError(
          WidgetServiceUtils::ConvertErrorCode(ret), ("widget_service_unset_lifecycle_event_cb() failed"));
    }
    listener_map_.erase(it);
  }

  return TizenSuccess();
}

TizenResult WidgetServiceInstance::ChangeUpdatePeriod(picojson::object const& args) {
  ScopeLogger();

  CHECK_EXIST(args, kWidgetId, out)
  CHECK_EXIST(args, kInstanceId, out)
  CHECK_EXIST(args, kSeconds, out)

  const auto& widget_id = args.find(kWidgetId)->second.get<std::string>();
  const auto& instance_id = args.find(kInstanceId)->second.get<std::string>();
  const double seconds = args.find(kSeconds)->second.get<double>();

  int ret = widget_service_change_period(widget_id.c_str(), instance_id.c_str(), seconds);

  if (WIDGET_ERROR_NONE != ret) {
    LogAndReturnTizenError(
        WidgetServiceUtils::ConvertErrorCode(ret), ("widget_service_change_period() failed"));
  }

  return TizenSuccess();
}

TizenResult WidgetServiceInstance::SendContent(picojson::object const& args) {
  ScopeLogger();

  CHECK_EXIST(args, kWidgetId, out)
  CHECK_EXIST(args, kInstanceId, out)
  CHECK_EXIST(args, kData, out)
  CHECK_EXIST(args, kUpdateIfPaused, out)

  const auto& widget_id = args.find(kWidgetId)->second.get<std::string>();
  const auto& instance_id = args.find(kInstanceId)->second.get<std::string>();
  const int force = args.find(kUpdateIfPaused)->second.get<bool>() ? 1 : 0;

  bundle* data = bundle_create();
  int ret = get_last_result();
  if (BUNDLE_ERROR_NONE != ret) {
    LogAndReturnTizenError(common::AbortError(ret), ("bundle_create() failed"));
  }

  SCOPE_EXIT {
    bundle_free(data);
  };

  ret = bundle_add(data, kData.c_str(), args.find(kData)->second.serialize().c_str());
  if (BUNDLE_ERROR_NONE != ret) {
    LogAndReturnTizenError(common::AbortError(ret), ("bundle_add() failed"));
  }

  ret = widget_service_trigger_update(widget_id.c_str(), instance_id.c_str(), data, force);
  if (WIDGET_ERROR_NONE != ret) {
    LogAndReturnTizenError(
        WidgetServiceUtils::ConvertErrorCode(ret), ("widget_service_trigger_update() failed"));
  }

  return TizenSuccess();
}

TizenResult WidgetServiceInstance::GetContent(picojson::object const& args, const common::AsyncToken& token) {
  ScopeLogger();

  CHECK_EXIST(args, kWidgetId, out)
  CHECK_EXIST(args, kInstanceId, out)

  const auto& widget_id = args.find(kWidgetId)->second.get<std::string>();
  const auto& instance_id = args.find(kInstanceId)->second.get<std::string>();

  auto get_content = [this, widget_id, instance_id](const common::AsyncToken& token) -> void {
    bundle* bundle_data = bundle_create();

    int ret = get_last_result();
    if (BUNDLE_ERROR_NONE != ret) {
      LoggerE("bundle_create() failed");
      this->Post(token, common::AbortError(ret));
      return;
    }

    SCOPE_EXIT {
      bundle_free(bundle_data);
    };

    ret = widget_service_get_content_of_widget_instance(widget_id.c_str(),
                                                        instance_id.c_str(), &bundle_data);
    if (WIDGET_ERROR_NONE != ret) {
      LoggerE("widget_service_get_content_of_widget_instance() failed");
      this->Post(token, WidgetServiceUtils::ConvertErrorCode(ret));
      return;
    }

    char* data_str = nullptr;
    ret = bundle_get_str(bundle_data, kData.c_str(), &data_str);
    if (BUNDLE_ERROR_NONE != ret) {
      LoggerE("bundle_get_str() failed");
      this->Post(token, common::AbortError(ret));
      return;
    }

    picojson::value response;
    std::string err;
    picojson::parse(response, data_str, data_str + strlen(data_str), &err);
    if (!err.empty()) {
      LoggerE("Failed to parse bundle data() failed [%s]", err.c_str());
      this->Post(token, common::AbortError());
      return;
    }

    this->Post(token, TizenSuccess{response});
  };

  std::thread(get_content, token).detach();

  return TizenSuccess();
}

} // namespace widgetservice
} // namespace extension
