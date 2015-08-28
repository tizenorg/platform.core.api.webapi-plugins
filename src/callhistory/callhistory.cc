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

#include "callhistory.h"

#include <thread>

#include <tapi_common.h>
#include <ITapiSim.h>
#include <contacts_db_extension.h>

#include "common/logger.h"
#include "common/platform_exception.h"
#include "common/task-queue.h"
#include "common/scope_exit.h"
#include "callhistory_instance.h"
#include "callhistory_types.h"

using namespace common;
using namespace tools;

namespace extension {
namespace callhistory {

std::vector<CallHistory*> CallHistory::instances_;
std::mutex CallHistory::instances_mutex_;

namespace {
static void get_sim_msisdn_cb(TapiHandle *handle, int result, void *data, void *user_data)
{
  LoggerD("Entered");

  TelSimMsisdnList_t *list;
  std::promise<std::string> *prom = reinterpret_cast<std::promise<std::string> *>(user_data);
  char *number = NULL;

  if (TAPI_SIM_ACCESS_SUCCESS == result) {
    list = static_cast<TelSimMsisdnList_t *>(data);
    if (list->count) {
      number = list->list[0].num;
    }
    LoggerD("Phone number: %s", number);
  } else {
    LoggerE("Failed to access sim: %d", result);
  }

  std::string n = number ? std::string(number) : "";
  prom->set_value(n);
}
}

CallHistory::CallHistory(CallHistoryInstance& instance)
    : m_is_listener_set(false),
      instance_(instance),
      utils_(*this) {
  LoggerD("Entered");
  if (CONTACTS_ERROR_NONE == contacts_connect()) {
    LoggerD("Successful to connect Call history DB");
  } else {
    LoggerD("Failed to connect Call history DB");
  }

  {
    std::lock_guard<std::mutex> lock(instances_mutex_);
    instances_.push_back(this);
  }
}

CallHistory::~CallHistory()
{
  LoggerD("Entered");

  if (m_is_listener_set) {
    int ret = contacts_db_remove_changed_cb_with_info(_contacts_phone_log._uri,
                                                      changeListenerCB, NULL);

    if (CONTACTS_ERROR_NONE != ret) {
      LoggerW("Failed to remove ChangeListener");
    }
  }

  if (CONTACTS_ERROR_NONE == contacts_disconnect()) {
    LoggerD("Successful to disconnect Call history DB");
  } else {
    LoggerD("Failed to disconnect Call history DB");
  }

  {
    std::lock_guard<std::mutex> lock(instances_mutex_);
    for (auto it = instances_.begin(); it != instances_.end(); ++it) {
      if (*it == this) {
        instances_.erase(it);
        break;
      }
    }
  }
}

void CallHistory::FindThread(const picojson::object& args, CallHistory* call_history)
{
  LoggerD("Entered");

  std::shared_ptr<picojson::value> response{new picojson::value(picojson::object())};
  int phone_numbers = call_history->getPhoneNumbers().size();
  const double callback_id = args.find("callbackId")->second.get<double>();

  if (phone_numbers == 0) {
    LoggerE("Phone numbers list is empty.");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Phone numbers list is empty."),
                &response->get<picojson::object>());
  } else {
    const auto it_args_end = args.end();
    const auto it_filter = args.find("filter");
    picojson::object filter_obj;
    if (it_filter != it_args_end &&
        it_filter->second.is<picojson::object>()) {
      filter_obj = it_filter->second.get<picojson::object>();
    }

    const auto it_sort_mode = args.find("sortMode");
    picojson::object sort_mode;
    if (it_sort_mode != it_args_end &&
        it_sort_mode->second.is<picojson::object>()) {
      sort_mode = it_sort_mode->second.get<picojson::object>();
    }

    std::string sort_attr_name;
    std::string sort_order;
    if (!sort_mode.empty()) {
      const auto it_sort_end = sort_mode.end();
      const auto it_sort_attr_name = sort_mode.find("attributeName");
      if (it_sort_attr_name != it_sort_end &&
          it_sort_attr_name->second.is<std::string>()) {
        sort_attr_name = it_sort_attr_name->second.get<std::string>();
      }

      const auto it_sort_order = sort_mode.find("order");
      if (it_sort_order != it_sort_end &&
          it_sort_order->second.is<std::string>()) {
        sort_order = it_sort_order->second.get<std::string>();
      }
    }

    const auto it_limit = args.find("limit");
    int limit = 0;
    if (it_limit != it_args_end &&
        it_limit->second.is<double>()) {
      limit = static_cast<int>(it_limit->second.get<double>());
    }

    const auto it_offset = args.find("offset");
    int offset = 0;
    if (it_offset != it_args_end &&
        it_offset->second.is<double>()) {
      offset = static_cast<int>(it_offset->second.get<double>());
    }

    contacts_query_h query = nullptr;
    contacts_filter_h filter = nullptr;
    contacts_list_h record_list = nullptr;

    SCOPE_EXIT {
      contacts_query_destroy(query);
      contacts_filter_destroy(filter);
      contacts_list_destroy(record_list, true);
    };

    int ret = contacts_connect_on_thread();
    if (CONTACTS_ERROR_NONE != ret) {
      LoggerW("contacts_connect_on_thread failed");
    }

    ret = contacts_query_create(_contacts_phone_log._uri, &query);
    if (CONTACTS_ERROR_NONE != ret) {
      LoggerW("contacts_query_create failed");
    }

    ret = contacts_filter_create(_contacts_phone_log._uri, &filter);
    if (CONTACTS_ERROR_NONE != ret) {
      LoggerW("contacts_filter_create failed");
    }

    //filter
    CallHistoryUtils& utils = call_history->getUtils();
    if (!filter_obj.empty()) {
      LoggerD("Filter is set");
      utils.createFilter(filter, filter_obj);
      ret = contacts_filter_add_operator(filter, CONTACTS_FILTER_OPERATOR_AND);
      if (CONTACTS_ERROR_NONE != ret) {
        LoggerW("contacts_filter_add_operator failed");
      }
    }

    ret = contacts_filter_add_int(filter, _contacts_phone_log.log_type,
                                  CONTACTS_MATCH_LESS_THAN_OR_EQUAL, CONTACTS_PLOG_TYPE_VIDEO_BLOCKED);
    if (CONTACTS_ERROR_NONE != ret) {
      LoggerW("contacts_filter_add_int failed");
    }

    ret = contacts_query_set_filter(query, filter);
    if (CONTACTS_ERROR_NONE != ret) {
      LoggerW("contacts_query_set_filter failed");
    }

    //sort mode
    bool is_asc = false;
    if (!sort_order.empty()) {
      if (STR_ORDER_ASC == sort_order) {
        is_asc = true;
      }
      unsigned int attribute = utils.convertAttributeName(sort_attr_name);
      ret = contacts_query_set_sort(query, attribute, is_asc);
    } else {
      ret = contacts_query_set_sort(query, _contacts_phone_log.id, is_asc);
    }

    if (CONTACTS_ERROR_NONE != ret) {
      LoggerW("contacts_query_set_sort failed");
    }

    //get records with limit and offset
    ret = contacts_db_get_records_with_query(query, offset, limit, &record_list);
    if (CONTACTS_ERROR_NONE != ret) {
      LoggerW("contacts_db_get_records_with_query failed");
    }

    picojson::object& obj = response->get<picojson::object>();
    picojson::array& array = obj.insert(std::make_pair(STR_DATA, picojson::value(
        picojson::array()))).first->second.get<picojson::array>();
    if (record_list) {
      utils.parseRecordList(&record_list, array);
    }

    ret = contacts_disconnect_on_thread();
    if (CONTACTS_ERROR_NONE != ret) {
      LoggerW("contacts_disconnect_on_thread failed");
    }

    ReportSuccess(response->get<picojson::object>());
  }

  auto find_response = [call_history, callback_id](const std::shared_ptr<picojson::value>& response) -> void {
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair("callbackId", picojson::value(callback_id)));
    CallHistory::PostMessage(call_history, response->serialize());
  };

  TaskQueue::GetInstance().Async<picojson::value>(find_response, response);
}

void CallHistory::LoadPhoneNumbers(const picojson::object& args, CallHistory* call_history)
{
  LoggerD("Entered");

  char** cp_list =  tel_get_cp_name_list();

  if (cp_list) {
    unsigned int modem_num = 0;
    std::vector<std::string>& phone_numbers = call_history->getPhoneNumbers();

    while (cp_list[modem_num]) {
      std::string n = "";
      TapiHandle* handle = nullptr;
      do {
        std::promise<std::string> prom;
        handle = tel_init(cp_list[modem_num]);
        if (!handle) {
          LoggerE("Failed to init tapi handle.");
          break;
        }

        int card_changed;
        TelSimCardStatus_t card_status = TAPI_SIM_STATUS_UNKNOWN;
        int ret = tel_get_sim_init_info(handle, &card_status, &card_changed);
        if (TAPI_API_SUCCESS != ret) {
          LoggerE("Failed to get sim init info: %d", ret);
          break;
        }
        LoggerD("Card status: %d Card Changed: %d", card_status, card_changed);
        if (TAPI_SIM_STATUS_SIM_INIT_COMPLETED != card_status) {
          LoggerW("SIM is not ready, we can't get other properties");
          break;
        }

        ret = tel_get_sim_msisdn(handle, get_sim_msisdn_cb, &prom);
        if (TAPI_API_SUCCESS != ret) {
          LoggerE("Failed to get msisdn : %d", ret);
          break;
        }

        auto fut = prom.get_future();
        LoggerD("wait...");
        fut.wait();
        n = fut.get();
        LoggerD("Phone number [%d] : %s", modem_num, n.c_str());
      } while(false);

      phone_numbers.push_back(n);
      tel_deinit(handle);
      modem_num++;
    }

    g_strfreev(cp_list);
  }

  FindThread(args, call_history);
}

void CallHistory::find(const picojson::object& args) {
  LoggerD("Entered");

  if (m_phone_numbers.size() == 0) {
    std::thread(LoadPhoneNumbers, args, this).detach();
  } else {
    std::thread(FindThread, args, this).detach();
  }
}

PlatformResult CallHistory::remove(const picojson::object& args)
{
  LoggerD("Entered");

  const auto it_uid = args.find("uid");
  const auto it_args_end = args.end();

  if (it_uid == it_args_end ||
      !it_uid->second.is<std::string>()) {
    LoggerE("Invalid parameter was passed.");
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                          "Invalid parameter was passed.");
  }

  int uid = atoi((it_uid->second.get<std::string>()).c_str());
  int ret = contacts_db_delete_record(_contacts_phone_log._uri, (int)uid);
  if (CONTACTS_ERROR_NONE != ret) {
    LoggerE("Failed to delete log record [%d] with error: %d", uid, ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to delete log record.");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

common::PlatformResult CallHistory::removeBatch(const picojson::object& args)
{
  LoggerD("Entered");

  const auto it_uid = args.find("uid");
  const auto it_args_end = args.end();

  if (it_uid == it_args_end ||
      !it_uid->second.is<picojson::array>()) {
    LoggerE("Invalid parameter was passed.");
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                          "Invalid parameter was passed.");
  }
  const picojson::array& uids = it_uid->second.get<picojson::array>();
  const double callback_id = args.find("callbackId")->second.get<double>();

  auto remove_batch = [uids](const std::shared_ptr<picojson::value>& response) -> void {
    if (uids.size() == 0) {
      ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR,
                                 "Object is null."),
                  &response->get<picojson::object>());
      return;
    }

    int ret = CONTACTS_ERROR_NONE;
    for (unsigned int i = 0; i < uids.size(); ++i) {
      int uid = atoi(uids[i].get<std::string>().c_str());
      ret = contacts_db_delete_record(_contacts_phone_log._uri, (int)uid);
      if (CONTACTS_ERROR_NONE != ret) {
        LoggerE("Failed to delete log [%d] with code %d", uid, ret);
        ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR,
                                   "Remove record failed."),
                    &response->get<picojson::object>());
        return;
      }
    }
    ReportSuccess(response->get<picojson::object>());
  };

  auto remove_batch_response = [this, callback_id](const std::shared_ptr<picojson::value>& response) -> void {
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair("callbackId", picojson::value(callback_id)));
    CallHistory::PostMessage(this, response->serialize());
  };

  TaskQueue::GetInstance().Queue<picojson::value>(
      remove_batch,
      remove_batch_response,
      std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));
  return PlatformResult(ErrorCode::NO_ERROR);
}

void CallHistory::removeAll(const picojson::object& args)
{
  LoggerD("Entered");

  const double callback_id = args.find("callbackId")->second.get<double>();

  auto remove_all = [](const std::shared_ptr<picojson::value>& response) -> void {

    contacts_list_h record_list = nullptr;
    int* list = NULL;
    SCOPE_EXIT {
      contacts_list_destroy(record_list, true);
      delete[] list;
    };

    contacts_record_h record = NULL;
    int total = 0;
    int value;
    unsigned int cnt = 0;

    int ret = contacts_connect_on_thread();
    if (CONTACTS_ERROR_NONE != ret) {
      LoggerW("contacts_connect_on_thread failed");
    }

    ret = contacts_db_get_all_records(_contacts_phone_log._uri, 0, 0, &record_list);
    if (CONTACTS_ERROR_NONE != ret || !record_list) {
      LoggerE("Failed to get all records list");
      ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR,
                                 "Failed to get all records list."),
                  &response->get<picojson::object>());
      return;
    }

    ret = contacts_list_get_count(record_list, &total);
    if (CONTACTS_ERROR_NONE != ret) {
      LoggerW("Failed to get count");
    }

    list = new int[total];
    for (int i = 0; i < total; i++) {
      LoggerD("Record number: %d", i);
      ret = contacts_list_get_current_record_p(record_list, &record);
      if (CONTACTS_ERROR_NONE != ret) {
        LoggerW("contacts_list_get_current_record_p function failed");
      }

      if (!record) {
        ret = contacts_list_next(record_list);
        if (CONTACTS_ERROR_NONE != ret && CONTACTS_ERROR_NO_DATA != ret) {
          LoggerE("contacts_list_next function failed");
          ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR,
                                     "Get next record from list failed."),
                      &response->get<picojson::object>());
          return;
        }
        continue;
      }

      if (CONTACTS_ERROR_NONE == ret) {
        ret = contacts_record_get_int(record, _contacts_phone_log.id , &value);
        if (CONTACTS_ERROR_NONE == ret) {
          list[cnt++] = value;
        }
      }

      value = 0;
      ret = contacts_list_next(record_list);
      if (CONTACTS_ERROR_NONE != ret && CONTACTS_ERROR_NO_DATA != ret) {
        LoggerE("contacts_list_next function failed");
        ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR,
                                   "Get next record from list failed."),
                    &response->get<picojson::object>());
        return;
      }
    }

    if (cnt > 0) {
      ret = contacts_db_delete_records(_contacts_phone_log._uri, list, cnt);
      if (CONTACTS_ERROR_NONE != ret) {
        LoggerE("contacts_db_delete_records function failed");
        ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR,
                                   "Delete records function failed."),
                    &response->get<picojson::object>());
        return;
      }
    }

    if (CONTACTS_ERROR_NONE != ret) {
      LoggerW("contacts_list_destroy failed");
    }

    ret = contacts_disconnect_on_thread();
    if (CONTACTS_ERROR_NONE != ret) {
      LoggerW("contacts_disconnect_on_thread failed");
    }

    ReportSuccess(response->get<picojson::object>());
  };

  auto remove_all_response = [this, callback_id](const std::shared_ptr<picojson::value>& response) -> void {
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair("callbackId", picojson::value(callback_id)));
    CallHistory::PostMessage(this, response->serialize());
  };

  TaskQueue::GetInstance().Queue<picojson::value>(
      remove_all,
      remove_all_response,
      std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));
}

std::vector<std::string>& CallHistory::getPhoneNumbers()
{
  return m_phone_numbers;
}

CallHistoryUtils& CallHistory::getUtils()
{
  return utils_;
}

void CallHistory::changeListenerCB(const char* view_uri, char *changes, void* user_data)
{
  LoggerD("Entered");

  CallHistory* h = static_cast<CallHistory*>(user_data);

  if (NULL == changes) {
    LoggerW("changes is NULL");
    return;
  }
  if (0 == strlen(changes)) {
    LoggerW("changes is empty");
    return;
  }

  char seps[] = ",:";
  char* token_type = NULL;
  int change_id = 0;

  picojson::value added = picojson::value(picojson::object());
  picojson::object& added_obj = added.get<picojson::object>();
  picojson::array& added_array = added_obj.insert(std::make_pair(STR_DATA, picojson::value(
      picojson::array()))).first->second.get<picojson::array>();

  picojson::value changed = picojson::value(picojson::object());
  picojson::object& changed_obj = changed.get<picojson::object>();
  picojson::array& changed_array = changed_obj.insert(std::make_pair(STR_DATA, picojson::value(
      picojson::array()))).first->second.get<picojson::array>();

  picojson::value removed = picojson::value(picojson::object());
  picojson::object& removed_obj = removed.get<picojson::object>();
  picojson::array& removed_array = removed_obj.insert(std::make_pair(STR_DATA, picojson::value(
      picojson::array()))).first->second.get<picojson::array>();

  char* saveptr = nullptr;

  token_type = strtok_r(changes, seps, &saveptr);
  while (NULL != token_type) {
    char* token_id = strtok_r(NULL, seps, &saveptr);
    int change_type = atoi((const char*)token_type);

    if (NULL != token_id) {
      change_id = atoi((const char*)token_id);
    } else {
      LoggerD("There is no (more) changed Item : %s", token_id);
      break;
    }

    contacts_query_h query = NULL;
    contacts_filter_h filter = NULL;
    contacts_list_h record_list = NULL;

    contacts_query_create(_contacts_phone_log._uri, &query);
    contacts_filter_create(_contacts_phone_log._uri, &filter);
    contacts_filter_add_int(filter, _contacts_phone_log.id, CONTACTS_MATCH_EQUAL, change_id);

    contacts_query_set_filter(query, filter);
    int ret = contacts_query_set_sort(query, _contacts_phone_log.id, false);
    if (CONTACTS_ERROR_NONE != ret) {
      LoggerD("Callhistory query error: %d", ret);
    }

    ret = contacts_db_get_records_with_query(query, 0, 1, &record_list);
    if (CONTACTS_ERROR_NONE != ret) {
      contacts_list_destroy(record_list, true);
      contacts_query_destroy(query);
      contacts_filter_destroy(filter);
      LoggerD("Callhistory query error: %d", ret);
      return;
    }

    if (CONTACTS_CHANGE_INSERTED == change_type) {
      h->utils_.parseRecordList(&record_list, added_array);
    } else if (CONTACTS_CHANGE_UPDATED == change_type) {
      h->utils_.parseRecordList(&record_list, changed_array);
    } else if (CONTACTS_CHANGE_DELETED == change_type) {
      removed_array.push_back(picojson::value(token_id));
    }

    contacts_list_destroy(record_list, true);
    contacts_query_destroy(query);
    contacts_filter_destroy(filter);

    token_type = strtok_r( NULL, seps, &saveptr);
  }

  if (added_array.size() > 0) {
    added_obj[STR_ACTION] = picojson::value("onadded");
    h->instance_.CallHistoryChange(added_obj);
  }
  if (changed_array.size() > 0) {
    changed_obj[STR_ACTION] = picojson::value("onchanged");
    h->instance_.CallHistoryChange(changed_obj);
  }
  if (removed_array.size() > 0) {
    removed_obj[STR_ACTION] = picojson::value("onremoved");
    h->instance_.CallHistoryChange(removed_obj);
  }
}

void CallHistory::PostMessage(const CallHistory* instance, const std::string& msg) {
  LoggerD("Entered");
  std::lock_guard<std::mutex> lock(instances_mutex_);

  for (auto it = instances_.begin(); it != instances_.end(); ++it) {
    if (*it == instance) {
      Instance::PostMessage(&instance->instance_, msg.c_str());
      return;
    }
  }

  LoggerE("Instance [%p] not found, ignoring message", instance);
}

PlatformResult CallHistory::startCallHistoryChangeListener()
{
  LoggerD("Entered");

  if (!m_is_listener_set) {
    int ret = contacts_db_add_changed_cb_with_info(_contacts_phone_log._uri,
                                                   changeListenerCB, this);

    if (CONTACTS_ERROR_NONE != ret) {
      LoggerE("Failed to add ChangeListener");
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Failed to add ChangeListener");
    }
  }

  m_is_listener_set = true;
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CallHistory::stopCallHistoryChangeListener()
{
  LoggerD("Entered");
  if (m_is_listener_set) {
    int ret = contacts_db_remove_changed_cb_with_info(_contacts_phone_log._uri,
                                                      changeListenerCB, this);

    if (CONTACTS_ERROR_NONE != ret) {
      LoggerE("Failed to remove ChangeListener");
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Failed to remove ChangeListener");
    }
  }
  m_is_listener_set = false;
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CallHistory::setMissedDirection(int uid)
{
  LoggerD("Entered");

  contacts_record_h record = nullptr;
  SCOPE_EXIT {
    contacts_record_destroy(record, true);
  };

  int log_type = CONTACTS_PLOG_TYPE_NONE;

  int ret = contacts_db_get_record(_contacts_phone_log._uri, uid, &record);
  if (CONTACTS_ERROR_NONE != ret) {
    LoggerE("Failed to get record [%d]", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to get record");
  }

  ret = contacts_record_get_int(record, _contacts_phone_log.log_type, &log_type);
  if (CONTACTS_ERROR_NONE != ret) {
    LoggerE("Failed to get log type [%d]", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to get log type");
  }

  if (CONTACTS_PLOG_TYPE_VOICE_INCOMMING_UNSEEN == log_type) {
    ret = contacts_record_set_int(
        record, _contacts_phone_log.log_type, CONTACTS_PLOG_TYPE_VOICE_INCOMMING_SEEN);
  } else if (CONTACTS_PLOG_TYPE_VIDEO_INCOMMING_UNSEEN == log_type) {
    ret = contacts_record_set_int(
        record, _contacts_phone_log.log_type, CONTACTS_PLOG_TYPE_VIDEO_INCOMMING_SEEN);
  } else {
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  if (CONTACTS_ERROR_NONE != ret) {
    LoggerE("Failed to set direction [%d]", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to set direction");
  }

  ret = contacts_db_update_record(record);
  if (CONTACTS_ERROR_NONE != ret) {
    LoggerE("Failed to update record [%d]", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to update record");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

} // namespace callhistory
} // namespace extension
