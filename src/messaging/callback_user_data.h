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

#ifndef MESSAGING_CALLBACK_USER_DATA_H_
#define MESSAGING_CALLBACK_USER_DATA_H_

#include <memory>

#include "common/picojson.h"

#include "messaging/messaging_util.h"

namespace extension {
namespace messaging {

class CallbackUserData {
 public:
  CallbackUserData(PostQueue& queue, long cid, bool keep = false);
  virtual ~CallbackUserData();

  bool IsError() const;
  void SetError(const common::PlatformResult& error);
  void SetSuccess(const picojson::value& data = picojson::value());
  void SetAction(const char* action, const picojson::value& data);

  void AddToQueue();
  void Post();
  void AddAndPost(PostPriority p);
  bool HasQueue(const PostQueue& q) const;

 private:
  void AddJsonData(const char* key, const picojson::value& value);

  picojson::value json_;
  picojson::object& obj_;
  long cid_;
  PostQueue& queue_;
  common::PlatformResult result_;
};

}  // messaging
}  // extension

#endif  // MESSAGING_CALLBACK_USER_DATA_H_
