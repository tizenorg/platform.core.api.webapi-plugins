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

#ifndef __TIZEN_SEND_PROXY_H
#define __TIZEN_SEND_PROXY_H

#include "common/platform_result.h"
#include "EmailSignalProxy.h"

namespace extension {
namespace messaging {
namespace DBus {

class SendProxy;
typedef std::shared_ptr<SendProxy> SendProxyPtr;

class SendProxy: public EmailSignalProxy {
public:
    virtual ~SendProxy();
    static common::PlatformResult create(SendProxyPtr* send_proxy);
protected:
    SendProxy();
    virtual void handleEmailSignal(const int status,
            const int account_id,
            const std::string& source,
            const int op_handle,
            const int error_code);
};

} //DBus
} //messaging
} //extension

#endif /* __TIZEN_SEND_PROXY_H */

