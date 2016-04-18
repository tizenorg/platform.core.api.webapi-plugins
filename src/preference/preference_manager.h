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

#ifndef PREFERENCE_PREFERENCE_MANAGER_H_
#define PREFERENCE_PREFERENCE_MANAGER_H_

#include <mutex>

#include "common/picojson.h"
#include "common/tizen_instance.h"

namespace extension {
namespace preference {

class PreferenceManager {
public:
  ~PreferenceManager();
  common::TizenResult GetAll(const common::PostCallback& callback);
  common::TizenResult SetValue(const std::string& key, const picojson::value& value);
  common::TizenResult GetValue(const std::string& key);
  common::TizenResult Remove(const std::string& key);
  common::TizenResult RemoveAll(void);
  common::TizenResult Exists(const std::string& key);
  common::TizenResult SetChangeListener(const std::string& key, const common::PostCallback callback);
  common::TizenResult UnsetChangeListener(const std::string& key);

private:
  common::PostCallback post_changed_callback_;
  std::vector<std::string> key_listeners_;
  static std::mutex key_listener_mtx_;
};

} // namespace preference
} // namespace extension

#endif // PREFERENCE_PREFERENCE_MANAGER_H_
