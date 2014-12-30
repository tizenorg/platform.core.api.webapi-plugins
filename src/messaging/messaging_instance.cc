
// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "messaging_instance.h"

#include <sstream>

#include "common/logger.h"

#include "MsgCommon/AbstractFilter.h"
#include "messages_change_callback.h"
#include "messages_callback_user_data.h"
#include "find_msg_callback_user_data.h"
#include "folders_callback_data.h"
#include "messaging_manager.h"
#include "messaging_util.h"
#include "message_storage.h"
#include "message.h"

namespace extension {
namespace messaging {

namespace{
const char* FUN_GET_MESSAGE_SERVICES = "Messaging_getMessageServices";
const char* GET_MESSAGE_SERVICES_ARGS_MESSAGE_SERVICE_TYPE = "messageServiceType";

const char* FUN_MESSAGE_SERVICE_SEND_MESSAGE =  "MessageService_sendMessage";
const char* SEND_MESSAGE_ARGS_MESSAGE = "message";
const char* SEND_MESSAGE_ARGS_SIMINDEX = "simIndex";

const char* FUN_MESSAGE_SERVICE_LOAD_MESSAGE_BODY = "MessageService_loadMessageBody";
const char* LOAD_MESSAGE_BODY_ARGS_MESSAGE = "message";

const char* FUN_MESSAGE_SERVICE_LOAD_MESSAGE_ATTACHMENT = "MessageService_loadMessageAttachment";
const char* LOAD_MESSAGE_ATTACHMENT_ARGS_ATTACHMENT = "attachment";

const char* FUN_MESSAGE_SERVICE_SYNC = "MessageService_sync";
const char* SYNC_ARGS_ID = "id";
const char* SYNC_ARGS_LIMIT = "limit";

const char* FUN_MESSAGE_SERVICE_SYNC_FOLDER = "MessageService_syncFolder";
const char* SYNC_FOLDER_ARGS_FOLDER = "folder";
const char* SYNC_FOLDER_ARGS_LIMIT = "limit";

const char* FUN_MESSAGE_SERVICE_STOP_SYNC = "MessageService_stopSync";
const char* STOP_SYNC_ARGS_ID = "id";
const char* STOP_SYNC_ARGS_OPID = "opId";

const char* FUN_MESSAGE_STORAGE_ADD_DRAFT_MESSAGE = "MessageStorage_addDraftMessage";
const char* ADD_DRAFT_MESSAGE_ARGS_MESSAGE = "message";

const char* FUN_MESSAGE_STORAGE_FIND_MESSAGES = "MessageStorage_findMessages";
const char* FIND_MESSAGES_ARGS_FILTER = "filter";
const char* FIND_MESSAGES_ARGS_SORT = "sort";
const char* FIND_MESSAGES_ARGS_LIMIT = "limit";
const char* FIND_MESSAGES_ARGS_OFFSET = "offset";

const char* FUN_MESSAGE_STORAGE_REMOVE_MESSAGES = "MessageStorage_removeMessages";
const char* REMOVE_MESSAGES_ARGS_MESSAGES = "messages";

const char* FUN_MESSAGE_STORAGE_UPDATE_MESSAGES = "MessageStorage_updateMessages";
const char* UPDATE_MESSAGES_ARGS_MESSAGES = "messages";

const char* FUN_MESSAGE_STORAGE_FIND_CONVERSATIONS = "MessageStorage_findConversations";
const char* FIND_CONVERSATIONS_ARGS_FILTER = "filter";
const char* FIND_CONVERSATIONS_ARGS_SORT = "sort";
const char* FIND_CONVERSATIONS_ARGS_LIMIT = "limit";
const char* FIND_CONVERSATIONS_ARGS_OFFSET = "offset";

const char* FUN_MESSAGE_STORAGE_REMOVE_CONVERSATIONS = "MessageStorage_removeConversations";
const char* REMOVE_CONVERSATIONS_ARGS_CONVERSATIONS = "conversations";

const char* FUN_MESSAGE_STORAGE_FIND_FOLDERS = "MessageStorage_findFolders";
const char* FIND_FOLDERS_ARGS_FILTER = "filter";
const char* FIND_FOLDERS_ARGS_SORT = "sort";
const char* FIND_FOLDERS_ARGS_LIMIT = "limit";
const char* FIND_FOLDERS_ARGS_OFFSET = "offset";

const char* FUN_MESSAGE_STORAGE_ADD_MESSAGES_CHANGE_LISTENER =
        "MessageStorage_addMessagesChangeListener";
const char* ADD_MESSAGES_CHANGE_LISTENER_ARGS_LISTENERS = "listeners";
const char* ADD_MESSAGES_CHANGE_LISTENER_ARGS_FILTER = "filter";

const char* FUN_MESSAGE_STORAGE_ADD_CONVERSATIONS_CHANGE_LISTENER =
        "MessageStorage_addConversationsChangeListener";
const char* ADD_CONVERSATION_CHANGE_LISTENER_ARGS_LISTENERS =
        "listeners";
const char* ADD_CONVERSATION_CHANGE_LISTENER_ARGS_FILTER = "filter";

const char* FUN_MESSAGE_STORAGE_ADD_FOLDER_CHANGE_LISTENER =
        "MessageStorage_addFoldersChangeListener";
const char* ADD_FOLDER_CHANGE_LISTENER_ARGS_LISTENERS = "listeners";
const char* ADD_FOLDER_CHANGE_LISTENER_ARGS_FILTER = "filter";

const char* FUN_MESSAGE_STORAGE_REMOVE_CHANGE_LISTENER = "MessageStorage_removeChangeListener";
const char* REMOVE_CHANGE_LISTENER_ARGS_WATCHID = "watchId";

const char* FUNCTIONS_HIDDEN_ARGS_SERVICE_ID = "serviceId";
}

MessagingInstance& MessagingInstance::getInstance()
{
    static MessagingInstance instance;
    return instance;
}

MessagingInstance::MessagingInstance()
{
    LoggerD("Entered");
    using namespace std::placeholders;
    #define REGISTER_ASYNC(c,x) \
      RegisterHandler(c, std::bind(&MessagingInstance::x, this, _1, _2));
      REGISTER_ASYNC(FUN_GET_MESSAGE_SERVICES, GetMessageServices);
      REGISTER_ASYNC(FUN_MESSAGE_SERVICE_SEND_MESSAGE, MessageServiceSendMessage);
      REGISTER_ASYNC(FUN_MESSAGE_SERVICE_LOAD_MESSAGE_BODY, MessageServiceLoadMessageBody);
      REGISTER_ASYNC(FUN_MESSAGE_SERVICE_LOAD_MESSAGE_ATTACHMENT, MessageServiceLoadMessageAttachment);
      REGISTER_ASYNC(FUN_MESSAGE_SERVICE_SYNC_FOLDER, MessageServiceSyncFolder);
      REGISTER_ASYNC(FUN_MESSAGE_STORAGE_ADD_DRAFT_MESSAGE, MessageStorageAddDraft);
      REGISTER_ASYNC(FUN_MESSAGE_STORAGE_FIND_MESSAGES, MessageStorageFindMessages);
      REGISTER_ASYNC(FUN_MESSAGE_STORAGE_REMOVE_MESSAGES, MessageStorageRemoveMessages);
      REGISTER_ASYNC(FUN_MESSAGE_STORAGE_UPDATE_MESSAGES, MessageStorageUpdateMessages);
      REGISTER_ASYNC(FUN_MESSAGE_STORAGE_FIND_CONVERSATIONS, MessageStorageFindConversations);
      REGISTER_ASYNC(FUN_MESSAGE_STORAGE_REMOVE_CONVERSATIONS, MessageStorageRemoveConversations);
      REGISTER_ASYNC(FUN_MESSAGE_STORAGE_FIND_FOLDERS, MessageStorageFindFolders);
      REGISTER_ASYNC(FUN_MESSAGE_STORAGE_ADD_MESSAGES_CHANGE_LISTENER, MessageStorageAddMessagesChangeListener);
      REGISTER_ASYNC(FUN_MESSAGE_STORAGE_ADD_CONVERSATIONS_CHANGE_LISTENER, MessageStorageAddConversationsChangeListener);
      REGISTER_ASYNC(FUN_MESSAGE_STORAGE_ADD_FOLDER_CHANGE_LISTENER, MessageStorageAddFolderChangeListener);
    #undef REGISTER_ASYNC
    #define REGISTER_SYNC(c,x) \
      RegisterSyncHandler(c, std::bind(&MessagingInstance::x, this, _1, _2));
      REGISTER_SYNC(FUN_MESSAGE_SERVICE_SYNC, MessageServiceSync);
      REGISTER_SYNC(FUN_MESSAGE_SERVICE_STOP_SYNC, MessageServiceStopSync);
      REGISTER_SYNC(FUN_MESSAGE_STORAGE_ADD_MESSAGES_CHANGE_LISTENER, MessageStorageAddMessagesChangeListener);
      REGISTER_SYNC(FUN_MESSAGE_STORAGE_REMOVE_CHANGE_LISTENER, MessageStorageRemoveChangeListener);
    #undef REGISTER_SYNC
}

MessagingInstance::~MessagingInstance()
{
    LoggerD("Entered");
}

void MessagingInstance::GetMessageServices(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::value serviceTag = data.at(GET_MESSAGE_SERVICES_ARGS_MESSAGE_SERVICE_TYPE);
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();

    // above values should be validated in js
    MessagingManager::getInstance().getMessageServices(serviceTag.to_str(), callbackId);
}

/*  Code used to testing in node.js console
    // Define the success callback.
    function serviceListCB(services) {
         if (services.length > 0) {
             var initDictionary = {
                 subject: "Testing subject",
                 to: ["r.klepaczko.testmail@gmail.com"],
                 cc: ["r.klepaczko.testmail@gmail.com"],
                 bcc: ["r.klepaczko.testmail@gmail.com"],
                 plainBody: "simple plain body",
                 htmlBody: "simle html body",
                 isHightPriority: false
             }
             msg = new tizen.Message("messaging.email", initDictionary);
             msg.attachments = [new tizen.MessageAttachment("images/myimage.png", "image/png")];
             services[0].sendMessage(msg, function(data){
                 console.log("Send email success");
                 console.dir(data);
             }, function(){
                 console.log("Send email failed");
             });
         }
     }
     tizen.messaging.getMessageServices("messaging.email", serviceListCB);
 */
void MessagingInstance::MessageServiceSendMessage(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::value v_message = data.at(SEND_MESSAGE_ARGS_MESSAGE);
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();

    MessageRecipientsCallbackData* callback = new MessageRecipientsCallbackData();
    callback->setMessage(MessagingUtil::jsonToMessage(v_message));
    auto serviceId = static_cast<int>
            (MessagingUtil::getValueFromJSONObject<double>(data,FUNCTIONS_HIDDEN_ARGS_SERVICE_ID));
    callback->setAccountId(serviceId);

    auto json = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_CALLBACK_ID] = picojson::value(callbackId);

    auto simIndex = static_cast<long>
            (MessagingUtil::getValueFromJSONObject<double>(data,SEND_MESSAGE_ARGS_SIMINDEX));

    if (!callback->setSimIndex(simIndex)) {
        PostMessage(json->serialize().c_str());
        delete callback;
        callback = NULL;
        return;
    }

    callback->setJson(json);

    auto service = MessagingManager::getInstance().getMessageServiceEmail(serviceId);
    service->sendMessage(callback);
}

void MessagingInstance::MessageServiceLoadMessageBody(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::value message = data.at(ADD_DRAFT_MESSAGE_ARGS_MESSAGE);
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();

    MessageBodyCallbackData* callback = new MessageBodyCallbackData();
    callback->setMessage(MessagingUtil::jsonToMessage(message));

    auto serviceId = static_cast<int>(
            MessagingUtil::getValueFromJSONObject<double>(data, FUNCTIONS_HIDDEN_ARGS_SERVICE_ID));

    auto json = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_CALLBACK_ID] = picojson::value(callbackId);
    callback->setJson(json);

    auto service = MessagingManager::getInstance().getMessageServiceEmail(serviceId);
    service->loadMessageBody(callback);
}

void MessagingInstance::MessageServiceLoadMessageAttachment(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");
}

void MessagingInstance::MessageServiceSync(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::value v_id = data.at(SYNC_ARGS_ID);
    picojson::value v_limit = data.at(SYNC_ARGS_LIMIT);
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();

    int id = static_cast<int>(v_id.get<double>());
    long limit = 0;
    if (v_limit.is<double>()) {
        limit = static_cast<long>(v_limit.get<double>());
    }

    auto json = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_CALLBACK_ID] = picojson::value(callbackId);

    SyncCallbackData *callback = new SyncCallbackData();
    callback->setJson(json);
    callback->setAccountId(id);
    callback->setLimit(limit);

    long op_id = MessagingManager::getInstance().getMessageServiceEmail(id)->sync(callback);

    picojson::value v_op_id(static_cast<double>(op_id));
    ReportSuccess(v_op_id, out);
}

void MessagingInstance::MessageServiceSyncFolder(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");
}

void MessagingInstance::MessageServiceStopSync(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::value v_id = data.at(STOP_SYNC_ARGS_ID);
    picojson::value v_op_id = data.at(STOP_SYNC_ARGS_OPID);

    int id = static_cast<int>(v_id.get<double>());
    long op_id = 0;
    if (v_op_id.is<double>()) {
        op_id = static_cast<long>(v_op_id.get<double>());
    }

    MessagingManager::getInstance().getMessageServiceEmail(id)->stopSync(op_id);

    ReportSuccess(out);
}

/*  Code used to testing in node.js console
    // Define the success callback.
    function serviceListCB(services) {
         if (services.length > 0) {
             var initDictionary = {
                 subject: "Testing subject",
                 to: ["a.jacak.testmail@gmail.com", "r.klepaczko.testmail@gmail.com"],
                 cc: ["a.jacak.testmail@gmail.com", "r.klepaczko.testmail@gmail.com"],
                 bcc: ["a.jacak.testmail@gmail.com", "r.klepaczko.testmail@gmail.com"],
                 plainBody: "simple plain body",
                 htmlBody: "simle html body",
                 isHightPriority: false
             }
             msg = new tizen.Message("messaging.email", initDictionary);
             msg.attachments = [new tizen.MessageAttachment("images/myimage.png", "image/png")];
             services[0].messageStorage.addDraftMessage(msg, function(){
                 console.log("Add draft success");
             }, function(){
                 console.log("Add draft failed");
             });
         }
     }
     tizen.messaging.getMessageServices("messaging.email", serviceListCB);
 */
void MessagingInstance::MessageStorageAddDraft(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::value v_message = data.at(ADD_DRAFT_MESSAGE_ARGS_MESSAGE);
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();

    MessageCallbackUserData* callback = new MessageCallbackUserData();
    callback->setMessage(MessagingUtil::jsonToMessage(v_message));

    auto serviceId = static_cast<int>(data.at(FUNCTIONS_HIDDEN_ARGS_SERVICE_ID).get<double>());
    callback->setAccountId(serviceId);

    auto json = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_CALLBACK_ID] = picojson::value(callbackId);
    callback->setJson(json);

    auto service = MessagingManager::getInstance().getMessageServiceEmail(serviceId);
    service->getMsgStorage()->addDraftMessage(callback);
}

void MessagingInstance::MessageStorageFindMessages(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();

    // TODO add support to AttributeRangeFilter
    auto filter = MessagingUtil::jsonToAttributeFilter(data);
    auto sortMode = MessagingUtil::jsonToSortMode(data);
    long limit = static_cast<long>
            (MessagingUtil::getValueFromJSONObject<double>(data, FIND_FOLDERS_ARGS_LIMIT));
    long offset = static_cast<long>
            (MessagingUtil::getValueFromJSONObject<double>(data, FIND_FOLDERS_ARGS_OFFSET));

    int serviceId = static_cast<int>(data.at(FUNCTIONS_HIDDEN_ARGS_SERVICE_ID).get<double>());
    auto storage = MessagingManager::getInstance().getMessageServiceEmail(serviceId)->getMsgStorage();

    FindMsgCallbackUserData* callback = new FindMsgCallbackUserData();
    callback->setFilter(filter);
    callback->setLimit(limit);
    callback->setOffset(offset);
    callback->setAccountId(serviceId);

    auto json = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_CALLBACK_ID] = picojson::value(callbackId);
    callback->setJson(json);

    storage->findMessages(callback);
}

void MessagingInstance::MessageStorageRemoveMessages(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");
    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::array messages = data.at(REMOVE_MESSAGES_ARGS_MESSAGES).get<picojson::array>();
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();

    MessagesCallbackUserData* callback = new MessagesCallbackUserData();

    auto each = [callback] (picojson::value& v)->void {
        callback->addMessage(MessagingUtil::jsonToMessage(v));
    };

    for_each(messages.begin(), messages.end(), each);

    auto json = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_CALLBACK_ID] = picojson::value(callbackId);
    callback->setJson(json);

    int serviceId = static_cast<int>(data.at(FUNCTIONS_HIDDEN_ARGS_SERVICE_ID).get<double>());
    auto service = MessagingManager::getInstance().getMessageServiceEmail(serviceId);

    service->getMsgStorage()->removeMessages(callback);
}

void MessagingInstance::MessageStorageUpdateMessages(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::value pico_messages = data.at(UPDATE_MESSAGES_ARGS_MESSAGES);
    auto pico_array = pico_messages.get<picojson::array>();
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();

    auto callback = new MessagesCallbackUserData();

    std::vector<std::shared_ptr<Message>> messages;
    std::for_each(pico_array.begin(), pico_array.end(), [&callback](picojson::value& v)->void {
       callback->addMessage(MessagingUtil::jsonToMessage(v));
    });

    auto json = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_CALLBACK_ID] = picojson::value(callbackId);
    callback->setJson(json);

    int serviceId = static_cast<int>
            (MessagingUtil::getValueFromJSONObject<double>(data,FUNCTIONS_HIDDEN_ARGS_SERVICE_ID));
    auto service = MessagingManager::getInstance().getMessageServiceEmail(serviceId);

    service->getMsgStorage()->updateMessages(callback);
}

void MessagingInstance::MessageStorageFindConversations(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();

    // TODO add support to AttributeRangeFilter
    auto filter = MessagingUtil::jsonToAttributeFilter(data);
    auto sortMode = MessagingUtil::jsonToSortMode(data);
    long limit = static_cast<long>
            (MessagingUtil::getValueFromJSONObject<double>(data, FIND_CONVERSATIONS_ARGS_LIMIT));
    long offset = static_cast<long>
            (MessagingUtil::getValueFromJSONObject<double>(data, FIND_CONVERSATIONS_ARGS_OFFSET));
    int serviceId = static_cast<int>(data.at(FUNCTIONS_HIDDEN_ARGS_SERVICE_ID).get<double>());

    ConversationCallbackData* callback = new ConversationCallbackData();
    callback->setFilter(filter);
    callback->setLimit(limit);
    callback->setOffset(offset);
    callback->setAccountId(serviceId);

    auto json = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_CALLBACK_ID] = picojson::value(callbackId);
    callback->setJson(json);

    auto storage = MessagingManager::getInstance().getMessageServiceEmail(serviceId)->getMsgStorage();
    storage->findConversations(callback);
}

void MessagingInstance::MessageStorageRemoveConversations(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");
    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::array conversations = data.at(REMOVE_CONVERSATIONS_ARGS_CONVERSATIONS).get<picojson::array>();
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();

    ConversationCallbackData* callback = new ConversationCallbackData();

    auto each = [callback] (picojson::value& v)->void {
        callback->addConversation(MessagingUtil::jsonToMessageConversation(v));
    };
    for_each(conversations.begin(), conversations.end(), each);

    auto json = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_CALLBACK_ID] = picojson::value(callbackId);
    callback->setJson(json);

    int serviceId = static_cast<int>(data.at(FUNCTIONS_HIDDEN_ARGS_SERVICE_ID).get<double>());
    auto service = MessagingManager::getInstance().getMessageServiceEmail(serviceId);

    service->getMsgStorage()->removeConversations(callback);
}

void MessagingInstance::MessageStorageFindFolders(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();

    // TODO add support to AttributeRangeFilter
    auto filter = MessagingUtil::jsonToAttributeFilter(data);
    int serviceId = static_cast<int>(data.at(FUNCTIONS_HIDDEN_ARGS_SERVICE_ID).get<double>());

    FoldersCallbackData* callback = new FoldersCallbackData();
    callback->setFilter(filter);

    auto json = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_CALLBACK_ID] = picojson::value(callbackId);
    callback->setJson(json);

    auto storage = MessagingManager::getInstance().getMessageServiceEmail(serviceId)->getMsgStorage();
    storage->findFolders(callback);
}

void MessagingInstance::MessageStorageAddMessagesChangeListener(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");
    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    const long callbackId = static_cast<long>(args.get(JSON_CALLBACK_ID).get<double>());

    int serviceId = static_cast<int>(data.at(FUNCTIONS_HIDDEN_ARGS_SERVICE_ID).get<double>());
    auto service = MessagingManager::getInstance().getMessageServiceEmail(serviceId);

    std::shared_ptr<MessagesChangeCallback> callback(new MessagesChangeCallback(
                callbackId, serviceId, service->getMsgServiceType()));

    // TODO filter
    // callback->setFilter(tizen::AbstractFilterPtr(new tizen::AbstractFilter()));

    long op_id = service->getMsgStorage()->addMessagesChangeListener(callback);

    picojson::value v(static_cast<double>(op_id));
    ReportSuccess(v, out);
}

void MessagingInstance::MessageStorageAddConversationsChangeListener(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");
}

void MessagingInstance::MessageStorageAddFolderChangeListener(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");
}

void MessagingInstance::MessageStorageRemoveChangeListener(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");
}

} // namespace messaging
} // namespace extension

