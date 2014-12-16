
// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "messaging_instance.h"

#include <sstream>

#include "common/logger.h"

#include "messaging_manager.h"
#include "messaging_util.h"

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
const char* SERVICE_SYNC_ARGS_ID = "id";
const char* SERVICE_SYNC_ARGS_LIMIT = "limit";

const char* FUN_MESSAGE_SERVICE_SYNC_FOLDER = "MessageService_syncFolder";
const char* SYNC_FOLDER_ARGS_FOLDER = "folder";
const char* SYNC_FOLDER_ARGS_LIMIT = "limit";

const char* FUN_MESSAGE_SERVICE_STOP_SYNC = "MessageService_stopSync";
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
      REGISTER_ASYNC(FUN_MESSAGE_SERVICE_SYNC, MessageServiceSync);
      REGISTER_ASYNC(FUN_MESSAGE_SERVICE_SYNC_FOLDER, MessageServiceSyncFolder);
      REGISTER_ASYNC(FUN_MESSAGE_STORAGE_ADD_DRAFT_MESSAGE, MessageStorageAddDraft);
      REGISTER_ASYNC(FUN_MESSAGE_STORAGE_FIND_MESSAGES, MessageStorageFindMessages);
      REGISTER_ASYNC(FUN_MESSAGE_STORAGE_REMOVE_MESSAGES, MessageStorageRemoveMessages);
      REGISTER_ASYNC(FUN_MESSAGE_STORAGE_FIND_CONVERSATIONS, MessageStorageFindConversations);
      REGISTER_ASYNC(FUN_MESSAGE_STORAGE_REMOVE_CONVERSATIONS, MessageStorageRemoveConversations);
      REGISTER_ASYNC(FUN_MESSAGE_STORAGE_FIND_FOLDERS, MessageStorageFindFolders);
      REGISTER_ASYNC(FUN_MESSAGE_STORAGE_ADD_MESSAGES_CHANGE_LISTENER, MessageStorageAddMessagesChangeListener);
      REGISTER_ASYNC(FUN_MESSAGE_STORAGE_ADD_CONVERSATIONS_CHANGE_LISTENER, MessageStorageAddConversationsChangeListener);
      REGISTER_ASYNC(FUN_MESSAGE_STORAGE_ADD_FOLDER_CHANGE_LISTENER, MessageStorageAddFolderChangeListener);
    #undef REGISTER_ASYNC
    #define REGISTER_SYNC(c,x) \
      RegisterSyncHandler(c, std::bind(&MessagingInstance::x, this, _1, _2));
      REGISTER_SYNC(FUN_MESSAGE_SERVICE_STOP_SYNC, MessageServiceStopSync);
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

void MessagingInstance::MessageServiceSendMessage(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");
}

void MessagingInstance::MessageServiceLoadMessageBody(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");
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
    picojson::value v_id = data.at(SERVICE_SYNC_ARGS_ID);
    picojson::value v_limit = data.at(SERVICE_SYNC_ARGS_LIMIT);
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();

    int id = static_cast<int>(v_id.get<double>());
    long limit = 0;
    if (v_limit.is<double>()) {
        limit = static_cast<long>(v_limit.get<double>());
    }
    MessagingManager::getInstance().getMessageServiceEmail(id)->sync(callbackId, limit);
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

             var msg = new tizen.Message("messaging.email", initDictionary);
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
    LoggerD("%s", v_message.serialize().c_str());
}

void MessagingInstance::MessageStorageFindMessages(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");
}

void MessagingInstance::MessageStorageRemoveMessages(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");
}

void MessagingInstance::MessageStorageUpdateMessages(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");
}

void MessagingInstance::MessageStorageFindConversations(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");
}

void MessagingInstance::MessageStorageRemoveConversations(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");
}

void MessagingInstance::MessageStorageFindFolders(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");
}

void MessagingInstance::MessageStorageAddMessagesChangeListener(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");
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

