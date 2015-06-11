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
 * @file        EmailSignalProxy.cpp
 */

#include "EmailSignalProxy.h"
#include "common/logger.h"
#include <cstring>
#include "messaging/DBus/DBusTypes.h"

namespace extension {
namespace messaging {
namespace DBus {

EmailSignalProxy::EmailSignalProxy(const std::string& proxy_path,
        const std::string& proxy_iface) :
        common::dbus::Proxy (proxy_path,
                      proxy_iface,
                      kDBusNameSignalEmail,   //specify email signal details
                      kDBusPathNetworkStatus,
                      kDBusIfaceNetworkStatus)
{
}

EmailSignalProxy::~EmailSignalProxy()
{

}

void EmailSignalProxy::signalCallback(GDBusConnection* connection,
        const gchar* sender_name,
        const gchar* object_path,
        const gchar* interface_name,
        const gchar* signal_name,
        GVariant* parameters)
{
    int status, mail_id, op_handle, error_code;
    char* source = NULL;

    g_variant_get(parameters, "(iisii)",
            &status,
            &mail_id,
            &source,
            &op_handle,
            &error_code);

    //It is better to log this only when subclass is responsible of handling
    //passed signal (usually determined by status value).
    //
    //LoggerD("email:\n  status: %d\n  mail_id: %d\n  "
    //        "source: %s\n  op_handle: %d\n  error_code: %d",
    //        status, mail_id, source, op_handle, error_code);

    handleEmailSignal(status, mail_id, source, op_handle, error_code);

    g_free(source);
}

} //namespace DBus
} //namespace Messaging
} //namespace DeviceAPI
