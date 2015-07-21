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

#include <sstream>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <msg_storage.h>
#include <email-api.h>

#include "common/logger.h"
#include "common/platform_exception.h"

#include "conversation_callback_data.h"
#include "messaging_database_manager.h"
#include "messaging_manager.h"

using namespace common;
using namespace extension::tizen;

namespace extension {
namespace messaging {

AttributeInfo::AttributeInfo() :
        sql_name(),
        sql_type(UNDEFINED_TYPE),
        any_type(PrimitiveType_NoType)
{
    LoggerD("Entered");
}

AttributeInfo::AttributeInfo(const std::string& in_sql_name,
        const SQLAttributeType in_sql_type,
        const tizen::PrimitiveType in_any_type) :
        sql_name(in_sql_name),
        sql_type(in_sql_type),
        any_type(in_any_type)
{
    LoggerD("Entered");
}

AttributeInfo::AttributeInfo(const AttributeInfo& other) :
        sql_name(other.sql_name),
        sql_type(other.sql_type),
        any_type(other.any_type)
{
    LoggerD("Entered");
}

AttributeInfo& AttributeInfo::operator=(const AttributeInfo& other)
{
    sql_name = other.any_type;
    sql_type = other.sql_type;
    any_type = other.any_type;
    return *this;
}

MessagingDatabaseManager::MessagingDatabaseManager()
{
    LoggerD("Entered");
// Attributes map for short messages ==========================================
    m_msg_attr_map.insert(std::make_pair("id",
            AttributeInfo("A.MSG_ID", INTEGER, PrimitiveType_String)));
    m_msg_attr_map.insert(std::make_pair("serviceId",
            AttributeInfo("A.MAIN_TYPE", INTEGER, PrimitiveType_String)));
    m_msg_attr_map.insert(std::make_pair("folderId",
            AttributeInfo("A.FOLDER_ID", INTEGER, PrimitiveType_String)));
    m_msg_attr_map.insert(std::make_pair("type",
            AttributeInfo("A.MAIN_TYPE", INTEGER, PrimitiveType_String)));
    m_msg_attr_map.insert(std::make_pair("timestamp",
            AttributeInfo("A.DISPLAY_TIME", DATETIME, PrimitiveType_Time)));
    m_msg_attr_map.insert(std::make_pair("from",
            AttributeInfo("B.ADDRESS_VAL", TEXT, PrimitiveType_String)));
    m_msg_attr_map.insert(std::make_pair("to",
            AttributeInfo("B.ADDRESS_VAL", TEXT, PrimitiveType_String)));
    m_msg_attr_map.insert(std::make_pair("body.plainBody",
            AttributeInfo("A.MSG_TEXT", TEXT, PrimitiveType_String)));
    m_msg_attr_map.insert(std::make_pair("isRead",
            AttributeInfo("A.READ_STATUS", INTEGER, PrimitiveType_Boolean)));
    m_msg_attr_map.insert(std::make_pair("hasAttachment",
            AttributeInfo("A.ATTACHMENT_COUNT", INTEGER, PrimitiveType_Boolean)));
    m_msg_attr_map.insert(std::make_pair("isHighPriority",
            AttributeInfo("A.PRIORITY", INTEGER, PrimitiveType_Boolean)));
    m_msg_attr_map.insert(std::make_pair("subject",
            AttributeInfo("A.SUBJECT", TEXT, PrimitiveType_String)));
    m_msg_attr_map.insert(std::make_pair("direction",
            AttributeInfo("A.MSG_DIRECTION", INTEGER, PrimitiveType_String)));

// Attributes map for emails ==================================================
    m_email_attr_map.insert(std::make_pair("id",
            AttributeInfo("mail_id", INTEGER, PrimitiveType_String)));
    m_email_attr_map.insert(std::make_pair("serviceId",
            AttributeInfo("account_id", INTEGER, PrimitiveType_String)));
    m_email_attr_map.insert(std::make_pair("folderId",
            AttributeInfo("mailbox_id", INTEGER, PrimitiveType_String)));
    m_email_attr_map.insert(std::make_pair("type",
            AttributeInfo("account_id", INTEGER, PrimitiveType_String)));
    m_email_attr_map.insert(std::make_pair("timestamp",
            AttributeInfo("date_time", DATETIME, PrimitiveType_Time)));
    m_email_attr_map.insert(std::make_pair("from",
            AttributeInfo("full_address_from", TEXT, PrimitiveType_String)));
    m_email_attr_map.insert(std::make_pair("to",
            AttributeInfo("full_address_to", TEXT, PrimitiveType_String)));
    m_email_attr_map.insert(std::make_pair("cc",
            AttributeInfo("full_address_cc", TEXT, PrimitiveType_String)));
    m_email_attr_map.insert(std::make_pair("bcc",
            AttributeInfo("full_address_bcc", TEXT, PrimitiveType_String)));
    m_email_attr_map.insert(std::make_pair("body.plainBody",
            AttributeInfo("preview_text", TEXT, PrimitiveType_String)));
    m_email_attr_map.insert(std::make_pair("isRead",
            AttributeInfo("flags_seen_field", BOOLEAN, PrimitiveType_Boolean)));
    m_email_attr_map.insert(std::make_pair("hasAttachment",
            AttributeInfo("attachment_count", INTEGER, PrimitiveType_Boolean)));
    m_email_attr_map.insert(std::make_pair("isHighPriority",
            AttributeInfo("priority", INTEGER, PrimitiveType_Boolean)));
    m_email_attr_map.insert(std::make_pair("subject",
            AttributeInfo("subject", TEXT, PrimitiveType_String)));

// Attributes map for short message conversations =============================
    m_msg_conv_attr_map.insert(std::make_pair("id",
            AttributeInfo("A.CONV_ID", INTEGER, PrimitiveType_String)));
    m_msg_conv_attr_map.insert(std::make_pair("type",
            AttributeInfo("B.MAIN_TYPE", INTEGER, PrimitiveType_String)));
    m_msg_conv_attr_map.insert(std::make_pair("timestamp",
            AttributeInfo("A.DISPLAY_TIME", DATETIME, PrimitiveType_Time)));
    m_msg_conv_attr_map.insert(std::make_pair("messageCount",
            AttributeInfo("(A.SMS_CNT + A.MMS_CNT)", INTEGER, PrimitiveType_ULong)));
    m_msg_conv_attr_map.insert(std::make_pair("unreadMessages",
            AttributeInfo("A.UNREAD_CNT", INTEGER, PrimitiveType_ULong)));
    m_msg_conv_attr_map.insert(std::make_pair("preview",
            AttributeInfo("A.MSG_TEXT", TEXT, PrimitiveType_String)));
    m_msg_conv_attr_map.insert(std::make_pair("from",
            AttributeInfo("C.ADDRESS_VAL", TEXT, PrimitiveType_String)));
    m_msg_conv_attr_map.insert(std::make_pair("to",
            AttributeInfo("C.ADDRESS_VAL", TEXT, PrimitiveType_String)));
    m_msg_conv_attr_map.insert(std::make_pair("msgId",
            AttributeInfo("B.MSG_ID", INTEGER, PrimitiveType_String)));
    m_msg_conv_attr_map.insert(std::make_pair("direction",
            AttributeInfo("B.MSG_DIRECTION", INTEGER, PrimitiveType_String)));

// Attributes map for email conversations =====================================
    m_email_conv_attr_map.insert(std::make_pair("id",
            AttributeInfo("thread_id", INTEGER, PrimitiveType_String)));
    m_email_conv_attr_map.insert(std::make_pair("serviceId",
            AttributeInfo("account_id", INTEGER, PrimitiveType_String)));
    m_email_conv_attr_map.insert(std::make_pair("type",
            AttributeInfo("account_id", INTEGER, PrimitiveType_String)));
    m_email_conv_attr_map.insert(std::make_pair("timestamp",
            AttributeInfo("date_time", DATETIME, PrimitiveType_Time)));
    m_email_conv_attr_map.insert(std::make_pair("messageCount",
            AttributeInfo("thread_item_count", INTEGER, PrimitiveType_ULong)));
    m_email_conv_attr_map.insert(std::make_pair("unreadMessages",
            AttributeInfo(std::string("thread_id IN (SELECT thread_id ")
                    + std::string("FROM mail_tbl WHERE flags_seen_field = 0 ")
                    + std::string("GROUP BY thread_id HAVING COUNT(thread_id)"),
                    INTEGER,
                    PrimitiveType_ULong)));
    m_email_conv_attr_map.insert(std::make_pair("preview",
            AttributeInfo("preview_text", TEXT, PrimitiveType_String)));
    m_email_conv_attr_map.insert(std::make_pair("subject",
            AttributeInfo("subject", TEXT, PrimitiveType_String)));
    m_email_conv_attr_map.insert(std::make_pair("from",
            AttributeInfo("full_address_from", TEXT, PrimitiveType_String)));
    m_email_conv_attr_map.insert(std::make_pair("to",
            AttributeInfo("full_address_to", TEXT, PrimitiveType_String)));
}

MessagingDatabaseManager::~MessagingDatabaseManager()
{
    LoggerD("Entered");
}

MessagingDatabaseManager& MessagingDatabaseManager::getInstance()
{
    LoggerD("Entered");
    static MessagingDatabaseManager instance;
    return instance;
}

__thread sqlite3* sqlHandle = NULL;
__thread sqlite3_stmt* stmt = NULL;

msg_error_t MessagingDatabaseManager::connect()
{
    LoggerD("Entered");
    if (NULL == sqlHandle) {
        char strDBName[64];

        memset(strDBName, 0x00, sizeof(strDBName));
        snprintf(strDBName, sizeof(strDBName), "%s", MSG_DB_NAME);

        int err = db_util_open(strDBName, &sqlHandle, DB_UTIL_REGISTER_HOOK_METHOD);

        if (SQLITE_OK != err) {
            LoggerE("DB connecting fail [%d]", err);
            return MSG_ERR_DB_CONNECT;
        }

        LoggerD("DB connecting success: [%d]", sqlHandle);
    } else {
        LoggerD("DB connection exists: [%d]", sqlHandle);
    }

    return MSG_SUCCESS;
}

msg_error_t MessagingDatabaseManager::disconnect()
{
    LoggerD("Entered");
    msg_error_t err = 0;
    if (NULL != sqlHandle) {
        err = db_util_close(sqlHandle);

        if (SQLITE_OK != err) {
            LoggerE("DB disconnecting fail [%d]", err);
            return MSG_ERR_DB_DISCONNECT;
        }

        sqlHandle = NULL;
        LoggerD("DB disconnecting success");
    }

    return MSG_SUCCESS;
}

msg_error_t MessagingDatabaseManager::getTable(std::string sqlQuery,
        char*** results,
        int* resultsCount)
{
    LoggerD("Entered");
    msg_error_t err = 0;
    *resultsCount = 0;

    freeTable(results);
    connect();


    char* error_msg = NULL;
    err = sqlite3_get_table(sqlHandle, sqlQuery.c_str(), results,
            resultsCount, 0, &error_msg);

    if (SQLITE_OK != err) {
        LoggerE("Getting table fail [%d] error_msg:%s querry was:%s", err, error_msg,
                sqlQuery.c_str());
        freeTable(results);
        return MSG_ERR_DB_GETTABLE;
    }

    LoggerD("Getting table success");
    if (0 == *resultsCount) {
        LoggerD("No results");
    }

    disconnect();
    return MSG_SUCCESS;
}

void MessagingDatabaseManager::freeTable(char*** results)
{
    LoggerD("Entered");
    if (*results) {
        sqlite3_free_table(*results);
        *results = NULL;
    }
}

int MessagingDatabaseManager::cellToInt(char** array, int cellId)
{
    LoggerD("Entered");
    if (NULL == array) {
        LoggerD("Array is NULL");
        return 0;
    }

    char* tmp = *(array + cellId);
    if (NULL == tmp) {
        LoggerD("Cell is NULL");
        return 0;
    }

    return static_cast<int>(strtol(tmp, (char**) NULL, 10));
}

std::string MessagingDatabaseManager::getMatchString(tizen::AnyPtr match_value,
        const PrimitiveType type) const
{
    LoggerD("Entered");
    if(!match_value) {
        LoggerD("Warning: match value is NULL");
        return std::string();
    }

    std::ostringstream converter;
    switch(type) {
        case PrimitiveType_NoType: {
            LoggerD("Warning: match value is no type");
            return std::string();
        }
        case PrimitiveType_Null: {
            LoggerD("Warning: match value is null");
            return std::string();
        }
        case PrimitiveType_Boolean: {
            converter << match_value->toBool();
            return converter.str();
        }
        case PrimitiveType_Long: {
            converter << match_value->toLong();
            return converter.str();
        }
        case PrimitiveType_ULong: {
            converter << match_value->toULong();
            return converter.str();
        }
        case PrimitiveType_LongLong: {
            converter << match_value->toLongLong();
            return converter.str();
        }
        case PrimitiveType_ULongLong: {
            converter << match_value->toULongLong();
            return converter.str();
        }
        case PrimitiveType_Double: {
            converter << match_value->toDouble();
            return converter.str();
        }
        case PrimitiveType_String: {
            return match_value->toString();
        }
        case PrimitiveType_Time: {
            converter << match_value->toTimeT();
            return converter.str();
        }
        default: {
            LoggerD("Warning: match value is not specified");
            return std::string();
        }
    }
}

PlatformResult MessagingDatabaseManager::getAttributeFilterQuery(AbstractFilterPtr filter,
        AttributeInfoMap& attribute_map, MessageType msgType, std::string* result)
{
    LoggerD("Entered");

    std::ostringstream sqlQuery;
    AttributeFilterPtr attr_filter = castToAttributeFilter(filter);
    if(!attr_filter) {
        LoggerE("passed filter is not valid AttributeFilter!");
        return PlatformResult(ErrorCode::UNKNOWN_ERR,
                              "Wrong filter type - not attribute filter");
    }

    const std::string attribute_name = attr_filter->getAttributeName();
    AttributeInfoMap::iterator it = attribute_map.find(attribute_name);

    if (it != attribute_map.end()) {
        sqlQuery << "(" << attribute_map[attribute_name].sql_name << " ";
    } else {
        LoggerE("The attribute: %s does not exist.", attribute_name.c_str());
        return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                              "The attribute does not exist.");
    }

    AnyPtr match_value_any_ptr = attr_filter->getMatchValue();
    const AttributeInfo& attr_info = it->second;
    std::string match_value = getMatchString(match_value_any_ptr, attr_info.any_type);
    const FilterMatchFlag match_flag = attr_filter->getMatchFlag();

    LoggerD("match_value_any_ptr:%p any_type:%d attr_name:%s match_value:%s",
            match_value_any_ptr.get(), attr_info.any_type, attribute_name.c_str(),
            match_value.c_str());

    if ("serviceId" == attribute_name) {
        LoggerD("Attribute name: serviceId");

        int i_matchValue;
        std::istringstream iss(match_value);
        iss >> i_matchValue;

        switch(i_matchValue) {
        case MessageServiceAccountId::SMS_ACCOUNT_ID: {
            sqlQuery << "= " << MessageType::SMS;
            break;
        }
        case MessageServiceAccountId::MMS_ACCOUNT_ID: {
            sqlQuery << "= " << MessageType::MMS;
            break;
        }
        default:
            sqlQuery << "= " << match_value;
        }
    }
    else if ("type" == attribute_name) {
        LoggerD("Attribute name: type");
        if ("messaging.sms" == match_value && MessageType::SMS == msgType) {
            sqlQuery << "= " << msgType;
        } else if ("messaging.mms" == match_value && MessageType::MMS == msgType) {
            sqlQuery << "= " << msgType;
        } else if ("messaging.email" == match_value && MessageType::EMAIL == msgType) {
            sqlQuery << "= " << attr_info.sql_name;
        } else {
            LoggerE("attribute \"type\" matchValue:%s "
                    "does not match messaging.sms/mms/email\n"
                    "msgType:%d does not match SMS(%d), MMS(%d) nor EMAIL(%d)!",
                    match_value.c_str(), msgType, MessageType::SMS, MessageType::MMS,
                    MessageType::EMAIL);
            return PlatformResult(ErrorCode::UNKNOWN_ERR,
                                  "The value does not match service type.");
        }
    }
    else if ("isRead" == attribute_name || "hasAttachment" == attribute_name) {
        LoggerD("Attribute name: isRead or hasAttachment");
        if (attr_filter->getMatchValue()->toBool()) {
            sqlQuery << "> 0";
        } else {
            sqlQuery << "= 0";
        }
    }
    else if ("isHighPriority" == attribute_name) {
        LoggerD("Attribute name: isHighPriority");
        if (attr_filter->getMatchValue()->toBool()) {
            sqlQuery << "= ";
        } else {
            sqlQuery << "<> ";
        }

        if (MessageType::SMS == msgType || MessageType::MMS == msgType) {
            sqlQuery << MSG_MESSAGE_PRIORITY_HIGH;
        } else if (MessageType::EMAIL == msgType) {
            sqlQuery << EMAIL_MAIL_PRIORITY_HIGH;
        }
    }
    else {
        LoggerD("Another attribute");
        // Addresses which are stored in database can have different form than in filters
        if (MessageType::EMAIL == msgType && ("from" == attribute_name ||
                "to" == attribute_name || "cc" == attribute_name ||
                "bcc" == attribute_name)) {
            std::size_t foundPos;
            while ((foundPos = match_value.find('<')) != std::string::npos) {
                match_value.erase(foundPos, 1);
            }

            while ((foundPos = match_value.find('>')) != std::string::npos) {
                match_value.erase(foundPos, 1);
            }

            if (EXACTLY == match_flag) {
                match_value = "%<" + match_value + ">%";
            } else if (CONTAINS == match_flag) {
                match_value = "%<%" + match_value + "%>%";
            } else if (STARTSWITH == match_flag) {
                match_value = "%<" + match_value + "%>%";
            } else if (ENDSWITH == match_flag) {
                match_value = "%<%" + match_value + ">%";
            }
        }

        switch (match_flag) {
            /*
            case NONE: {
                // Determines if the apostrophes have to be added over match value
                if (TEXT == attribute_map[attribute_name].sql_type) {
                    sqlQuery << "NOT LIKE '" << match_value << "'";
                } else {
                    sqlQuery << "<> " << match_value;
                }
                break;
            }*/
            case EXACTLY: {
                // Determines if the apostrophes have to be added over match value
                if (TEXT == attribute_map[attribute_name].sql_type) {
                    sqlQuery << "LIKE '" << match_value << "'";
                } else {
                    sqlQuery << "= " << match_value;
                }
                break;
            }
            case CONTAINS: {
                sqlQuery << "LIKE '%" << match_value << "%'";
                break;
            }
            case STARTSWITH: {
                sqlQuery << "LIKE '" << match_value << "%'";
                break;
            }
            case ENDSWITH: {
                sqlQuery << "LIKE '%" << match_value << "'";
                break;
            }
            case EXISTS: {
                if ("unreadMessages" != attribute_name) {
                    sqlQuery << "IS NOT NULL";
                } else {
                    sqlQuery << "!= 0";
                }
                break;
            }
            default:
              return PlatformResult(ErrorCode::UNKNOWN_ERR,
                                    "The match flag is incorrect.");
        }

        if (MessageType::SMS == msgType || MessageType::MMS == msgType) {
            if ("from" == attribute_name) {
                // "From" and "to" attributes require message direction value
                sqlQuery << " AND " << attribute_map["direction"].sql_name << " = 1";
            } else if ("to" == attribute_name) {
                sqlQuery << " AND " << attribute_map["direction"].sql_name << " <> 1";
            }
        } else if (MessageType::EMAIL == msgType) {
            if("unreadMessages" == attribute_name) {
                sqlQuery << ")";
            }
        }
    }
    sqlQuery << ") ";
    *result = sqlQuery.str();
    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult MessagingDatabaseManager::getAttributeRangeFilterQuery(AbstractFilterPtr filter,
        AttributeInfoMap& attribute_map, MessageType msg_type, std::string* result)
{
    LoggerD("Entered");

    std::ostringstream sql_query, converter;
    std::string initial_value, end_value;

    AttributeRangeFilterPtr attr_range_filter = castToAttributeRangeFilter(filter);
    if(!attr_range_filter) {
        LoggerE("passed filter is not valid AttributeRangeFilter!");
        return PlatformResult(ErrorCode::UNKNOWN_ERR,
                              "Wrong filter type - not attribute range filter");
    }

    converter << attr_range_filter->getInitialValue()->toTimeT();
    initial_value = converter.str();
    converter.str("");
    converter << attr_range_filter->getEndValue()->toTimeT();
    end_value = converter.str();

    sql_query << "(" << attribute_map[attr_range_filter->getAttributeName()].sql_name << " ";
    sql_query << "BETWEEN " << initial_value << " AND " << end_value << ") ";
    *result = sql_query.str();
    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult MessagingDatabaseManager::getCompositeFilterQuery(AbstractFilterPtr filter,
        AttributeInfoMap& attribute_map, MessageType msg_type, std::string* result)
{
    LoggerD("Entered");
    std::ostringstream sql_query;

    CompositeFilterPtr comp_filter = castToCompositeFilter(filter);
    if(!comp_filter) {
        LoggerE("passed filter is not valid CompositeFilter!");
        return PlatformResult(ErrorCode::UNKNOWN_ERR,
                              "Wrong filter type - not composite filter");
    }

    AbstractFilterPtrVector filters_arr = comp_filter->getFilters();

    std::string logical_operator;
    if (UNION == comp_filter->getType()) {
        logical_operator = "OR ";
    } else {
        logical_operator = "AND ";
    }

    sql_query << "(";
    const unsigned int size = filters_arr.size();
    for (unsigned int i = 0; i < size; ++i) {

        const FilterType filter_type = filters_arr[i]->getFilterType();
        switch (filter_type) {
            case ATTRIBUTE_FILTER: {
              std::string query;
              PlatformResult ret = getAttributeFilterQuery(filters_arr[i], attribute_map, msg_type, &query);
              if (ret.IsError()) {
                  LoggerE("Get attribute filter query failed (%s)", ret.message().c_str());
                  return ret;
              }
              sql_query << query;
              break;
            }
            case ATTRIBUTE_RANGE_FILTER: {
              std::string query;
              PlatformResult ret = getAttributeRangeFilterQuery(filters_arr[i], attribute_map, msg_type, &query);
              if (ret.IsError()) {
                  LoggerE("Get attribute filter range filter query failed (%s)", ret.message().c_str());
                  return ret;
              }
              sql_query << query;
              break;
            }
            case COMPOSITE_FILTER: {
              std::string query;
              PlatformResult ret = getCompositeFilterQuery(filters_arr[i], attribute_map, msg_type, &query);
              if (ret.IsError()) {
                  LoggerE("Get composite filter query failed (%s)", ret.message().c_str());
                  return ret;
              }
              sql_query << query;
              break;
            }
            default:
                LoggerE("Error while querying message - unsupported filter type: %d",
                        filter_type);
                return PlatformResult(ErrorCode::UNKNOWN_ERR,
                                      "Error while querying message.");
        }

        if (i != (size - 1)) {
            sql_query << logical_operator;
        }
    }
    sql_query << ") ";

    *result = sql_query.str();
    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult MessagingDatabaseManager::addFilters(AbstractFilterPtr filter,
        SortModePtr sort_mode, long limit, long offset, AttributeInfoMap& attribute_map,
        MessageType msg_type, std::string* result)
{
    LoggerD("Entered");
    std::ostringstream sql_query;

    // Service type query
    if (MessageType::SMS == msg_type || MessageType::MMS == msg_type) {
        if (UNDEFINED != msg_type) {
            sql_query << attribute_map["type"].sql_name << " = " << msg_type << " AND ";
        } else {
            LoggerE("The service type is incorrect - msg_type is UNDEFINED");
            return PlatformResult(ErrorCode::UNKNOWN_ERR, "The service type is incorrect.");
        }
    }

    if(filter) {
        // Filter query
        switch (filter->getFilterType()) {
            case ATTRIBUTE_FILTER: {
              std::string query;
              PlatformResult ret = getAttributeFilterQuery(filter, attribute_map, msg_type, &query);
              if (ret.IsError()) {
                  LoggerE("Get attribute filter query failed (%s)", ret.message().c_str());
                  return ret;
              }
              sql_query << query;
              break;
            }
            case ATTRIBUTE_RANGE_FILTER: {
              std::string query;
              PlatformResult ret = getAttributeRangeFilterQuery(filter, attribute_map, msg_type, &query);
              if (ret.IsError()) {
                  LoggerE("Get attribute range filter query failed (%s)", ret.message().c_str());
                  return ret;
              }
              sql_query << query;
              break;
            }
            case COMPOSITE_FILTER : {
              std::string query;
              PlatformResult ret = getCompositeFilterQuery(filter, attribute_map, msg_type, &query);
              if (ret.IsError()) {
                  LoggerE("Get composite filter query failed (%s)", ret.message().c_str());
                  return ret;
              }
              sql_query << query;
              break;
            }
            default:
                LoggerE("The filter type is incorrect: %d", filter->getFilterType());
                return PlatformResult(ErrorCode::UNKNOWN_ERR, "The filter type is incorrect.");
        }
    }

    // SortMode query
    if (sort_mode) {
        if (attribute_map.find(sort_mode->getAttributeName()) != attribute_map.end()) {
            sql_query << "ORDER BY "
                    << attribute_map[sort_mode->getAttributeName()].sql_name << " ";
        } else {
            LoggerE("The attribute does not exist.");
            return PlatformResult(ErrorCode::UNKNOWN_ERR, "The attribute does not exist.");
        }

        if (ASC == sort_mode->getOrder()) {
            sql_query << "ASC ";
        } else {
            sql_query << "DESC ";
        }
    }

    // Limit query
    if (0 != limit) {
        sql_query << "LIMIT " << limit << " ";
    }

    // Offset query
    if (0 != offset) {
        if( 0 == limit ) {
            //Ugly fix proposed by mySQL team:
            //http://dev.mysql.com/doc/refman/5.0/en/select.html
            //To retrieve all rows from a certain offset up to the end of the result set,
            //you can use some large number for the second parameter.
            //
            //Reason: to use OFFSET you need to have LIMIT statement
            //18446744073709551615 is 2^64-1 - max value of big int
            //However we will use -1 since it will work fine for various int sizes (this
            //trick have been used in old implementation).

            sql_query << "LIMIT -1 ";
        }
        sql_query << "OFFSET " << offset << " ";
    }

    *result = sql_query.str();
    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult MessagingDatabaseManager::findShortMessages(
        FindMsgCallbackUserData* callback, std::vector<int>* result)
{
    LoggerD("Entered");
    std::ostringstream sqlQuery;
    int attributesCount = 1; // It has to be set manually each time when the query is changed
    int cellId = attributesCount;
    char** results = NULL;
    int resultsCount;
    std::vector<int> messagesIds;

    sqlQuery << "SELECT " << "DISTINCT(" << m_msg_attr_map["id"].sql_name << ") "
            << "FROM " << MSG_MESSAGE_TABLE_NAME << " A "
            << "JOIN " << MSG_ADDRESS_TABLE_NAME << " B "
            << "ON A.CONV_ID = B.CONV_ID " << "WHERE B.ADDRESS_ID <> 0 AND ";

    // Adding filters query
    AbstractFilterPtr filter = callback->getFilter();
    SortModePtr sortMode = callback->getSortMode();
    long limit = callback->getLimit();
    long offset = callback->getOffset();
    MessageType msgType = callback->getMessageServiceType();

    std::string filters_query;
    PlatformResult ret = addFilters(filter, sortMode, limit, offset, m_msg_attr_map,
                                    msgType, &filters_query);
    if (ret.IsError()) {
        LoggerE("Add filters failed (%s)", ret.message().c_str());
        return ret;
    }
    sqlQuery << filters_query;
    LoggerD("%s", sqlQuery.str().c_str());

    // Getting results from database
    msg_error_t err = getTable(sqlQuery.str(), &results, &resultsCount);
    if (MSG_SUCCESS != err) {
        LoggerE("Getting results from database failed [%d]", err);
        freeTable(&results);
        return PlatformResult(ErrorCode::UNKNOWN_ERR,
                              "Error while getting data from database.");
    }

    for (int i = 0; i < resultsCount; ++i) {
        messagesIds.push_back(cellToInt(results, cellId++));
        LoggerD("id: %d", messagesIds.at(messagesIds.size() - 1));
    }

    freeTable(&results);
    *result = messagesIds;
    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult MessagingDatabaseManager::findEmails(
        FindMsgCallbackUserData* callback, std::pair<int, email_mail_data_t*>* result)
{
    LoggerD("Entered");
    std::ostringstream sqlWhereClause;
    int resultsCount;
    email_mail_data_t* results;

    // Adding filters query
    AbstractFilterPtr filter = callback->getFilter();
    SortModePtr sortMode = callback->getSortMode();
    long limit = callback->getLimit();
    long offset = callback->getOffset();
    MessageType msgType = callback->getMessageServiceType();
    int accountId = callback->getAccountId();

    std::string filters_query;
    PlatformResult ret = addFilters(filter, sortMode, limit, offset,
                                    m_email_attr_map, msgType, &filters_query);
    if (ret.IsError()) {
        LoggerE("Add filters failed (%s)", ret.message().c_str());
        return ret;
    }
    sqlWhereClause << "WHERE "
            << m_email_attr_map["serviceId"].sql_name << " = " << accountId << " AND "
            << filters_query;
    LoggerD("%s", sqlWhereClause.str().c_str());

    // Getting results from database
    msg_error_t err = email_query_mails(const_cast<char*>(sqlWhereClause.str().c_str()),
                &results, &resultsCount);
    if (EMAIL_ERROR_NONE != err) {
        LoggerE("Getting mail list fail [%d]", err);

        if (EMAIL_ERROR_MAIL_NOT_FOUND == err) {
            resultsCount = 0;
        } else {
          return PlatformResult(ErrorCode::UNKNOWN_ERR,
                                "Error while getting data from database.");
        }
    }

    *result = std::make_pair(resultsCount, results);
    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult MessagingDatabaseManager::findShortMessageConversations(
        ConversationCallbackData* callback, std::vector<int>* result)
{
    LoggerD("Entered");
    std::ostringstream sqlQuery;
    int attributesCount = 1; // It has to be set manually each time when the query is changed
    int cellId = attributesCount;
    char** results = NULL;
    int resultsCount;
    std::vector<int> conversationsIds;

    sqlQuery << "SELECT " << "DISTINCT(" << m_msg_conv_attr_map["id"].sql_name << ") "
            << "FROM " << MSG_CONVERSATION_TABLE_NAME << " A "
            << "JOIN " << MSG_MESSAGE_TABLE_NAME << " B "
            << "ON A.CONV_ID = B.CONV_ID "
            << "JOIN " << MSG_ADDRESS_TABLE_NAME << " C "
            << "ON A.CONV_ID = C.CONV_ID "
            << "WHERE (A.SMS_CNT > 0 OR A.MMS_CNT > 0) AND ";

    // Adding filters query
    AbstractFilterPtr filter = callback->getFilter();
    SortModePtr sortMode = callback->getSortMode();
    long limit = callback->getLimit();
    long offset = callback->getOffset();
    MessageType msgType = callback->getMessageServiceType();

    std::string filters_query;
    PlatformResult ret = addFilters(filter, sortMode, limit, offset, m_msg_conv_attr_map,
                                    msgType, &filters_query);
    if (ret.IsError()) {
        LoggerE("Add filters failed (%s)", ret.message().c_str());
        return ret;
    }
    sqlQuery << filters_query;
    LoggerD("%s", sqlQuery.str().c_str());

    // Getting results from database
    msg_error_t err = getTable(sqlQuery.str(), &results, &resultsCount);
    if (MSG_SUCCESS != err) {
        LoggerE("Getting results from database failed [%d]", err);
        freeTable(&results);
        return PlatformResult(ErrorCode::UNKNOWN_ERR,
                              "Error while getting data from database.");
    }

    for (int i = 0; i < resultsCount; ++i) {
        conversationsIds.push_back(cellToInt(results, cellId++));
        LoggerD("id: %d", conversationsIds.at(conversationsIds.size() - 1));
    }

    freeTable(&results);
    *result = conversationsIds;
    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult MessagingDatabaseManager::findEmailConversations(
        ConversationCallbackData* callback, std::vector<EmailConversationInfo>* result)
{
    LoggerD("Entered");
    std::ostringstream sqlWhereClause;
    int resultsCount;
    email_mail_data_t* results;
    std::map<int, int> conversationsBag;
    std::vector<EmailConversationInfo> conversationsInfo;

    // Adding filters query
    AbstractFilterPtr filter = callback->getFilter();
    SortModePtr sortMode = callback->getSortMode();
    long limit = callback->getLimit();
    long offset = callback->getOffset();
    MessageType msgType = callback->getMessageServiceType();
    int accountId = callback->getAccountId();

    std::string filters_query;
    PlatformResult ret = addFilters(filter, sortMode, limit, offset, m_email_conv_attr_map,
                                    msgType, &filters_query);
    if (ret.IsError()) {
        LoggerE("Add filters failed (%s)", ret.message().c_str());
        return ret;
    }
    sqlWhereClause << "WHERE "
            << m_email_conv_attr_map["serviceId"].sql_name << " = " << accountId << " AND "
            << filters_query;
    LoggerD("%s", sqlWhereClause.str().c_str());

    // Getting results from database
    msg_error_t err = email_query_mails(const_cast<char*>(sqlWhereClause.str().c_str()),
                &results, &resultsCount);
    if (EMAIL_ERROR_NONE != err) {
        LoggerE("Getting mail list fail [%d]", err);

        if (EMAIL_ERROR_MAIL_NOT_FOUND == err) {
            resultsCount = 0;
        } else {
          return PlatformResult(ErrorCode::UNKNOWN_ERR,
                                "Error while getting data from database.");
        }
    }

    // Assigning found emails to conversation
    for (int i = 0; i < resultsCount; ++i) {
        if (conversationsBag.find(results[i].thread_id) == conversationsBag.end()) {
            EmailConversationInfo info;
            info.id = results[i].thread_id;
            conversationsInfo.push_back(info);
            conversationsBag.insert(std::make_pair(results[i].thread_id, 0));
        }

        if (!(static_cast<bool>(results[i].flags_seen_field))) {
            ++conversationsBag[results[i].thread_id];
        }
    }

    for (std::vector<EmailConversationInfo>::iterator it = conversationsInfo.begin();
            it != conversationsInfo.end(); ++it) {
        (*it).unreadMessages = conversationsBag[(*it).id];
    }

    email_free_mail_data(&results, resultsCount);
    *result = conversationsInfo;
    return PlatformResult(ErrorCode::NO_ERROR);
}

} // Messaging
} // DeviceAPI
