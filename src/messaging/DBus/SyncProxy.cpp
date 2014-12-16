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
 * @file        SyncProxy.cpp
 */

#include "SyncProxy.h"
#include "common/logger.h"
//#include <PlatformException.h>
#include <cstring>
#include <email-types.h>
#include "../message_service.h"

namespace extension {
namespace messaging {
namespace DBus {

SyncProxy::SyncProxy(const std::string& path,
        const std::string& iface) :
        EmailSignalProxy(path, iface)
{

}

SyncProxy::~SyncProxy()
{

}

void SyncProxy::addCallback(long op_id, common::CallbackUserData* callbackOwned)
{
    m_callback_map.insert(std::make_pair(op_id, callbackOwned));
}

common::CallbackUserData* SyncProxy::getCallback(long op_id)
{
    common::CallbackUserData* cb = NULL;
    CallbackMap::iterator it = m_callback_map.find(op_id);
    if (it != m_callback_map.end()) {
        cb = it->second;
        return cb;
    }
    LOGE("Could not find callback");
    //TODO throw Common::UnknownException("Could not find callback");
    return cb;
}

void SyncProxy::removeCallback(long op_id){
    CallbackMap::iterator it = m_callback_map.find(op_id);
    if (it != m_callback_map.end()) {
        common::CallbackUserData* cb = it->second;
        delete cb;
        cb = NULL;
        m_callback_map.erase(it);
    }
    else {
        LOGE("Could not find callback");
        //TODO throw Common::UnknownException("Could not find callback");
    }
}

void SyncProxy::handleEmailSignal(const int status,
            const int mail_id,
            const std::string& source,
            const int op_handle,
            const int error_code)
{
    if( NOTI_DOWNLOAD_START != status &&
            NOTI_DOWNLOAD_FINISH != status &&
            NOTI_DOWNLOAD_FAIL != status ) {
        // Nothing to do: this status is not related to sync nor syncFolder request
        return;
    }

    LOGD("received email signal with:\n  status: %d\n  mail_id: %d\n  "
            "source: %s\n  op_handle: %d\n  error_code: %d",
            status, mail_id, source.c_str(), op_handle, error_code);

    if (NOTI_DOWNLOAD_START == status) {
        LOGD("Sync started...");
        // There is nothing more to do so we can return now.
        return;
    }

    common::CallbackUserData* callback = NULL;
    CallbackMap::iterator callback_it;

    try {
        callback_it =  findSyncCallbackByOpHandle(op_handle);
        callback = callback_it->second;
        if (!callback) {
            LOGE("Callback is null");
            //TODO throw Common::UnknownException("Callback is null");
        }

        switch (status) {
            case NOTI_DOWNLOAD_FINISH:
                LoggerD("Sync finished!");
                //TODO callback->callSuccessCallback();
                break;

            case NOTI_DOWNLOAD_FAIL:
                LoggerD("Sync failed!");
                //TODO callback->callErrorCallback();
                break;

            default:
                break;
        }
    }
//    catch (const Common::BasePlatformException& e) {
//        // this situation may occur when there is no callback in the
//        // map with specified opId (for example stopSync() has
//        // removed it), but sync() was already started - only
//        // warning here:
//        LOGE("Exception in signal callback");
//    }
    catch(...)
    {
        LOGE("Exception in signal callback");
    }

    if(callback) {
        delete callback;
        m_callback_map.erase(callback_it);
    }
}

SyncProxy::CallbackMap::iterator SyncProxy::findSyncCallbackByOpHandle(
        const int op_handle)
{
    CallbackMap::iterator it = m_callback_map.begin();
    for (; it != m_callback_map.end(); ++it) {
        SyncCallbackData* cb = dynamic_cast<SyncCallbackData*>(it->second);
        if (!cb) continue;

        if (op_handle == cb->getOperationHandle()) {
            return it;
        }
    }
    // this situation may occur when there is no callback in the
    // map with specified opId (for example stopSync() has
    // removed it), but sync() was already started - only
    // warning here:
    LOGW("Could not find callback with op_handle: %d", op_handle);
    //TODO throw Common::UnknownException("Could not find callback");
}

} //namespace DBus
} //namespace Messaging
} //namespace DeviceAPI
