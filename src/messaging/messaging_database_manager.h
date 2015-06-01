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

#ifndef __TIZEN_MSG_DATABASE_MANAGER_H__
#define __TIZEN_MSG_DATABASE_MANAGER_H__

#include <iostream>
#include <map>

#include <msg.h>
#include <db-util.h>
#include "MsgCommon/AbstractFilter.h"

#include "common/platform_result.h"
#include "find_msg_callback_user_data.h"
//#include "ConversationCallbackData.h"

namespace extension {
namespace messaging {

// =================================================================
#define MSG_DB_NAME                     "/opt/usr/dbspace/.msg_service.db"
#define MSG_MESSAGE_TABLE_NAME          "MSG_MESSAGE_TABLE"
#define MSG_FOLDER_TABLE_NAME           "MSG_FOLDER_TABLE"
#define MSG_ADDRESS_TABLE_NAME          "MSG_ADDRESS_TABLE"
#define MSG_CONVERSATION_TABLE_NAME     "MSG_CONVERSATION_TABLE"
#define MSG_SIM_MSG_TABLE_NAME          "MSG_SIM_TABLE"
#define MSG_FILTER_TABLE_NAME           "MSG_FILTER_TABLE"
#define MSG_PUSH_MSG_TABLE_NAME         "MSG_PUSH_TABLE"
#define MSG_CB_MSG_TABLE_NAME           "MSG_CBMSG_TABLE"
#define MMS_PLUGIN_MESSAGE_TABLE_NAME   "MSG_MMS_MESSAGE_TABLE"
#define MSG_SYNCML_MSG_TABLE_NAME       "MSG_SYNCML_TABLE"
#define MSG_SCHEDULED_MSG_TABLE_NAME    "MSG_SCHEDULED_TABLE"
#define MSG_SMS_SENDOPT_TABLE_NAME      "MSG_SMS_SENDOPT_TABLE"
// =================================================================
enum SQLAttributeType {
    UNDEFINED_TYPE,
    BOOLEAN,
    INTEGER,
    DATETIME,
    TEXT,
};

struct AttributeInfo {
    AttributeInfo();
    AttributeInfo(const std::string& in_sql_name,
            const SQLAttributeType in_sql_type,
            const tizen::PrimitiveType in_any_type);
    AttributeInfo(const AttributeInfo& other);
    AttributeInfo& operator=(const AttributeInfo& other);

    std::string sql_name;
    SQLAttributeType sql_type;
    tizen::PrimitiveType any_type;
};

typedef std::map<std::string, AttributeInfo> AttributeInfoMap;

struct EmailConversationInfo {
    int id, unreadMessages;
};
// =================================================================

class MessagingDatabaseManager {
public:
    static MessagingDatabaseManager& getInstance();
    common::PlatformResult findShortMessages(FindMsgCallbackUserData* callback, std::vector<int>* result);
    common::PlatformResult findEmails(FindMsgCallbackUserData* callback,
                                                  std::pair<int, email_mail_data_t*>* result);
    common::PlatformResult findShortMessageConversations(ConversationCallbackData* callback,
                                                         std::vector<int>* result);
    common::PlatformResult findEmailConversations(ConversationCallbackData* callback,
                                                  std::vector<EmailConversationInfo>* result);

private:
    MessagingDatabaseManager();
    MessagingDatabaseManager(const MessagingDatabaseManager &);
    void operator=(const MessagingDatabaseManager &);
    virtual ~MessagingDatabaseManager();

    msg_error_t connect();
    msg_error_t disconnect();
    msg_error_t getTable(std::string query, char*** results, int* resultsCount);
    void freeTable(char*** array);
    int cellToInt(char** array, int cellId);
    std::string getMatchString(tizen::AnyPtr matchValue,
            const tizen::PrimitiveType type) const;
    common::PlatformResult getAttributeFilterQuery(tizen::AbstractFilterPtr filter,
            AttributeInfoMap& attributeMap, MessageType msgType, std::string* result);
    common::PlatformResult getAttributeRangeFilterQuery(tizen::AbstractFilterPtr filter,
            AttributeInfoMap& attributeMap, MessageType msgType, std::string* result);
    common::PlatformResult getCompositeFilterQuery(tizen::AbstractFilterPtr filter,
            AttributeInfoMap& attributeMap, MessageType msgType, std::string* result);
    common::PlatformResult addFilters(tizen::AbstractFilterPtr filter,
                                      tizen::SortModePtr sortMode, long limit, long offset,
                                      AttributeInfoMap& attributeMap, MessageType msgType,
                                      std::string* result);

    AttributeInfoMap m_msg_attr_map;
    AttributeInfoMap m_email_attr_map;

    AttributeInfoMap m_msg_conv_attr_map;
    AttributeInfoMap m_email_conv_attr_map;
};

} // Messaging
} // DeviceAPI

#endif // __TIZEN_MSG_DATABASE_MANAGER_H__
