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
 * @file        Proxy.h
 */

#ifndef __TIZEN_DBUS_PROXY_H__
#define __TIZEN_DBUS_PROXY_H__

#include "common/GDBus/connection.h"
#include <memory>
#include <string>
#include <mutex>
#include <map>
#include "common/callback_user_data.h"
#include "common/picojson.h"
#include "common/platform_result.h"

namespace common {
namespace dbus {


class Proxy;
typedef std::shared_ptr<Proxy> ProxyPtr;

/**
 * This is generic dbus signal listener proxy.
 */
class Proxy {
public:

    virtual ~Proxy();
    bool isNotProxyGot() { return !m_proxy || m_error; };

    void signalSubscribe();
    void signalUnsubscribe();

    const std::string& getSignalName() const;
    const std::string& getSignalPath() const;
    const std::string& getSignalInterfaceName() const;

protected:
    /**
     * @param proxy_path - path of this proxy
     * @param proxy_iface - interface name of this proxy
     *
     * @param signal_name - expected signal name
     * @param signal_path - expected signal path
     * @param signal_iface - expected signal interface name
     */
    Proxy(const std::string& proxy_path,
            const std::string& proxy_iface,
            const std::string& signal_name,
            const std::string& signal_path,
            const std::string& signal_iface);
    /**
     * Please implement this method in subclass to handle signal.
     * Executed by static void signalCallbackProxy(...).
     */
    virtual void signalCallback(GDBusConnection *connection,
            const gchar *sender_name,
            const gchar *object_path,
            const gchar *interface_name,
            const gchar *signal_name,
            GVariant *parameters) = 0;

private:
    /**
     * This method (registered with g_dbus_connection_signal_subscribe) is executed by
     * DBus when signal is received. It calls
     * (static_cast<Proxy*>(user_data))->signalCallback(...)
     */

    static void signalCallbackProxy(GDBusConnection *connection,
            const gchar *sender_name,
            const gchar *object_path,
            const gchar *interface_name,
            const gchar *signal_name,
            GVariant *parameters,
            gpointer user_data);

    Connection& m_conn;
    guint m_sub_id;

    std::string m_path;
    std::string m_iface;

    std::string m_signal_name;
    std::string m_signal_path;
    std::string m_signal_iface;

    GError* m_error;
    GDBusProxy* m_proxy;
    bool m_dbus_signal_subscribed;
};

} //namespace dbus
} //namespace common

#endif // __TIZEN_DBUS_PROXY_H__
