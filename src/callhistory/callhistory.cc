// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "callhistory.h"

#include <tapi_common.h>
#include <ITapiSim.h>

#include "common/logger.h"
#include "common/platform_exception.h"
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

    loadPhoneNumbers();
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

void CallHistory::find()
{

}

void CallHistory::remove()
{

}

void CallHistory::removeBatch()
{

}

void CallHistory::removeAll()
{

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
