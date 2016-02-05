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

#include "common/tizen_instance.h"

#include "common/logger.h"
#include "common/task-queue.h"

namespace common {

AsyncToken::AsyncToken(const picojson::object& obj) {
  value_ = obj.at("callbackId").get<double>();
}

void AsyncToken::ToJson(picojson::object* obj) const {
  obj->insert(std::make_pair("callbackId", picojson::value{value_}));
}

void ListenerToken::ToJson(picojson::object* obj) const {
  obj->insert(std::make_pair("listenerId", picojson::value{lid_}));
}

TizenInstance::TizenInstance() {
  ScopeLogger();
}

TizenInstance::~TizenInstance() {
  ScopeLogger();
}

void TizenInstance::RegisterHandler(const std::string& name, const AsyncHandler& func) {
  ScopeLogger();

  // TODO: change to RegisterHandler() when applicable
  ParsedInstance::RegisterSyncHandler(name, [func](const picojson::value& args, picojson::object& out) -> void {
    const auto& a = args.get<picojson::object>();
    auto result = func(a, AsyncToken(a));
    result.ToJson(&out);
  });
}

void TizenInstance::RegisterSyncHandler(const std::string& name, const SyncHandler& func) {
  ScopeLogger();

  ParsedInstance::RegisterSyncHandler(name, [func](const picojson::value& args, picojson::object& out) -> void {
    auto result = func(args.get<picojson::object>());
    result.ToJson(&out);
  });
}

void TizenInstance::Post(const AsyncToken& token, const TizenResult& result) {
  ScopeLogger();

  picojson::value msg{picojson::object{}};
  auto& obj = msg.get<picojson::object>();

  token.ToJson(&obj);
  result.ToJson(&obj);

  TaskQueue::GetInstance().Async([this, msg]() {
    ParsedInstance::PostMessage(this, msg.serialize().c_str());
  });
}

PostCallback TizenInstance::SimplePost(const AsyncToken& token) {
  ScopeLogger();

  return [this, token](const common::TizenResult& result, const picojson::value& value) {
    ScopeLogger("simple post");

    Post(token, result ? TizenSuccess{value} : result);
  };
}

}  // namespace common
