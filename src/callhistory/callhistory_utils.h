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

#ifndef CALLHISTORY_CALLHISTORY_UTILS_H_
#define CALLHISTORY_CALLHISTORY_UTILS_H_

#include <contacts.h>
#include <contacts_internal.h>

#include "common/picojson.h"

namespace extension {
namespace callhistory {

class CallHistory;

class CallHistoryUtils {
 public:
  explicit CallHistoryUtils(CallHistory& history);

  void parseRecordList(contacts_list_h *record_list, picojson::array& array);
  void parseRecord(contacts_record_h *record, picojson::object& obj);
  void parseLogType(contacts_phone_log_type_e log_type, picojson::object& obj);
  void parseRemoteParties(contacts_record_h *record, picojson::object& obj);
  void parseCallingParty(contacts_record_h *record, picojson::object& obj);
  unsigned int convertAttributeName(const std::string& attribute_name);
  void createFilter(contacts_filter_h filter, const picojson::object filter_obj);

 private:
  CallHistory& history_;
};

} // namespace callhistory
} // namespace extension

#endif // CALLHISTORY_CALLHISTORY_UTILS_H_
