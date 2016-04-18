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

#include "common/logger.h"
#include "common/picojson.h"
#include "common/task-queue.h"
#include "common/tools.h"
#include "common/scope_exit.h"
#include "preference/preference_instance.h"


namespace extension {
namespace preference {

namespace {
const char* kKey = "key";
const char* kValue = "value";

const common::ListenerToken kPreferenceChangeListenerToken{"PREFERENCE_CHANGED"};
} // namespace

#define CHECK_EXIST(args, name) \
  if (args.end() == args.find(name)) { \
    return common::TypeMismatchError(std::string(name) + " is required argument"); \
  }


PreferenceInstance::PreferenceInstance() {
  ScopeLogger();

  using std::placeholders::_1;
  using std::placeholders::_2;

#define REGISTER(c,x) \
    RegisterSyncHandler(c, std::bind(&PreferenceInstance::x, this, _1));
  REGISTER("PreferenceManager_setValue", SetValue);
  REGISTER("PreferenceManager_getValue", GetValue);
  REGISTER("PreferenceManager_remove", Remove);
  REGISTER("PreferenceManager_removeAll", RemoveAll);
  REGISTER("PreferenceManager_exists", Exists);
  REGISTER("PreferenceManager_setChangeListener", SetChangeListener);
  REGISTER("PreferenceManager_unsetChangeListener", UnsetChangeListener);
#undef REGISTER
#define REGISTER_ASYNC(c,x) \
    RegisterHandler(c, std::bind(&PreferenceInstance::x, this, _1, _2));
  REGISTER_ASYNC("PreferenceManager_getAll", GetAll);
#undef REGISTER_ASYNC
}

PreferenceInstance::~PreferenceInstance()
{
  ScopeLogger();
}

common::TizenResult PreferenceInstance::GetAll(const picojson::object& args, const common::AsyncToken& token) {
  ScopeLogger();

  return manager_.GetAll(SimplePost(token));
}

common::TizenResult PreferenceInstance::SetValue(const picojson::object& args) {
  ScopeLogger();

  CHECK_EXIST(args, kKey)
  CHECK_EXIST(args, kValue)

  const auto& key = args.find(kKey)->second.get<std::string>();
  const auto& value = args.find(kValue)->second;

  return manager_.SetValue(key, value);
}

common::TizenResult PreferenceInstance::GetValue(const picojson::object& args) {
  ScopeLogger();

  CHECK_EXIST(args, kKey)
  const auto& key = args.find(kKey)->second.get<std::string>();
  return manager_.GetValue(key);
}

common::TizenResult PreferenceInstance::Remove(const picojson::object& args) {
  ScopeLogger();

  CHECK_EXIST(args, kKey)
  const auto& key = args.find(kKey)->second.get<std::string>();
  return manager_.Remove(key);
}

common::TizenResult PreferenceInstance::RemoveAll(const picojson::object& args) {
  ScopeLogger();
  return manager_.RemoveAll();
}

common::TizenResult PreferenceInstance::Exists(const picojson::object& args) {
  ScopeLogger();

  CHECK_EXIST(args, kKey)
  const auto& key = args.find(kKey)->second.get<std::string>();
  return manager_.Exists(key);
}

common::TizenResult PreferenceInstance::SetChangeListener(const picojson::object& args) {
  ScopeLogger();
  CHECK_EXIST(args, kKey)
  const auto& key = args.find(kKey)->second.get<std::string>();

  common::PostCallback callback = [this](const common::TizenResult&, const picojson::value& v) {
    Post(kPreferenceChangeListenerToken, common::TizenSuccess{v});
  };

  return manager_.SetChangeListener(key, callback);
}

common::TizenResult PreferenceInstance::UnsetChangeListener(const picojson::object& args) {
  ScopeLogger();

  CHECK_EXIST(args, kKey)
  const auto& key = args.find(kKey)->second.get<std::string>();
  return manager_.UnsetChangeListener(key);
}

} // namespace preference
} // namespace extension
