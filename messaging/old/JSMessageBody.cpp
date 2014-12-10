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
#include <JSUtil.h>

#include <GlobalContextManager.h>
#include <ArgumentValidator.h>
#include <Export.h>
#include <Logger.h>

#include "JSMessageBody.h"
#include "MessageBody.h"
#include "JSMessageAttachment.h"

#include "plugin_config.h"

using namespace DeviceAPI::Common;

namespace DeviceAPI {
namespace Messaging {

namespace {
const char* MESSAGE_BODY = "MessageBody";

const char* MESSAGE_BODY_ID = "messageId";
const char* MESSAGE_BODY_LOADED = "loaded";
const char* MESSAGE_BODY_PLAIN_BODY = "plainBody";
const char* MESSAGE_BODY_HTML_BODY = "htmlBody";
const char* MESSAGE_BODY_INLINE_ATT = "inlineAttachments";
}

JSClassRef JSMessageBody::m_jsClassRef = NULL;

JSClassDefinition JSMessageBody::m_classInfo = {
    0,
    kJSClassAttributeNone,
    MESSAGE_BODY,
    NULL,
    JSMessageBody::m_property,
    NULL,
    JSMessageBody::initialize,
    JSMessageBody::finalize,
    NULL, //hasProperty,
    NULL, //getProperty
    NULL, //setProperty
    NULL, //deleteProperty,
    NULL, //getPropertyNames
    NULL, //callAsFunction,
    NULL, //callAsConstructor,
    NULL, //hasInstance,
    NULL, //convertToType,
};

JSStaticValue JSMessageBody::m_property[] = {
    { MESSAGE_BODY_ID, getId, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { MESSAGE_BODY_LOADED, getLoaded, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { MESSAGE_BODY_PLAIN_BODY, getPlainBody, setPlainBody, kJSPropertyAttributeDontDelete },
    { MESSAGE_BODY_HTML_BODY, getHTMLBody, setHTMLBody, kJSPropertyAttributeDontDelete },
    { MESSAGE_BODY_INLINE_ATT, getInlineAtt, setInlineAtt, kJSPropertyAttributeDontDelete },
    { 0, 0, 0, 0 }
};

const JSClassDefinition* JSMessageBody::getClassInfo()
{
    return &(m_classInfo);
}

JSClassRef JSMessageBody::getClassRef()
{
    if (!m_jsClassRef) {
        m_jsClassRef = JSClassCreate(&m_classInfo);
    }
    return m_jsClassRef;
}

void JSMessageBody::initialize(JSContextRef context,
        JSObjectRef object)
{
    LOGD("Entered");
}

void JSMessageBody::finalize(JSObjectRef object)
{
    LOGD("Entered");
    // Below holder is fetched and deleted from JSObject because last shared_ptr
    // instace should be removed
    MessageBodyHolder* priv = static_cast<MessageBodyHolder*>(
            JSObjectGetPrivate(object));
    JSObjectSetPrivate(object, NULL);
    delete priv;
}

std::shared_ptr<MessageBody> JSMessageBody::getPrivateObject(
        JSContextRef context,
        JSValueRef value)
{
    if (!JSValueIsObjectOfClass(context, value, getClassRef())) {
        LOGE("Object type do not match");
        throw TypeMismatchException("Object type is not MessageBody");
    }

    JSObjectRef object = JSUtil::JSValueToObject(context, value);
    MessageBodyHolder* priv = static_cast<MessageBodyHolder*>(
            JSObjectGetPrivate(object));
    if (!priv) {
        LOGE("NULL private data");
        throw UnknownException("Private data holder is null");
    }
    if (!(priv->ptr)) {
        LOGE("NULL shared pointer in private data");
        throw UnknownException("Private data is null");
    }

    return priv->ptr;
}

void JSMessageBody::setPrivateObject(JSObjectRef object,
        std::shared_ptr<MessageBody> data)
{
    if (!data) {
        LOGE("NULL shared pointer given to set as private data");
        throw UnknownException("NULL private data given");
    }
    MessageBodyHolder* priv = static_cast<MessageBodyHolder*>(
            JSObjectGetPrivate(object));
    if (priv) {
        priv->ptr = data;
    }
    else {
        priv = new(std::nothrow) MessageBodyHolder();
        if (!priv) {
            LOGE("Memory allocation failure");
            throw UnknownException("Failed to allocate memory");
        }
        priv->ptr = data;
        if(!JSObjectSetPrivate(object, static_cast<void*>(priv))) {
            delete priv;
            priv = NULL;
            LOGE("Failed to set private data in MessageBody");
            throw UnknownException(
                    "Failed to set MessageBody private data");
        }
    }
}

JSObjectRef JSMessageBody::makeJSObject(JSContextRef context,
        std::shared_ptr<MessageBody> ptr)
{
    if (!ptr) {
        LOGE("NULL pointer to message body given");
        throw UnknownException("NULL pointer to message body given");
    }

    MessageBodyHolder* priv = new(std::nothrow) MessageBodyHolder();
    if (!priv) {
        LOGW("Failed to allocate memory for MessageBodyHolder");
        throw UnknownException("Priv is null");
    }
    priv->ptr = ptr;

    JSObjectRef obj = JSObjectMake(context, getClassRef(), NULL);
    if(!JSObjectSetPrivate(obj, static_cast<void*>(priv))) {
        LOGE("Failed to set private in MessageBody");
        throw UnknownException("Private data not set");
    }
    return obj;
}

JSValueRef JSMessageBody::getId(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);

        if(priv->is_message_id_set()) {
            std::string stringid = std::to_string(priv->getMessageId());
            return JSUtil::toJSValueRef(context, stringid);
        }
        else {
            return JSValueMakeNull(context);
        }
    }
    catch (const BasePlatformException& err) {
        LOGE("%s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageBody::getLoaded(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);

        return JSUtil::toJSValueRef(context, priv->getLoaded());
    }
    catch (const BasePlatformException& err) {
        LOGE("%s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageBody::getPlainBody(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);

        return JSUtil::toJSValueRef(context, priv->getPlainBody());
    }
    catch (const BasePlatformException& err) {
        LOGE("%s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageBody::getHTMLBody(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);

        return JSUtil::toJSValueRef(context, priv->getHtmlBody());
    }
    catch (const BasePlatformException& err) {
        LOGE("%s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageBody::getInlineAtt(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);

        return priv->getJSInlineAttachments(
            Common::GlobalContextManager::getInstance()
                ->getGlobalContext(context));
    }
    catch (const BasePlatformException& err) {
        LOGE("%s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    return JSValueMakeUndefined(context);
}

bool JSMessageBody::setPlainBody(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef value,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);

        std::string c_value = JSUtil::JSValueToString(context, value);
        priv->setPlainBody(c_value);
    }
    catch (const BasePlatformException& err) {
        LOGE("%s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    return true;
}

bool JSMessageBody::setHTMLBody(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef value,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);

        std::string c_value = JSUtil::JSValueToString(context, value);
        priv->setHtmlBody(c_value);
    }
    catch (const BasePlatformException& err) {
        LOGE("%s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    return true;
}

bool JSMessageBody::setInlineAtt(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef value,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        if(!JSIsArrayValue(context,value)) {
            return true;
        }

        auto priv = getPrivateObject(context, object);

        AttachmentPtrVector atts;
        atts = JSUtil::JSArrayToType_<std::shared_ptr<MessageAttachment>>(
                context, value, JSMessageAttachment::getPrivateObject);
        priv->setInlineAttachments(atts);
    }
    catch (const BasePlatformException& err) {
        LOGE("%s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    return true;
}

} //Messaging
} //DeviceAPI

