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
 
#ifndef __TIZEN_CHANGE_LISTENER_CONTAINER_H__
#define __TIZEN_CHANGE_LISTENER_CONTAINER_H__

#include <memory>
#include <mutex>
#include <vector>
#include <map>

#include "common/logger.h"
#include "common/platform_exception.h"

#include "message.h"

//#include "MessageConversation.h"
//#include "MessageFolder.h"

#include "messaging_util.h"
#include "messages_change_callback.h"
#include "conversations_change_callback.h"
#include "folders_change_callback.h"

namespace extension {
namespace messaging {

//! Data related to MessageChange event passed to add/update/remove callbacks
struct EventMessages {
    int service_id;
    MessageType service_type;
    MessagePtrVector items;
    ConversationPtrVector removed_conversations;
    // TODO: Filtering support
};

//! Data related to ConversationChange event passed to add/update/remove callbacks
struct EventConversations {
    int service_id;
    MessageType service_type;
    ConversationPtrVector items;
    // TODO: Filtering support
};

//! Data related to FolderChange event passed to add/update/remove callbacks
struct EventFolders {
    int service_id;
    MessageType service_type;
    FolderPtrVector items;
    // TODO: Filtering support
};

template <class T > struct CallbackDataHolder {
    std::shared_ptr<T> ptr;
    int operation_type;
};

//! Map that stores MessageChangeListeners
typedef std::map<long, std::shared_ptr<MessagesChangeCallback>> MCLmap;
//! Map that stores ConversationsChangeListeners
typedef std::map<long, std::shared_ptr<ConversationsChangeCallback>> CCLmap;
//! Map that stores FoldersChangeListeners
typedef std::map<long, std::shared_ptr<FoldersChangeCallback>> FCLmap;


/**
 * Singleton class for managing (storing and calling) ChangeListeners for
 * short message (SMS/MMS) service and email service.
 *
 * Two mutexes used to lock separately addition and searching of Short and
 * Email message related listeners. Listeneres removal functions locks access
 * to both types of listeners (havind listener id only it is not possible
 * to determine message (service) type.
 */
class ChangeListenerContainer {
    public:
        static ChangeListenerContainer& getInstance();


        // Interface for listener's manipulation (registration and removal).
        long addMessageChangeListener(std::shared_ptr<MessagesChangeCallback> callback);
        long addConversationChangeListener(std::shared_ptr<ConversationsChangeCallback> callback);
        long addFolderChangeListener(std::shared_ptr<FoldersChangeCallback> callback);
        void removeChangeListener(long id);

        // Methods used to invoke registered listeners
        void callMessageAdded(EventMessages* event);
        void callMessageUpdated(EventMessages* event);
        void callMessageRemoved(EventMessages* event);
        void callConversationAdded(EventConversations* event);
        void callConversationUpdated(EventConversations* event);
        void callConversationRemoved(EventConversations* event);
        void callFolderAdded(EventFolders* event);
        void callFolderUpdated(EventFolders* event);
        void callFolderRemoved(EventFolders* event);

    private:
        //! Highest used id (in most cases id of last registered listener)
        long m_current_id;
        //! Mutex for preventing race conditions on SMS/MMS callbacks collection
        std::mutex m_short_lock;
        //! Mutex for preventing race conditions on email callbacks collection
        std::mutex m_email_lock;

        //! Functions for listener id incrementation with thread safe mutex locking
        int getNextId();

        /* Callbacks for emails and short messages should be stored in separate
         * collections to simplyfy adding ang searching on call */

        // Callbacks for short messages service
        MCLmap m_short_message_callbacks;
        CCLmap m_short_conversation_callbacks;
        FCLmap m_short_folder_callbacks;

        // Callbacks for email service
        MCLmap m_email_message_callbacks;
        CCLmap m_email_conversation_callbacks;
        FCLmap m_email_folder_callbacks;

        ChangeListenerContainer();

        template<class T> static bool removeCallbackIfExists(
                std::map<long,std::shared_ptr<T>>& collection, long id ) {
            LoggerD("Entered");

            auto itr = collection.find(id);
            if ( itr != collection.end()) {
                itr->second->setActive(false);
                collection.erase(id);
                return true;
            }
            return false;
        }

        /* Templates below written for code reusage (each template is used
         * 2 times in each callXxxx[Added | Removed | Updated] function. */

        /**
         * Template function for calling "added" callback for all listeners
         * from given collection with given event.
         * */
        template<class T, class U> void callAdded(
                std::map<long,std::shared_ptr<T>>& collection,
                U* event) {
            typename std::map<long,std::shared_ptr<T>>::iterator itstart = collection.begin();
            typename std::map<long,std::shared_ptr<T>>::iterator itend = collection.end();
            try {
                for (; itstart != itend; ++itstart) {
                    auto callback = (*itstart).second;
                    if (callback->getServiceType() == event->service_type
                            && callback->getServiceId() == event->service_id) {
                        LoggerD("Found callback for given service id (%d) and type (%d)",
                                event->service_id, event->service_type);
                        //@todo filter msgs
                        callback->added(event->items);
                    }
                }
            }catch (const common::PlatformException &err) {
                LoggerE("callAdded failed, %s: %s", err.name().c_str(),
                        err.message().c_str());
            }
            catch (...) {
                LoggerE("callAdded failed");
            }
        }

        /**
         * Template function for calling "updated" callback for all listeners
         * from given collection with given event.
         * */
        template<class T, class U> void callUpdated(
                std::map<long,std::shared_ptr<T>>& collection,
                U* event) {
            typename std::map<long,std::shared_ptr<T>>::iterator itstart = collection.begin();
            typename std::map<long,std::shared_ptr<T>>::iterator itend = collection.end();
            try {
                for (; itstart != itend; ++itstart) {
                    auto callback = (*itstart).second;
                    if (callback->getServiceType() == event->service_type
                            && callback->getServiceId() == event->service_id) {
                        LoggerD("Found callback for given service id (%d) and type (%d)",
                                event->service_id, event->service_type);
                        //@todo filter msgs
                        callback->updated(event->items);
                    }
                }
            }catch (const common::PlatformException &err) {
                LoggerE("callUpdated failed, %s: %s", err.name().c_str(),
                        err.message().c_str());
            }
            catch (...) {
                LoggerE("callUpdated failed");
            }
        }

        /**
         * Template function for calling "removed" callback for all listeners
         * from given collection with given event.
         * */

        template<class T, class U> void callRemoved(
                std::map<long,std::shared_ptr<T>>& collection,
                U* event) {
            typename std::map<long,std::shared_ptr<T>>::iterator itstart = collection.begin();
            typename std::map<long,std::shared_ptr<T>>::iterator itend = collection.end();
            try {
                for (; itstart != itend; ++itstart) {
                    auto callback = (*itstart).second;
                    if (callback->getServiceType() == event->service_type
                            && callback->getServiceId() == event->service_id) {
                        LoggerD("Found callback for given service id (%d) and type (%d)",
                                event->service_id, event->service_type);
                        //@todo filter msgs
                        callback->removed(event->items);
                    }
                }
            }catch (const common::PlatformException &err) {
                LoggerE("callRemoved failed, %s: %s", err.name().c_str(),
                        err.message().c_str());
            }
            catch (...) {
                LoggerE("callRemoved failed");
            }
        }
};


} //namespace messaging
} //namespace extension

#endif // __TIZEN_CHANGE_LISTENER_CONTAINER_H__
