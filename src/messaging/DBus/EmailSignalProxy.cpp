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
#include "common/platform_exception.h"

namespace extension {
namespace messaging {
namespace DBus {

EmailSignalProxy::EmailSignalProxy(const std::string& proxy_path,
        const std::string& proxy_iface) :
        Proxy (proxy_path,
                      proxy_iface,
                      Proxy::DBUS_NAME_SIGNAL_EMAIL,   //specify email signal details
                      DBUS_PATH_NETWORK_STATUS,
                      DBUS_IFACE_NETWORK_STATUS)
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

    try {
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

    } catch(const common::PlatformException& exception) {
        LoggerE("Unhandled exception: %s (%s)!", (exception.name()).c_str(),
             (exception.message()).c_str());
    } catch(...) {
        LoggerE("Unhandled exception!");
    }

    g_free(source);
}

} //namespace DBus
} //namespace Messaging
} //namespace DeviceAPI
