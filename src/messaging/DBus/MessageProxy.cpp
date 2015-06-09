//
// Tizen Web Device API
// Copyright (c) 2013 Samsung Electronics Co., Ltd.
//
// Licensed under the Apache License, Version 2.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "MessageProxy.h"
#include "common/GDBus/connection.h"
#include "common/logger.h"
#include "../message.h"
#include "../message_email.h"
#include "../message_conversation.h"
//#include <MessageFolder.h>
#include "../change_listener_container.h"
#include "../email_manager.h"
#include "messaging/DBus/DBusTypes.h"

namespace extension {
namespace messaging {
namespace DBus {

using namespace common;

MessageProxy::MessageProxy():
        common::dbus::Proxy(kDBusPathEmailStorageChange,
                     kDBusIfaceEmailStorageChange,
                     kDBusNameSignalEmail,
                     kDBusPathEmailStorageChange,
                     kDBusIfaceEmailStorageChange)
{
}

MessageProxy::~MessageProxy()
{
}

PlatformResult MessageProxy::create(MessageProxyPtr* message_proxy) {
    message_proxy->reset(new MessageProxy());
    if ((*message_proxy)->isNotProxyGot()) {
        LoggerE("Could not get proxy");
        message_proxy->reset();
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Could not get proxy");
    } else {
        return PlatformResult(ErrorCode::NO_ERROR);
    }
}

void MessageProxy::signalCallback(GDBusConnection *connection,
            const gchar *sender_name,
            const gchar *object_path,
            const gchar *interface_name,
            const gchar *signal_name,
            GVariant *parameters)
{
    LoggerD("Enter");
    int status, account_id, object_id, thread_id;
    char* name;
    g_variant_get(parameters, "(iiisi)",
            &status,
            &account_id,
            &object_id,
            &name,
            &thread_id);
    LoggerD("status: %d", status);
    LoggerD("account_id: %d", account_id);
    LoggerD("object_id: %d", object_id);
    LoggerD("name: %s", name);
    LoggerD("thread_id: %d", thread_id);

    PlatformResult ret(ErrorCode::NO_ERROR);
    switch (status) {
      case NOTI_MAIL_ADD:
      case NOTI_MAIL_UPDATE:
        ret = handleEmailEvent(account_id, object_id, thread_id, status);
        break;
      case NOTI_MAIL_DELETE:
        //ids of removing messages are sent with name in format:
        //id1,id2,id3,
        handleEmailRemoveEvent(account_id, name);
        break;
      case NOTI_MAIL_DELETE_FINISH:
      case NOTI_MAIL_DELETE_FAIL:
        //notify EmailManager, maybe it tries to delete mail
        notifyEmailManager(name, static_cast<email_noti_on_storage_event>(status));
        break;
      case NOTI_THREAD_DELETE:
        handleThreadRemoveEvent(account_id, object_id);
        break;
      case NOTI_MAILBOX_ADD:
      case NOTI_MAILBOX_UPDATE:
      case NOTI_MAILBOX_FIELD_UPDATE:
      case NOTI_MAILBOX_RENAME:
      case NOTI_MAILBOX_DELETE:
        ret = handleMailboxEvent(account_id, object_id, status);
        break;
      default:
        LoggerD("Unrecognized status: %d", status);
    }
    if (ret.IsError()){
        LoggerE("%d (%s)", ret.error_code(), (ret.message()).c_str());
    }
    g_free(name);
}

PlatformResult MessageProxy::handleEmailEvent(int account_id, int mail_id, int thread_id, int event)
{
    LoggerD("Enter");

    if(NOTI_MAIL_UPDATE == event) {
        //getting thread_id from message
        email_mail_data_t *mail_data = NULL;

        if(EMAIL_ERROR_NONE != email_get_mail_data(mail_id, &mail_data)) {
            if (mail_data) email_free_mail_data(&mail_data, 1);

            LoggerE("Failed to get mail data during setting conversation id in MessageProxy.");
            //TODO maybe error should be ignored
            return PlatformResult(ErrorCode::UNKNOWN_ERR,
                                  "Failed to get mail data during setting"
                                  " conversation id in MessageProxy.");
        }

        thread_id = mail_data->thread_id;

        if(EMAIL_ERROR_NONE != email_free_mail_data(&mail_data,1)) {
            LoggerE("Failed to free mail data memory");
        }
    }

    email_mail_data_t* mail_data = EmailManager::getInstance().loadMessage(mail_id);
    if (mail_data == NULL) {
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to load email");
    }
    std::shared_ptr<Message> msg;
    PlatformResult ret = Message::convertPlatformEmailToObject(*mail_data, &msg);
    if (ret.IsError()) return ret;
    ConversationPtr conv;
    ret = MessageConversation::convertEmailConversationToObject(
        thread_id, &conv);
    if (ret.IsError()) return ret;

    EventMessages* eventMsg = new EventMessages();
    eventMsg->service_type = MessageType::EMAIL;
    eventMsg->service_id = account_id;
    eventMsg->items.push_back(msg);
    EventConversations* eventConv = new EventConversations();
    eventConv->service_type = MessageType::EMAIL;
    eventConv->service_id = account_id;
    eventConv->items.push_back(conv);
    switch (event) {
        case NOTI_MAIL_ADD:
            ChangeListenerContainer::getInstance().callMessageAdded(eventMsg);
            if (conv->getMessageCount() == 1) {
                LoggerD("This thread is new, triggering conversationAdded");
                ChangeListenerContainer::getInstance().callConversationAdded(eventConv);
            } else {
                LoggerD("This thread is not new, but it's updated");
                ChangeListenerContainer::getInstance().callConversationUpdated(eventConv);
            }
            break;
        case NOTI_MAIL_UPDATE:
            ChangeListenerContainer::getInstance().callMessageUpdated(eventMsg);
            ChangeListenerContainer::getInstance().callConversationUpdated(eventConv);
            break;
        default:
            LoggerW("Unknown event type: %d", event);
            break;

    }
    delete eventMsg;
    delete eventConv;

    EmailManager::getInstance().freeMessage(mail_data);
    return PlatformResult(ErrorCode::NO_ERROR);
}

std::vector<int> getMailIds(const std::string& idsString)
{
    std::stringstream idsStream(idsString);
    std::string item;
    std::vector<int> ids;
    while (std::getline(idsStream, item, ',')) {
        if (item.length() > 0) {
            int id;
            std::stringstream stream(item);
            stream >> id;
            if (stream) {
                LoggerD("Mail delete id: %d", id);
                ids.push_back(id);
            }
        }
    }
    return ids;
}

void MessageProxy::handleEmailRemoveEvent(int account_id, const std::string& idsString)
{
    LoggerD("Enter");
    std::vector<int> ids = getMailIds(idsString);
    if (ids.empty()) {
        LoggerD("Mail id list is empty.");
        return;
    }
    EventMessages* eventMsg = new EventMessages();
    eventMsg->service_type = MessageType::EMAIL;
    eventMsg->service_id = account_id;
    for (auto it = ids.begin(); it != ids.end(); ++it) {
        //it turns out that this event is triggered after messages are removed
        //so we just create empty messages with id and type
        std::shared_ptr<Message> msg = std::make_shared<MessageEmail>();
        msg->setId(*it);
        eventMsg->items.push_back(msg);
    }
    ChangeListenerContainer::getInstance().callMessageRemoved(eventMsg);
    delete eventMsg;
    eventMsg = NULL;
}

void MessageProxy::notifyEmailManager(const std::string& idsString,
        email_noti_on_storage_event status)
{
    LoggerD("Enter");
    std::vector<int> ids = getMailIds(idsString);
    if (ids.empty()) {
        LoggerD("Mail id list is empty.");
        return;
    }
    EmailManager::getInstance().removeStatusCallback(ids, status);
}

void MessageProxy::handleThreadRemoveEvent(int account_id, int thread_id)
{
    LoggerD("Enter");
    //event is called after thread is removed, so we just set thread id and type
    ConversationPtr conv = std::make_shared<MessageConversation>();
    conv->setConversationId(thread_id);
    conv->setType(MessageType::EMAIL);

    EventConversations* eventConv = new EventConversations();
    eventConv->service_type = MessageType::EMAIL;
    eventConv->service_id = account_id;
    eventConv->items.push_back(conv);
    ChangeListenerContainer::getInstance().callConversationRemoved(eventConv);
    delete eventConv;
    eventConv = NULL;
}

PlatformResult MessageProxy::handleMailboxEvent(int account_id, int mailbox_id, int event)
{
    LoggerD("Enter");
    EventFolders* eventFolder = new EventFolders();
    eventFolder->service_type = MessageType::EMAIL;
    eventFolder->service_id = account_id;
    FolderPtr folder;
    if (event == NOTI_MAILBOX_DELETE) {
        //this event is triggered after mailbox is removed
        //so we just create folder with id
        folder.reset(new MessageFolder(std::to_string(mailbox_id),
                "", //parent_id
                "", //service_id
                "", //content_type
                "", //name
                "", //path
                MessageFolderType::MESSAGE_FOLDER_TYPE_NOTSTANDARD,
                false));
    } else {
        email_mailbox_t* mail_box = NULL;
        if (EMAIL_ERROR_NONE != email_get_mailbox_by_mailbox_id(mailbox_id, &mail_box)) {
            LoggerE("Mailbox not retrieved");
            delete eventFolder;
            return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to load mailbox");
        }
        folder.reset(new MessageFolder(*mail_box));
        if (EMAIL_ERROR_NONE != email_free_mailbox(&mail_box, 1)) {
            LoggerD("Failed to free email_free_mailbox");
        }
    }
    eventFolder->items.push_back(folder);
    switch (event) {
        case NOTI_MAILBOX_ADD:
            ChangeListenerContainer::getInstance().callFolderAdded(eventFolder);
            break;
        case NOTI_MAILBOX_UPDATE:
        case NOTI_MAILBOX_FIELD_UPDATE:
            ChangeListenerContainer::getInstance().callFolderUpdated(eventFolder);
            break;
        case NOTI_MAILBOX_DELETE:
            ChangeListenerContainer::getInstance().callFolderRemoved(eventFolder);
            break;
        default:
            LoggerW("Unknown event type: %d", event);
    }
    delete eventFolder;
    return PlatformResult(ErrorCode::NO_ERROR);
}

} //namespace DBus
} //namespace messaging
} //namespace extension
