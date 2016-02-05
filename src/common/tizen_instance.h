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

#ifndef COMMON_TIZEN_INSTANCE_H_
#define COMMON_TIZEN_INSTANCE_H_

#include <memory>
#include <unordered_map>

#include "common/extension.h"
#include "common/picojson.h"
#include "common/tizen_result.h"

namespace common {

class AsyncToken {
 public:
  explicit AsyncToken(const picojson::object& msg);
  virtual ~AsyncToken() {}

  virtual void ToJson(picojson::object* msg) const;

 protected:
  AsyncToken() : value_(-1.0) {}

 private:
  double value_;
};

class ListenerToken : public AsyncToken {
 public:
  explicit ListenerToken(const std::string& lid) : lid_(lid) {}

  virtual void ToJson(picojson::object* msg) const override;

 private:
  std::string lid_;
};

using AsyncHandler = std::function<TizenResult(const picojson::object&, const AsyncToken&)>;
using SyncHandler = std::function<TizenResult(const picojson::object&)>;
using PostCallback = std::function<void(const TizenResult&, const picojson::value&)>;

class TizenInstance : public ParsedInstance {
 public:
  TizenInstance();
  ~TizenInstance();

 protected:
  void RegisterHandler(const std::string& name, const AsyncHandler& func);
  void RegisterSyncHandler(const std::string& name, const SyncHandler& func);
  void Post(const AsyncToken& token, const TizenResult& result);
  PostCallback SimplePost(const AsyncToken& token);
};

}  // namespace common

#endif  // COMMON_TIZEN_INSTANCE_H_
