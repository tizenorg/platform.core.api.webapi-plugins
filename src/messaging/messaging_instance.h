// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MESSAGING_MESSAGING_INSTANCE_H_
#define MESSAGING_MESSAGING_INSTANCE_H_

#include "common/extension.h"

namespace extension {
namespace messaging {

class MessagingInstance: public common::ParsedInstance {
public:
    static MessagingInstance& getInstance();
private:
    MessagingInstance();
    virtual ~MessagingInstance();

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
};

} // namespace messaging
} // namespace extension

#endif // MESSAGING_MESSAGING_INSTANCE_H_
