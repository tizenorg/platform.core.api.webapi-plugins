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

#include "messaging/callback_user_data.h"

#include "common/logger.h"
#include "common/tools.h"

using common::tools::ReportSuccess;
using common::tools::ReportError;

namespace extension {
namespace messaging {

CallbackUserData::CallbackUserData(PostQueue& queue, long cid, bool keep /* = false */)
    : json_(picojson::object()),
      obj_(json_.get<picojson::object>()),
      cid_(cid),
      queue_(queue),
      result_(common::ErrorCode::NO_ERROR) {
  LoggerD("Entered");
  if (!keep) {
    // this is not listener, add callbackId
    AddJsonData(JSON_CALLBACK_ID, picojson::value(static_cast<double>(cid_)));
  }
}

CallbackUserData::~CallbackUserData() {
  LoggerD("Entered");
}

bool CallbackUserData::IsError() const {
  LoggerD("Entered");
  return result_.IsError();
}

void CallbackUserData::SetError(const common::PlatformResult& error) {
  LoggerD("Entered");
  // keep only the first error
  if (!IsError()) {
    ReportError(error, &obj_);
    result_ = error;
  }
}

void CallbackUserData::SetSuccess(const picojson::value& data /* = picojson::value()*/) {
  LoggerD("Entered");
  // do not allow to overwrite the error
  if (!IsError()) {
    ReportSuccess(data, obj_);
  }
}

void CallbackUserData::SetAction(const char* action, const picojson::value& data) {
  LoggerD("Entered");

  AddJsonData(JSON_ACTION, picojson::value(action));
  // ReportSuccess cannot be used here, update of this field is necessary (this is a special case)
  AddJsonData(JSON_RESULT, data);
}

void CallbackUserData::SetListenerId(const char* id) {
  AddJsonData(JSON_LISTENER_ID, picojson::value(id));
}

void CallbackUserData::AddToQueue() {
  LoggerD("Entered");
  queue_.add(cid_, PostPriority::HIGH);
}

void CallbackUserData::Post() {
  LoggerD("Entered");
  queue_.resolve(cid_, json_.serialize());
}

void CallbackUserData::AddAndPost(PostPriority p) {
  queue_.addAndResolve(cid_, p, json_.serialize());
}

bool CallbackUserData::HasQueue(const PostQueue& q) const {
  LoggerD("Entered");
  return &q == &queue_;
}

void CallbackUserData::AddJsonData(const char* key, const picojson::value& value) {
  LoggerD("Entered");
  // always overwrite
  obj_[key] = value;
}

}  // messaging
}  // extension
