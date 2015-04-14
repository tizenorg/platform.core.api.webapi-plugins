// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "conversation_callback_data.h"
#include "messaging_util.h"

namespace extension {
namespace messaging {

ConversationCallbackData::ConversationCallbackData(PostQueue& queue):
        m_filter(),
        m_sort(),
        m_limit(0),
        m_offset(0),
        m_is_error(false),
        m_account_id(0),
        m_service_type(UNDEFINED),
        queue_(queue)
{
}

ConversationCallbackData::ConversationCallbackData(long cid, PostQueue& queue, bool keep):
        m_filter(),
        m_sort(),
        m_limit(0),
        m_offset(0),
        m_is_error(false),
        m_account_id(0),
        m_service_type(UNDEFINED),
        queue_(queue)
{
    auto json = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    picojson::object& o = json->get<picojson::object>();
    o[JSON_CALLBACK_ID] = picojson::value(static_cast<double>(cid));
    o[JSON_CALLBACK_KEEP] = picojson::value(keep);
    setJson(json);
}

ConversationCallbackData::~ConversationCallbackData()
{
}

void ConversationCallbackData::setFilter(AbstractFilterPtr filter)
{
    m_filter = filter;
}

void ConversationCallbackData::setSortMode(SortModePtr sortMode)
{
    m_sort = sortMode;
}

void ConversationCallbackData::setLimit(long limit)
{
    m_limit = limit;
}

void ConversationCallbackData::setOffset(long offset)
{
    m_offset = offset;
}

void ConversationCallbackData::addConversation(std::shared_ptr<MessageConversation> msg)
{
    m_conversations.push_back(msg);
}

std::vector<std::shared_ptr<MessageConversation>> ConversationCallbackData::getConversations() const
{
    return m_conversations;
}

void ConversationCallbackData::setError(const std::string& err_name,
        const std::string& err_message)
{
    // keep only first error in chain
    if (!m_is_error) {
        picojson::object& obj = m_json->get<picojson::object>();
        obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_ERROR);
        auto objData = picojson::object();

        objData[JSON_ERROR_NAME] = picojson::value(err_name);
        objData[JSON_ERROR_MESSAGE] = picojson::value(err_message);

        obj[JSON_DATA] = picojson::value(objData);

        m_is_error = true;
        m_err_name = err_name;
        m_err_message = err_message;
    }
}

void ConversationCallbackData::SetError(const common::PlatformResult& error)
{
  // keep only first error in chain
  if (!m_is_error) {
    m_is_error = true;
    picojson::object& obj = m_json->get<picojson::object>();
    obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_ERROR);
    auto obj_data = picojson::object();
    obj_data[JSON_ERROR_CODE] = picojson::value(static_cast<double>(error.error_code()));
    obj_data[JSON_ERROR_MESSAGE] = picojson::value(error.message());
    obj[JSON_DATA] = picojson::value(obj_data);
  }
}
bool ConversationCallbackData::isError() const
{
    return m_is_error;
}

std::string ConversationCallbackData::getErrorName() const
{
    return m_err_name;
}

std::string ConversationCallbackData::getErrorMessage() const
{
    return m_err_message;
}

void ConversationCallbackData::setAccountId(int account_id){
    m_account_id = account_id;
}

int ConversationCallbackData::getAccountId() const
{
    return m_account_id;
}

void ConversationCallbackData::setMessageServiceType(MessageType m_msg_type)
{
    m_service_type = m_msg_type;
}

MessageType ConversationCallbackData::getMessageServiceType() const
{
    return m_service_type;
}

AbstractFilterPtr ConversationCallbackData::getFilter() const
{
    return m_filter;
}

SortModePtr ConversationCallbackData::getSortMode() const
{
    return m_sort;
}

long ConversationCallbackData::getLimit() const
{
    return m_limit;
}

long ConversationCallbackData::getOffset() const
{
    return m_offset;
}

}//messaging
}//extension
