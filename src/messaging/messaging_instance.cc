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

#include "messaging_instance.h"

#include <sstream>
#include <stdexcept>

#include "common/logger.h"

#include "MsgCommon/AbstractFilter.h"
#include "messages_change_callback.h"
#include "conversations_change_callback.h"
#include "folders_change_callback.h"
#include "messages_callback_user_data.h"
#include "find_msg_callback_user_data.h"
#include "folders_callback_data.h"
#include "messaging_manager.h"
#include "messaging_util.h"
#include "message_storage.h"
#include "message.h"

using common::ErrorCode;
using common::PlatformResult;

namespace extension {
namespace messaging {

namespace{
const char* FUN_GET_MESSAGE_SERVICES = "Messaging_getMessageServices";
const char* GET_MESSAGE_SERVICES_ARGS_MESSAGE_SERVICE_TYPE = "messageServiceType";

const char* FUN_MESSAGE_SERVICE_SEND_MESSAGE =  "MessageService_sendMessage";
const char* SEND_MESSAGE_ARGS_MESSAGE = "message";
const char* SEND_MESSAGE_ARGS_SIMINDEX = "simIndex";

const char* FUN_MESSAGE_SERVICE_LOAD_MESSAGE_BODY = "MessageService_loadMessageBody";

const char* FUN_MESSAGE_SERVICE_LOAD_MESSAGE_ATTACHMENT = "MessageService_loadMessageAttachment";
const char* LOAD_MESSAGE_ATTACHMENT_ARGS_ATTACHMENT = "attachment";

const char* FUN_MESSAGE_SERVICE_SYNC = "MessageService_sync";
const char* SYNC_ARGS_ID = "id";
const char* SYNC_ARGS_LIMIT = "limit";

const char* FUN_MESSAGE_SERVICE_SYNC_FOLDER = "MessageService_syncFolder";
const char* SYNC_FOLDER_ARGS_ID = "id";
const char* SYNC_FOLDER_ARGS_FOLDER = "folder";
const char* SYNC_FOLDER_ARGS_LIMIT = "limit";

const char* FUN_MESSAGE_SERVICE_STOP_SYNC = "MessageService_stopSync";
const char* STOP_SYNC_ARGS_ID = "id";
const char* STOP_SYNC_ARGS_OPID = "opId";

const char* FUN_MESSAGE_STORAGE_ADD_DRAFT_MESSAGE = "MessageStorage_addDraftMessage";
const char* ADD_DRAFT_MESSAGE_ARGS_MESSAGE = "message";

const char* FUN_MESSAGE_STORAGE_FIND_MESSAGES = "MessageStorage_findMessages";

const char* FUN_MESSAGE_STORAGE_REMOVE_MESSAGES = "MessageStorage_removeMessages";
const char* REMOVE_MESSAGES_ARGS_MESSAGES = "messages";

const char* FUN_MESSAGE_STORAGE_UPDATE_MESSAGES = "MessageStorage_updateMessages";
const char* UPDATE_MESSAGES_ARGS_MESSAGES = "messages";

const char* FUN_MESSAGE_STORAGE_FIND_CONVERSATIONS = "MessageStorage_findConversations";
const char* FIND_CONVERSATIONS_ARGS_LIMIT = "limit";
const char* FIND_CONVERSATIONS_ARGS_OFFSET = "offset";

const char* FUN_MESSAGE_STORAGE_REMOVE_CONVERSATIONS = "MessageStorage_removeConversations";
const char* REMOVE_CONVERSATIONS_ARGS_CONVERSATIONS = "conversations";

const char* FUN_MESSAGE_STORAGE_FIND_FOLDERS = "MessageStorage_findFolders";
const char* FIND_FOLDERS_ARGS_LIMIT = "limit";
const char* FIND_FOLDERS_ARGS_OFFSET = "offset";

const char* FUN_MESSAGE_STORAGE_ADD_MESSAGES_CHANGE_LISTENER =
        "MessageStorage_addMessagesChangeListener";

const char* FUN_MESSAGE_STORAGE_ADD_CONVERSATIONS_CHANGE_LISTENER =
        "MessageStorage_addConversationsChangeListener";

const char* FUN_MESSAGE_STORAGE_ADD_FOLDER_CHANGE_LISTENER =
        "MessageStorage_addFoldersChangeListener";

const char* FUN_MESSAGE_STORAGE_REMOVE_CHANGE_LISTENER = "MessageStorage_removeChangeListener";
const char* REMOVE_CHANGE_LISTENER_ARGS_WATCHID = "watchId";

const char* FUNCTIONS_HIDDEN_ARGS_SERVICE_ID = "serviceId";

auto getServiceIdFromJSON = [](picojson::object& data) -> int {
    std::string serviceStrId;
    try {
        serviceStrId =
        MessagingUtil::getValueFromJSONObject<std::string>(data,FUNCTIONS_HIDDEN_ARGS_SERVICE_ID);
        return std::stoi(serviceStrId);
    }
    catch(...) {
        return -1;
    }
};

}

MessagingInstance::MessagingInstance():
    manager_(*this),
    queue_(*this)
{
    LoggerD("Entered");
    using std::placeholders::_1;
    using std::placeholders::_2;
    #define REGISTER_ASYNC(c,x) \
      RegisterHandler(c, std::bind(&MessagingInstance::x, this, _1, _2));
      REGISTER_ASYNC(FUN_GET_MESSAGE_SERVICES, GetMessageServices);
      REGISTER_ASYNC(FUN_MESSAGE_SERVICE_SEND_MESSAGE, MessageServiceSendMessage);
      REGISTER_ASYNC(FUN_MESSAGE_SERVICE_LOAD_MESSAGE_BODY, MessageServiceLoadMessageBody);
      REGISTER_ASYNC(FUN_MESSAGE_SERVICE_LOAD_MESSAGE_ATTACHMENT, MessageServiceLoadMessageAttachment);
      REGISTER_ASYNC(FUN_MESSAGE_STORAGE_ADD_DRAFT_MESSAGE, MessageStorageAddDraft);
      REGISTER_ASYNC(FUN_MESSAGE_STORAGE_FIND_MESSAGES, MessageStorageFindMessages);
      REGISTER_ASYNC(FUN_MESSAGE_STORAGE_REMOVE_MESSAGES, MessageStorageRemoveMessages);
      REGISTER_ASYNC(FUN_MESSAGE_STORAGE_UPDATE_MESSAGES, MessageStorageUpdateMessages);
      REGISTER_ASYNC(FUN_MESSAGE_STORAGE_FIND_CONVERSATIONS, MessageStorageFindConversations);
      REGISTER_ASYNC(FUN_MESSAGE_STORAGE_REMOVE_CONVERSATIONS, MessageStorageRemoveConversations);
      REGISTER_ASYNC(FUN_MESSAGE_STORAGE_FIND_FOLDERS, MessageStorageFindFolders);
    #undef REGISTER_ASYNC
    #define REGISTER_SYNC(c,x) \
      RegisterSyncHandler(c, std::bind(&MessagingInstance::x, this, _1, _2));
      REGISTER_SYNC(FUN_MESSAGE_SERVICE_SYNC, MessageServiceSync);
      REGISTER_SYNC(FUN_MESSAGE_SERVICE_STOP_SYNC, MessageServiceStopSync);
      REGISTER_SYNC(FUN_MESSAGE_SERVICE_SYNC_FOLDER, MessageServiceSyncFolder);
      REGISTER_SYNC(FUN_MESSAGE_STORAGE_ADD_MESSAGES_CHANGE_LISTENER, MessageStorageAddMessagesChangeListener);
      REGISTER_SYNC(FUN_MESSAGE_STORAGE_ADD_CONVERSATIONS_CHANGE_LISTENER, MessageStorageAddConversationsChangeListener);
      REGISTER_SYNC(FUN_MESSAGE_STORAGE_ADD_FOLDER_CHANGE_LISTENER, MessageStorageAddFolderChangeListener);
      REGISTER_SYNC(FUN_MESSAGE_STORAGE_REMOVE_CHANGE_LISTENER, MessageStorageRemoveChangeListener);
    #undef REGISTER_SYNC
}

MessagingInstance::~MessagingInstance()
{
    LoggerD("Entered");
}

#define POST_AND_RETURN(ret, json, obj, action) \
    LoggerE("Error occured: (%s)", ret.message().c_str()); \
    picojson::object args; \
    ReportError(ret, &args); \
    obj[JSON_DATA] = picojson::value(args); \
    obj[JSON_ACTION] = picojson::value(action); \
    queue_.addAndResolve( \
            obj.at(JSON_CALLBACK_ID).get<double>(), \
            PostPriority::HIGH, \
            json->serialize() \
    ); \
    return;

void MessagingInstance::GetMessageServices(const picojson::value& args,
                                           picojson::object& out)
{
  LoggerD("Entered");

  if (!args.contains(JSON_DATA) || !args.contains(JSON_CALLBACK_ID) ||
      !args.get(JSON_CALLBACK_ID).is<double>()) {
    LoggerE("json is incorrect - missing required member");
    return;
  }

  picojson::value data = args.get(JSON_DATA);
  picojson::value serviceTag = data.get<picojson::object>().
      at(GET_MESSAGE_SERVICES_ARGS_MESSAGE_SERVICE_TYPE);
  const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();
  // above values should be validated in js
  manager_.getMessageServices(serviceTag.to_str(), callbackId);
}

void MessagingInstance::MessageServiceSendMessage(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");

    if (!args.contains(JSON_DATA) || !args.contains(JSON_CALLBACK_ID) ||
        !args.get(JSON_CALLBACK_ID).is<double>()) {
      LoggerE("json is incorrect - missing required member");
      return;
    }

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::value v_message = data.at(SEND_MESSAGE_ARGS_MESSAGE);
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();

    auto json = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_CALLBACK_ID] = picojson::value(callbackId);

    std::shared_ptr<Message> message;
    PlatformResult ret = MessagingUtil::jsonToMessage(v_message, &message);
    if (ret.IsError()) {
      POST_AND_RETURN(ret, json, obj, JSON_CALLBACK_ERROR)
    }

    MessageRecipientsCallbackData* callback = new MessageRecipientsCallbackData(queue_);
    long simIndex = 0;
    int serviceId = 0;

    callback->setJson(json);
    callback->setMessage(message);
    serviceId = getServiceIdFromJSON(data);
    callback->setAccountId(serviceId);
    simIndex = static_cast<long>
      (MessagingUtil::getValueFromJSONObject<double>(data,SEND_MESSAGE_ARGS_SIMINDEX));

    if (!callback->setSimIndex(simIndex)) {
      delete callback;
      callback = nullptr;
      POST_AND_RETURN(PlatformResult(ErrorCode::UNKNOWN_ERR, "set sim index failed"),
                      json, obj, JSON_CALLBACK_ERROR)
    }

    queue_.add(static_cast<long>(callbackId), PostPriority::HIGH);
    auto service = manager_.getMessageService(serviceId);

    ret = service->sendMessage(callback);
    if (!ret) {
      POST_AND_RETURN(ret, json, obj, JSON_CALLBACK_ERROR)
    }
}

void MessagingInstance::MessageServiceLoadMessageBody(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");

    if (!args.contains(JSON_DATA) || !args.contains(JSON_CALLBACK_ID) ||
        !args.get(JSON_CALLBACK_ID).is<double>()) {
      LoggerE("json is incorrect - missing required member");
      return;
    }

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();

    picojson::value json_message = data.at(ADD_DRAFT_MESSAGE_ARGS_MESSAGE);
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();

    auto json = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_CALLBACK_ID] = picojson::value(callbackId);

    std::shared_ptr<Message> message;
    PlatformResult ret = MessagingUtil::jsonToMessage(json_message, &message);
    if (ret.IsError()) {
      POST_AND_RETURN(ret, json, obj, JSON_CALLBACK_ERROR)
    }

    MessageBodyCallbackData* callback = new MessageBodyCallbackData(queue_);

    callback->setJson(json);
    callback->setMessage(message);

    queue_.add(static_cast<long>(callbackId), PostPriority::HIGH);
    auto service = manager_.getMessageService(getServiceIdFromJSON(data));
    ret = service->loadMessageBody(callback);
    if (ret.IsError()) {
      POST_AND_RETURN(ret, json, obj, JSON_CALLBACK_ERROR)
    }
}

void MessagingInstance::MessageServiceLoadMessageAttachment(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");

    if (!args.contains(JSON_DATA) || !args.contains(JSON_CALLBACK_ID) ||
        !args.get(JSON_CALLBACK_ID).is<double>()) {
      LoggerE("json is incorrect - missing required member");
      return;
    }

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::value attachment = data.at(LOAD_MESSAGE_ATTACHMENT_ARGS_ATTACHMENT);
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();

    MessageAttachmentCallbackData* callback = new MessageAttachmentCallbackData(queue_);
    callback->setMessageAttachment(MessagingUtil::jsonToMessageAttachment(attachment));

    auto json = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_CALLBACK_ID] = picojson::value(callbackId);
    callback->setJson(json);

    queue_.add(static_cast<long>(callbackId), PostPriority::HIGH);
    auto service = manager_.getMessageService(getServiceIdFromJSON(data));
    const auto result = service->loadMessageAttachment(callback);
    if (result.IsError()) {
      POST_AND_RETURN(result, json, obj, JSON_CALLBACK_ERROR)
    }
}

void MessagingInstance::MessageServiceSync(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");

    if (!args.contains(JSON_DATA) || !args.contains(JSON_CALLBACK_ID) ||
        !args.get(JSON_CALLBACK_ID).is<double>()) {
      LoggerE("json is incorrect - missing required member");
      return;
    }

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::value v_id = data.at(SYNC_ARGS_ID);
    picojson::value v_limit = data.at(SYNC_ARGS_LIMIT);
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();

    int id = -1;
    try {
        id = std::stoi(v_id.get<std::string>());
    } catch(...) {
        LoggerE("Problem with MessageService");
        ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR), &out);
        return;
    }
    long limit = 0;
    if (v_limit.is<double>()) {
        limit = static_cast<long>(v_limit.get<double>());
    }

    auto json = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_CALLBACK_ID] = picojson::value(callbackId);

    SyncCallbackData *callback = new SyncCallbackData(queue_);
    callback->setJson(json);
    callback->setAccountId(id);
    callback->setLimit(limit);

    queue_.add(static_cast<long>(callbackId), PostPriority::HIGH);
    long op_id = -1;

    const auto result = manager_.getMessageService(id)->sync(callback, &op_id);

    if (result) {
      ReportSuccess(picojson::value(static_cast<double>(op_id)), out);
    } else {
      ReportError(result, &out);
    }
}

void MessagingInstance::MessageServiceSyncFolder(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");

    if (!args.contains(JSON_DATA) || !args.contains(JSON_CALLBACK_ID) ||
        !args.get(JSON_CALLBACK_ID).is<double>()) {
      LoggerE("json is incorrect - missing required member");
      return;
    }

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::value v_id = data.at(SYNC_FOLDER_ARGS_ID);
    picojson::value v_folder = data.at(SYNC_FOLDER_ARGS_FOLDER);
    picojson::value v_limit = data.at(SYNC_FOLDER_ARGS_LIMIT);
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();

    int id = -1;
    try {
        id = std::stoi(v_id.get<std::string>());
    } catch(...) {
        LoggerE("Problem with MessageService");
        ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR), &out);
        return;
    }

    long limit = 0;
    if (v_limit.is<double>()) {
        limit = static_cast<long>(v_limit.get<double>());
    }

    auto json = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_CALLBACK_ID] = picojson::value(callbackId);

    SyncFolderCallbackData *callback = new SyncFolderCallbackData(queue_);
    callback->setJson(json);
    callback->setAccountId(id);
    callback->setMessageFolder(MessagingUtil::jsonToMessageFolder(v_folder));
    callback->setLimit(limit);

    queue_.add(static_cast<long>(callbackId), PostPriority::HIGH);
    long op_id = -1;

    const auto result = manager_.getMessageService(id)->syncFolder(callback, &op_id);
    if (result) {
        ReportSuccess(picojson::value(static_cast<double>(op_id)), out);
    } else {
        ReportError(result, &out);
    }
}

void MessagingInstance::MessageServiceStopSync(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");

    if (!args.contains(JSON_DATA) || !args.contains(JSON_CALLBACK_ID) ||
        !args.get(JSON_CALLBACK_ID).is<double>()) {
      LoggerE("json is incorrect - missing required member");
      return;
    }

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();

    if (data.find(STOP_SYNC_ARGS_ID) != data.end()) {
        picojson::value v_id = data.at(STOP_SYNC_ARGS_ID);
        picojson::value v_op_id = data.at(STOP_SYNC_ARGS_OPID);

        int id = -1;
        try {
            id = std::stoi(v_id.get<std::string>());
        } catch(...) {
            LoggerD("Problem with MessageService");
            ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR), &out);
            return;
        }

        long op_id = 0;
        if (v_op_id.is<double>()) {
            op_id = static_cast<long>(v_op_id.get<double>());
        }

        const auto result = manager_.getMessageService(id)->stopSync(op_id);

        if (result) {
            ReportSuccess(out);
        } else {
            ReportError(result, &out);
        }
    } else {
        LoggerE("Unknown error");
        ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR), &out);
    }
}

void MessagingInstance::MessageStorageAddDraft(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");

    if (!args.contains(JSON_DATA) || !args.contains(JSON_CALLBACK_ID) ||
        !args.get(JSON_CALLBACK_ID).is<double>()) {
      LoggerE("json is incorrect - missing required member");
      return;
    }

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::value v_message = data.at(ADD_DRAFT_MESSAGE_ARGS_MESSAGE);
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();

    auto json = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_CALLBACK_ID] = picojson::value(callbackId);

    std::shared_ptr<Message> message;
    PlatformResult ret = MessagingUtil::jsonToMessage(v_message, &message);
    if (ret.IsError()) {
      POST_AND_RETURN(ret, json, obj, JSON_CALLBACK_ERROR)
    }

    MessageCallbackUserData* callback = new MessageCallbackUserData(queue_);
    callback->setMessage(message);

    int serviceId = getServiceIdFromJSON(data);
    callback->setAccountId(serviceId);

    callback->setJson(json);

    queue_.add(static_cast<long>(callbackId), PostPriority::HIGH);
    auto service = manager_.getMessageService(serviceId);
    service->getMsgStorage()->addDraftMessage(callback);
}

void MessagingInstance::MessageStorageFindMessages(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");

    if (!args.contains(JSON_DATA) || !args.contains(JSON_CALLBACK_ID) ||
        !args.get(JSON_CALLBACK_ID).is<double>()) {
      LoggerE("json is incorrect - missing required member");
      return;
    }

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();

    auto json = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_CALLBACK_ID] = picojson::value(callbackId);

    AbstractFilterPtr filter;
    PlatformResult ret = MessagingUtil::jsonToAbstractFilter(data, &filter);
    if (ret.IsError()) {
      POST_AND_RETURN(ret, json, obj, JSON_CALLBACK_ERROR)
    }
    auto sortMode = MessagingUtil::jsonToSortMode(data);

    long limit = static_cast<long>
            (MessagingUtil::getValueFromJSONObject<double>(data, FIND_FOLDERS_ARGS_LIMIT));

    long offset = static_cast<long>
            (MessagingUtil::getValueFromJSONObject<double>(data, FIND_FOLDERS_ARGS_OFFSET));

    int serviceId = getServiceIdFromJSON(data);
    auto storage = manager_.getMessageService(serviceId)->getMsgStorage();

    FindMsgCallbackUserData* callback = new FindMsgCallbackUserData(queue_);
    callback->setFilter(filter);
    callback->setLimit(limit);
    callback->setOffset(offset);
    callback->setAccountId(serviceId);
    callback->setSortMode(sortMode);
    callback->setJson(json);

    queue_.add(static_cast<long>(callbackId), PostPriority::HIGH);
    storage->findMessages(callback);
}

void MessagingInstance::MessageStorageRemoveMessages(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");

    if (!args.contains(JSON_DATA) || !args.contains(JSON_CALLBACK_ID) ||
        !args.get(JSON_CALLBACK_ID).is<double>()) {
      LoggerE("json is incorrect - missing required member");
      return;
    }

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::array messages = data.at(REMOVE_MESSAGES_ARGS_MESSAGES).get<picojson::array>();
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();

    MessagesCallbackUserData* callback = new MessagesCallbackUserData(queue_);

    auto each = [callback] (picojson::value& v)->void {
      std::shared_ptr<Message> message;
      PlatformResult ret = MessagingUtil::jsonToMessage(v, &message);
      if (ret.IsSuccess()) {
        callback->addMessage(message);
      }
    };

    for_each(messages.begin(), messages.end(), each);

    auto json = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_CALLBACK_ID] = picojson::value(callbackId);
    callback->setJson(json);

    auto service = manager_.getMessageService(getServiceIdFromJSON(data));

    queue_.add(static_cast<long>(callbackId), PostPriority::HIGH);
    service->getMsgStorage()->removeMessages(callback);
}

void MessagingInstance::MessageStorageUpdateMessages(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");

    if (!args.contains(JSON_DATA) || !args.contains(JSON_CALLBACK_ID) ||
        !args.get(JSON_CALLBACK_ID).is<double>()) {
      LoggerE("json is incorrect - missing required member");
      return;
    }

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::value pico_messages = data.at(UPDATE_MESSAGES_ARGS_MESSAGES);
    auto pico_array = pico_messages.get<picojson::array>();
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();

    auto callback = new MessagesCallbackUserData(queue_);

    std::for_each(pico_array.begin(), pico_array.end(), [&callback](picojson::value& v)->void {
       std::shared_ptr<Message> message;
       PlatformResult ret = MessagingUtil::jsonToMessage(v, &message);
       if (ret.IsSuccess()) {
         callback->addMessage(message);
       }
    });

    auto json = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_CALLBACK_ID] = picojson::value(callbackId);
    callback->setJson(json);

    auto service = manager_.getMessageService(getServiceIdFromJSON(data));

    queue_.add(static_cast<long>(callbackId), PostPriority::HIGH);
    service->getMsgStorage()->updateMessages(callback);
}

void MessagingInstance::MessageStorageFindConversations(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");

    if (!args.contains(JSON_DATA) || !args.contains(JSON_CALLBACK_ID) ||
        !args.get(JSON_CALLBACK_ID).is<double>()) {
      LoggerE("json is incorrect - missing required member");
      return;
    }

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();

    auto json = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_CALLBACK_ID] = picojson::value(callbackId);

    AbstractFilterPtr filter;
    PlatformResult ret = MessagingUtil::jsonToAbstractFilter(data, &filter);
    if (ret.IsError()) {
      POST_AND_RETURN(ret, json, obj, JSON_CALLBACK_ERROR)
    }
    auto sortMode = MessagingUtil::jsonToSortMode(data);
    long limit = static_cast<long>
            (MessagingUtil::getValueFromJSONObject<double>(data, FIND_CONVERSATIONS_ARGS_LIMIT));
    long offset = static_cast<long>
            (MessagingUtil::getValueFromJSONObject<double>(data, FIND_CONVERSATIONS_ARGS_OFFSET));

    int serviceId = getServiceIdFromJSON(data);

    ConversationCallbackData* callback = new ConversationCallbackData(queue_);
    callback->setFilter(filter);
    callback->setLimit(limit);
    callback->setOffset(offset);
    callback->setAccountId(serviceId);
    callback->setSortMode(sortMode);
    callback->setJson(json);

    queue_.add(static_cast<long>(callbackId), PostPriority::HIGH);
    auto storage = manager_.getMessageService(serviceId)->getMsgStorage();
    storage->findConversations(callback);
}

void MessagingInstance::MessageStorageRemoveConversations(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");

    if (!args.contains(JSON_DATA) || !args.contains(JSON_CALLBACK_ID) ||
        !args.get(JSON_CALLBACK_ID).is<double>()) {
      LoggerE("json is incorrect - missing required member");
      return;
    }

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::array conversations = data.at(REMOVE_CONVERSATIONS_ARGS_CONVERSATIONS).get<picojson::array>();
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();

    auto json = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_CALLBACK_ID] = picojson::value(callbackId);

    ConversationCallbackData* callback = new ConversationCallbackData(queue_);

    PlatformResult ret(ErrorCode::NO_ERROR);
    for (auto it = conversations.begin(); it != conversations.end(); ++it) {
      std::shared_ptr<MessageConversation> conversation;
      ret = MessagingUtil::jsonToMessageConversation(*it, &conversation);
      if (ret.IsError()) {
        delete callback;
        POST_AND_RETURN(ret, json, obj, JSON_CALLBACK_ERROR)
      }
      callback->addConversation(conversation);
    }

    callback->setJson(json);

    auto service = manager_.getMessageService(getServiceIdFromJSON(data));

    queue_.add(static_cast<long>(callbackId), PostPriority::HIGH);
    service->getMsgStorage()->removeConversations(callback);
}

void MessagingInstance::MessageStorageFindFolders(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");

    if (!args.contains(JSON_DATA) || !args.contains(JSON_CALLBACK_ID) ||
        !args.get(JSON_CALLBACK_ID).is<double>()) {
      LoggerE("json is incorrect - missing required member");
      return;
    }

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();

    auto json = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_CALLBACK_ID] = picojson::value(callbackId);

    AbstractFilterPtr filter;
    PlatformResult ret = MessagingUtil::jsonToAbstractFilter(data, &filter);
    if (ret.IsError()) {
      POST_AND_RETURN(ret, json, obj, JSON_CALLBACK_ERROR)
    }

    FoldersCallbackData* callback = new FoldersCallbackData(queue_);
    callback->setFilter(filter);
    callback->setJson(json);

    queue_.add(static_cast<long>(callbackId), PostPriority::HIGH);
    auto service = manager_.getMessageService(getServiceIdFromJSON(data));
    service->getMsgStorage()->findFolders(callback);
}

void MessagingInstance::MessageStorageAddMessagesChangeListener(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");

    if (!args.contains(JSON_DATA) || !args.contains(JSON_CALLBACK_ID) ||
        !args.get(JSON_CALLBACK_ID).is<double>()) {
      LoggerE("json is incorrect - missing required member");
      return;
    }

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();

    auto json = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_CALLBACK_ID] = picojson::value(callbackId);

    AbstractFilterPtr filter;
    PlatformResult ret = MessagingUtil::jsonToAbstractFilter(data, &filter);
    if (ret.IsError()) {
      POST_AND_RETURN(ret, json, obj, JSON_CALLBACK_ERROR)
    }

    int serviceId = getServiceIdFromJSON(data);

    auto service = manager_.getMessageService(serviceId);

    std::shared_ptr<MessagesChangeCallback> callback(new MessagesChangeCallback(
        static_cast<long>(callbackId), serviceId, service->getMsgServiceType(),queue_));

    callback->setFilter(filter);

    long op_id = service->getMsgStorage()->addMessagesChangeListener(callback);

    picojson::value v(static_cast<double>(op_id));
    ReportSuccess(v, out);
}

void MessagingInstance::MessageStorageAddConversationsChangeListener(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");

    if (!args.contains(JSON_DATA) || !args.contains(JSON_CALLBACK_ID) ||
        !args.get(JSON_CALLBACK_ID).is<double>()) {
      LoggerE("json is incorrect - missing required member");
      return;
    }

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();

    auto json = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_CALLBACK_ID] = picojson::value(callbackId);

    AbstractFilterPtr filter;
    PlatformResult ret = MessagingUtil::jsonToAbstractFilter(data, &filter);
    if (ret.IsError()) {
      POST_AND_RETURN(ret, json, obj, JSON_CALLBACK_ERROR)
    }

    int serviceId = getServiceIdFromJSON(data);

    auto service = manager_.getMessageService(serviceId);

    std::shared_ptr<ConversationsChangeCallback> callback(new ConversationsChangeCallback(
        static_cast<long>(callbackId), serviceId, service->getMsgServiceType(), queue_));

    callback->setFilter(filter);

    long op_id = service->getMsgStorage()->addConversationsChangeListener(callback);

    picojson::value v(static_cast<double>(op_id));
    ReportSuccess(v, out);
}

void MessagingInstance::MessageStorageAddFolderChangeListener(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");

    if (!args.contains(JSON_DATA) || !args.contains(JSON_CALLBACK_ID) ||
        !args.get(JSON_CALLBACK_ID).is<double>()) {
      LoggerE("json is incorrect - missing required member");
      return;
    }

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();

    auto json = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_CALLBACK_ID] = picojson::value(callbackId);

    AbstractFilterPtr filter;
    PlatformResult ret = MessagingUtil::jsonToAbstractFilter(data, &filter);
    if (ret.IsError()) {
      POST_AND_RETURN(ret, json, obj, JSON_CALLBACK_ERROR)
    }

    int serviceId = getServiceIdFromJSON(data);

    auto service = manager_.getMessageService(serviceId);

    std::shared_ptr<FoldersChangeCallback> callback(new FoldersChangeCallback(
        static_cast<long>(callbackId), serviceId, service->getMsgServiceType(), queue_));

    callback->setFilter(filter);

    long op_id = service->getMsgStorage()->addFoldersChangeListener(callback);

    picojson::value v(static_cast<double>(op_id));
    ReportSuccess(v, out);
}

void MessagingInstance::MessageStorageRemoveChangeListener(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");

    if (!args.contains(JSON_DATA)) {
      LoggerE("json is incorrect - missing required member");
      return;
    }

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    const long watchId = static_cast<long>(
            data.at(REMOVE_CHANGE_LISTENER_ARGS_WATCHID).get<double>());

    auto service = manager_.getMessageService(getServiceIdFromJSON(data));

    service->getMsgStorage()->removeChangeListener(watchId);
    ReportSuccess(out);
}

} // namespace messaging
} // namespace extension

