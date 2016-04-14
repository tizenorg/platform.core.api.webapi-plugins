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

#ifndef PREFERENCE_PREFERENCE_INSTANCE_H_
#define PREFERENCE_PREFERENCE_INSTANCE_H_

#include "common/tizen_instance.h"
#include "preference/preference_manager.h"

namespace extension {
namespace preference {

class PreferenceInstance : public common::TizenInstance {
public:
  PreferenceInstance();
  virtual ~PreferenceInstance();

private:
  common::TizenResult GetAll(const picojson::object& args, const common::AsyncToken& token);
  common::TizenResult SetValue(const picojson::object& args);
  common::TizenResult GetValue(const picojson::object& args);
  common::TizenResult Remove(const picojson::object& args);
  common::TizenResult RemoveAll(const picojson::object& args);
  common::TizenResult Exists(const picojson::object& args);
  common::TizenResult SetChangeListener(const picojson::object& args);
  common::TizenResult UnsetChangeListener(const picojson::object& args);

  PreferenceManager manager_;
};

} // namespace preference
} // namespace extension

#endif // PREFERENCE_PREFERENCE_INSTANCE_H_
