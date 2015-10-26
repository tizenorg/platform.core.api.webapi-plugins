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

#include "messaging/message.h"
#include "messaging/message_conversation.h"

namespace extension {
namespace messaging {

//! Data related to MessageChange event passed to add/update/remove callbacks
struct EventMessages {
  int service_id;
  MessageType service_type;
  MessagePtrVector items;
  ConversationPtrVector removed_conversations;
};

//! Data related to ConversationChange event passed to add/update/remove callbacks
struct EventConversations {
  int service_id;
  MessageType service_type;
  ConversationPtrVector items;
};

//! Data related to FolderChange event passed to add/update/remove callbacks
struct EventFolders {
  int service_id;
  MessageType service_type;
  FolderPtrVector items;
};

template <class T> struct CallbackDataHolder {
  std::shared_ptr<T> ptr;
  int operation_type;
};

class MessagesChangeCallback;
class ConversationsChangeCallback;
class FoldersChangeCallback;

/**
 * Singleton class for managing (storing and calling) ChangeListeners for
 * short message (SMS/MMS) service and email service.
 */
class ChangeListenerContainer {
 public:
  static ChangeListenerContainer& getInstance();

  // Interface for listener's manipulation (registration and removal).
  long addMessageChangeListener(const std::shared_ptr<MessagesChangeCallback>& callback);
  long addConversationChangeListener(const std::shared_ptr<ConversationsChangeCallback>& callback);
  long addFolderChangeListener(const std::shared_ptr<FoldersChangeCallback>& callback);

  void removeChangeListener(long id);

  // Methods used to invoke registered listeners
  void callMessageAdded(EventMessages* event) const;
  void callMessageUpdated(EventMessages* event) const;
  void callMessageRemoved(EventMessages* event) const;

  void callConversationAdded(EventConversations* event) const;
  void callConversationUpdated(EventConversations* event) const;
  void callConversationRemoved(EventConversations* event) const;

  void callFolderAdded(EventFolders* event) const;
  void callFolderUpdated(EventFolders* event) const;
  void callFolderRemoved(EventFolders* event) const;

  bool isEmailListenerRegistered() const;

 private:
  class ChangeListeners;

  ChangeListenerContainer();

  std::shared_ptr<ChangeListeners> listeners_;
};


}  // namespace messaging
}  // namespace extension

#endif // __TIZEN_CHANGE_LISTENER_CONTAINER_H__
