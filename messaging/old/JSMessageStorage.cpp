//
// Tizen Web Device API
// Copyright (c) 2012 Samsung Electronics Co., Ltd.
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
#include <Export.h>
#include <Logger.h>
#include <PlatformException.h>
#include <JSAttributeFilter.h>
#include <JSSortMode.h>
#include <JSUtil.h>
#include <vector>
#include <TimeTracer.h>


#include "JSMessage.h"
#include "JSMessageConversation.h"
#include "Message.h"
#include "MessageCallbackUserData.h"
#include "JSMessageStorage.h"
#include "MessagesChangeCallback.h"
#include "ConversationsChangeCallback.h"
#include "FoldersChangeCallback.h"
#include "AbstractFilter.h"

#include "plugin_config_impl.h"
#include "MessagesCallbackUserData.h"
#include "FindMsgCallbackUserData.h"
#include "ConversationCallbackData.h"

using namespace std;
using namespace DeviceAPI::Common;
using namespace WrtDeviceApis::Commons;

namespace DeviceAPI {
namespace Messaging {

namespace {
const char* MESSAGE_STORAGE = "MessageStorage";
}

JSClassRef JSMessageStorage::m_jsClassRef = NULL;

JSClassDefinition JSMessageStorage::m_classInfo = {
    0,
    kJSClassAttributeNone,
    MESSAGE_STORAGE,
    NULL,
    NULL,
    JSMessageStorage::m_function,
    JSMessageStorage::initialize,
    JSMessageStorage::finalize,
    NULL, //hasProperty,
    NULL, //getProperty,
    NULL, //setProperty,
    NULL, //deleteProperty,
    NULL, //getPropertyNames,
    NULL,
    NULL,
    NULL,
    NULL
};

JSStaticFunction JSMessageStorage::m_function[] = {
    { MESSAGING_FUNCTION_API_ADD_DRAFT_MESSAGE, JSMessageStorage::addDraftMessage, kJSPropertyAttributeDontDelete },
    { MESSAGING_FUNCTION_API_FIND_MESSAGES, JSMessageStorage::findMessages, kJSPropertyAttributeDontDelete },
    { MESSAGING_FUNCTION_API_REMOVE_MESSAGES, JSMessageStorage::removeMessages, kJSPropertyAttributeDontDelete },
    { MESSAGING_FUNCTION_API_UPDATE_MESSAGES, JSMessageStorage::updateMessages, kJSPropertyAttributeDontDelete },
    { MESSAGING_FUNCTION_API_FIND_CONVERSATIONS, JSMessageStorage::findConversations, kJSPropertyAttributeDontDelete },
    { MESSAGING_FUNCTION_API_REMOVE_CONVERSATIONS, JSMessageStorage::removeConversations, kJSPropertyAttributeDontDelete },
    { MESSAGING_FUNCTION_API_FIND_FOLDERS, JSMessageStorage::findFolders, kJSPropertyAttributeDontDelete },
    { MESSAGING_FUNCTION_API_ADD_MESSAGES_CHANGE_LISTNER, JSMessageStorage::addMessagesChangeListener, kJSPropertyAttributeDontDelete },
    { MESSAGING_FUNCTION_API_ADD_CONVERSATIONS_CHANGE_LISTNER, JSMessageStorage::addConversationsChangeListener, kJSPropertyAttributeDontDelete },
    { MESSAGING_FUNCTION_API_ADD_FOLDERS_CHANGE_LISTNER, JSMessageStorage::addFoldersChangeListener, kJSPropertyAttributeDontDelete },
    { MESSAGING_FUNCTION_API_REMOVE_CHANGE_LISTENER, JSMessageStorage::removeChangeListener, kJSPropertyAttributeDontDelete },
    { 0, 0, 0 }
};

const JSClassRef JSMessageStorage::getClassRef()
{
    LOGD("Entered");
    if (!m_jsClassRef)
    {
        m_jsClassRef = JSClassCreate(&m_classInfo);
    }
    return m_jsClassRef;
}

std::shared_ptr<MessageStorage> JSMessageStorage::getPrivateObject(
        JSContextRef context,
        JSValueRef value)
{
    if (!JSValueIsObjectOfClass(context, value, getClassRef())) {
        LOGE("Object type do not match");
        throw Common::TypeMismatchException("Object type is not MessageStorage");
    }

    JSObjectRef object = JSUtil::JSValueToObject(context, value);
    MessageStorageHolder* priv = static_cast<MessageStorageHolder*>(
            JSObjectGetPrivate(object));
    if (!priv) {
        LOGE("NULL private data");
        throw Common::UnknownException("Private data holder is null");
    }
    if (!(priv->ptr)) {
        LOGE("NULL shared pointer in private data");
        throw Common::UnknownException("Private data is null");
    }

    return priv->ptr;
}

void JSMessageStorage::setPrivateObject(JSObjectRef object,
        std::shared_ptr<MessageStorage> data)
{
    if (!data) {
        LOGE("NULL shared pointer given to set as private data");
        throw Common::UnknownException("NULL private data given");
    }
    MessageStorageHolder* priv = static_cast<MessageStorageHolder*>(
            JSObjectGetPrivate(object));
    if (priv) {
        priv->ptr = data;
    } else {
        priv = new(std::nothrow) MessageStorageHolder();
        if (!priv) {
            LOGE("Memory allocation failure");
            throw Common::UnknownException("Failed to allocate memory");
        }
        priv->ptr = data;
        if(!JSObjectSetPrivate(object, static_cast<void*>(priv))) {
            delete priv;
            priv = NULL;
            LOGE("Failed to set private data in MessageStorage");
            throw Common::UnknownException(
                    "Failed to set MessageStorage private data");
        }
    }
}

JSObjectRef JSMessageStorage::makeJSObject(JSContextRef context,
    std::shared_ptr<MessageStorage> ptr)
{
    LOGD("Entered");
    if (!ptr) {
        LOGE("Private data is null");
        throw Common::UnknownException("Private object is NULL");
    }

    MessageStorageHolder* priv = new(std::nothrow) MessageStorageHolder();
    if (!priv) {
        LOGE("Failed to allocate memory for MessageStorageHolder");
        throw Common::UnknownException("Failed to allocate memory");
    }
    priv->ptr = ptr;

    JSObjectRef jsObject = JSObjectMake(context, getClassRef(),
            static_cast<void*>(priv));
    if (!jsObject) {
        LOGE("Object creation failed");
        throw Common::UnknownException("Object creation failed");
    }

    return jsObject;
}

void JSMessageStorage::initialize(JSContextRef context, JSObjectRef object)
{
    LOGD("Entered");
}

void JSMessageStorage::finalize(JSObjectRef object)
{
    LOGD("Entered");
    MessageStorageHolder* priv =
            static_cast<MessageStorageHolder*>(JSObjectGetPrivate(object));
    if (priv) {
        JSObjectSetPrivate(object, NULL);
        delete priv;
        priv = NULL;
    }
}

JSValueRef JSMessageStorage::addDraftMessage(JSContextRef context,
        JSObjectRef object,
        JSObjectRef thisObject,
        size_t argumentCount,
        const JSValueRef arguments[],
        JSValueRef* exception)
{
    SET_TIME_TRACER_ITEM(0);
    LOGD("Entered");

    MessageCallbackUserData* callback = NULL;
    try {
        std::shared_ptr<MessageStorage> priv = getPrivateObject(context, thisObject);
        TIZEN_CHECK_ACCESS(context, exception, priv.get(),
                MESSAGING_FUNCTION_API_ADD_DRAFT_MESSAGE);

        // void addDraftMessage(Message message,
        //         optional SuccessCallback? successCallback,
        //.        optional ErrorCallback? errorCallback);
        ArgumentValidator validator(context, argumentCount, arguments);
        std::shared_ptr<Message> message = JSMessage::getPrivateObject(context,
                validator.toJSValueRef(0));

        JSContextRef g_ctx = GlobalContextManager::getInstance()
                ->getGlobalContext(context);
        callback = new(std::nothrow) MessageCallbackUserData(g_ctx);
        LOGD("created new callback: %p", callback);

        if (!callback) {
            LOGE("Callback data creation failed");
            throw Common::UnknownException("Callback data creation failed");
        }
        callback->setMessage(message);
        callback->setSuccessCallback(validator.toFunction(1, true));
        callback->setErrorCallback(validator.toFunction(2, true));

        priv->addDraftMessage(callback);
    }
    catch (const BasePlatformException& err) {
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        delete callback;
        callback = NULL;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }
    catch (...) {
        Common::UnknownException err("Unknown error, cannot add draft message");
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        delete callback;
        callback = NULL;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }

    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageStorage::findMessages(JSContextRef context,
        JSObjectRef object,
        JSObjectRef thisObject,
        size_t argumentCount,
        const JSValueRef arguments[],
        JSValueRef* exception)
{
    SET_TIME_TRACER_ITEM(0);
    LOGD("Entered");

    FindMsgCallbackUserData* callback = NULL;
    try {
        std::shared_ptr<MessageStorage> priv = getPrivateObject(context, thisObject);
        TIZEN_CHECK_ACCESS(context, exception, priv.get(),
                MESSAGING_FUNCTION_API_FIND_MESSAGES);
        /*
         * void findMessages(AbstractFilter filter,
         *          MessageArraySuccessCallback successCallback,
         *          optional ErrorCallback? errorCallback,
         *          optional SortMode? sort,
         *          optional unsigned long? limit,
         *          optional unsigned long? offset);
         */
        JSContextRef g_ctx = GlobalContextManager::getInstance()
                ->getGlobalContext(context);
        callback = new FindMsgCallbackUserData(g_ctx);
        LOGD("created new callback: %p", callback);

        ArgumentValidator validator(context, argumentCount, arguments);

        DeviceAPI::Tizen::AbstractFilterPtr filter = Tizen::AbstractFilter::
                getPrivateObject(context, validator.toJSValueRef(0));

        if (!filter) {
            LOGE("Wrong filter");
            throw Common::TypeMismatchException("Invalid filter");
        }
        callback->setFilter(filter);

        callback->setSuccessCallback(validator.toFunction(1));
        callback->setErrorCallback(validator.toFunction(2, true));

        JSObjectRef sortmodeobj = validator.toObject(3, true);
        if(sortmodeobj != NULL){
            DeviceAPI::Tizen::SortModePtr sortMode =
                    Tizen::JSSortMode::getPrivateObject(context, sortmodeobj);
            LOGD("sort mode is set");
            callback->setSortMode(sortMode);
        }

        callback->setLimit(validator.toULong(4, true, 0));
        callback->setOffset(validator.toULong(5, true, 0));
        priv->findMessages(callback);
    }
    catch (const WrtDeviceApis::Commons::Exception& exc) {
        LOGE("Wrong sort/filer mode: %s", exc.GetMessage().c_str());
        Common::TypeMismatchException err(exc.GetMessage().c_str());
        delete callback;
        callback = NULL;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }
    catch (const BasePlatformException& err) {
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        delete callback;
        callback = NULL;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }catch (...) {
        Common::UnknownException err("Unknown error, cannot find messages");
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        delete callback;
        callback = NULL;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }

    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageStorage::removeMessages(JSContextRef context,
        JSObjectRef object,
        JSObjectRef thisObject,
        size_t argumentCount,
        const JSValueRef arguments[],
        JSValueRef* exception)
{
    SET_TIME_TRACER_ITEM(0);
    LOGD("Entered");

    MessagesCallbackUserData* callback = NULL;
    try {
        std::shared_ptr<MessageStorage> priv = getPrivateObject(context, thisObject);
        TIZEN_CHECK_ACCESS(context, exception, priv.get(),
                MESSAGING_FUNCTION_API_REMOVE_MESSAGES);
        /**
         * void removeMessages(Message[] messages,
         *          optional SuccessCallback? successCallback,
         *          optional ErrorCallback? errorCallback);
         */
        ArgumentValidator validator(context, argumentCount, arguments);
        std::vector<JSValueRef> messages = validator.toJSValueRefVector(0);
        JSContextRef g_ctx = GlobalContextManager::getInstance()
                ->getGlobalContext(context);
        callback = new MessagesCallbackUserData(g_ctx);
        LOGD("created new callback: %p", callback);

        callback->addMessages(context, messages);
        callback->setSuccessCallback(validator.toFunction(1, true));
        callback->setErrorCallback(validator.toFunction(2, true));

        priv->removeMessages(callback);
    }
    catch (const BasePlatformException& err) {
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        delete callback;
        callback = NULL;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }
    catch (...) {
        Common::UnknownException err("Unknown error, cannot remove messages");
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        delete callback;
        callback = NULL;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }

    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageStorage::updateMessages(JSContextRef context,
        JSObjectRef object,
        JSObjectRef thisObject,
        size_t argumentCount,
        const JSValueRef arguments[],
        JSValueRef* exception)
{
    SET_TIME_TRACER_ITEM(0);
    LOGD("Entered");

    MessagesCallbackUserData* callback = NULL;
    try {
        std::shared_ptr<MessageStorage> priv = getPrivateObject(context, thisObject);
        TIZEN_CHECK_ACCESS(context, exception, priv.get(),
                MESSAGING_FUNCTION_API_UPDATE_MESSAGES);
        /**
         * void updateMessages (Message[] messages,
         *          optional SuccessCallback? successCallback,
         *          optional ErrorCallback? errorCallback)
         */
        ArgumentValidator validator(context, argumentCount, arguments);
        std::vector<JSValueRef> messages = validator.toJSValueRefVector(0);
        JSContextRef g_ctx = GlobalContextManager::getInstance()
                ->getGlobalContext(context);
        callback = new MessagesCallbackUserData(g_ctx);
        LOGD("created new callback: %p", callback);

        callback->addMessages(context, messages);
        callback->setSuccessCallback(validator.toFunction(1, true));
        callback->setErrorCallback(validator.toFunction(2, true));

        priv->updateMessages(callback);
    }
    catch (const BasePlatformException& err) {
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        delete callback;
        callback = NULL;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }
    catch (...) {
        Common::UnknownException err("Unknown error, cannot update messages");
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        delete callback;
        callback = NULL;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }

    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageStorage::findConversations(JSContextRef context,
        JSObjectRef object,
        JSObjectRef thisObject,
        size_t argumentCount,
        const JSValueRef arguments[],
        JSValueRef* exception)
{
    SET_TIME_TRACER_ITEM(0);
    LOGD("Entered");

    ConversationCallbackData* callback = NULL;
    try {
        std::shared_ptr<MessageStorage> priv = getPrivateObject(context, thisObject);
        TIZEN_CHECK_ACCESS(context, exception, priv.get(),
                MESSAGING_FUNCTION_API_FIND_CONVERSATIONS);
        // void findConversations (AbstractFilter filter,
        //         MessageConversationArraySuccessCallback successCallback,
        //         optional ErrorCallback? errorCallback,
        //         optional SortMode? sort,
        //         optional unsigned long? limit,
        //         optional unsigned long? offset)
        ArgumentValidator validator(context, argumentCount, arguments);

        DeviceAPI::Tizen::AbstractFilterPtr filter = Tizen::AbstractFilter::
                getPrivateObject(context, validator.toJSValueRef(0));
        if (!filter) {
            LOGE("Wrong filter");
            throw Common::TypeMismatchException("Invalid filter");
        }

        JSContextRef g_ctx =
                GlobalContextManager::getInstance()->getGlobalContext(context);
        callback = new ConversationCallbackData(g_ctx);
        LOGD("created new callback: %p", callback);

        callback->setFilter(filter);
        callback->setSuccessCallback(validator.toFunction(1));
        callback->setErrorCallback(validator.toFunction(2, true));

        JSObjectRef sortmodeobj = validator.toObject(3, true);
        if(sortmodeobj != NULL){
            DeviceAPI::Tizen::SortModePtr sortMode =
                    Tizen::JSSortMode::getPrivateObject(context, sortmodeobj);
            if (!sortMode) {
                LOGE("Wrong sort mode");
                throw Common::TypeMismatchException("Invalid sort mode");
            }
            LOGD("sort mode is set");
            callback->setSortMode(sortMode);
        }

        callback->setLimit(validator.toULong(4, true, 0));
        callback->setOffset(validator.toULong(5, true, 0));

        priv->findConversations(callback);
    }
    catch (const WrtDeviceApis::Commons::Exception& exc) {
        LOGE("Wrong sort/filer mode: %s", exc.GetMessage().c_str());
        Common::TypeMismatchException err(exc.GetMessage().c_str());
        delete callback;
        callback = NULL;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }
    catch (const BasePlatformException& err) {
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        delete callback;
        callback = NULL;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }
    catch (...) {
        Common::UnknownException err("Cannot find conversations");
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        delete callback;
        callback = NULL;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }

    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageStorage::removeConversations(JSContextRef context,
        JSObjectRef object,
        JSObjectRef thisObject,
        size_t argumentCount,
        const JSValueRef arguments[],
        JSValueRef* exception)
{
    SET_TIME_TRACER_ITEM(0);
    LOGD("Entered");

    ConversationCallbackData* callback = NULL;
    try {
        std::shared_ptr<MessageStorage> priv = getPrivateObject(context,
                thisObject);
        TIZEN_CHECK_ACCESS(context, exception, priv.get(),
                MESSAGING_FUNCTION_API_REMOVE_CONVERSATIONS);
        // void removeConversation(MessageConversation[] conversations,
        //         optional SuccessCallback? successCallback,
        //         optional ErrorCallback? errorCallback);
        ArgumentValidator validator(context, argumentCount, arguments);
        std::vector<JSValueRef> conversations = validator.toJSValueRefVector(0);

        JSContextRef g_ctx =
                GlobalContextManager::getInstance()->getGlobalContext(context);
        callback = new ConversationCallbackData(g_ctx);
        LOGD("created new callback: %p", callback);

        callback->addConversations(context, conversations);
        callback->setSuccessCallback(validator.toFunction(1, true));
        callback->setErrorCallback(validator.toFunction(2, true));

        priv->removeConversations(callback);
    } catch (const BasePlatformException& err) {
        LOGE("%s : %s", err.getName().c_str(), err.getMessage().c_str());
        delete callback;
        callback = NULL;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    } catch (...) {
        LOGE("Unknown error while removing conversation");
        delete callback;
        callback = NULL;
        Common::UnknownException err("Cannot remove conversations");
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }

    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageStorage::findFolders(JSContextRef context,
        JSObjectRef object,
        JSObjectRef thisObject,
        size_t argumentCount,
        const JSValueRef arguments[],
        JSValueRef* exception)
{

    SET_TIME_TRACER_ITEM(0);
    LOGD("Entered");

    FoldersCallbackData* callback = NULL;
    try {
        std::shared_ptr<MessageStorage> priv = getPrivateObject(context, thisObject);
        TIZEN_CHECK_ACCESS(context, exception, priv.get(),
                MESSAGING_FUNCTION_API_FIND_FOLDERS);
        // void findFolders(AbstractFilter filter,
        //         MessageFolderArraySuccessCallback successCallback,
        //         optional ErrorCallback? errorCallback);
        ArgumentValidator validator(context, argumentCount, arguments);

        DeviceAPI::Tizen::AbstractFilterPtr filter = Tizen::AbstractFilter::
                getPrivateObject(context, validator.toJSValueRef(0));
        if (!filter) {
            LOGE("Wrong filter");
            throw Common::TypeMismatchException("Invalid filter");
        }

        JSContextRef global_ctx =
                GlobalContextManager::getInstance()->getGlobalContext(context);

        callback = new FoldersCallbackData(global_ctx);
        LOGD("created new callback: %p", callback);

        callback->setFilter(filter);
        callback->setSuccessCallback(validator.toFunction(1));
        callback->setErrorCallback(validator.toFunction(2, true));
        priv->findFolders(callback);
    }
    catch (const BasePlatformException& err) {
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        delete callback;
        callback = NULL;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }
    catch (...) {
        Common::UnknownException err("Cannot find folders");
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        delete callback;
        callback = NULL;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }

    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageStorage::addMessagesChangeListener(JSContextRef context,
        JSObjectRef object,
        JSObjectRef thisObject,
        size_t argumentCount,
        const JSValueRef arguments[],
        JSValueRef* exception)
{
    SET_TIME_TRACER_ITEM(0);
    LOGD("Entered");

    try {
        std::shared_ptr<MessageStorage> priv = getPrivateObject(context, thisObject);
        TIZEN_CHECK_ACCESS(context, exception, priv.get(),
                MESSAGING_FUNCTION_API_ADD_MESSAGES_CHANGE_LISTNER);
        // long addMessagesChangeListener (
        //         MessagesChangeCallback messagesChangeCallback,
        //         optional AbstractFilter filter)
        ArgumentValidator validator(context, argumentCount, arguments);
        JSObjectRef js_callbacks_obj = validator.toCallbackObject(0, false,
                MESSAGESADDED, MESSAGESUPDATED, MESSAGESREMOVED, NULL);

        JSObjectRef on_added_obj = getFunctionFromCallbackObj(context,
                js_callbacks_obj, MESSAGESADDED);
        JSObjectRef on_updateded_obj = getFunctionFromCallbackObj(context,
                js_callbacks_obj, MESSAGESUPDATED);
        JSObjectRef on_removed_obj = getFunctionFromCallbackObj(context,
                js_callbacks_obj, MESSAGESREMOVED);


        JSContextRef global_ctx =
                GlobalContextManager::getInstance()->getGlobalContext(context);

        auto callback = std::make_shared<MessagesChangeCallback>(
                global_ctx,
                on_added_obj,
                on_updateded_obj,
                on_removed_obj,
                priv->getMsgServiceId(),
                priv->getMsgServiceType());

        JSObjectRef filterobj = validator.toObject(1, true);
        if (filterobj != NULL) {
            DeviceAPI::Tizen::AbstractFilterPtr filter = Tizen::AbstractFilter::
                    getPrivateObject(context, validator.toJSValueRef(1));
            if (!filter) {
                LOGE("Wrong filter");
                throw Common::TypeMismatchException("Invalid filter");
            }
            LOGD("filter is set");
            callback->setFilter(filter);
        }
        return JSValueMakeNumber(context, priv->addMessagesChangeListener(callback));
    }
    catch (const WrtDeviceApis::Commons::Exception& exc) {
        LOGE("Wrong sort/filer mode: %s", exc.GetMessage().c_str());
        Common::TypeMismatchException err(exc.GetMessage().c_str());
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }
    catch (const BasePlatformException& err) {
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }
    catch (...) {
        Common::UnknownException err("Cannot add listener for messages");
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }

}

JSValueRef JSMessageStorage::addConversationsChangeListener(JSContextRef context,
        JSObjectRef object,
        JSObjectRef thisObject,
        size_t argumentCount,
        const JSValueRef arguments[],
        JSValueRef* exception)
{
    SET_TIME_TRACER_ITEM(0);
    LOGD("Entered");

    try {
        std::shared_ptr<MessageStorage> priv = getPrivateObject(context,
                thisObject);
        TIZEN_CHECK_ACCESS(context, exception, priv.get(),
                MESSAGING_FUNCTION_API_ADD_CONVERSATIONS_CHANGE_LISTNER);
        // long addConversationsChangeListener(
        //         MessageConversationsChangeCallback conversationsChangeCallback,
        //         optional AbstractFilter? filter)
        ArgumentValidator validator(context, argumentCount, arguments);
        JSObjectRef js_callbacks_obj = validator.toCallbackObject(0,false,
                CONVERSATIONSADDED, CONVERSATIONSUPDATED, CONVERSATIONSREMOVED,
                NULL);

        JSObjectRef on_added_obj = getFunctionFromCallbackObj(context,
                js_callbacks_obj, CONVERSATIONSADDED);
        JSObjectRef on_updateded_obj = getFunctionFromCallbackObj(context,
                js_callbacks_obj, CONVERSATIONSUPDATED);
        JSObjectRef on_removed_obj = getFunctionFromCallbackObj(context,
                js_callbacks_obj, CONVERSATIONSREMOVED);

        JSContextRef global_ctx =
                GlobalContextManager::getInstance()->getGlobalContext(context);

        auto callback = std::make_shared<ConversationsChangeCallback>(
                global_ctx,
                on_added_obj,
                on_updateded_obj,
                on_removed_obj,
                priv->getMsgServiceId(),
                priv->getMsgServiceType());

        JSObjectRef filterobj = validator.toObject(1, true);
        if (filterobj != NULL) {
            DeviceAPI::Tizen::AbstractFilterPtr filter = Tizen::AbstractFilter::
                    getPrivateObject(context, validator.toJSValueRef(1));
            if (!filter) {
                LOGE("Wrong filter");
                throw Common::TypeMismatchException("Invalid filter");
            }
            LOGD("filter is set");
            callback->setFilter(filter);
        }

        return JSValueMakeNumber(context,
                priv->addConversationsChangeListener(callback));
    }
    catch (const WrtDeviceApis::Commons::Exception& exc) {
        LOGE("Wrong filer mode: %s", exc.GetMessage().c_str());
        Common::TypeMismatchException err(exc.GetMessage().c_str());
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }
    catch (const BasePlatformException& err) {
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }
    catch (...) {
        Common::UnknownException err("Cannot add listener for conversations");
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }

}


JSValueRef JSMessageStorage::addFoldersChangeListener(JSContextRef context,
        JSObjectRef object,
        JSObjectRef thisObject,
        size_t argumentCount,
        const JSValueRef arguments[],
        JSValueRef* exception)
{
    SET_TIME_TRACER_ITEM(0);
    LOGD("Entered");

    try {
        std::shared_ptr<MessageStorage> priv = getPrivateObject(context,
                thisObject);
        TIZEN_CHECK_ACCESS(context, exception, priv.get(),
                MESSAGING_FUNCTION_API_ADD_FOLDERS_CHANGE_LISTNER);
        // long addFoldersChangeListener(
        //         MessageFoldersChangeCallback foldersChangeCallback,
        //         optional AbstractFilter? filter);
        ArgumentValidator validator(context, argumentCount, arguments);
        JSObjectRef js_callbacks_obj = validator.toCallbackObject(0, false,
                FOLDERSADDED, FOLDERSUPDATED, FOLDERSREMOVED, NULL);

        JSObjectRef on_added_obj = getFunctionFromCallbackObj(context,
                js_callbacks_obj, FOLDERSADDED);
        JSObjectRef on_updateded_obj = getFunctionFromCallbackObj(context,
                js_callbacks_obj, FOLDERSUPDATED);
        JSObjectRef on_removed_obj = getFunctionFromCallbackObj(context,
                js_callbacks_obj, FOLDERSREMOVED);

        JSContextRef global_ctx =
                GlobalContextManager::getInstance()->getGlobalContext(context);

        auto callback = std::make_shared<FoldersChangeCallback>(
                global_ctx,
                on_added_obj,
                on_updateded_obj,
                on_removed_obj,
                priv->getMsgServiceId(),
                priv->getMsgServiceType());

        JSObjectRef filterobj = validator.toObject(1, true);
        if (filterobj != NULL) {
            DeviceAPI::Tizen::AbstractFilterPtr filter = Tizen::AbstractFilter::
                    getPrivateObject(context, validator.toJSValueRef(1));
            if (!filter) {
                LOGE("Wrong filter");
                throw Common::TypeMismatchException("Invalid filter");
            }
            LOGD("filter is set");
            callback->setFilter(filter);
        }

        return JSValueMakeNumber(context, priv->addFoldersChangeListener(callback));
    }
    catch (const WrtDeviceApis::Commons::Exception& exc) {
        LOGE("Wrong filer mode: %s", exc.GetMessage().c_str());
        Common::TypeMismatchException err(exc.GetMessage().c_str());
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }
    catch (const BasePlatformException& err) {
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }
    catch (...) {
        Common::UnknownException err("Cannot add listener for folders");
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }
}

JSValueRef JSMessageStorage::removeChangeListener(JSContextRef context,
        JSObjectRef function,
        JSObjectRef thisObject,
        size_t argumentCount,
        const JSValueRef arguments[],
        JSValueRef* exception)
{
    SET_TIME_TRACER_ITEM(0);
    LOGD("Entered");

    try {
        std::shared_ptr<MessageStorage> priv = getPrivateObject(context,
                thisObject);
        TIZEN_CHECK_ACCESS(context, exception, priv.get(),
                MESSAGING_FUNCTION_API_REMOVE_CHANGE_LISTENER);
        // void removeChangeListener(long watchId);
        ArgumentValidator validator(context, argumentCount, arguments);
        long watchId = validator.toLong(0);

        priv->removeChangeListener(context, watchId);
    }
    catch (const BasePlatformException& err) {
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }
    catch (...) {
        Common::UnknownException err("Cannot add listener for folders");
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }

    return JSValueMakeUndefined(context);
}

JSObjectRef JSMessageStorage::getFunctionFromCallbackObj(JSContextRef context,
        JSObjectRef obj,
        const char* name)
{
    JSObjectRef function_obj = NULL;
    JSStringRef propertyName = JSStringCreateWithUTF8CString(name);
    bool has = JSObjectHasProperty(context, obj, propertyName);
    JSStringRelease(propertyName);
    if (has) {
        JSValueRef value = JSUtil::getProperty(context, obj, name);
        function_obj = JSUtil::JSValueToObject(context, value);
    }
    return function_obj;
}

} //Messaging
} //DeviceAPI

