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
#include "common/platform_result.h"
#include <cstring>
#include <email-types.h>
#include "../message_service.h"

namespace extension {
namespace messaging {
namespace DBus {

using namespace common;

SyncProxy::SyncProxy(const std::string& path,
        const std::string& iface) :
        EmailSignalProxy(path, iface)
{

}

SyncProxy::~SyncProxy()
{

}

PlatformResult SyncProxy::create(const std::string& path,
                                 const std::string& iface,
                                 SyncProxyPtr* sync_proxy) {
    sync_proxy->reset(new SyncProxy(path, iface));
    if ((*sync_proxy)->isNotProxyGot()) {
        LoggerE("Could not get sync proxy");
        sync_proxy->reset();
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Could not get sync proxy");
    } else {
        return PlatformResult(ErrorCode::NO_ERROR);
    }
}

void SyncProxy::addCallback(long op_id, common::CallbackUserData* callbackOwned)
{
    m_callback_map.insert(std::make_pair(op_id, callbackOwned));
}

common::CallbackUserData* SyncProxy::getCallback(long op_id) {
  common::CallbackUserData* cb = nullptr;
  const auto it = m_callback_map.find(op_id);

  if (it != m_callback_map.end()) {
    cb = it->second;
  } else {
    LoggerE("Could not find callback");
  }

  return cb;
}

void SyncProxy::removeCallback(long op_id) {
  auto it = m_callback_map.find(op_id);
  if (it != m_callback_map.end()) {
    delete it->second;
    m_callback_map.erase(it);
  } else {
    LoggerE("Could not find callback");
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

    LoggerD("received email signal with:\n  status: %d\n  mail_id: %d\n  "
            "source: %s\n  op_handle: %d\n  error_code: %d",
            status, mail_id, source.c_str(), op_handle, error_code);

    if (NOTI_DOWNLOAD_START == status) {
        LoggerD("Sync started...");
        // There is nothing more to do so we can return now.
        return;
    }

    SyncCallbackData* callback = NULL;
    CallbackMap::iterator callback_it;

    PlatformResult ret = findSyncCallbackByOpHandle(op_handle, &callback_it);
    if (ret.IsError()) {
        LoggerE("Failed to find callback by handle - (%s)", ret.message().c_str());
        return;
    }

    callback = dynamic_cast<SyncCallbackData*>(callback_it->second);
    if (!callback) {
        LoggerE("Callback is null");
        return;
    }

    std::shared_ptr<picojson::value> response = callback->getJson();
    picojson::object& obj = response->get<picojson::object>();
    switch (status) {
        case NOTI_DOWNLOAD_FINISH:
            LoggerD("Sync finished!");
            obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);
            callback->getQueue().resolve(
                    obj.at(JSON_CALLBACK_ID).get<double>(),
                    response->serialize()
            );
            break;

        case NOTI_DOWNLOAD_FAIL:
        {
            LoggerD("Sync failed!");
            callback->setError("UnknownError", "Sync failed!");
            callback->getQueue().resolve(
                    obj.at(JSON_CALLBACK_ID).get<double>(),
                    response->serialize()
            );
        }
        break;

    }

    if (callback) {
        delete callback;
        m_callback_map.erase(callback_it);
    }
}

PlatformResult SyncProxy::findSyncCallbackByOpHandle(const int op_handle,
                                                     SyncProxy::CallbackMap::iterator* it)
{
    *it = m_callback_map.begin();
    for (; *it != m_callback_map.end(); ++(*it)) {
        SyncCallbackData* cb = dynamic_cast<SyncCallbackData*>((*it)->second);
        if (!cb) continue;

        if (op_handle == cb->getOperationHandle()) {
            return PlatformResult(ErrorCode::NO_ERROR);
        }
    }
    // this situation may occur when there is no callback in the
    // map with specified opId (for example stopSync() has
    // removed it), but sync() was already started - only
    // warning here:
    LoggerW("Could not find callback with op_handle: %d", op_handle);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Could not find callback");
}

} //namespace DBus
} //namespace Messaging
} //namespace DeviceAPI
