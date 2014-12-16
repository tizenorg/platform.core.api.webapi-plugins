// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CALLHISTORY_CALLHISTORY_UTILS_H_
#define CALLHISTORY_CALLHISTORY_UTILS_H_

#include <contacts.h>
#include <contacts_internal.h>

#include "common/picojson.h"

namespace extension {
namespace callhistory {

class CallHistoryUtils {
public:
    static void parseRecordList(contacts_list_h *record_list, picojson::array& array);
    static void parseRecord(contacts_record_h *record, picojson::object& obj);
    static void parseLogType(contacts_phone_log_type_e log_type, picojson::object& obj);
    static void parseRemoteParties(contacts_record_h *record, picojson::object& obj);
    static void parseCallingParty(contacts_record_h *record, picojson::object& obj);
    static unsigned int convertAttributeName(const std::string attribute_name);
    static void createFilter(contacts_filter_h filter, const picojson::object filter_obj);
};

} // namespace callhistory
} // namespace extension

#endif // CALLHISTORY_CALLHISTORY_UTILS_H_
