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
#include <bundle.h>
#include <bundle_internal.h>

#include "widget/widget_utils.h"
#include "common/scope_exit.h"

namespace extension {
namespace widget {

using common::TizenResult;
using common::TizenSuccess;

namespace {
const std::string kPrivilegeWidget = "http://tizen.org/privilege/widget.viewer";

const std::string kLang = "lang";
const std::string kInstanceId = "instanceId";
const std::string kPeriod = "period";
const std::string kForce = "force";
const std::string kData = "data";

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

}  // namespace

WidgetInstance::WidgetInstance() {
  ScopeLogger();
  using std::placeholders::_1;
  using std::placeholders::_2;

#define REGISTER_SYNC(c, x) \
  RegisterSyncHandler(c, std::bind(&WidgetInstance::x, this, _1));

  REGISTER_SYNC("WidgetManager_getWidget", GetWidget);
  REGISTER_SYNC("WidgetManager_getPrimaryWidgetId", GetPrimaryWidgetId);
  REGISTER_SYNC("WidgetManager_getSize", GetSize);
  REGISTER_SYNC("Widget_getName", GetName);
  REGISTER_SYNC("Widget_getVariant", GetVariant);
  REGISTER_SYNC("Widget_addChangeListener", AddChangeListener);
  REGISTER_SYNC("Widget_removeChangeListener", RemoveChangeListener);
  REGISTER_SYNC("WidgetInstance_changeUpdatePeriod", ChangeUpdatePeriod);
  REGISTER_SYNC("WidgetInstance_sendContent", SendContent);

#undef REGISTER_SYNC

#define REGISTER_ASYNC(c, x) \
  RegisterHandler(c, std::bind(&WidgetInstance::x, this, _1, _2));

  REGISTER_ASYNC("WidgetManager_getWidgets", GetWidgets);
  REGISTER_ASYNC("Widget_getInstances", GetInstances);
  REGISTER_ASYNC("Widget_getVariants", GetVariants);
  REGISTER_ASYNC("WidgetInstance_getContent", GetContent);
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

TizenResult WidgetInstance::GetPrimaryWidgetId(const picojson::object& args) {
  ScopeLogger();

  //CHECK_PRIVILEGE_ACCESS(kPrivilegeWidget, &out);
  CHECK_EXIST(args, kId, out)

  std::string id = args.find(kId)->second.get<std::string>();

  char* widget_id = widget_service_get_widget_id(id.c_str());
  if (!widget_id) {
    LogAndReturnTizenError(
        WidgetUtils::ConvertErrorCode(get_last_result()), ("widget_service_get_widget_id() failed"));
  }

  SCOPE_EXIT {
    free(widget_id);
  };

  return TizenSuccess(picojson::value(widget_id));
}

TizenResult WidgetInstance::GetSize(const picojson::object& args) {
  ScopeLogger();

  CHECK_EXIST(args, kSizeType, out)

  widget_size_type_e type = WidgetUtils::ToSizeType(args.find(kSizeType)->second.get<std::string>());
  if (WIDGET_SIZE_TYPE_UNKNOWN == type) {
    LogAndReturnTizenError(common::InvalidValuesError(), ("incorrect size type"));
  }

  picojson::value value{picojson::object{}};
  auto* obj = &value.get<picojson::object>();

  auto result = WidgetUtils::SizeToJson(type, obj);
  if (!result) {
    LogAndReturnTizenError(result, ("GetSize() failed"));
  }

  return TizenSuccess(value);
}

TizenResult WidgetInstance::GetName(picojson::object const& args) {
  ScopeLogger();

  //CHECK_PRIVILEGE_ACCESS(kPrivilegeWidget, &out);
  CHECK_EXIST(args, kWidgetId, out)

  const auto& widget_id = args.find(kWidgetId)->second.get<std::string>();
  char* lang = nullptr;

  const auto lang_it = args.find(kLang);
  if (args.end() != lang_it) {
    lang = const_cast<char*>(lang_it->second.get<std::string>().c_str());
  }

  char* name = widget_service_get_name(widget_id.c_str(), lang);
  if (!name) {
    LogAndReturnTizenError(
        WidgetUtils::ConvertErrorCode(get_last_result()), ("widget_service_get_name() failed"));
  }

  SCOPE_EXIT {
    free(name);
  };

  return TizenSuccess(picojson::value(name));
}

TizenResult WidgetInstance::GetInstances(picojson::object const& args, const common::AsyncToken& token) {
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
      result = WidgetUtils::ConvertErrorCode(ret);
    } else {
      result = TizenSuccess{response};
    }

    this->Post(token, result);
  };

  std::thread(get_instances, token).detach();

  return TizenSuccess();
}

TizenResult WidgetInstance::GetVariant(picojson::object const& args) {
  ScopeLogger();

  CHECK_EXIST(args, kWidgetId, out)
  CHECK_EXIST(args, kSizeType, out)

  const auto& widget_id = args.find(kWidgetId)->second.get<std::string>();
  const auto& type = args.find(kSizeType)->second.get<std::string>();

  widget_size_type_e size_type = WidgetUtils::ToSizeType(type);
  if (WIDGET_SIZE_TYPE_UNKNOWN == size_type) {
    LogAndReturnTizenError(common::InvalidValuesError(), ("incorrect size type"));
  }

  picojson::value value{picojson::object{}};
  auto* obj = &value.get<picojson::object>();

  auto result = WidgetUtils::SizeToJson(size_type, obj);
  if (!result) {
    LogAndReturnTizenError(result, ("GetVariant() failed"));
  }

  result = WidgetUtils::WidgetVariantToJson(widget_id.c_str(), size_type, obj);
  if (!result) {
    LogAndReturnTizenError(result, ("GetVariant() failed"));
  }

  //sizeType
  obj->insert(std::make_pair(kSizeType, picojson::value(type)));

  return TizenSuccess(value);
}

TizenResult WidgetInstance::GetVariants(picojson::object const& args, const common::AsyncToken& token) {
  ScopeLogger();

  //CHECK_PRIVILEGE_ACCESS(kPrivilegeWidget, &out);
  CHECK_EXIST(args, kWidgetId, out)

  std::string widget_id = args.find(kWidgetId)->second.get<std::string>();

  auto get_variants = [this, widget_id](const common::AsyncToken& token) -> void {
    int count = 0;
    int* type_array = nullptr;
    int ret = widget_service_get_supported_size_types(widget_id.c_str(), &count, &type_array);

    if (WIDGET_ERROR_NONE != ret) {
      LoggerE("widget_service_get_supported_size_types() failed");
      this->Post(token, WidgetUtils::ConvertErrorCode(ret));
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
      result = WidgetUtils::SizeToJson(size_type, obj);
      if (!result) {
        break;
      }

      result = WidgetUtils::WidgetVariantToJson(widget_id.c_str(), size_type, obj);
      if (!result) {
        break;
      }

      obj->insert(std::make_pair(kSizeType, picojson::value(WidgetUtils::FromSizeType(size_type))));
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

TizenResult WidgetInstance::AddChangeListener(picojson::object const& args) {
  ScopeLogger();

  return common::NotSupportedError();
}

TizenResult WidgetInstance::RemoveChangeListener(picojson::object const& args) {
  ScopeLogger();

  return common::NotSupportedError();
}

TizenResult WidgetInstance::ChangeUpdatePeriod(picojson::object const& args) {
  ScopeLogger();

  CHECK_EXIST(args, kWidgetId, out)
  CHECK_EXIST(args, kInstanceId, out)
  CHECK_EXIST(args, kPeriod, out)

  const auto& widget_id = args.find(kWidgetId)->second.get<std::string>();
  const auto& instance_id = args.find(kInstanceId)->second.get<std::string>();
  const double period = args.find(kPeriod)->second.get<double>();

  int ret = widget_service_change_period(widget_id.c_str(), instance_id.c_str(), period);

  if (WIDGET_ERROR_NONE != ret) {
    LogAndReturnTizenError(
        WidgetUtils::ConvertErrorCode(ret), ("widget_service_change_period() failed"));
  }

  return TizenSuccess();
}

TizenResult WidgetInstance::SendContent(picojson::object const& args) {
  ScopeLogger();

  CHECK_EXIST(args, kWidgetId, out)
  CHECK_EXIST(args, kInstanceId, out)
  CHECK_EXIST(args, kData, out)
  CHECK_EXIST(args, kForce, out)

  const auto& widget_id = args.find(kWidgetId)->second.get<std::string>();
  const auto& instance_id = args.find(kInstanceId)->second.get<std::string>();
  const int force = args.find(kForce)->second.get<bool>() ? 1 : 0;

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
        WidgetUtils::ConvertErrorCode(ret), ("widget_service_trigger_update() failed"));
  }

  return TizenSuccess();
}

TizenResult WidgetInstance::GetContent(picojson::object const& args, const common::AsyncToken& token) {
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
      this->Post(token, WidgetUtils::ConvertErrorCode(ret));
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

} // namespace widget
} // namespace extension
