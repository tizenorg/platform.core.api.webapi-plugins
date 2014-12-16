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

#ifndef __TIZEN_MESSAGE_PROXY_H
#define __TIZEN_MESSAGE_PROXY_H

#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <gio/gio.h>
#include <memory>
#include <string>
#include <email-types.h>
#include "Proxy.h"

namespace DeviceAPI {
namespace Messaging {
namespace DBus {

class MessageProxy: public Proxy {
public:
    MessageProxy();
    virtual ~MessageProxy();
protected:
    virtual void signalCallback(GDBusConnection *connection,
            const gchar *sender_name,
            const gchar *object_path,
            const gchar *interface_name,
            const gchar *signal_name,
            GVariant *parameters);
    /**
     * Handles e-mail add and update only.
     * @param account_id
     * @param mail_id
     * @param thread_id
     * @param event
     */
    void handleEmailEvent(int account_id, int mail_id, int thread_id, int event);
    void handleEmailRemoveEvent(int account_id, const std::string& idsString);
    void notifyEmailManager(const std::string& idsString, email_noti_on_storage_event status);
    void handleThreadRemoveEvent(int account_id, int thread_id);
    void handleMailboxEvent(int account_id, int mailbox_id, int event);
};

typedef std::shared_ptr<MessageProxy> MessageProxyPtr;

} //DBus
} //Messaging
} //DeviceAPI

#endif /* __TIZEN_MESSAGE_PROXY_H */

