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
 * @file        proxy.cpp
 */

#include "common/GDBus/proxy.h"
#include "common/logger.h"
#include "common/platform_result.h"
#include <cstring>

namespace common {
namespace dbus {

using namespace common;

Proxy::Proxy(const std::string& proxy_path,
            const std::string& proxy_iface,
            const std::string& signal_name,
            const std::string& signal_path,
            const std::string& signal_iface) :
                m_conn(Connection::getInstance()),
                m_sub_id(0),
                m_path(proxy_path),
                m_iface(proxy_iface),
                m_signal_name(signal_name),
                m_signal_path(signal_path),
                m_signal_iface(signal_iface),
                m_error(NULL),
                m_dbus_signal_subscribed(false)
{
    LoggerD("Proxy:\n"
            "  proxy_path: %s\n  proxy_iface: %s"
            "  signal_name: %s\n signal_path:%s\n signal_iface:%s",
            m_path.c_str(), m_iface.c_str(),
            m_signal_name.c_str(), m_signal_path.c_str(), m_signal_iface.c_str());

    const gchar* unique_name = g_dbus_connection_get_unique_name(m_conn.getDBus());
    LoggerD("Generated unique name: %d", unique_name);

    // path and interface are not obligatory to receive, but
    // they should be set to send the signals.
    m_proxy = g_dbus_proxy_new_sync(m_conn.getDBus(),
            G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
            NULL, unique_name, m_path.c_str(), m_iface.c_str(), NULL, &m_error);
}

Proxy::~Proxy()
{
    signalUnsubscribe();
}

void Proxy::signalCallbackProxy(GDBusConnection *connection,
        const gchar *sender_name,
        const gchar *object_path,
        const gchar *interface_name,
        const gchar *signal_name,
        GVariant *parameters,
        gpointer user_data)
{
    Proxy* this_ptr = static_cast<Proxy*>(user_data);
    if (!this_ptr) {
        LoggerW("Proxy is null, nothing to do");
        return;
    }

    //It is better to log this only when subclass is responsible of handling
    //passed signal. If you need it put it into your signalCallback(...) method
    //LoggerD("signal: %s from: %s path: %s interface: %s",
    //        signal_name, sender_name, object_path, interface_name);
    this_ptr->signalCallback(connection, sender_name, object_path, interface_name,
            signal_name, parameters);
}

void Proxy::signalSubscribe()
{
    if(m_dbus_signal_subscribed) {
        LoggerW("Proxy has already subscribed for listening DBus signal");
        return;
    }

    const char* sender = NULL;
    m_sub_id = g_dbus_connection_signal_subscribe(m_conn.getDBus(),
            sender,
            m_signal_iface.c_str(),
            m_signal_name.c_str(),
            m_signal_path.c_str(),
            NULL,
            G_DBUS_SIGNAL_FLAGS_NONE,
            signalCallbackProxy,
            static_cast<gpointer>(this),
            NULL);
    LoggerD("g_dbus_connection_signal_subscribe returned id: %d", m_sub_id);

    m_dbus_signal_subscribed = true;
}

void Proxy::signalUnsubscribe()
{
    if (!m_dbus_signal_subscribed) {
        LoggerW("Proxy hasn't subscribed for listening DBus signal");
        return;
    }

    g_dbus_connection_signal_unsubscribe(m_conn.getDBus(), m_sub_id);
    LoggerD("g_dbus_connection_signal_unsubscribe finished");

    m_dbus_signal_subscribed = false;
}

const std::string& Proxy::getSignalName() const
{
    return m_signal_name;
}

const std::string& Proxy::getSignalPath() const
{
    return m_signal_path;
}

const std::string& Proxy::getSignalInterfaceName() const
{
    return m_signal_iface;
}

} //namespace dbus
} //namespace common
