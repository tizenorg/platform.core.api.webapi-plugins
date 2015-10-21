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

#include "callhistory_utils.h"

#include <stack>
#include <ctime>

#include "callhistory_types.h"
#include "callhistory.h"
#include "common/logger.h"

namespace extension {
namespace callhistory {

namespace {
enum FilterTypeEnum {
  FILTER_ATTRIBUTE = 0,
  FILTER_ATTRIBUTE_RANGE = 1,
  FILTER_COMPOSITE = 2,
  FILTER_UNKNOWN = 3
};

enum CompositeTypeEnum {
  UNION = 0,
  INTERSECTION = 1,
  NONE = 2
};
}

CallHistoryUtils::CallHistoryUtils(CallHistory& history)
    : history_(history) {
}

void CallHistoryUtils::parseRecordList(contacts_list_h *record_list, picojson::array& array)
{
  LoggerD("Entered");

  contacts_record_h record = NULL;
  int total = 0;

  int ret = contacts_list_get_count(*record_list, &total);
  if (CONTACTS_ERROR_NONE != ret) {
    LoggerW("Failed to get contacts list: %d", ret);
    return;
  }

  for (int i = 0; i < total; i++) {
    ret = contacts_list_get_current_record_p(*record_list, &record);
    if (NULL != record) {
      array.push_back(picojson::value(picojson::object()));
      picojson::object& obj = array.back().get<picojson::object>();
      parseRecord(&record, obj);
    } else {
      ret = contacts_list_next(*record_list);
      if (CONTACTS_ERROR_NONE != ret && CONTACTS_ERROR_NO_DATA != ret) {
        LoggerW("Callhistory list parse error: %d", ret);
        return;
      }
    }
    ret = contacts_list_next(*record_list);
    if (CONTACTS_ERROR_NONE != ret && CONTACTS_ERROR_NO_DATA != ret) {
      LoggerW("Callhistory list parse error: %d", ret);
      return;
    }
  }
}

void CallHistoryUtils::parseRecord(contacts_record_h *record, picojson::object& obj)
{
  LoggerD("Entered");

  int int_data;

  int ret = contacts_record_get_int(*record, _contacts_phone_log.id, &int_data);
  if (CONTACTS_ERROR_NONE != ret) {
    LoggerD("Failed to get contacts phone log id: %d", ret);
  } else {
    obj[STR_ENTRY_ID] = picojson::value(static_cast<double>(int_data));
  }

  ret = contacts_record_get_int(*record, _contacts_phone_log.log_type, &int_data);
  if (CONTACTS_ERROR_NONE != ret) {
    LoggerD("Failed to get contacts phone log type: %d", ret);
  } else {
    parseLogType(static_cast<contacts_phone_log_type_e>(int_data), obj);
  }

  ret = contacts_record_get_int(*record, _contacts_phone_log.log_time, &int_data);
  if (CONTACTS_ERROR_NONE != ret) {
    LoggerD("Failed to get contacts phone log time: %d", ret);
  } else {
    obj[STR_START_TIME] = picojson::value(static_cast<double>(int_data));
  }

  ret = contacts_record_get_int(*record, _contacts_phone_log.extra_data1, &int_data);
  if (CONTACTS_ERROR_NONE != ret) {
    LoggerD("Failed to get contacts phone log extra data: %d", ret);
  } else {
    obj[STR_DURATION] = picojson::value(static_cast<double>(int_data));
  }

  parseRemoteParties(record, obj);
  parseCallingParty(record, obj);
}

void CallHistoryUtils::parseLogType(contacts_phone_log_type_e log_type, picojson::object& obj)
{
  LoggerD("Entered");

  picojson::value val = picojson::value(picojson::array());
  picojson::array& features = val.get<picojson::array>();

  switch(log_type) {
    case CONTACTS_PLOG_TYPE_VOICE_INCOMMING:
      obj[STR_CALL_TYPE] = picojson::value(STR_CALLTYPE_TEL);
      obj[STR_DIRECTION] = picojson::value(STR_RECEIVED);
      features.push_back(picojson::value(STR_CALL_VOICE));
      break;
    case CONTACTS_PLOG_TYPE_VOICE_OUTGOING:
      obj[STR_CALL_TYPE] = picojson::value(STR_CALLTYPE_TEL);
      obj[STR_DIRECTION] = picojson::value(STR_DIALED);
      features.push_back(picojson::value(STR_CALL_VOICE));
      break;
    case CONTACTS_PLOG_TYPE_VOICE_INCOMMING_SEEN:
      obj[STR_CALL_TYPE] = picojson::value(STR_CALLTYPE_TEL);
      obj[STR_DIRECTION] = picojson::value(STR_MISSED);
      features.push_back(picojson::value(STR_CALL_VOICE));
      break;
    case CONTACTS_PLOG_TYPE_VOICE_INCOMMING_UNSEEN:
      obj[STR_CALL_TYPE] = picojson::value(STR_CALLTYPE_TEL);
      obj[STR_DIRECTION] = picojson::value(STR_MISSED_NEW);
      features.push_back(picojson::value(STR_CALL_VOICE));
      break;
    case CONTACTS_PLOG_TYPE_VOICE_REJECT:
      obj[STR_CALL_TYPE] = picojson::value(STR_CALLTYPE_TEL);
      obj[STR_DIRECTION] = picojson::value(STR_REJECTED);
      features.push_back(picojson::value(STR_CALL_VOICE));
      break;
    case CONTACTS_PLOG_TYPE_VOICE_BLOCKED:
      obj[STR_CALL_TYPE] = picojson::value(STR_CALLTYPE_TEL);
      obj[STR_DIRECTION] = picojson::value(STR_BLOCKED);
      features.push_back(picojson::value(STR_CALL_VOICE));
      break;
    case CONTACTS_PLOG_TYPE_VIDEO_INCOMMING:
      obj[STR_CALL_TYPE] = picojson::value(STR_CALLTYPE_TEL);
      obj[STR_DIRECTION] = picojson::value(STR_RECEIVED);
      features.push_back(picojson::value(STR_CALL_VIDEO));
      break;
    case CONTACTS_PLOG_TYPE_VIDEO_OUTGOING:
      obj[STR_CALL_TYPE] = picojson::value(STR_CALLTYPE_TEL);
      obj[STR_DIRECTION] = picojson::value(STR_DIALED);
      features.push_back(picojson::value(STR_CALL_VIDEO));
      break;
    case CONTACTS_PLOG_TYPE_VIDEO_INCOMMING_SEEN:
      obj[STR_CALL_TYPE] = picojson::value(STR_CALLTYPE_TEL);
      obj[STR_DIRECTION] = picojson::value(STR_MISSED);
      features.push_back(picojson::value(STR_CALL_VIDEO));
      break;
    case CONTACTS_PLOG_TYPE_VIDEO_INCOMMING_UNSEEN:
      obj[STR_CALL_TYPE] = picojson::value(STR_CALLTYPE_TEL);
      obj[STR_DIRECTION] = picojson::value(STR_MISSED_NEW);
      features.push_back(picojson::value(STR_CALL_VIDEO));
      break;
    case CONTACTS_PLOG_TYPE_VIDEO_REJECT:
      obj[STR_CALL_TYPE] = picojson::value(STR_CALLTYPE_TEL);
      obj[STR_DIRECTION] = picojson::value(STR_REJECTED);
      features.push_back(picojson::value(STR_CALL_VIDEO));
      break;
    case CONTACTS_PLOG_TYPE_VIDEO_BLOCKED:
      obj[STR_CALL_TYPE] = picojson::value(STR_CALLTYPE_TEL);
      obj[STR_DIRECTION] = picojson::value(STR_BLOCKED);
      features.push_back(picojson::value(STR_CALL_VIDEO));
      break;
    default:
      LoggerW("Wrong phone log type: %d", log_type);
      return;
  }

  if (features.size() > 0) {
    obj.insert(std::make_pair(STR_TAGS, picojson::value(features)));
  }
}

void CallHistoryUtils::parseRemoteParties(contacts_record_h *record, picojson::object& obj)
{
  LoggerD("Entered");

  char * char_data = NULL;
  int int_data;

  picojson::array& remote_parties = obj.insert(std::make_pair(STR_REMOTE_PARTIES, picojson::value(
      picojson::array()))).first->second.get<picojson::array>();
  remote_parties.push_back(picojson::value(picojson::object()));
  picojson::object& parties_obj = remote_parties.back().get<picojson::object>();

  int ret = contacts_record_get_int(*record, _contacts_phone_log.person_id, &int_data);
  if (CONTACTS_ERROR_NONE != ret) {
    LoggerD("Failed to get contacts phone log person id: %d", ret);
  } else {
    parties_obj[STR_PERSON_ID] = picojson::value(static_cast<double>(int_data));
  }

  ret = contacts_record_get_str_p(*record, _contacts_phone_log.address, &char_data);
  if (CONTACTS_ERROR_NONE != ret) {
    LoggerD("Failed to get contacts phone log address: %d", ret);
  } else if (NULL != char_data) {
    parties_obj[STR_REMOTE_PARTY] = picojson::value(char_data);
  }
}

void CallHistoryUtils::parseCallingParty(contacts_record_h *record, picojson::object& obj)
{
  LoggerD("Entered");

  const std::vector<std::string>& phone_numbers = history_.getPhoneNumbers();
  int sim_count = phone_numbers.size();
  int sim_index;

  int ret = contacts_record_get_int(*record, _contacts_phone_log.sim_slot_no, &sim_index);
  if (CONTACTS_ERROR_NONE != ret) {
    LoggerW("Failed to get sim slot no. %d", ret);
  }

  if (sim_index >= sim_count) {
    LoggerE("sim slot no. [%d] is out of count %d", sim_index, sim_count);
  } else if (sim_index > 0) {
    obj[STR_CALLING_PARTY] = picojson::value(phone_numbers.at(sim_index));
  }
}

unsigned int CallHistoryUtils::convertAttributeName(const std::string& attribute_name)
{
  LoggerD("Entered");

  if (STR_RP_REMOTEPARTY == attribute_name) {
    return _contacts_phone_log.address;
  } else if (STR_START_TIME == attribute_name) {
    return _contacts_phone_log.log_time;
  } else if (STR_DURATION == attribute_name) {
    return _contacts_phone_log.extra_data1;
  } else if (STR_DIRECTION == attribute_name) {
    return _contacts_phone_log.log_type;
  } else if (STR_ENTRY_ID == attribute_name) {
    return _contacts_phone_log.id;
  } else {
    return 0;
  }
}

static FilterTypeEnum getFilterType(const picojson::object &filter)
{
  LoggerD("Entered");

  const auto it_end = filter.end();

  FilterTypeEnum type = FILTER_UNKNOWN;

  const auto it_match_flag = filter.find("matchFlag");
  if (it_match_flag != it_end) {
    type = FILTER_ATTRIBUTE;
  }

  const auto it_initial_value = filter.find("initialValue");
  const auto it_end_value = filter.find("endValue");
  if (it_initial_value != it_end && it_end_value != it_end) {
    type = FILTER_ATTRIBUTE_RANGE;
  }

  const auto it_type = filter.find("type");
  if (it_type != it_end) {
    type = FILTER_COMPOSITE;
  }
  return type;
}

static CompositeTypeEnum getCompositeType(const picojson::object &filter)
{
  LoggerD("Entered");
  CompositeTypeEnum type = NONE;
  const std::string& str_type = filter.find("type")->second.get<std::string>();

  if ("UNION" == str_type) {
    type = UNION;
  } else if ("INTERSECTION" == str_type) {
    type = INTERSECTION;
  }
  return type;
}

static contacts_match_str_flag_e getMatchFlag(const std::string& match_flag)
{
  LoggerD("Entered");
  if (STR_FILTER_FULLSTRING == match_flag) {
    return CONTACTS_MATCH_FULLSTRING;
  } else if (STR_FILTER_CONTAINS == match_flag) {
    return CONTACTS_MATCH_CONTAINS;
  } else if (STR_FILTER_STARTSWITH == match_flag) {
    return CONTACTS_MATCH_STARTSWITH;
  } else if (STR_FILTER_ENDSWITH == match_flag) {
    return CONTACTS_MATCH_ENDSWITH;
  } else if (STR_FILTER_EXISTS == match_flag) {
    return CONTACTS_MATCH_EXISTS;
  } else {
    return CONTACTS_MATCH_EXACTLY;
  }
}

static std::time_t toTimeT(const std::string &value)
{
  LoggerD("Entered");
  struct tm date;
  if (nullptr == strptime(value.c_str(), "%Y-%m-%dT%H:%M:%S", &date)) {
    LOGW("Couldn't convert supplied date.");
  }
  return mktime(&date);
}

static void visitAttribute(std::stack<contacts_filter_h>& stack, const picojson::object filter)
{
  LoggerD("Entered");

  contacts_filter_h filter_top = stack.top();
  contacts_filter_h sub_filter = NULL;
  contacts_filter_create(_contacts_phone_log._uri, &sub_filter);

  const auto it_attr_name = filter.find("attributeName");
  const std::string &attr_name = it_attr_name->second.get<std::string>();

  const auto it_match_flag = filter.find("matchFlag");
  const std::string &match_flag = it_match_flag->second.get<std::string>();
  contacts_match_str_flag_e c_match_flag = getMatchFlag(match_flag);

  const auto it_match_value = filter.find("matchValue");
  std::string match_value_str;
  if (!it_match_value->second.is<picojson::null>()) {
    if (it_match_value->second.is<double>()) {
      match_value_str = std::to_string(it_match_value->second.get<double>());
    } else {
      match_value_str = it_match_value->second.get<std::string>();
    }
  }

  if (STR_DIRECTION == attr_name) {
    if (STR_RECEIVED == match_value_str) {
      contacts_filter_add_int(sub_filter, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_EQUAL, CONTACTS_PLOG_TYPE_VOICE_INCOMMING);
      contacts_filter_add_operator(sub_filter, CONTACTS_FILTER_OPERATOR_OR);
      contacts_filter_add_int(sub_filter, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_EQUAL, CONTACTS_PLOG_TYPE_VIDEO_INCOMMING);
      contacts_filter_add_filter(filter_top, sub_filter);
    }
    else if (STR_DIALED == match_value_str) {
      contacts_filter_add_int(sub_filter, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_EQUAL, CONTACTS_PLOG_TYPE_VOICE_OUTGOING);
      contacts_filter_add_operator(sub_filter, CONTACTS_FILTER_OPERATOR_OR);
      contacts_filter_add_int(sub_filter, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_EQUAL, CONTACTS_PLOG_TYPE_VIDEO_OUTGOING);
      contacts_filter_add_filter(filter_top, sub_filter);
    }
    else if (STR_MISSED == match_value_str) {
      contacts_filter_add_int(sub_filter, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_EQUAL, CONTACTS_PLOG_TYPE_VOICE_INCOMMING_SEEN);
      contacts_filter_add_operator(sub_filter, CONTACTS_FILTER_OPERATOR_OR);
      contacts_filter_add_int(sub_filter, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_EQUAL, CONTACTS_PLOG_TYPE_VIDEO_INCOMMING_SEEN);
      contacts_filter_add_filter(filter_top, sub_filter);
    }
    else if (STR_MISSED_NEW == match_value_str) {
      contacts_filter_add_int(sub_filter, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_EQUAL, CONTACTS_PLOG_TYPE_VOICE_INCOMMING_UNSEEN);
      contacts_filter_add_operator(sub_filter, CONTACTS_FILTER_OPERATOR_OR);
      contacts_filter_add_int(sub_filter, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_EQUAL, CONTACTS_PLOG_TYPE_VIDEO_INCOMMING_UNSEEN);
      contacts_filter_add_filter(filter_top, sub_filter);
    }
    else if (STR_REJECTED == match_value_str) {
      contacts_filter_add_int(sub_filter, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_EQUAL, CONTACTS_PLOG_TYPE_VOICE_REJECT);
      contacts_filter_add_operator(sub_filter, CONTACTS_FILTER_OPERATOR_OR);
      contacts_filter_add_int(sub_filter, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_EQUAL, CONTACTS_PLOG_TYPE_VIDEO_REJECT);
      contacts_filter_add_filter(filter_top, sub_filter);
    }
    else if (STR_BLOCKED == match_value_str) {
      contacts_filter_add_int(sub_filter, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_EQUAL, CONTACTS_PLOG_TYPE_VOICE_BLOCKED);
      contacts_filter_add_operator(sub_filter, CONTACTS_FILTER_OPERATOR_OR);
      contacts_filter_add_int(sub_filter, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_EQUAL, CONTACTS_PLOG_TYPE_VIDEO_BLOCKED);
      contacts_filter_add_filter(filter_top, sub_filter);
    }
    else {
      contacts_filter_add_int(filter_top, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_EQUAL, CONTACTS_PLOG_TYPE_NONE);
    }
  }
  else if (STR_TAGS == attr_name) {
    if (STR_CALL == match_value_str) {
      contacts_filter_add_int(sub_filter, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_GREATER_THAN_OR_EQUAL,
                              CONTACTS_PLOG_TYPE_VOICE_INCOMMING);
      contacts_filter_add_operator(sub_filter, CONTACTS_FILTER_OPERATOR_AND);
      contacts_filter_add_int(sub_filter, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_LESS_THAN_OR_EQUAL, CONTACTS_PLOG_TYPE_VIDEO_BLOCKED);
      contacts_filter_add_filter(filter_top, sub_filter);
    }
    else if (STR_CALL_VOICE == match_value_str) {
      contacts_filter_add_int(sub_filter, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_EQUAL, CONTACTS_PLOG_TYPE_VOICE_INCOMMING);
      contacts_filter_add_operator(sub_filter, CONTACTS_FILTER_OPERATOR_OR);
      contacts_filter_add_int(sub_filter, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_EQUAL, CONTACTS_PLOG_TYPE_VOICE_OUTGOING);
      contacts_filter_add_operator(sub_filter, CONTACTS_FILTER_OPERATOR_OR);
      contacts_filter_add_int(sub_filter, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_EQUAL, CONTACTS_PLOG_TYPE_VOICE_INCOMMING_UNSEEN);
      contacts_filter_add_operator(sub_filter, CONTACTS_FILTER_OPERATOR_OR);
      contacts_filter_add_int(sub_filter, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_EQUAL, CONTACTS_PLOG_TYPE_VOICE_INCOMMING_SEEN);
      contacts_filter_add_operator(sub_filter, CONTACTS_FILTER_OPERATOR_OR);
      contacts_filter_add_int(sub_filter, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_EQUAL, CONTACTS_PLOG_TYPE_VOICE_REJECT);
      contacts_filter_add_operator(sub_filter, CONTACTS_FILTER_OPERATOR_OR);
      contacts_filter_add_int(sub_filter, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_EQUAL, CONTACTS_PLOG_TYPE_VOICE_BLOCKED);
      contacts_filter_add_filter(filter_top, sub_filter);
    }
    else if (STR_CALL_VIDEO == match_value_str) {
      contacts_filter_add_int(sub_filter, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_EQUAL, CONTACTS_PLOG_TYPE_VIDEO_INCOMMING);
      contacts_filter_add_operator(sub_filter, CONTACTS_FILTER_OPERATOR_OR);
      contacts_filter_add_int(sub_filter, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_EQUAL, CONTACTS_PLOG_TYPE_VIDEO_OUTGOING);
      contacts_filter_add_operator(sub_filter, CONTACTS_FILTER_OPERATOR_OR);
      contacts_filter_add_int(sub_filter, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_EQUAL, CONTACTS_PLOG_TYPE_VIDEO_INCOMMING_UNSEEN);
      contacts_filter_add_operator(sub_filter, CONTACTS_FILTER_OPERATOR_OR);
      contacts_filter_add_int(sub_filter, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_EQUAL, CONTACTS_PLOG_TYPE_VIDEO_INCOMMING_SEEN);
      contacts_filter_add_operator(sub_filter, CONTACTS_FILTER_OPERATOR_OR);
      contacts_filter_add_int(sub_filter, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_EQUAL, CONTACTS_PLOG_TYPE_VIDEO_REJECT);
      contacts_filter_add_operator(sub_filter, CONTACTS_FILTER_OPERATOR_OR);
      contacts_filter_add_int(sub_filter, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_EQUAL, CONTACTS_PLOG_TYPE_VIDEO_BLOCKED);
      contacts_filter_add_filter(filter_top, sub_filter);
    }
    else if (STR_CALL_EMERGENCY == match_value_str) {
      contacts_filter_add_str(filter_top, _contacts_phone_log.extra_data2,
                              CONTACTS_MATCH_EXACTLY, "001");
    }
    else {
      contacts_filter_add_int(filter_top, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_EQUAL, CONTACTS_PLOG_TYPE_NONE);
    }
  }
  else if (STR_RP_REMOTEPARTY == attr_name) {
    contacts_filter_add_str(filter_top, _contacts_phone_log.address, c_match_flag,
                            match_value_str.c_str());
  }
  else if (STR_RP_PERSONID == attr_name) {
    contacts_filter_add_str(filter_top, _contacts_phone_log.person_id, c_match_flag,
                            match_value_str.c_str());
  }
  else if (STR_START_TIME == attr_name) {
    LoggerD("DATE _____>>>>>>>>> %s", match_value_str.c_str());
    contacts_filter_add_int(filter_top, _contacts_phone_log.log_time,
                            CONTACTS_MATCH_EQUAL, static_cast<int>(toTimeT(match_value_str)));
  }
  else if (STR_DURATION == attr_name) {
    contacts_filter_add_str(filter_top, _contacts_phone_log.extra_data1, c_match_flag,
                            match_value_str.c_str());
  }
  else if (STR_CALL_TYPE == attr_name) {
    if (STR_CALLTYPE_TEL == match_value_str) {
      contacts_filter_add_int(sub_filter, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_GREATER_THAN_OR_EQUAL,
                              CONTACTS_PLOG_TYPE_VOICE_INCOMMING);
      contacts_filter_add_operator(sub_filter, CONTACTS_FILTER_OPERATOR_AND);
      contacts_filter_add_int(sub_filter, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_LESS_THAN_OR_EQUAL, CONTACTS_PLOG_TYPE_VIDEO_BLOCKED);
      contacts_filter_add_filter(filter_top, sub_filter);
    }
    else {
      contacts_filter_add_int(filter_top, _contacts_phone_log.log_type,
                              CONTACTS_MATCH_EQUAL, CONTACTS_PLOG_TYPE_NONE);
    }
  }
  else if (STR_ENTRY_ID == attr_name) {
    contacts_filter_add_str(filter_top, _contacts_phone_log.id, c_match_flag,
                            match_value_str.c_str());
  }
  else {
    contacts_filter_add_int(filter_top, _contacts_phone_log.log_type,
                            CONTACTS_MATCH_EQUAL, CONTACTS_PLOG_TYPE_NONE);
  }

  contacts_filter_destroy(sub_filter);
}

static void visitAttributeRange(std::stack<contacts_filter_h>& stack, const picojson::object filter)
{
  LoggerD("Entered");

  unsigned int prop_id = 0;
  const auto it_attr_name = filter.find("attributeName");
  const std::string &attr_name = it_attr_name->second.get<std::string>();

  if (STR_RP_REMOTEPARTY == attr_name) {
    prop_id = _contacts_phone_log.address;
  } else if (STR_RP_PERSONID == attr_name) {
    prop_id = _contacts_phone_log.person_id;
  } else if (STR_START_TIME == attr_name) {
    prop_id = _contacts_phone_log.log_time;
  } else if (STR_DURATION == attr_name) {
    prop_id = _contacts_phone_log.extra_data1;
  } else if (STR_ENTRY_ID == attr_name) {
    prop_id = _contacts_phone_log.id;
  } else {
    LoggerW("attribute: %s is not supported", attr_name.c_str());
    return;
  }

  std::string i_val;
  std::string e_val;
  contacts_filter_h filter_top = stack.top();
  contacts_filter_h sub_filter = NULL;

  const auto it_initial_value = filter.find("initialValue");
  const auto it_end_value = filter.find("endValue");
  bool is_i_val_null = it_initial_value->second.is<picojson::null>();
  bool is_e_val_null = it_end_value->second.is<picojson::null>();

  if (!is_i_val_null) {
    if (it_initial_value->second.is<double>()) {
      i_val = std::to_string(it_initial_value->second.is<double>());
    } else {
      i_val = it_initial_value->second.get<std::string>();
    }
  }

  if (!is_e_val_null) {
    if (it_end_value->second.is<double>()) {
      e_val = std::to_string(it_end_value->second.is<double>());
    } else {
      e_val = it_end_value->second.get<std::string>();
    }
  }

  contacts_filter_create(_contacts_phone_log._uri, &sub_filter);

  if (_contacts_phone_log.address == prop_id) {
    if (!is_i_val_null && is_e_val_null) {
      contacts_filter_add_str(filter_top, prop_id, CONTACTS_MATCH_STARTSWITH,i_val.c_str());
    }
    else if (is_i_val_null && !is_e_val_null) {
      contacts_filter_add_str(filter_top, prop_id, CONTACTS_MATCH_ENDSWITH, e_val.c_str());
    }
    else if (!is_i_val_null && !is_e_val_null) {
      contacts_filter_add_str(sub_filter, prop_id, CONTACTS_MATCH_STARTSWITH, i_val.c_str());
      contacts_filter_add_operator(sub_filter, CONTACTS_FILTER_OPERATOR_AND);
      contacts_filter_add_str(sub_filter, prop_id, CONTACTS_MATCH_ENDSWITH, e_val.c_str());
      contacts_filter_add_filter(filter_top, sub_filter);
    }
  }
  else {
    int i_val_int = atoi(i_val.c_str());
    int e_val_int = atoi(e_val.c_str());

    if (!is_i_val_null && is_e_val_null) {
      contacts_filter_add_int(filter_top, prop_id,
                              CONTACTS_MATCH_GREATER_THAN_OR_EQUAL, i_val_int);
    }
    else if (is_i_val_null && !is_e_val_null) {
      contacts_filter_add_int(filter_top, prop_id, CONTACTS_MATCH_LESS_THAN, e_val_int);
    }
    else if (!is_i_val_null && !is_e_val_null) {
      contacts_filter_add_int(sub_filter, prop_id,
                              CONTACTS_MATCH_GREATER_THAN_OR_EQUAL, i_val_int);
      contacts_filter_add_operator(sub_filter, CONTACTS_FILTER_OPERATOR_AND);
      contacts_filter_add_int(sub_filter, prop_id, CONTACTS_MATCH_LESS_THAN, e_val_int);
      contacts_filter_add_filter(filter_top, sub_filter);
    }
  }
  contacts_filter_destroy(sub_filter);
}

static void generateFilter(std::stack<contacts_filter_h>& stack,
                           const picojson::object filter,
                           CompositeTypeEnum type)
{
  LoggerD("Entered");
  switch (getFilterType(filter)) {
    case FILTER_ATTRIBUTE: {
      visitAttribute(stack, filter);
      if (type != NONE) {
        contacts_filter_h top_filter = stack.top();
        if (type == UNION) {
          contacts_filter_add_operator(top_filter, CONTACTS_FILTER_OPERATOR_OR);
        } else {
          contacts_filter_add_operator(top_filter, CONTACTS_FILTER_OPERATOR_AND);
        }
      }
    }
    break;
    case FILTER_ATTRIBUTE_RANGE: {
      visitAttributeRange(stack, filter);
      if (type != NONE) {
        contacts_filter_h top_filter = stack.top();
        if (type == UNION) {
          contacts_filter_add_operator(top_filter, CONTACTS_FILTER_OPERATOR_OR);
        } else {
          contacts_filter_add_operator(top_filter, CONTACTS_FILTER_OPERATOR_AND);
        }
      }
    }
    break;
    case FILTER_COMPOSITE: {
      contacts_filter_h contact_filter = NULL;
      contacts_filter_create(_contacts_phone_log._uri, &contact_filter);
      if (contact_filter != NULL) {
        stack.push(contact_filter);
      }

      CompositeTypeEnum composite_type = getCompositeType(filter);
      const picojson::array& filters =
          filter.find("filters")->second.get<picojson::array>();

      for (auto it = filters.begin(); it != filters.end(); ++it) {
        generateFilter(stack, it->get<picojson::object>(), composite_type);
      }

      contacts_filter_h top_filter = stack.top();
      if (stack.size() > 1) {
        stack.pop();
        contacts_filter_h parent_filter = stack.top();
        contacts_filter_add_filter(parent_filter, top_filter);
        contacts_filter_destroy(top_filter);
      }
    }
    break;
    case FILTER_UNKNOWN:
      return;
      break;
  }
}

void CallHistoryUtils::createFilter(contacts_filter_h filter, const picojson::object filter_obj)
{
  LoggerD("Entered");
  std::stack<contacts_filter_h> filter_stack;
  filter_stack.push(filter);
  generateFilter(filter_stack, filter_obj, CompositeTypeEnum::NONE);
}

} // namespace callhistory
} // namespace extension
