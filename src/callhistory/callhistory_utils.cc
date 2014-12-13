// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "callhistory_utils.h"
#include "callhistory_types.h"
#include "callhistory.h"
#include "common/logger.h"

namespace extension {
namespace callhistory {

void CallHistoryUtils::parseRecordList(contacts_list_h *record_list, picojson::array& array)
{
    LoggerD("Entered");

    int ret = CONTACTS_ERROR_NONE;
    contacts_record_h record = NULL;
    int total = 0;

    ret = contacts_list_get_count(*record_list, &total);
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
    }
}

void CallHistoryUtils::parseRecord(contacts_record_h *record, picojson::object& obj)
{
    LoggerD("Entered");

    int ret = CONTACTS_ERROR_NONE;
    int int_data;

    ret = contacts_record_get_int(*record, _contacts_phone_log.id, &int_data);
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
        obj.insert(std::make_pair(STR_TAGS, features));
    }
}

void CallHistoryUtils::parseRemoteParties(contacts_record_h *record, picojson::object& obj)
{
    LoggerD("Entered");

    int ret = CONTACTS_ERROR_NONE;
    char * char_data = NULL;
    int int_data;

    picojson::array& remote_parties = obj.insert(std::make_pair(STR_REMOTE_PARTIES, picojson::value(
                                      picojson::array()))).first->second.get<picojson::array>();
    remote_parties.push_back(picojson::value(picojson::object()));
    picojson::object& parties_obj = remote_parties.back().get<picojson::object>();

    ret = contacts_record_get_int(*record, _contacts_phone_log.person_id, &int_data);
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

    int ret = CONTACTS_ERROR_NONE;
    const std::vector<std::string>& phone_numbers = CallHistory::getInstance()->getPhoneNumbers();
    int sim_count = phone_numbers.size();
    int sim_index;

    ret = contacts_record_get_int(*record, _contacts_phone_log.sim_slot_no, &sim_index);
    if (CONTACTS_ERROR_NONE != ret) {
        LoggerW("Failed to get sim slot no. %d", ret);
    }

    if (sim_index >= sim_count) {
        LoggerE("sim slot no. [%d] is out of count %d", sim_index, sim_count);
    } else if (sim_index > 0) {
        obj[STR_CALLING_PARTY] = picojson::value(phone_numbers.at(sim_index));
    }
}

} // namespace callhistory
} // namespace extension
