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

#include "alarm_utils.h"

#include "common/logger.h"

namespace extension {
namespace alarm {
namespace util {

using namespace common;

PlatformResult AppControlToService(const picojson::object& obj, app_control_h *app_control) {
  LoggerD("Entered");

  const auto it_end = obj.end();

  const auto it_operation = obj.find("operation");
  if (it_operation == it_end || !it_operation->second.is<std::string>()) {
    LoggerE("Invalid parameter passed.");
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid parameter passed.");
  }

  app_control_create(app_control);

  int ret = app_control_set_operation(*app_control, it_operation->second.get<std::string>().c_str());
  if (APP_CONTROL_ERROR_NONE != ret) {
    LoggerE("Failed app_control_set_operation() (%d)", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Error while setting operation.");
  }

  const auto it_uri = obj.find("uri");
  if (it_end != it_uri && it_uri->second.is<std::string>()) {
    ret = app_control_set_uri(*app_control, it_uri->second.get<std::string>().c_str());
    if (APP_CONTROL_ERROR_NONE != ret) {
      LoggerE("Failed app_control_set_uri() (%d)", ret);
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Error while setting uri.");
    }
  }

  const auto it_mime = obj.find("mime");
  if (it_end != it_mime && it_mime->second.is<std::string>()) {
    ret = app_control_set_mime(*app_control, it_mime->second.get<std::string>().c_str());
    if (APP_CONTROL_ERROR_NONE != ret) {
      LoggerE("Failed app_control_set_mime() (%d)", ret);
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Error while setting mime.");
    }
  }

  const auto it_category = obj.find("category");
  if (it_end != it_category && it_category->second.is<std::string>()) {
    ret = app_control_set_category(*app_control, it_category->second.get<std::string>().c_str());
    if (APP_CONTROL_ERROR_NONE != ret) {
      LoggerE("Failed app_control_set_category() (%d)", ret);
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Error while setting category.");
    }
  }

  const auto it_data = obj.find("data");
  if (it_end != it_data && it_data->second.is<picojson::array>()) {
    const picojson::array& data = it_data->second.get<picojson::array>();
    PlatformResult result = PlatformResult(ErrorCode::NO_ERROR);

    for (auto iter = data.begin(); iter != data.end(); ++iter) {
      result = AppControlToServiceExtraData(iter->get<picojson::object>(), app_control);
      if (!result) {
        LoggerE("Failed AppControlToServiceExtraData()");
        return result;
      }
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult AppControlToServiceExtraData(const picojson::object& app_obj,
                                            app_control_h *app_control) {
  LoggerD("Entered");

  const auto it_key = app_obj.find("key");
  const auto it_value = app_obj.find("value");
  const auto it_end = app_obj.end();

  if (it_key == it_end ||
      it_value == it_end ||
      !it_key->second.is<std::string>() ||
      !it_value->second.is<picojson::array>()) {
    LoggerE("Problem with key or value.");
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Problem with key or value.");
  }

  const std::string& key = it_key->second.get<std::string>();
  const picojson::array& values = it_value->second.get<picojson::array>();

  const size_t size = values.size();
  const char** arr = new const char*[size];
  size_t i = 0;

  for (auto iter = values.begin(); iter != values.end(); ++iter, ++i) {
    if (iter->is<std::string>()) {
      arr[i] = iter->get<std::string>().c_str();
    }
  }

  int ret = APP_CONTROL_ERROR_NONE;
  if (1 == size) {
    ret = app_control_add_extra_data(*app_control, key.c_str(), arr[0]);
  } else {
    ret = app_control_add_extra_data_array(*app_control, key.c_str(), arr, size);
  }
  delete[] arr;

  if (APP_CONTROL_ERROR_NONE != ret) {
    LoggerD("Error while setting data.");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Error while setting data.");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

} // util
} // alarm
} // extension
