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

#include "SendProxy.h"

#include "common/logger.h"

#include <email-types.h>
#include "../email_manager.h"
#include "messaging/DBus/DBusTypes.h"

namespace extension {
namespace messaging {
namespace DBus {

using namespace common;

SendProxy::SendProxy():
        EmailSignalProxy(kDBusPathNetworkStatus,
            kDBusIfaceNetworkStatus)
{
}

SendProxy::~SendProxy()
{
}

PlatformResult SendProxy::create(SendProxyPtr* send_proxy) {
    send_proxy->reset(new SendProxy());
    if ((*send_proxy)->isNotProxyGot()) {
        LoggerE("Could not get send proxy");
        send_proxy->reset();
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Could not get send proxy");
    } else {
        return PlatformResult(ErrorCode::NO_ERROR);
    }
}

void SendProxy::handleEmailSignal(const int status,
            const int account_id,
            const std::string& source,
            const int mail_id,
            const int error_code)
{
    LoggerD("Enter");
    switch (status) {
        case NOTI_SEND_FINISH:
        case NOTI_SEND_FAIL:
            LoggerD("Recognized status for email send");
            LoggerD("received email signal with:\n  status: %d\n  account_id: %d\n  "
                "source: %s\n  mail_id: %d\n  error_code: %d",
                status, account_id, source.c_str(), mail_id, error_code);
            EmailManager::getInstance().sendStatusCallback(mail_id,
                    static_cast<email_noti_on_network_event>(status),
                    error_code);
            break;
        default:
            LoggerD("Unrecognized status %d, ignoring", status);
    }
}


} //DBus
} //messaging
} //extension
