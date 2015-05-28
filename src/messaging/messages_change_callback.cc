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
 
//#include <JSWebAPIErrorFactory.h>
//#include <JSUtil.h>

#include "common/logger.h"
#include "common/platform_exception.h"
#include "messaging_instance.h"

#include "messages_change_callback.h"
//#include "JSMessage.h"
//#include "AbstractFilter.h"
//#include "MessagingUtil.h"

namespace extension {
namespace messaging {

namespace {

std::string limitedString(const std::string& src,
                          const size_t max_len = 40)
{
    LoggerD("Entered");
    if(src.length() > max_len) {
        return src.substr(0,max_len);
    } else {
        return src;
    }
}

} //Anonymous namespace

const char* MESSAGESADDED = "messagesadded";
const char* MESSAGESUPDATED = "messagesupdated";
const char* MESSAGESREMOVED = "messagesremoved";

MessagesChangeCallback::MessagesChangeCallback(
        long cid,
        int service_id,
        MessageType service_type,
        PostQueue& queue) :
        m_callback_data(cid, queue, true),
        m_service_id(service_id),
        m_msg_type(service_type),
        m_is_act(true)
{
    LoggerD("Entered");
}

MessagesChangeCallback::~MessagesChangeCallback()
{
    LoggerD("Entered");
}

MessagePtrVector MessagesChangeCallback::filterMessages(
        tizen::AbstractFilterPtr filter,
        const MessagePtrVector& source_messages,
        const int service_id)
{
    LoggerD("Entered sourceMessages.size():%d filter:%s", source_messages.size(),
            (filter ? "PRESENT" : "NULL"));

    if (filter) {
        MessagePtrVector filtered_messages;
        MessagePtrVector::const_iterator it = source_messages.begin();
        MessagePtrVector::const_iterator end_it = source_messages.end();

        for(int i = 0; it != end_it ; ++i, ++it) {
            const MessagePtr& message = *it;
            message->setServiceId(service_id);

            const bool matched = filter->isMatching(message.get());
            if(matched) {
                filtered_messages.push_back(message);
            }

            LoggerD("[%d] is Message(%p) {", i, message.get());
            LoggerD("[%d] messageId: %d", i, message->getId());
            LoggerD("[%d] message subject: %s", i, message->getSubject().c_str());
            LoggerD("[%d] from: %s", i, message->getFrom().c_str());

            if(message->getBody()) {
                const std::string& pBody = message->getBody()->getPlainBody();
                LoggerD("[%d] message plainBody: %s", i, limitedString(pBody).c_str());
            }

            LoggerD("[%d] matched filter: %s", i, matched ? "YES" : "NO");
            LoggerD("[%d] }");
        }

        LoggerD("returning matching %d of %d messages", filtered_messages.size(),
                source_messages.size());
        return filtered_messages;
    }
    else {
        LoggerD("Abstract filter pointer is null");
        return source_messages;
    }
}

void MessagesChangeCallback::added(const MessagePtrVector& msgs)
{
    LoggerD("Entered num messages: %d", msgs.size());
    if (!m_is_act) {
        return;
    }
    MessagePtrVector filtered_msgs = filterMessages(m_filter, msgs, m_service_id);
    //cancel callback only if filter did remove all messages
    //if callback was called with empty msgs list, call it
    if (msgs.size() > 0 && filtered_msgs.size() == 0) {
        LoggerD("All messages were filtered out, not calling callback");
        return;
    }

    picojson::array array;
    auto each = [&array] (std::shared_ptr<Message> m)->void {
        array.push_back(MessagingUtil::messageToJson(m));
    };

    for_each(filtered_msgs.begin(), filtered_msgs.end(), each);

    LoggerD("Calling:%s with:%d added messages", MESSAGESADDED,
        filtered_msgs.size());

    auto json = m_callback_data.getJson();
    picojson::object& obj = json->get<picojson::object>();
    if (json->contains(JSON_CALLBACK_ID) && obj.at(JSON_CALLBACK_ID).is<double>()) {
      obj[JSON_ACTION] = picojson::value(MESSAGESADDED);
      obj[JSON_DATA] = picojson::value(array);

      m_callback_data.getQueue().addAndResolve(obj.at(
          JSON_CALLBACK_ID).get<double>(), PostPriority::MEDIUM, json->serialize());
    } else {
      LoggerE("json is incorrect - missing required member");
    }
}

void MessagesChangeCallback::updated(const MessagePtrVector& msgs)
{
    LoggerD("Entered num messages: %d", msgs.size());
    if (!m_is_act) {
        return;
    }
    MessagePtrVector filtered_msgs = filterMessages(m_filter, msgs, m_service_id);
    //cancel callback only if filter did remove all messages
    //if callback was called with empty msgs list, call it
    if (msgs.size() > 0 && filtered_msgs.size() == 0) {
        LoggerD("All messages were filtered out, not calling callback");
        return;
    }

    picojson::array array;
    auto each = [&array] (std::shared_ptr<Message> m)->void {
        array.push_back(MessagingUtil::messageToJson(m));
    };

    for_each(filtered_msgs.begin(), filtered_msgs.end(), each);

    LoggerD("Calling:%s with:%d updated messages", MESSAGESUPDATED,
        filtered_msgs.size());

    auto json = m_callback_data.getJson();
    picojson::object& obj = json->get<picojson::object>();
    if (json->contains(JSON_CALLBACK_ID) && obj.at(JSON_CALLBACK_ID).is<double>()) {
      obj[JSON_ACTION] = picojson::value(MESSAGESUPDATED);
      obj[JSON_DATA] = picojson::value(array);

      m_callback_data.getQueue().addAndResolve(obj.at(
          JSON_CALLBACK_ID).get<double>(), PostPriority::LOW, json->serialize());
    } else {
      LoggerE("json is incorrect - missing required member");
    }
}

void MessagesChangeCallback::removed(const MessagePtrVector& msgs)
{
    LoggerD("Enter event: msgs.size() = %d", msgs.size());
    if (!m_is_act) {
        return;
    }

    MessagePtrVector filtered_msgs = filterMessages(m_filter, msgs, m_service_id);
    //cancel callback only if filter did remove all messages
    //if callback was called with empty msgs list, call it
    if (msgs.size() > 0 && filtered_msgs.size() == 0) {
        LoggerD("All messages were filtered out, not calling callback");
        return;
    }

    picojson::array array;
    auto each = [&array] (std::shared_ptr<Message> m)->void {
        array.push_back(MessagingUtil::messageToJson(m));
    };

    for_each(filtered_msgs.begin(), filtered_msgs.end(), each);

    LoggerD("Calling:%s with:%d removed messages", MESSAGESREMOVED,
        filtered_msgs.size());

    auto json = m_callback_data.getJson();
    picojson::object& obj = json->get<picojson::object>();
    if (json->contains(JSON_CALLBACK_ID) && obj.at(JSON_CALLBACK_ID).is<double>()) {
      obj[JSON_ACTION] = picojson::value(MESSAGESREMOVED);
      LoggerD("MESSAGES: %s", picojson::value(array).serialize().c_str());
      obj[JSON_DATA] = picojson::value(array);

      m_callback_data.getQueue().addAndResolve(obj.at(
          JSON_CALLBACK_ID).get<double>(), PostPriority::LAST, json->serialize());
    } else {
      LoggerE("json is incorrect - missing required member");
    }
}

void MessagesChangeCallback::setFilter(tizen::AbstractFilterPtr filter)
{
    m_filter = filter;
}

AbstractFilterPtr MessagesChangeCallback::getFilter() const
{
    return m_filter;
}

int MessagesChangeCallback::getServiceId() const
{
    return m_service_id;
}

MessageType MessagesChangeCallback::getServiceType() const
{
    return m_msg_type;
}

void MessagesChangeCallback::setActive(bool act) {
    m_is_act = act;
}

bool MessagesChangeCallback::isActive() {
    return m_is_act;
}

void MessagesChangeCallback::setItems(MessagePtrVector& items)
{
    m_items = items;
}
MessagePtrVector MessagesChangeCallback::getItems()
{
    return m_items;
}


} //namespace messaging
} //namespace extension
