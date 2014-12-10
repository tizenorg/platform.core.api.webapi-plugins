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

#include <JSWebAPIErrorFactory.h>
#include <SecurityExceptions.h>

#include <GlobalContextManager.h>
#include <ArgumentValidator.h>
#include <JSUtil.h>
#include <Export.h>
#include <Logger.h>
#include <PlatformException.h>
#include <TimeTracer.h>
#include <tapi_common.h>

#include "JSMessage.h"
#include "JSMessageService.h"
#include "MessageService.h"
#include "MessagingUtil.h"

#include "plugin_config.h"
#include "JSMessageStorage.h"

#include "JSMessageFolder.h"
#include "JSMessageAttachment.h"

#include "MessagingManager.h"

using namespace std;
using namespace DeviceAPI::Common;
using namespace WrtDeviceApis::Commons;

namespace DeviceAPI {
namespace Messaging {

namespace {
const char* MESSAGE_SERVICE = "MessageService";

const char* MESSAGE_SERVICE_ID = "id";
const char* MESSAGE_SERVICE_TYPE = "type";
const char* MESSAGE_SERVICE_NAME = "name";
const char* MESSAGE_SERVICE_MSG_STORAGE = "messageStorage";
}

JSClassRef JSMessageService::m_jsClassRef = NULL;

JSClassDefinition JSMessageService::m_classInfo = {
    0,
    kJSClassAttributeNone,
    MESSAGE_SERVICE,
    NULL,
    JSMessageService::m_property,
    JSMessageService::m_function,
    JSMessageService::initialize,
    JSMessageService::finalize,
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

JSStaticValue JSMessageService::m_property[] = {
    { MESSAGE_SERVICE_ID, getId, NULL, kJSPropertyAttributeDontDelete
            | kJSPropertyAttributeReadOnly },
    { MESSAGE_SERVICE_TYPE, getType, NULL, kJSPropertyAttributeDontDelete
            | kJSPropertyAttributeReadOnly },
    { MESSAGE_SERVICE_NAME, getName, NULL, kJSPropertyAttributeDontDelete
            | kJSPropertyAttributeReadOnly },
    { MESSAGE_SERVICE_MSG_STORAGE, getMsgStorage, NULL, kJSPropertyAttributeDontDelete
            | kJSPropertyAttributeReadOnly },
    { 0, 0, 0, 0 }
};

JSStaticFunction JSMessageService::m_function[] = {
    { MESSAGING_FUNCTION_API_SEND_MESSAGE, JSMessageService::sendMessage,
            kJSPropertyAttributeNone },
    { MESSAGING_FUNCTION_API_LOAD_MESSAGE_BODY, JSMessageService::loadMessageBody,
            kJSPropertyAttributeNone },
    { MESSAGING_FUNCTION_API_LOAD_MESSAGE_ATTACHMENT,
            JSMessageService::loadMessageAttachment, kJSPropertyAttributeNone },
    { MESSAGING_FUNCTION_API_SYNC, JSMessageService::sync,
            kJSPropertyAttributeNone },
    { MESSAGING_FUNCTION_API_SYNC_FOLDER, JSMessageService::syncFolder,
            kJSPropertyAttributeNone },
    { MESSAGING_FUNCTION_API_STOP_SYNC, JSMessageService::stopSync,
            kJSPropertyAttributeNone },
    { 0, 0, 0 }
};

const JSClassRef JSMessageService::getClassRef()
{
    LOGD("Entered");
    if (!m_jsClassRef) {
        m_jsClassRef = JSClassCreate(&m_classInfo);
    }
    return m_jsClassRef;
}

MessageService* JSMessageService::getPrivateObject(
        JSContextRef context,
        JSValueRef value)
{
    if (!JSValueIsObjectOfClass(context, value, getClassRef())) {
        LOGE("Object type do not match");
        throw Common::TypeMismatchException("Object type is not MessageService");
    }

    JSObjectRef object = JSUtil::JSValueToObject(context, value);
    MessageService* priv = static_cast<MessageService*>(JSObjectGetPrivate(object));
    if (!priv) {
        LOGE("NULL private data");
        throw Common::UnknownException("Private data is null");
    }

    return priv;
}

JSObjectRef JSMessageService::createJSObject(JSContextRef context,
        MessageService* priv)
{
    LOGD("Entered");
    if (!priv) {
        LOGE("Private data is null");
        return NULL;
    }
    priv->copyAceCheckAccessFunction(MessagingManager::getInstance());
    JSObjectRef jsObject = JSObjectMake(context, getClassRef(),
            static_cast<void*>(priv));
    if (NULL == jsObject) {
        LOGE("object creation error");
    }

    return jsObject;
}

void JSMessageService::initialize(JSContextRef context,
        JSObjectRef object)
{
    LOGD("Entered");
}

void JSMessageService::finalize(JSObjectRef object)
{
    LOGD("Entered");
    MessageService* priv = static_cast<MessageService*>(JSObjectGetPrivate(object));
    if (priv) {
        JSObjectSetPrivate(object, NULL);
        delete priv;
        priv = NULL;
    }
}

JSValueRef JSMessageService::getType(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        MessageService* priv = getPrivateObject(context, object);

        return JSUtil::toJSValueRef(context,
            MessagingUtil::messageTypeToString(priv->getMsgServiceType()));
    }
    catch (const BasePlatformException& err) {
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
    }
    catch (...) {
        LOGE("Unknown error, cannot get property");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageService::getName(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        MessageService* priv = getPrivateObject(context, object);

        return JSUtil::toJSValueRef(context, priv->getMsgServiceName());
    }
    catch (const BasePlatformException& err) {
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
    }
    catch (...) {
        LOGE("Unknown error, cannot get property");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageService::getId(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        MessageService* priv = getPrivateObject(context, object);

        return JSUtil::toJSValueRef(context, priv->getMsgServiceIdStr());
    }
    catch (const BasePlatformException& err) {
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
    }
    catch (...) {
        LOGE("Unknown error, cannot get property");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageService::getMsgStorage(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        MessageService* priv = getPrivateObject(context, object);

        std::shared_ptr<MessageStorage> storage = priv->getMsgStorage();
        storage->copyAceCheckAccessFunction(priv);

        return JSMessageStorage::makeJSObject(context, storage);
    }
    catch (const BasePlatformException& err) {
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
    }
    catch (...) {
        LOGE("Unknown error, cannot get property");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageService::sendMessage(JSContextRef context,
        JSObjectRef function,
        JSObjectRef thisObject,
        size_t argumentCount,
        const JSValueRef arguments[],
        JSValueRef* exception)
{
    SET_TIME_TRACER_ITEM(0);
    LOGD("Entered");

    MessageRecipientsCallbackData *callback = NULL;
    try {
        MessageService* priv = getPrivateObject(context, thisObject);
        TIZEN_CHECK_ACCESS(context, exception, priv,
                MESSAGING_FUNCTION_API_SEND_MESSAGE);

        // void sendMessage(Message message,
        //         optional MessageRecipientsCallback successCallback,
        //         optional ErrorCallback? errorCallback);
        ArgumentValidator validator(context, argumentCount, arguments);
        std::shared_ptr<Message> message = JSMessage::getPrivateObject(context,
                validator.toJSValueRef(0));

        JSContextRef g_ctx =
                GlobalContextManager::getInstance()->getGlobalContext(context);
        callback = new(std::nothrow) MessageRecipientsCallbackData(g_ctx);
        if (!callback) {
            LOGE("Callback data creation failed");
            throw Common::UnknownException("Callback data creation failed");
        }
        callback->setMessage(message);
        callback->setSuccessCallback(validator.toFunction(1, true));
        callback->setErrorCallback(validator.toFunction(2, true));

        // internally TelNetworkDefaultDataSubs_t consists of -1(unknown), 0(sim_1), 1(sim_2)
        // but in spec, simIndex parameter starts with 1.
        // so if user set simIndex param, then minus 1 on it.
        long tmp = validator.toLong(3, true, 0);
        char **cp_list = tel_get_cp_name_list();
        int sim_count = 0;

        if (cp_list) {
            while (cp_list[sim_count]) {
                sim_count++;
            }
            g_strfreev(cp_list);
        } else {
            LOGD("Empty cp name list");
        }

        tmp--;
        if (tmp >= sim_count || tmp < -1) {
            LOGE("Sim index out of bound %d : %d", tmp, sim_count);
            Common::InvalidValuesException err("The index of sim is out of bound");
            callback->setError(err.getName().c_str(), err.getMessage().c_str());
            callback->callErrorCallback();

            delete callback;
            callback = NULL;
            return JSValueMakeUndefined(context);
        }

        callback->setSimIndex(static_cast<TelNetworkDefaultDataSubs_t>(tmp));
        priv->sendMessage(callback);
    }
    catch (const BasePlatformException& err) {
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        delete callback;
        callback = NULL;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }
    catch (...) {
        Common::UnknownException err("Unknown error, cannot send message");
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        delete callback;
        callback = NULL;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }

    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageService::loadMessageBody(JSContextRef context,
        JSObjectRef function,
        JSObjectRef thisObject,
        size_t argumentCount,
        const JSValueRef arguments[],
        JSValueRef* exception)
{
    SET_TIME_TRACER_ITEM(0);
    LOGD("Entered");


    MessageBodyCallbackData *callback = NULL;
    try {
        MessageService* priv = getPrivateObject(context, thisObject);
        TIZEN_CHECK_ACCESS(context, exception, priv,
                MESSAGING_FUNCTION_API_LOAD_MESSAGE_BODY);
        // void loadMessageBody(Message message,
        //         MessageBodySuccessCallback successCallback,
        //         optional ErrorCallback? errorCallback);
        ArgumentValidator validator(context, argumentCount, arguments);
        std::shared_ptr<Message> message = JSMessage::getPrivateObject(context,
                validator.toJSValueRef(0));

        JSContextRef g_ctx =
                GlobalContextManager::getInstance()->getGlobalContext(context);
        callback = new(std::nothrow) MessageBodyCallbackData(g_ctx);
        if (!callback) {
            LOGE("Callback data creation failed");
            throw Common::UnknownException("Callback data creation failed");
        }
        callback->setMessage(message);
        callback->setSuccessCallback(validator.toFunction(1));
        callback->setErrorCallback(validator.toFunction(2, true));

        priv->loadMessageBody(callback);
    }
    catch (const BasePlatformException& err) {
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        delete callback;
        callback = NULL;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    } catch (...) {
        Common::UnknownException err("Cannot load message body");
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        delete callback;
        callback = NULL;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }

    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageService::loadMessageAttachment(JSContextRef context,
        JSObjectRef function,
        JSObjectRef thisObject,
        size_t argumentCount,
        const JSValueRef arguments[],
        JSValueRef* exception)
{
    SET_TIME_TRACER_ITEM(0);
    LOGD("Entered");

    MessageAttachmentCallbackData *callback = NULL;
    try {
        MessageService* priv = getPrivateObject(context, thisObject);
        TIZEN_CHECK_ACCESS(context, exception, priv,
                MESSAGING_FUNCTION_API_LOAD_MESSAGE_ATTACHMENT);
        // void loadMessageAttachment(MessageAttachment attachment,
        //         MessageAttachmentSuccessCallback successCallback,
        //         optional ErrorCallback? errorCallback);
        ArgumentValidator validator(context, argumentCount, arguments);
        JSValueRef jsMsgAttachment = validator.toJSValueRef(0);
        std::shared_ptr<MessageAttachment> attachment =
                JSMessageAttachment::getPrivateObject(context, jsMsgAttachment);

        JSContextRef g_ctx =
                GlobalContextManager::getInstance()->getGlobalContext(context);
        callback = new(std::nothrow) MessageAttachmentCallbackData(g_ctx);
        if (!callback) {
            LOGE("Callback data creation failed");
            throw Common::UnknownException("Callback data creation failed");
        }
        callback->setMessageAttachment(attachment);
        callback->setSuccessCallback(validator.toFunction(1));
        callback->setErrorCallback(validator.toFunction(2, true));

        priv->loadMessageAttachment(callback);
    }
    catch (const BasePlatformException& err) {
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        delete callback;
        callback = NULL;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    } catch (...) {
        Common::UnknownException err("Cannot load message attachment");
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        delete callback;
        callback = NULL;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }

    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageService::sync(JSContextRef context,
        JSObjectRef function,
        JSObjectRef thisObject,
        size_t argumentCount,
        const JSValueRef arguments[],
        JSValueRef* exception)
{
    SET_TIME_TRACER_ITEM(0);
    LOGD("Entered");

    SyncCallbackData *callback = NULL;
    try {
        MessageService* priv = getPrivateObject(context, thisObject);
        TIZEN_CHECK_ACCESS(context, exception, priv,
                MESSAGING_FUNCTION_API_SYNC);
        // long sync(optional SuccessCallback? successCallback,
        //           optional ErrorCallback? errorCallback,
        //           optional unsigned long? limit);
        ArgumentValidator validator(context, argumentCount, arguments);

        JSContextRef g_ctx =
                GlobalContextManager::getInstance()->getGlobalContext(context);
        callback = new(std::nothrow) SyncCallbackData(g_ctx);
        if (!callback) {
            LOGE("Callback data creation failed");
            throw Common::UnknownException("Callback data creation failed");
        }
        callback->setSuccessCallback(validator.toFunction(0, true));
        callback->setErrorCallback(validator.toFunction(1, true));
        // if limit is not provided or is null
        // default value is used in MessageService
        if (!validator.isOmitted(2) && !validator.isNull(2)) {
            callback->setLimit(validator.toULong(2));
        }

        long op_id = priv->sync(callback);
        return JSUtil::toJSValueRef(context, op_id);
    }
    catch (const BasePlatformException& err) {
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        delete callback;
        callback = NULL;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    } catch (...) {
        Common::UnknownException err("Cannot sync with external mail server");
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        delete callback;
        callback = NULL;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }

}

JSValueRef JSMessageService::syncFolder(JSContextRef context,
        JSObjectRef function,
        JSObjectRef thisObject,
        size_t argumentCount,
        const JSValueRef arguments[],
        JSValueRef* exception)
{
    SET_TIME_TRACER_ITEM(0);
    LOGD("Entered");

    SyncFolderCallbackData *callback = NULL;
    try {
        MessageService* priv = getPrivateObject(context, thisObject);
        TIZEN_CHECK_ACCESS(context, exception, priv,
                MESSAGING_FUNCTION_API_SYNC_FOLDER);
        // long syncFolder(MessageFolder folder,
        //         optional SuccessCallback? successCallback,
        //         optional ErrorCallback? errorCallback,
        //         optional unsigned long? limit);
        ArgumentValidator validator(context, argumentCount, arguments);
        std::shared_ptr<MessageFolder> messageFolder =
                JSMessageFolder::getPrivateObject(context, validator.toJSValueRef(0));

        JSContextRef g_ctx =
                GlobalContextManager::getInstance()->getGlobalContext(context);
        callback = new(std::nothrow) SyncFolderCallbackData(g_ctx);
        if (!callback) {
            LOGE("Callback data creation failed");
            throw Common::UnknownException("Callback data creation failed");
        }
        callback->setMessageFolder(messageFolder);
        callback->setSuccessCallback(validator.toFunction(1, true));
        callback->setErrorCallback(validator.toFunction(2, true));
        // if limit is not provided or is null
        // default value is used in MessageService
        if (!validator.isOmitted(3) && !validator.isNull(3)) {
            callback->setLimit(validator.toULong(3));
        }

        long op_id = priv->syncFolder(callback);
        return JSUtil::toJSValueRef(context, op_id);
    }
    catch (const BasePlatformException& err) {
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        delete callback;
        callback = NULL;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    } catch (...) {
        Common::UnknownException err("Cannot sync folder with external server");
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        delete callback;
        callback = NULL;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }
}

JSValueRef JSMessageService::stopSync(JSContextRef context,
        JSObjectRef function,
        JSObjectRef thisObject,
        size_t argumentCount,
        const JSValueRef arguments[],
        JSValueRef* exception)
{
    SET_TIME_TRACER_ITEM(0);
    LOGD("Entered");

    try {
        MessageService* priv = getPrivateObject(context, thisObject);
        TIZEN_CHECK_ACCESS(context, exception, priv,
                MESSAGING_FUNCTION_API_STOP_SYNC);

        // void stopSync(long opId);
        ArgumentValidator validator(context, argumentCount, arguments);
        long opId = validator.toLong(0);

        priv->stopSync(opId);
    }
    catch (const BasePlatformException& err) {
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        return JSWebAPIErrorFactory::postException(context, exception, err);
    } catch (...) {
        Common::UnknownException err("Cannot stop sync with external server");
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }

    return JSValueMakeUndefined(context);
}


} //Messaging
} //DeviceAPI

