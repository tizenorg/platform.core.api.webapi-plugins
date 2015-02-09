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
 * @file        SyncProxy.h
 */

#ifndef __TIZEN_DBUS_SYNC_PROXY_H__
#define __TIZEN_DBUS_SYNC_PROXY_H__

#include "EmailSignalProxy.h"
#include "common/platform_result.h"

namespace extension {
namespace messaging {
namespace DBus {

class SyncProxy;
typedef std::shared_ptr<SyncProxy> SyncProxyPtr;

class SyncProxy : public EmailSignalProxy {
public:

    // Callback is owned by this map
    typedef std::map<long, common::CallbackUserData*> CallbackMap;

    virtual ~SyncProxy();

    static common::PlatformResult create(const std::string& path,
                                         const std::string& iface,
                                         SyncProxyPtr* sync_proxy);
    //Passed callback will be owned by this proxy
    void addCallback(long op_id, common::CallbackUserData* callbackOwned);
    common::CallbackUserData* getCallback(long op_id);
    void removeCallback(long op_id);

protected:
    SyncProxy(const std::string& path,
            const std::string& iface);

    virtual void handleEmailSignal(const int status,
            const int mail_id,
            const std::string& source,
            const int op_handle,
            const int error_code);

private:
    /**
     * Find callback by operation handle returned from:
     *  int email_sync_header(..., int *handle);
     */
    common::PlatformResult findSyncCallbackByOpHandle(const int op_handle,
                                                      CallbackMap::iterator* it);
    CallbackMap m_callback_map;
};

} //namespace DBus
} //namespace Messaging
} //namespace DeviceAPI

#endif // __TIZEN_DBUS_SYNC_PROXY_H__
