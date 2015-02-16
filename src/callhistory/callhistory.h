// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CALLHISTORY_CALLHISTORY_H_
#define CALLHISTORY_CALLHISTORY_H_

#include <string>
#include <vector>
#include <future>
#include <contacts.h>
#include <contacts_internal.h>
#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace callhistory {

class CallHistoryInstance;

class CallHistory
{
 public:
  static CallHistory* getInstance();
  std::vector<std::string>& getPhoneNumbers();

  void find(const picojson::object& args);
  common::PlatformResult remove(const picojson::object& args);
  common::PlatformResult removeBatch(const picojson::object& args);
  void removeAll(const picojson::object& args);
  common::PlatformResult startCallHistoryChangeListener();
  common::PlatformResult stopCallHistoryChangeListener();

 private:
  CallHistory();
  virtual ~CallHistory();

  static void changeListenerCB(const char* view_uri, char *changes, void* user_data);
  void loadPhoneNumbers();

  bool m_is_listener_set;
  std::vector<std::string> m_phone_numbers;
};

} // namespace callhistory
} // namespace extension

#endif // CALLHISTORY_CALLHISTORY_H_
