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
#include <PlatformException.h>
#include <JSUtil.h>

#include <GlobalContextManager.h>
#include <ArgumentValidator.h>
#include <Export.h>
#include <Logger.h>

#include "JSMessageConversation.h"
#include "MessageConversation.h"

#include "plugin_config.h"

namespace DeviceAPI {
namespace Messaging {

using namespace DeviceAPI::Common;
using namespace WrtDeviceApis::Commons;

namespace {
const char* MESSAGE_CONVERSATION = "MessageConversation";
}

namespace JSMessageConversationKeys {
const char* MESSAGE_CONVERSATION_ID = "id";
const char* MESSAGE_CONVERSATION_TYPE = "type";
const char* MESSAGE_CONVERSATION_TIMESTAMP = "timestamp";
const char* MESSAGE_CONVERSATION_MSG_COUNT = "messageCount";
const char* MESSAGE_CONVERSATION_UNREAD_MSG = "unreadMessages";
const char* MESSAGE_CONVERSATION_PREVIEW = "preview";
const char* MESSAGE_CONVERSATION_SUBJECT = "subject";
const char* MESSAGE_CONVERSATION_IS_READ = "isRead";
const char* MESSAGE_CONVERSATION_FROM = "from";
const char* MESSAGE_CONVERSATION_TO = "to";
const char* MESSAGE_CONVERSATION_CC = "cc";
const char* MESSAGE_CONVERSATION_BCC = "bcc";
const char* MESSAGE_CONVERSATION_LAST_MSG_ID = "lastMessageId";
}

using namespace JSMessageConversationKeys;

JSClassRef JSMessageConversation::m_jsClassRef = NULL;

JSClassDefinition JSMessageConversation::m_classInfo =
{
    0,
    kJSClassAttributeNone,
    MESSAGE_CONVERSATION,
    NULL,
    JSMessageConversation::m_property,
    NULL,
    JSMessageConversation::initialize,
    JSMessageConversation::finalize,
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

JSStaticValue JSMessageConversation::m_property[] = {
    { MESSAGE_CONVERSATION_ID, getId, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { MESSAGE_CONVERSATION_TYPE, getType, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { MESSAGE_CONVERSATION_TIMESTAMP, getTimestamp, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { MESSAGE_CONVERSATION_MSG_COUNT, getMsgCount, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { MESSAGE_CONVERSATION_UNREAD_MSG, getUnreadMsg, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { MESSAGE_CONVERSATION_PREVIEW, getPreview, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { MESSAGE_CONVERSATION_SUBJECT, getSubject, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { MESSAGE_CONVERSATION_IS_READ, getIsRead, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { MESSAGE_CONVERSATION_FROM, getFrom, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { MESSAGE_CONVERSATION_TO, getTo, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { MESSAGE_CONVERSATION_CC, getCC, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { MESSAGE_CONVERSATION_BCC, getBCC, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { MESSAGE_CONVERSATION_LAST_MSG_ID, getLastMsgId, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { 0, 0, 0, 0 }
};

JSClassRef DLL_EXPORT JSMessageConversation::getClassRef()
{
    LOGD("Entered");
    if (!m_jsClassRef) {
        m_jsClassRef = JSClassCreate(&m_classInfo);
    }
    return m_jsClassRef;
}

const JSClassDefinition* JSMessageConversation::getClassInfo()
{
    LOGD("Entered");
    return &m_classInfo;
}

void JSMessageConversation::initialize(JSContextRef context,
        JSObjectRef object)
{
    LOGD("Entered");
}

void JSMessageConversation::finalize(JSObjectRef object)
{
    LOGD("Entered");
    MessageConversationHolder* priv =
            static_cast<MessageConversationHolder*>(JSObjectGetPrivate(object));
    JSObjectSetPrivate(object,NULL);
    delete priv;
}

std::shared_ptr<MessageConversation> JSMessageConversation::getPrivateObject(
        JSContextRef context, JSValueRef value)
{
    if(!JSValueIsObjectOfClass(context, value, getClassRef())) {
        LOGW("Type mismatch");
        throw TypeMismatchException("Type mismatch");
    }

    JSObjectRef object = JSUtil::JSValueToObject(context, value);

    MessageConversationHolder* priv = static_cast<MessageConversationHolder*>(
            JSObjectGetPrivate(object));
    if (!priv) {
        LOGE("NULL private data");
        throw UnknownException("Private data is null");
    }
    if (!(priv->ptr)) {
        LOGE("NULL shared pointer in private data");
        throw UnknownException("Private data is null");
    }

    return priv->ptr;
}

void JSMessageConversation::setPrivateObject(JSObjectRef object, std::shared_ptr<MessageConversation> data)
{
    if (!data) {
        LOGE("NULL shared pointer given to set as private data");
        throw UnknownException("NULL private data given");
    }
    MessageConversationHolder* priv = static_cast<MessageConversationHolder*>(
            JSObjectGetPrivate(object));
    if (priv) {
        priv->ptr = data;
    }
    else {
        priv = new(std::nothrow) MessageConversationHolder();
        if (!priv) {
            LOGE("Memory allocation failure");
            throw UnknownException("Failed to allocate memory");
        }
        priv->ptr = data;
        if(!JSObjectSetPrivate(object, static_cast<void*>(priv))) {
            delete priv;
            priv = NULL;
            LOGE("Failed to set private data in MessageConversation");
            throw UnknownException(
                    "Failed to set MessageConversation private data");
        }
    }
}

JSObjectRef JSMessageConversation::makeJSObject(JSContextRef context,
        std::shared_ptr<MessageConversation> ptr)
{
    if (!ptr) {
        LOGE("NULL pointer to message conversation given");
        throw UnknownException("NULL pointer to message conversation given");
    }

    MessageConversationHolder* priv = new(std::nothrow) MessageConversationHolder();
    if (!priv) {
        LOGW("Failed to allocate memory for MessageConversationHolder");
        throw UnknownException("Priv is null");
    }
    priv->ptr = ptr;

    JSObjectRef obj = JSObjectMake(context, getClassRef(), NULL);
    if(!JSObjectSetPrivate(obj, static_cast<void*>(priv))) {
        LOGE("Failed to set private in MessageConversation");
        throw UnknownException("Private data not set");
    }
    return obj;
}

JSValueRef JSMessageConversation::getId(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);
        std::string stringid = std::to_string(priv->getConversationId());
        return JSUtil::toJSValueRef(context, stringid);
    }
    catch (const BasePlatformException &err) {
        LOGE("%s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageConversation::getType(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);
        return JSUtil::toJSValueRef(context, MessagingUtil::messageTypeToString(priv->getType()));
    }
    catch (const BasePlatformException &err) {
        LOGE("%s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    catch(...) {
        LOGE("Unsupported error while getting message conversation type.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageConversation::getTimestamp(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);
        return JSUtil::makeDateObject(context, priv->getTimestamp());
    }
    catch (const BasePlatformException &err) {
        LOGE("%s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageConversation::getMsgCount(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);
        return JSUtil::toJSValueRef(context, priv->getMessageCount());
    }
    catch (const BasePlatformException &err) {
        LOGE("%s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageConversation::getUnreadMsg(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);
        return JSUtil::toJSValueRef(context, priv->getUnreadMessages());
    }
    catch (const BasePlatformException &err) {
        LOGE("%s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageConversation::getPreview(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);
        return JSUtil::toJSValueRef(context, priv->getPreview());
    }
    catch (const BasePlatformException &err) {
        LOGE("%s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    catch(...) {
        LOGE("Unsupported error while getting message conversation preview.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageConversation::getSubject(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);
        return JSUtil::toJSValueRef(context, priv->getSubject());
    }
    catch (const BasePlatformException &err) {
        LOGE("%s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    catch(...) {
        LOGE("Unsupported error while getting message conversation subject.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageConversation::getIsRead(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);
        return JSUtil::toJSValueRef(context, priv->getIsRead());
    }
    catch (const BasePlatformException &err) {
        LOGE("%s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    catch(...) {
        LOGE("Unsupported error while getting message conversation isRead flag.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageConversation::getFrom(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);
        return JSUtil::toJSValueRef(context, priv->getFrom());
    }
    catch (const BasePlatformException &err) {
        LOGE("%s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    catch(...) {
        LOGE("Unsupported error while getting message conversation source address.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageConversation::getTo(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);
        return JSUtil::toJSValueRef(context, priv->getTo());
    }
    catch (const BasePlatformException &err) {
        LOGE("%s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    catch(...) {
        LOGE("Unsupported error while getting message conversation destination address.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageConversation::getCC(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);
        return JSUtil::toJSValueRef(context, priv->getCC());
    }
    catch (const BasePlatformException &err) {
        LOGE("%s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    catch(...) {
        LOGE("Unsupported error while getting CC.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageConversation::getBCC(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);
        return JSUtil::toJSValueRef(context, priv->getBCC());
    }
    catch (const BasePlatformException &err) {
        LOGE("%s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    catch(...) {
        LOGE("Unsupported error while getting BCC.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageConversation::getLastMsgId(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);
        std::string stringid = std::to_string(priv->getLastMessageId());
        return JSUtil::toJSValueRef(context, stringid);
    }
    catch (const BasePlatformException &err) {
        LOGE("%s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    catch(...) {
        LOGE("Unsupported error while getting last message id.");
    }
    return JSValueMakeUndefined(context);
}

} //Messaging
} //DeviceAPI
