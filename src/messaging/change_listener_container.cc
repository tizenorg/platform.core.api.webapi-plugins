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

#include "messaging/change_listener_container.h"

#include <unordered_map>

#include "messaging/conversations_change_callback.h"
#include "messaging/email_manager.h"
#include "messaging/folders_change_callback.h"
#include "messaging/messages_change_callback.h"

namespace extension {
namespace messaging {

class ChangeListenerContainer::ChangeListeners {
 public:
  ChangeListeners() {
    LoggerD("Entered");

    groups_[SMS] = std::shared_ptr<ListenerGroup>(new ListenerGroup());
    groups_[MMS] = std::shared_ptr<ListenerGroup>(new ListenerGroup());
    groups_[EMAIL] = std::shared_ptr<ListenerGroup>(new ListenerGroup());

    // call GetNextId() function to properly initialize static mutex inside
    GetNextId();
  }

  template <class CallbackType>
  long Add(const std::shared_ptr<CallbackType>& c) {
    LoggerD("Entered");

    auto it = groups_.find(c->getServiceType());

    if (groups_.end() != it) {
      return it->second->template Get<CallbackType>().Add(c);
    } else {
      LoggerE("Requested type (%d) doesn't exist", c->getServiceType());
      return -1;
    }
  }

  void Remove(long id) {
    LoggerD("Entered");

    for (auto& it : groups_) {
      if (it.second->Remove(id)) {
        LoggerD("Listener with id: %d removed from group: %d", id, it.first);
        return;
      }
    }

    LoggerW("WatchId %d not found", id);
  }

  template <class CallbackType, class EventType>
  void Call(typename CallbackType::Signature m, EventType* e) const {
    LoggerD("Entered");

    auto it = groups_.find(e->service_type);

    if (groups_.end() != it) {
      it->second->template Get<CallbackType>().Call(m, e);
    } else {
      LoggerE("Requested type (%d) doesn't exist", e->service_type);
    }
  }

  bool HasListeners(MessageType type) const {
    LoggerD("Entered");

    auto it = groups_.find(type);

    if (groups_.end() != it) {
      return it->second->HasListeners();
    } else {
      LoggerE("Requested type (%d) doesn't exist", type);
      return false;
    }
  }

 private:
  template <class CallbackType>
  class Listener {
   public:
    long Add(const std::shared_ptr<CallbackType>& c) {
      LoggerD("Entered");

      std::lock_guard<std::mutex> lock(mutex_);
      auto id = GetNextId();
      callbacks_.insert(std::make_pair(id, c));
      return id;
    }

    bool Remove(long id) {
      LoggerD("Entered");

      std::lock_guard<std::mutex> lock(mutex_);
      return (0 != callbacks_.erase(id));
    }

    template <class EventType>
    void Call(typename CallbackType::Signature m, EventType* e) const {
      LoggerD("Entered");

      std::lock_guard<std::mutex> lock(mutex_);

      for (auto& i : callbacks_) {
        auto& c = i.second;
        if (c->getServiceId() == e->service_id) {
          (c.get()->*m)(e->items);
        }
      }
    }

    bool HasListeners() const {
      LoggerD("Entered");

      std::lock_guard<std::mutex> lock(mutex_);
      return !callbacks_.empty();
    }

   private:
    mutable std::mutex mutex_;
    std::unordered_map<long, std::shared_ptr<CallbackType>> callbacks_;
  };

  // This is the only class which knows what kind of callbacks are being
  // handled. If you want to add a new one, make changes here.
  class ListenerGroup {
   public:
    template <class CallbackType>
    Listener<CallbackType>& Get() {
      // you don't want to get here, this is a compilation error...
      // specializations below are the only ones allowed to be called
      return Listener<CallbackType>();
    }

    bool Remove(long id) {
      LoggerD("Entered");

      bool ret = false;

      if (true == (ret = message_callbacks_.Remove(id))) {
        LoggerD("Message listener with id: %d removed", id);
      } else if (true == (ret = conversation_callbacks_.Remove(id))) {
        LoggerD("Conversation listener with id: %d removed", id);
      } else if (true == (ret = folder_callbacks_.Remove(id))) {
        LoggerD("Folder listener with id: %d removed", id);
      }

      return ret;
    }

    bool HasListeners() const {
      LoggerD("Entered");
      return message_callbacks_.HasListeners() ||
             conversation_callbacks_.HasListeners() ||
             folder_callbacks_.HasListeners();
    }

   private:
    Listener<MessagesChangeCallback> message_callbacks_;
    Listener<ConversationsChangeCallback> conversation_callbacks_;
    Listener<FoldersChangeCallback> folder_callbacks_;
  };

  static long GetNextId() {
    LoggerD("Entered");

    static std::mutex mutex;
    static long next_id = 0;
    std::lock_guard<std::mutex> lock(mutex);

    return ++next_id;
  }

  std::unordered_map<MessageType, std::shared_ptr<ListenerGroup>, std::hash<int>> groups_;
};

// template specializations
template <>
ChangeListenerContainer::ChangeListeners::Listener<MessagesChangeCallback>&
ChangeListenerContainer::ChangeListeners::ListenerGroup::Get<MessagesChangeCallback>() {
  return message_callbacks_;
}

template <>
ChangeListenerContainer::ChangeListeners::Listener<ConversationsChangeCallback>&
ChangeListenerContainer::ChangeListeners::ListenerGroup::Get<ConversationsChangeCallback>() {
  return conversation_callbacks_;
}

template <>
ChangeListenerContainer::ChangeListeners::Listener<FoldersChangeCallback>&
ChangeListenerContainer::ChangeListeners::ListenerGroup::Get<FoldersChangeCallback>() {
  return folder_callbacks_;
}

ChangeListenerContainer& ChangeListenerContainer::getInstance() {
  LoggerD("Entered");
  EmailManager::getInstance();  //make sure that callbacks are registered in email-service
  //ShortMsgManager::getInstance(); //make sure that callbacks are registered in msg-service
  static ChangeListenerContainer clc;
  return clc;
}

ChangeListenerContainer::ChangeListenerContainer()
    : listeners_(new ChangeListeners()) {
  LoggerD("Entered");
}

// --- Listeners registration functions ---
long ChangeListenerContainer::addMessageChangeListener(
    const std::shared_ptr<MessagesChangeCallback>& callback) {
  LoggerD("Entered");
  return listeners_->Add(callback);
}

long ChangeListenerContainer::addConversationChangeListener(
    const std::shared_ptr<ConversationsChangeCallback>& callback) {
  LoggerD("Entered");
  return listeners_->Add(callback);
}

long ChangeListenerContainer::addFolderChangeListener(
    const std::shared_ptr<FoldersChangeCallback>& callback) {
  LoggerD("Entered");
  return listeners_->Add(callback);
}

// --- listeners removal ---
void ChangeListenerContainer::removeChangeListener(long id) {
  LoggerD("Entered");
  listeners_->Remove(id);
}

// --- Callback invoking functions ---
// -- for message --
void ChangeListenerContainer::callMessageAdded(EventMessages* event) const {
  LoggerD("Entered");
  listeners_->Call<MessagesChangeCallback>(&MessagesChangeCallback::added, event);
}

void ChangeListenerContainer::callMessageUpdated(EventMessages* event) const {
  LoggerD("Entered");
  listeners_->Call<MessagesChangeCallback>(&MessagesChangeCallback::updated, event);
}

void ChangeListenerContainer::callMessageRemoved(EventMessages* event) const {
  LoggerD("Entered");
  listeners_->Call<MessagesChangeCallback>(&MessagesChangeCallback::removed, event);
}

// -- for conversation --
void ChangeListenerContainer::callConversationAdded(EventConversations* event) const {
  LoggerD("Entered");
  listeners_->Call<ConversationsChangeCallback>(&ConversationsChangeCallback::added, event);
}

void ChangeListenerContainer::callConversationUpdated(EventConversations* event) const {
  LoggerD("Entered");
  listeners_->Call<ConversationsChangeCallback>(&ConversationsChangeCallback::updated, event);
}

void ChangeListenerContainer::callConversationRemoved(EventConversations* event) const {
  LoggerD("Entered");
  listeners_->Call<ConversationsChangeCallback>(&ConversationsChangeCallback::removed, event);
}

// -- for folder --
void ChangeListenerContainer::callFolderAdded(EventFolders* event) const {
  LoggerD("Entered");
  listeners_->Call<FoldersChangeCallback>(&FoldersChangeCallback::added, event);
}

void ChangeListenerContainer::callFolderUpdated(EventFolders* event) const {
  LoggerD("Entered");
  listeners_->Call<FoldersChangeCallback>(&FoldersChangeCallback::updated, event);
}

void ChangeListenerContainer::callFolderRemoved(EventFolders* event) const {
  LoggerD("Entered");
  listeners_->Call<FoldersChangeCallback>(&FoldersChangeCallback::removed, event);
}

bool ChangeListenerContainer::isEmailListenerRegistered() const {
  LoggerD("Entered");
  return listeners_->HasListeners(EMAIL);
}

}  // namespace messaging
}  // namespace extension
