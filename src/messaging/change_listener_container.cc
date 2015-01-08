// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "change_listener_container.h"

#include "email_manager.h"
//#include "ShortMsgManager.h"

namespace extension {
namespace messaging {

ChangeListenerContainer& ChangeListenerContainer::getInstance()
{
    EmailManager::getInstance(); //make sure that callbacks are registered in email-service
    //ShortMsgManager::getInstance(); //make sure that callbacks are registered in msg-service
    static ChangeListenerContainer clc;
    return clc;
}

ChangeListenerContainer::ChangeListenerContainer():
        m_current_id(0)
{
    LoggerD("Entered");
    // call getNextId() function to properly initialize static mutex inside
    getNextId();
}

// --- Listeners registration functions ---
long ChangeListenerContainer::addMessageChangeListener(
        std::shared_ptr<MessagesChangeCallback> callback)
{
    LoggerD("Entered");
    // Check type of service for which listener should be registered
    // and lock appropriate mutex
    MessageType mtype = callback->getServiceType();
    if (MessageType(SMS) == mtype || MessageType(MMS) == mtype)
    {
        //std::lock_guard<std::mutex> lock(m_short_lock);
        //int new_id = getNextId();
        //m_short_message_callbacks.insert(std::make_pair(new_id, callback));
        //LoggerD("Added callback for ShortMessage, watchId: %d", new_id);
        //return new_id;
    }
    else if (MessageType(EMAIL) == mtype) {
        std::lock_guard<std::mutex> lock(m_email_lock);
        int new_id = getNextId();
        m_email_message_callbacks.insert(std::make_pair(new_id, callback));
        LoggerD("Added callback for Email, watchId: %d", new_id);
        return new_id;
    }
    LoggerE("Listener with invalid MessageService type - failed to register");
    return -1;
}

long ChangeListenerContainer::addConversationChangeListener(
        std::shared_ptr<ConversationsChangeCallback> callback)
{
    LoggerD("Entered");
    // Check type of service for which listener should be registered
    // and lock appropriate mutex
    MessageType mtype = callback->getServiceType();
    if (MessageType(SMS) == mtype || MessageType(MMS) == mtype)
    {
        //std::lock_guard<std::mutex> lock(m_short_lock);
        //int new_id = getNextId();
        //m_short_conversation_callbacks.insert(std::make_pair(new_id, callback));
        //LoggerD("Added callback for ShortMessage, watchId: %d", new_id);
        //return new_id;
    }
    else if (MessageType(EMAIL) == mtype) {
        std::lock_guard<std::mutex> lock(m_email_lock);
        int new_id = getNextId();
        m_email_conversation_callbacks.insert(std::make_pair(new_id, callback));
        LoggerD("Added callback for Email, watchId: %d", new_id);
        return new_id;
    }
    LoggerE("Listener with invalid MessageService type - failed to register");
    return -1;
}

long ChangeListenerContainer::addFolderChangeListener(
        std::shared_ptr<FoldersChangeCallback> callback)
{
    LoggerD("Entered");
    // Check type of service for which listener should be registered
    // and lock appropriate mutex
    MessageType mtype = callback->getServiceType();
    if (MessageType(SMS) == mtype || MessageType(MMS) == mtype)
    {
        //std::lock_guard<std::mutex> lock(m_short_lock);
        //int new_id = getNextId();
        //m_short_folder_callbacks.insert(std::make_pair(new_id, callback));
        //LoggerD("Added callback for ShortMessage, watchId: %d", new_id);
        //return new_id;
    }
    else if (MessageType(EMAIL) == mtype) {
        std::lock_guard<std::mutex> lock(m_email_lock);
        int new_id = getNextId();
        m_email_folder_callbacks.insert(std::make_pair(new_id, callback));
        LoggerD("Added callback for Email, watchId: %d", new_id);
        return new_id;
    }
    LoggerE("Listener with invalid MessageService type - failed to register");
    return -1;
}

// --- listeners removal ---
void ChangeListenerContainer::removeChangeListener(long id)
{
    LoggerD("Entered");
    // Lock both types of collections - id does not indicate service type
    // TODO: consider additional map<listener_id, service_type> or
    //       map<lister_id, map<>&> to provide faster and less complicated removal
    std::lock_guard<std::mutex> shortlock(m_short_lock);
    std::lock_guard<std::mutex> maillock(m_email_lock);
    LoggerD("Locks done");
    if(id<0 || id > m_current_id) {
        LoggerE("Invalid id %d given.", id);
        return;
    }
    //if (removeCallbackIfExists<MessagesChangeCallback>(
            //context, m_short_message_callbacks,id)) {
        //LoggerD("ShortMessage message listener with id: %d removed", id);
    //}
    //else if (removeCallbackIfExists<ConversationsChangeCallback>(
            //context, m_short_conversation_callbacks, id)) {
        //LoggerD("ShortMessage conversation listener with id: %d removed", id);
    //}
    //else if (removeCallbackIfExists<FoldersChangeCallback>(
            //context, m_short_folder_callbacks, id)) {
        //LoggerD("ShortMessage folder listener with id: %d removed", id);
    //}
    /*else*/ if (removeCallbackIfExists<MessagesChangeCallback>(
            m_email_message_callbacks, id)) {
        LoggerD("Email message listener with id: %d removed", id);
    }
    else if (removeCallbackIfExists<ConversationsChangeCallback>(
            m_email_conversation_callbacks, id)) {
        LoggerD("Email conversation listener with id: %d removed", id);
    }
    //else if (removeCallbackIfExists<FoldersChangeCallback>(
            //context, m_email_folder_callbacks,id)) {
        //LoggerD("Email folder listener with id: %d removed", id);
    //}
    else {
        LoggerW("WatchId %d not found", id);
    }
}

// --- Callback invoking functions ---
// -- for message --
void ChangeListenerContainer::callMessageAdded(EventMessages* event)
{
    LoggerD("Entered");

    if(MessageType(SMS) == event->service_type ||
            MessageType(MMS) == event->service_type) {
        //LoggerD("Calling messageadded for ShortMessage");
        //MCLmap callbacksCopy;
        //{
            //std::lock_guard<std::mutex> shortlock(m_short_lock);
            //callbacksCopy = m_short_message_callbacks;
        //}
        //callAdded<MessagesChangeCallback, EventMessages>(
                //callbacksCopy, event);
    }
    else if(MessageType(EMAIL) == event->service_type) {
        LoggerD("Calling messageadded for Email");
        MCLmap callbacksCopy;
        {
            std::lock_guard<std::mutex> maillock(m_email_lock);
            callbacksCopy = m_email_message_callbacks;
        }
        callAdded<MessagesChangeCallback, EventMessages>(
                callbacksCopy, event);
    }
    else {
        LoggerW("Invalid event type (%d) - no callback called.", event->service_type);
    }
}

void ChangeListenerContainer::callMessageUpdated(EventMessages* event)
{
    LoggerD("Entered");

    if(MessageType(SMS) == event->service_type ||
            MessageType(MMS) == event->service_type) {
        //LoggerD("Calling messageupdated for ShortMessage");
        //MCLmap callbacksCopy;
        //{
            //std::lock_guard<std::mutex> shortlock(m_short_lock);
            //callbacksCopy = m_short_message_callbacks;
        //}
        //callUpdated<MessagesChangeCallback, EventMessages>(
                //callbacksCopy, event);
    }
    else if(MessageType(EMAIL) == event->service_type) {
        LoggerD("Calling messageupdated for Email");
        MCLmap callbacksCopy;
        {
            std::lock_guard<std::mutex> maillock(m_email_lock);
            callbacksCopy = m_email_message_callbacks;
        }
        callUpdated<MessagesChangeCallback, EventMessages>(
                callbacksCopy, event);
    }
    else {
        LoggerW("Invalid event type (%d) - no callback called.", event->service_type);
    }
}

void ChangeListenerContainer::callMessageRemoved(EventMessages* event)
{
    LoggerD("Entered");

    if(MessageType(SMS) == event->service_type ||
            MessageType(MMS) == event->service_type) {
        /*
         *LoggerD("Calling messageremoved for ShortMessage");
         *MCLmap callbacksCopy;
         *{
         *    std::lock_guard<std::mutex> shortlock(m_short_lock);
         *    callbacksCopy = m_short_message_callbacks;
         *}
         *callRemoved<MessagesChangeCallback, EventMessages>(
         *        callbacksCopy, event);
         */
    }
    else if(MessageType(EMAIL) == event->service_type) {
        LoggerD("Calling messageremoved for Email");
        MCLmap callbacksCopy;
        {
            std::lock_guard<std::mutex> maillock(m_email_lock);
            callbacksCopy = m_email_message_callbacks;
        }
        callRemoved<MessagesChangeCallback, EventMessages>(
                callbacksCopy, event);
    }
    else {
        LoggerW("Invalid event type (%d) - no callback called.", event->service_type);
    }
}

// -- for conversation --
void ChangeListenerContainer::callConversationAdded(EventConversations* event)
{
    LoggerD("Entered");

    if(MessageType(SMS) == event->service_type ||
            MessageType(MMS) == event->service_type) {
        //LoggerD("Calling converationadded for ShortMessage");
        //CCLmap callbacksCopy;
        //{
            //std::lock_guard<std::mutex> shortlock(m_short_lock);
            //callbacksCopy = m_short_conversation_callbacks;
        //}
        //callAdded<ConversationsChangeCallback, EventConversations>(
                //callbacksCopy, event);
    }
    else if(MessageType(EMAIL) == event->service_type) {
        LoggerD("Calling conversationadded for Email");
        CCLmap callbacksCopy;
        {
            std::lock_guard<std::mutex> maillock(m_email_lock);
            callbacksCopy = m_email_conversation_callbacks;
        }
        callAdded<ConversationsChangeCallback, EventConversations>(
                callbacksCopy, event);
    }
    else {
        LoggerW("Invalid event type (%d) - no callback called.", event->service_type);
    }
}

void ChangeListenerContainer::callConversationUpdated(EventConversations* event)
{
    LoggerD("Entered");

    if(MessageType(SMS) == event->service_type ||
            MessageType(MMS) == event->service_type) {
        //LoggerD("Calling conversationupdated for ShortConversation");
        //CCLmap callbacksCopy;
        //{
            //std::lock_guard<std::mutex> shortlock(m_short_lock);
            //callbacksCopy = m_short_conversation_callbacks;
        //}
        //callUpdated<ConversationsChangeCallback, EventConversations>(
                //callbacksCopy, event);
    }
    else if(MessageType(EMAIL) == event->service_type) {
        LoggerD("Calling conversationupdated for Email");
        CCLmap callbacksCopy;
        {
            std::lock_guard<std::mutex> maillock(m_email_lock);
            callbacksCopy = m_email_conversation_callbacks;
        }
        callUpdated<ConversationsChangeCallback, EventConversations>(
                callbacksCopy, event);
    }
    else {
        LoggerW("Invalid event type (%d) - no callback called.", event->service_type);
    }
}

void ChangeListenerContainer::callConversationRemoved(EventConversations* event)
{
    LoggerD("Entered");

    if(MessageType(SMS) == event->service_type ||
            MessageType(MMS) == event->service_type) {
        //LoggerD("Calling conversationremoved for ShortConversation");
        //CCLmap callbacksCopy;
        //{
            //std::lock_guard<std::mutex> shortlock(m_short_lock);
            //LoggerD("m_short_conversation_callbacks.size() = %d",
                    //m_short_conversation_callbacks.size());

            //callbacksCopy = m_short_conversation_callbacks;
        //}
        //callRemoved<ConversationsChangeCallback, EventConversations>(
                //callbacksCopy, event);
    }
    else if(MessageType(EMAIL) == event->service_type) {
        LoggerD("Calling conversationremoved for Email");
        CCLmap callbacksCopy;
        {
            std::lock_guard<std::mutex> maillock(m_email_lock);
            LoggerD("m_email_conversation_callbacks.size() = %d",
                m_email_conversation_callbacks.size());

            callbacksCopy = m_email_conversation_callbacks;
        }
        callRemoved<ConversationsChangeCallback, EventConversations>(
                callbacksCopy, event);
    }
    else {
        LoggerW("Invalid event type (%d) - no callback called.", event->service_type);
    }
}

// -- for folder --
void ChangeListenerContainer::callFolderAdded(EventFolders* event)
{
    LoggerD("Entered");

    if(MessageType(SMS) == event->service_type ||
            MessageType(MMS) == event->service_type) {
        //LoggerD("Calling folderadded for ShortMessage");
        //FCLmap callbacksCopy;
        //{
            //std::lock_guard<std::mutex> shortlock(m_short_lock);
            //callbacksCopy = m_short_folder_callbacks;
        //}
        //callAdded<FoldersChangeCallback, EventFolders>(
                //m_short_folder_callbacks, event);
    }
    else if(MessageType(EMAIL) == event->service_type) {
        LoggerD("Calling folderadded for Email");
        FCLmap callbacksCopy;
        {
            std::lock_guard<std::mutex> maillock(m_email_lock);
            callbacksCopy = m_email_folder_callbacks;
        }
        callAdded<FoldersChangeCallback, EventFolders>(
                callbacksCopy, event);
    }
    else {
        LoggerW("Invalid event type (%d) - no callback called.", event->service_type);
    }
}


void ChangeListenerContainer::callFolderUpdated(EventFolders* event)
{
    LoggerD("Entered");

    if(MessageType(SMS) == event->service_type ||
            MessageType(MMS) == event->service_type) {
        //LoggerD("Calling folderupdated for ShortFolder");
        //FCLmap callbacksCopy;
        //{
            //std::lock_guard<std::mutex> shortlock(m_short_lock);
            //callbacksCopy = m_short_folder_callbacks;
        //}
        //callUpdated<FoldersChangeCallback, EventFolders>(
                //callbacksCopy, event);
    }
    else if(MessageType(EMAIL) == event->service_type) {
        LoggerD("Calling folderupdated for Email");
        FCLmap callbacksCopy;
        {
            std::lock_guard<std::mutex> maillock(m_email_lock);
            callbacksCopy = m_email_folder_callbacks;
        }
        callUpdated<FoldersChangeCallback, EventFolders>(
                callbacksCopy, event);
    }
    else {
        LoggerW("Invalid event type (%d) - no callback called.", event->service_type);
    }
}

void ChangeListenerContainer::callFolderRemoved(EventFolders* event)
{
    LoggerD("Entered");

    if(MessageType(SMS) == event->service_type ||
            MessageType(MMS) == event->service_type) {
        //LoggerD("Calling folderremoved for ShortFolder");
        //FCLmap callbacksCopy;
        //{
            //std::lock_guard<std::mutex> shortlock(m_short_lock);
            //callbacksCopy = m_short_folder_callbacks;
        //}
        //callRemoved<FoldersChangeCallback, EventFolders>(
                //callbacksCopy, event);
    }
    else if(MessageType(EMAIL) == event->service_type) {
        LoggerD("Calling folderremoved for Email");
        FCLmap callbacksCopy;
        {
            std::lock_guard<std::mutex> maillock(m_email_lock);
            callbacksCopy = m_email_folder_callbacks;
        }
        callRemoved<FoldersChangeCallback, EventFolders>(
                callbacksCopy, event);
    }
    else {
        LoggerW("Invalid event type (%d) - no callback called.", event->service_type);
    }
}

int ChangeListenerContainer::getNextId() {
    // mutex is created only on first call (first call added to constructor
    // to initialize mutex correctly
    static std::mutex id_mutex;
    std::lock_guard<std::mutex> idlock(id_mutex);

    return m_current_id++;
}

} //namespace messaging
} //namespace extension
