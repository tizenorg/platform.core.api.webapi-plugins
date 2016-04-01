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

#include <app_preference.h>
#include "common/logger.h"
#include "common/tools.h"
#include "preference/preference_manager.h"
#include "preference/preference_instance.h"

namespace extension {
namespace preference {

namespace {
const char* kKey = "key";
const char* kValue = "value";

common::TizenResult MakeErrResult(int ret, const char* err_msg) {
  LoggerE("%s", err_msg);
  switch (ret) {
    case PREFERENCE_ERROR_INVALID_PARAMETER:
      return common::InvalidValuesError(std::string(err_msg));

    case PREFERENCE_ERROR_OUT_OF_MEMORY:
      return common::AbortError(std::string(err_msg));

    case PREFERENCE_ERROR_IO_ERROR:
      return common::IoError(std::string(err_msg));

    case PREFERENCE_ERROR_NO_KEY:
      return common::NotFoundError(std::string(err_msg));

    default:
      return common::AbortError(std::string(err_msg));
  }
}

int GetValueInternal(const std::string& key, picojson::value* val) {
  char* result_str = nullptr;
  int ret = preference_get_string(key.c_str(), &result_str);

  if (ret == PREFERENCE_ERROR_NONE) {
    *val = picojson::value(std::string(result_str));
    free(result_str);
  } else {
    double result_double = 0;
    ret = preference_get_double(key.c_str(), &result_double);

    if (ret == PREFERENCE_ERROR_NONE) {
      *val = picojson::value(result_double);
    } else {
      bool result_bool = false;
      ret = preference_get_boolean(key.c_str(), &result_bool);

      if (ret == PREFERENCE_ERROR_NONE) {
        *val = picojson::value(result_bool);
      } else {
        int result_int = 0;
        ret = preference_get_int(key.c_str(), &result_int);

        if (ret == PREFERENCE_ERROR_NONE) {
          *val = picojson::value(static_cast<double>(result_int));
        }
      }
    }
  }
  return ret;
}
} // namespace

common::TizenResult PreferenceManager::SetValue(const std::string& key, const picojson::value& value) {
  ScopeLogger();

  int ret = PREFERENCE_ERROR_NONE;

  if (value.is<bool>()) {
    ret = preference_set_boolean(key.c_str(), value.get<bool>());
  } else if (value.is<double>()) {
    ret = preference_set_double(key.c_str(), value.get<double>());
  } else if (value.is<std::string>()) {
    ret = preference_set_string(key.c_str(), value.get<std::string>().c_str());
  } else {
    ret = PREFERENCE_ERROR_INVALID_PARAMETER;
  }

  if (ret == PREFERENCE_ERROR_NONE) {
    return common::TizenSuccess();
  } else {
    return MakeErrResult(ret, "preference_set_ function error");
  }
}

common::TizenResult PreferenceManager::GetValue(const std::string& key) {
  ScopeLogger();

  picojson::value val;

  int ret = GetValueInternal(key, &val);

  if (ret == PREFERENCE_ERROR_NONE) {
    return common::TizenSuccess(val);
  } else {
    return MakeErrResult(ret, "preference_set_ function error");
  }
}

common::TizenResult PreferenceManager::Remove(const std::string& key) {
  ScopeLogger();

  int ret = preference_remove(key.c_str());

  if (ret == PREFERENCE_ERROR_NONE) {
    return common::TizenSuccess();
  } else {
    return MakeErrResult(ret, "preference_remove function error");
  }
}

common::TizenResult PreferenceManager::RemoveAll() {
  ScopeLogger();

  int ret = preference_remove_all();

  if (ret == PREFERENCE_ERROR_NONE) {
    return common::TizenSuccess();
  } else {
    return MakeErrResult(ret, "preference_remove_all function error");
  }
}

common::TizenResult PreferenceManager::Exists(const std::string& key) {
  ScopeLogger();

  bool is_existing = false;
  int ret = preference_is_existing(key.c_str(), &is_existing);

  if (ret == PREFERENCE_ERROR_NONE) {
    return common::TizenSuccess(picojson::value(is_existing));
  } else {
    return MakeErrResult(ret, "preference_is_existing function error");
  }
}

common::TizenResult PreferenceManager::SetChangeListener(const std::string& key,
                                                         common::PostCallback callback) {
  ScopeLogger();

  if (!post_callback_) {
    post_callback_ = callback;
  }

  int ret = preference_set_changed_cb(key.c_str(), ChangedCb, (void*) this);

  if (ret == PREFERENCE_ERROR_NONE) {
    return common::TizenSuccess();
  } else {
    return MakeErrResult(ret, "preference_set_changed_cb function error");
  }
}

common::TizenResult PreferenceManager::UnsetChangeListener(const std::string& key) {
  ScopeLogger();

  if (post_callback_) {
    int ret = preference_unset_changed_cb(key.c_str());

    if (ret == PREFERENCE_ERROR_NONE) {
      return common::TizenSuccess();
    } else {
      return MakeErrResult(ret, "preference_unset_changed_cb function error");
    }
  } else {
    return common::TizenSuccess();
  }
}

void PreferenceManager::ChangedCb(const char* key, void* user_data) {
  ScopeLogger();

  PreferenceManager* manager = static_cast<PreferenceManager*>(user_data);

  picojson::value result_val{picojson::object{}};
  picojson::object& result_obj = result_val.get<picojson::object>();

  result_obj.insert(std::make_pair(kKey, picojson::value(std::string(key))));

  picojson::value val;

  if (GetValueInternal(key, &val) == PREFERENCE_ERROR_NONE) {
    result_obj.insert(std::make_pair(kValue, val));
    if (manager->post_callback_) {
      manager->post_callback_(common::TizenSuccess(), result_val);
    }
  } else {
    LoggerE("preference_set_ function error");
  }
}

} // namespace preference
} // namespace extension
