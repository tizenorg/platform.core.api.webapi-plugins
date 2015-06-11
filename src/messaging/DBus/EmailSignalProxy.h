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

/**
 * @file        EmailSignalProxy.h
 */

#ifndef __TIZEN_DBUS_EMAIL_SIGNAL_PROXY_H__
#define __TIZEN_DBUS_EMAIL_SIGNAL_PROXY_H__

#include "common/GDBus/proxy.h"

namespace extension {
namespace messaging {
namespace DBus {

class EmailSignalProxy;
typedef std::shared_ptr<EmailSignalProxy> EmailSignalProxyPtr;

class EmailSignalProxy : public common::dbus::Proxy {
public:
    virtual ~EmailSignalProxy();

protected:
    EmailSignalProxy(const std::string& proxy_path,
            const std::string& proxy_iface);

    /**
     * Override this method in subclass to handle email signal
     */
    virtual void handleEmailSignal(const int status,
            const int mail_id,
            const std::string& source,
            const int op_handle,
            const int error_code) = 0;

    virtual void signalCallback(GDBusConnection *connection,
            const gchar *sender_name,
            const gchar *object_path,
            const gchar *interface_name,
            const gchar *signal_name,
            GVariant *parameters);

private:
};

} //namespace DBus
} //namespace Messaging
} //namespace DeviceAPI

#endif // __TIZEN_DBUS_EMAIL_SIGNAL_PROXY_H__
