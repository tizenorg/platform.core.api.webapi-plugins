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
 
#ifndef MESSAGING_MESSAGING_INSTANCE_H_
#define MESSAGING_MESSAGING_INSTANCE_H_

#include "common/extension.h"

#include "messaging_manager.h"
#include "messaging_util.h"

namespace extension {
namespace messaging {

class MessagingInstance: public common::ParsedInstance {
public:
  MessagingInstance();
  virtual ~MessagingInstance();
private:
    void GetMessageServices(const picojson::value& args, picojson::object& out);

    void MessageServiceSendMessage(const picojson::value& args,
            picojson::object& out);
    void MessageServiceLoadMessageBody(const picojson::value& args,
            picojson::object& out);
    void MessageServiceLoadMessageAttachment(const picojson::value& args,
            picojson::object& out);
    void MessageServiceSync(const picojson::value& args, picojson::object& out);
    void MessageServiceSyncFolder(const picojson::value& args,
            picojson::object& out);
    void MessageServiceStopSync(const picojson::value& args,
            picojson::object& out);

    void MessageStorageAddDraft(const picojson::value& args,
            picojson::object& out);
    void MessageStorageFindMessages(const picojson::value& args,
            picojson::object& out);
    void MessageStorageRemoveMessages(const picojson::value& args,
            picojson::object& out);
    void MessageStorageUpdateMessages(const picojson::value& args,
            picojson::object& out);
    void MessageStorageFindConversations(const picojson::value& args,
            picojson::object& out);
    void MessageStorageRemoveConversations(const picojson::value& args,
            picojson::object& out);
    void MessageStorageFindFolders(const picojson::value& args,
            picojson::object& out);
    void MessageStorageAddMessagesChangeListener(const picojson::value& args,
            picojson::object& out);
    void MessageStorageAddConversationsChangeListener(const picojson::value& args,
            picojson::object& out);
    void MessageStorageAddFolderChangeListener(const picojson::value& args,
            picojson::object& out);
    void MessageStorageRemoveChangeListener(const picojson::value& args,
            picojson::object& out);

    MessagingManager manager_;
    PostQueue queue_;
};

} // namespace messaging
} // namespace extension

#endif // MESSAGING_MESSAGING_INSTANCE_H_
