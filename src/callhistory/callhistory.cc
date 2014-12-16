// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "callhistory.h"

#include <tapi_common.h>
#include <ITapiSim.h>

#include "common/logger.h"
#include "common/platform_exception.h"
#include "common/task-queue.h"
#include "callhistory_instance.h"
#include "callhistory_types.h"
#include "callhistory_utils.h"

using namespace common;

namespace extension {
namespace callhistory {

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

void ReportSuccess(picojson::object& out) {
  out.insert(std::make_pair("status", picojson::value("success")));
}

void ReportError(const PlatformException& ex, picojson::object& out) {
  out.insert(std::make_pair("status", picojson::value("error")));
  out.insert(std::make_pair("error", ex.ToJSON()));
}

}

CallHistory::CallHistory():
        m_is_listener_set(false)
{
    LoggerD("Entered");
    if (CONTACTS_ERROR_NONE == contacts_connect()) {
        LoggerD("Successful to connect Call history DB");
    } else {
        LoggerD("Failed to connect Call history DB");
    }
    //TODO Uncomment below line if getting sim info will be possible
    //loadPhoneNumbers();
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
}

CallHistory* CallHistory::getInstance(){
    static CallHistory instance;
    return &instance;
}

void CallHistory::find(const picojson::object& args)
{
    LoggerD("Entered");

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

    const double callback_id = args.find("callbackId")->second.get<double>();
    int phone_numbers = m_phone_numbers.size();

    auto find = [filter_obj, sort_attr_name, sort_order, limit, offset, phone_numbers](
            const std::shared_ptr<picojson::value>& response) -> void {
        contacts_query_h query = NULL;
        contacts_filter_h filter = NULL;
        contacts_list_h record_list = NULL;

        try {
            if (phone_numbers == 0) {
                LoggerE("Phone numbers list is empty.");
                //Uncomment below line if gettin sim info will be possible (loadPhonesNumbers)
                //throw UnknownException("Phone numbers list is empty.");
            }

            int ret = CONTACTS_ERROR_NONE;
            ret = contacts_connect_on_thread();
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
            if (!filter_obj.empty()) {
                LoggerD("Filter is set");
                CallHistoryUtils::createFilter(filter, filter_obj);
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
                unsigned int attribute = CallHistoryUtils::convertAttributeName(sort_attr_name);
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
                CallHistoryUtils::parseRecordList(&record_list, array);
            }

            contacts_list_destroy(record_list, true);
            contacts_query_destroy(query);
            contacts_filter_destroy(filter);

            ret = contacts_disconnect_on_thread();
            if (CONTACTS_ERROR_NONE != ret) {
                LoggerW("contacts_disconnect_on_thread failed");
            }

            ReportSuccess(response->get<picojson::object>());
        } catch (const PlatformException& err) {
            contacts_list_destroy(record_list, true);
            contacts_query_destroy(query);
            contacts_filter_destroy(filter);
            ReportError(err, response->get<picojson::object>());
        }
    };

    auto find_response = [callback_id](const std::shared_ptr<picojson::value>& response) -> void {
        picojson::object& obj = response->get<picojson::object>();
        obj.insert(std::make_pair("callbackId", callback_id));
        CallHistoryInstance::getInstance().PostMessage(response->serialize().c_str());
    };

    TaskQueue::GetInstance().Queue<picojson::value>(
            find,
            find_response,
            std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));
}

void CallHistory::remove(const picojson::object& args)
{
    LoggerD("Entered");

    const auto it_uid = args.find("uid");
    const auto it_args_end = args.end();

    if (it_uid == it_args_end ||
        !it_uid->second.is<std::string>()) {
        LoggerE("Invalid parameter was passed.");
        throw InvalidValuesException("Invalid parameter was passed.");
    }

    int uid = atoi((it_uid->second.get<std::string>()).c_str());
    int ret = contacts_db_delete_record(_contacts_phone_log._uri, (int)uid);
    if (CONTACTS_ERROR_NONE != ret) {
        LoggerE("Failed to delete log record [%d] with error: %d", uid, ret);
        throw UnknownException("Failed to delete log record.");
    }
}

void CallHistory::removeBatch(const picojson::object& args)
{
    LoggerD("Entered");

    const auto it_uid = args.find("uid");
    const auto it_args_end = args.end();

    if (it_uid == it_args_end ||
        !it_uid->second.is<picojson::array>()) {
        throw InvalidValuesException("Invalid parameter was passed.");
    }
    const picojson::array& uids = it_uid->second.get<picojson::array>();
    const double callback_id = args.find("callbackId")->second.get<double>();

    auto remove_batch = [uids](const std::shared_ptr<picojson::value>& response) -> void {
        try {
            if (uids.size() == 0) {
                throw UnknownException("Object is null.");
            }

            int uid;
            int ret = CONTACTS_ERROR_NONE;
            for (unsigned int i = 0; i < uids.size(); ++i) {
                uid = atoi(uids[i].get<std::string>().c_str());
                ret = contacts_db_delete_record(_contacts_phone_log._uri, (int)uid);
                if (CONTACTS_ERROR_NONE != ret) {
                    LoggerE("Failed to delete log [%d] with code %d", uid, ret);
                    throw UnknownException("contacts_db_delete_record failed");
                }
            }
            ReportSuccess(response->get<picojson::object>());
        } catch (const PlatformException& err) {
            ReportError(err, response->get<picojson::object>());
        }
    };

    auto remove_batch_response = [callback_id](const std::shared_ptr<picojson::value>& response) -> void {
        picojson::object& obj = response->get<picojson::object>();
        obj.insert(std::make_pair("callbackId", callback_id));
        CallHistoryInstance::getInstance().PostMessage(response->serialize().c_str());
    };

    TaskQueue::GetInstance().Queue<picojson::value>(
            remove_batch,
            remove_batch_response,
            std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));
}

void CallHistory::removeAll(const picojson::object& args)
{
    LoggerD("Entered");

    const double callback_id = args.find("callbackId")->second.get<double>();

    auto remove_all = [](const std::shared_ptr<picojson::value>& response) -> void {
        contacts_list_h record_list = NULL;
        int* list = NULL;

        try {
            contacts_record_h record = NULL;
            int ret = CONTACTS_ERROR_NONE;
            int total = 0;
            int value;
            unsigned int cnt = 0;

            ret = contacts_connect_on_thread();
            if (CONTACTS_ERROR_NONE != ret) {
                LoggerW("contacts_connect_on_thread failed");
            }

            ret = contacts_db_get_all_records(_contacts_phone_log._uri, 0, 0, &record_list);
            if (CONTACTS_ERROR_NONE != ret || !record_list) {
                LoggerE("Failed to get all records list");
                throw UnknownException("Failed to get all records list");
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
                        throw UnknownException("contacts_list_next function failed");
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
                    throw UnknownException("contacts_list_next function failed");
                }
            }

            if (cnt > 0) {
                ret = contacts_db_delete_records(_contacts_phone_log._uri, list, cnt);
                if (CONTACTS_ERROR_NONE != ret) {
                    LoggerE("contacts_db_delete_records function failed");
                    throw UnknownException("contacts_db_delete_records function failed");
                }
            }

            delete[] list;
            ret = contacts_list_destroy(record_list, true);
            if (CONTACTS_ERROR_NONE != ret) {
                LoggerW("contacts_list_destroy failed");
            }

            ret = contacts_disconnect_on_thread();
            if (CONTACTS_ERROR_NONE != ret) {
                LoggerW("contacts_disconnect_on_thread failed");
            }

            ReportSuccess(response->get<picojson::object>());
        } catch (const PlatformException& err) {
            contacts_list_destroy(record_list, true);
            delete[] list;
            ReportError(err, response->get<picojson::object>());
        }
    };

    auto remove_all_response = [callback_id](const std::shared_ptr<picojson::value>& response) -> void {
        picojson::object& obj = response->get<picojson::object>();
        obj.insert(std::make_pair("callbackId", callback_id));
        CallHistoryInstance::getInstance().PostMessage(response->serialize().c_str());
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

void CallHistory::changeListenerCB(const char* view_uri, char *changes, void* user_data)
{
    LoggerD("Entered");

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
    char* token_id = NULL;
    int change_type = 0;
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

    token_type = strtok(changes, seps);
    while (NULL != token_type) {
        token_id = strtok(NULL, seps);
        change_type = atoi((const char*)token_type);

        if (NULL != token_id) {
            change_id = atoi((const char*)token_id);
        } else {
            LoggerD("There is no (more) changed Item : %s", token_id);
            break;
        }

        contacts_query_h query = NULL;
        contacts_filter_h filter = NULL;
        contacts_list_h record_list = NULL;
        int ret = CONTACTS_ERROR_NONE;

        contacts_query_create(_contacts_phone_log._uri, &query);
        contacts_filter_create(_contacts_phone_log._uri, &filter);
        contacts_filter_add_int(filter, _contacts_phone_log.id, CONTACTS_MATCH_EQUAL, change_id);

        contacts_query_set_filter(query, filter);
        ret = contacts_query_set_sort(query, _contacts_phone_log.id, false);
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
            CallHistoryUtils::parseRecordList(&record_list, added_array);
        } else if (CONTACTS_CHANGE_UPDATED == change_type) {
            CallHistoryUtils::parseRecordList(&record_list, changed_array);
        } else if (CONTACTS_CHANGE_DELETED == change_type) {
            removed_array.push_back(picojson::value(token_id));
        }

        contacts_list_destroy(record_list, true);
        contacts_query_destroy(query);
        contacts_filter_destroy(filter);

        token_type = strtok( NULL, seps);
    }

    if (added_array.size() > 0) {
        added_obj[STR_ACTION] = picojson::value("onadded");
        CallHistoryInstance::getInstance().CallHistoryChange(added_obj);
    }
    if (changed_array.size() > 0) {
        changed_obj[STR_ACTION] = picojson::value("onchanged");
        CallHistoryInstance::getInstance().CallHistoryChange(changed_obj);
    }
    if (removed_array.size() > 0) {
        removed_obj[STR_ACTION] = picojson::value("onremoved");
        CallHistoryInstance::getInstance().CallHistoryChange(removed_obj);
    }
}

void CallHistory::startCallHistoryChangeListener()
{
    LoggerD("Entered");

    if (!m_is_listener_set) {
        int ret = contacts_db_add_changed_cb_with_info(_contacts_phone_log._uri,
                changeListenerCB, NULL);

        if (CONTACTS_ERROR_NONE != ret) {
            LoggerE("Failed to add ChangeListener");
            throw UnknownException("Failed to add ChangeListener");
        }
    }

    m_is_listener_set = true;
}

void CallHistory::stopCallHistoryChangeListener()
{
    LoggerD("Entered");
    if (m_is_listener_set) {
        int ret = contacts_db_remove_changed_cb_with_info(_contacts_phone_log._uri,
                changeListenerCB, NULL);

        if (CONTACTS_ERROR_NONE != ret) {
            LoggerE("Failed to remove ChangeListener");
            throw UnknownException("Failed to remove ChangeListener");
        }
    }

    m_is_listener_set = false;
}

void CallHistory::loadPhoneNumbers()
{
    LoggerD("Entered");

    char **cp_list = NULL;
    cp_list = tel_get_cp_name_list();

    if (!cp_list) {
        LoggerE("Failed to get cp name list.");
        return;
    }

    unsigned int modem_num = 0;
    while (cp_list[modem_num]) {
        std::string n = "";
        TapiHandle* handle;
        do {
            std::promise<std::string> prom;
            handle = tel_init(cp_list[modem_num]);
            if (!handle) {
                LoggerE("Failed to init tapi handle.");
                break;
            }

            int card_changed;
            TelSimCardStatus_t card_status;
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

        m_phone_numbers.push_back(n);
        tel_deinit(handle);
        modem_num++;
    }

    g_strfreev(cp_list);
}

} // namespace callhistory
} // namespace extension
