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
 * @file        LoadBodyProxy.h
 */

#ifndef __TIZEN_DBUS_LOAD_BODY_PROXY_H__
#define __TIZEN_DBUS_LOAD_BODY_PROXY_H__

#include "common/platform_result.h"
#include "EmailSignalProxy.h"
#include <set>

namespace extension {
namespace messaging {

class MessageBodyCallbackData;

namespace DBus {

class LoadBodyProxy;
typedef std::shared_ptr<LoadBodyProxy> LoadBodyProxyPtr;

class LoadBodyProxy : public EmailSignalProxy {
public:

    // Callback is owned by this set
    typedef std::set<MessageBodyCallbackData*> CallbackSet;

    virtual ~LoadBodyProxy();

    static common::PlatformResult create(const std::string& path,
                                         const std::string& iface,
                                         LoadBodyProxyPtr* load_body_proxy);
    //Passed callback will be owned by this proxy
    void addCallback(MessageBodyCallbackData* callbackOwned);
    void removeCallback(MessageBodyCallbackData* callback);

protected:
    LoadBodyProxy(const std::string& path,
            const std::string& iface);
    virtual void handleEmailSignal(const int status,
            const int mail_id,
            const std::string& source,
            const int op_handle,
            const int error_code);

private:
    /**
     * Find callback by operation handle returned from:
     *  int email_download_body(..., int *handle);
     */
    MessageBodyCallbackData* findCallbackByOpHandle(const int op_handle);

    CallbackSet m_callback_set;
};

} //namespace DBus
} //namespace messaging
} //namespace extension

#endif // __TIZEN_DBUS_LOAD_BODY_PROXY_H__