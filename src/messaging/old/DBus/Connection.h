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

#ifndef __TIZEN_DBUS_CONNECTION_H__
#define __TIZEN_DBUS_CONNECTION_H__

#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <gio/gio.h>

namespace DeviceAPI {
namespace Messaging {
namespace DBus {

class Connection {
public:
    static Connection& getInstance();

    GDBusConnection* getDBus();

private:
    Connection();
    Connection(const Connection&);
    void operator=(const Connection&);
    virtual ~Connection();

    GDBusConnection* m_dbus;
    GError* m_error;
};

} //namespace DBus
} //namespace Messaging
} //namespace DeviceAPI

#endif
