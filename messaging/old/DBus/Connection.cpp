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

#include "Connection.h"
#include <Logger.h>
#include <PlatformException.h>
#include <cstring>
#include <email-types.h>
#include "MessageService.h"

namespace DeviceAPI {
namespace Messaging {
namespace DBus {

Connection& Connection::getInstance()
{
    LOGD("Entered");
    static Connection instance;
    return instance;
}

GDBusConnection* Connection::getDBus()
{
    return m_dbus;
}

Connection::Connection()
{
    dbus_g_thread_init();
    g_type_init();

    m_dbus = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &m_error);
    if (!m_dbus || m_error) {
        LOGE("Could not get connection");
    }
    LOGD("Connection set");
}

Connection::~Connection()
{
    g_object_unref(m_dbus);
}

} //namespace DBus
} //namespace Messaging
} //namespace DeviceAPI
