//
// Tizen Web Device API
// Copyright (c) 2012-2013 Samsung Electronics Co., Ltd.
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

#include <string.h>

#include <JSWebAPIErrorFactory.h>

#include <GlobalContextManager.h>
#include <ArgumentValidator.h>
#include <Export.h>
#include <Logger.h>
#include <TimeTracer.h>
#include <system_info.h>

#include "JSMessagingManager.h"
#include "MessagingManager.h"
#include "MessagingUtil.h"

#include "plugin_config_impl.h"

using namespace std;
using namespace DeviceAPI::Common;

namespace DeviceAPI {
namespace Messaging {

namespace {
const char* MESSAGING = "Messaging";
}

JSClassRef JSMessagingManager::m_jsClassRef = NULL;

JSClassDefinition JSMessagingManager::m_classInfo = {
    0,
    kJSClassAttributeNone,
    MESSAGING,
    NULL,
    NULL,
    JSMessagingManager::m_function,
    JSMessagingManager::initialize,
    JSMessagingManager::finalize,
    NULL, //hasProperty,
    NULL, //getProperty,
    NULL, //setProperty,
    NULL, //deleteProperty,
    NULL, //getPropertyNames,
    NULL,
    NULL,
    NULL, //hasInstance,
    NULL
};

JSStaticFunction JSMessagingManager::m_function[] = {
        { MESSAGING_FUNCTION_API_GET_MESSAGE_SERVICE,
                JSMessagingManager::getMessageServices, kJSPropertyAttributeNone },
        { 0, 0, 0 }
};

const JSClassDefinition* JSMessagingManager::getClassInfo() {
    LOGD("Entered");
    return &(m_classInfo);
}

const JSClassRef DLL_EXPORT JSMessagingManager::getClassRef() {
    LOGD("Entered");
    if (!m_jsClassRef) {
        m_jsClassRef = JSClassCreate(&m_classInfo);
    }
    return m_jsClassRef;
}

void JSMessagingManager::initialize(JSContextRef context,
        JSObjectRef object)
{
    LOGD("Entered");
    JSObjectSetPrivate(object, static_cast<void*>(&(MessagingManager::getInstance())));
}

void JSMessagingManager::finalize(JSObjectRef object)
{
    LOGD("Entered");
    JSObjectSetPrivate(object, NULL);
}

JSValueRef JSMessagingManager::getMessageServices(JSContextRef context,
        JSObjectRef object,
        JSObjectRef thisObject,
        size_t argumentCount,
        const JSValueRef arguments[],
        JSValueRef* exception)
{
    SET_TIME_TRACER_ITEM(0);
    LOGD("Entered");

    MessageServiceCallbackData *callback = NULL;
    bool isSupported = false;

    try {
        ArgumentValidator validator(context, argumentCount, arguments);
        MessageType msgTag = MessagingUtil::stringToMessageType(validator.toString(0));
        LOGD("Messaging Service Type: %d", msgTag);

        JSContextRef g_ctx =
                GlobalContextManager::getInstance()->getGlobalContext(context);
        callback = new(std::nothrow) MessageServiceCallbackData(g_ctx);
        if (!callback) {
            LOGE("Callback data creation failed");
            throw Common::UnknownException("Callback data creation failed");
        }
        callback->setSuccessCallback(validator.toFunction(1));
        callback->setErrorCallback(validator.toFunction(2, true));
        callback->setMessageType(msgTag);

        MessageType msgType = callback->getMessageType();
        switch (msgType) {
            case MessageType::SMS:
                if (system_info_get_platform_bool("tizen.org/feature/network.telephony", &isSupported) != SYSTEM_INFO_ERROR_NONE) {
                    LoggerE("Can't know whether SMS is supported or not");
                    //TODO: need to throw unknown error
                }
                LoggerD("isSMSSupported " << isSupported);

                if (isSupported == false) {
                    LoggerE("SMS is not supported");
                    throw DeviceAPI::Common::NotSupportedException("Unsupported message type (SMS)");
                }
                break;
            case MessageType::MMS:
                if (system_info_get_platform_bool("tizen.org/feature/network.telephony.mms", &isSupported) != SYSTEM_INFO_ERROR_NONE) {
                    LoggerE("Can't know whether mms is supported or not");
                    //TODO: need to throw unknown error
                }
                LoggerD("isSupported " << isSupported);

                if (isSupported == false) {
                    LoggerE("mms is not supported");
                    throw DeviceAPI::Common::NotSupportedException("Unsupported message type (MMS)");
                }
                break;
            case MessageType::EMAIL:
                LoggerD("Email type");
                break;
            default:
                LoggerE("This type is not supported");
                throw DeviceAPI::Common::TypeMismatchException("Unsupported message type");
        }
        MessagingManager::getInstance().getMessageServices(callback);
    }
    catch (const BasePlatformException& err) {
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        delete callback;
        callback = NULL;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }
    catch (...) {
        Common::UnknownException err("Unknown error, cannot get message services");
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        delete callback;
        callback = NULL;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }

    return JSValueMakeUndefined(context);
}

} // Messaging
} // DeviceAPI

