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

#ifndef CALLHISTORY_CALLHISTORY_H_
#define CALLHISTORY_CALLHISTORY_H_

#include <string>
#include <vector>
#include <future>

#include <contacts.h>
#include <contacts_internal.h>

#include "common/picojson.h"
#include "common/platform_result.h"

#include "callhistory/callhistory_utils.h"

namespace extension {
namespace callhistory {

class CallHistoryInstance;

class CallHistory
{
 public:
  explicit CallHistory(CallHistoryInstance& instance);
  ~CallHistory();

  std::vector<std::string>& getPhoneNumbers();
  CallHistoryUtils& getUtils();

  void find(const picojson::object& args);
  common::PlatformResult remove(const picojson::object& args);
  common::PlatformResult removeBatch(const picojson::object& args);
  void removeAll(const picojson::object& args);
  common::PlatformResult startCallHistoryChangeListener();
  common::PlatformResult stopCallHistoryChangeListener();
  common::PlatformResult setMissedDirection(int uid);

 private:
  static void changeListenerCB(const char* view_uri, char *changes, void* user_data);
  static void PostMessage(const CallHistory* instance, const std::string& msg);
  static void FindThread(const picojson::object& args, CallHistory* call_history);
  static void LoadPhoneNumbers(const picojson::object& args, CallHistory* call_history);

  bool m_is_listener_set;
  std::vector<std::string> m_phone_numbers;
  CallHistoryInstance& instance_;
  CallHistoryUtils utils_;

  static std::vector<CallHistory*> instances_;
  static std::mutex instances_mutex_;
};

} // namespace callhistory
} // namespace extension

#endif // CALLHISTORY_CALLHISTORY_H_
