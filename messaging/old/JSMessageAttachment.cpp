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

#include <SecurityExceptions.h>

#include <ArgumentValidator.h>
#include <Export.h>
#include <Logger.h>

#include "JSMessageAttachment.h"
#include "MessageAttachment.h"

#include "plugin_config.h"

#include <JSUtil.h>

using namespace DeviceAPI::Common;

namespace DeviceAPI {
namespace Messaging {

namespace {
const char* MESSAGE_ATTACHMENT = "MessageAttachment";

const char* MESSAGE_ATTACHMENT_ID = "id";
const char* MESSAGE_ATTACHMENT_MSG_ID = "messageId";
const char* MESSAGE_ATTACHMENT_MIME_TYPE = "mimeType";
const char* MESSAGE_ATTACHMENT_FILE_PATH = "filePath";
}

JSClassRef JSMessageAttachment::m_jsClassRef = NULL;

JSClassDefinition JSMessageAttachment::m_classInfo = {
    0,
    kJSClassAttributeNone,
    MESSAGE_ATTACHMENT,
    NULL,
    JSMessageAttachment::m_property,
    NULL,
    JSMessageAttachment::initialize,
    JSMessageAttachment::finalize,
    NULL, //hasProperty,
    NULL, //getProperty,
    NULL, //setProperty,
    NULL, //deleteProperty,
    NULL, //getPropertyNames,
    NULL, //callAsFunction,
    NULL, //callAsConstructor,
    NULL, //hasInstance,
    NULL, //convertToType,
};

JSStaticValue JSMessageAttachment::m_property[] = {
    { MESSAGE_ATTACHMENT_ID, getId, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { MESSAGE_ATTACHMENT_MSG_ID, getMsgId, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { MESSAGE_ATTACHMENT_MIME_TYPE, getMimeType, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { MESSAGE_ATTACHMENT_FILE_PATH, getFilePath, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { 0, 0, 0, 0 }
};

const JSClassDefinition* JSMessageAttachment::getClassInfo()
{
    LOGD("Entered");
    return &(m_classInfo);
}

JSClassRef DLL_EXPORT JSMessageAttachment::getClassRef()
{
    LOGD("Entered");
    if (!m_jsClassRef) {
        m_jsClassRef = JSClassCreate(&m_classInfo);
    }
    return m_jsClassRef;
}

void JSMessageAttachment::initialize(JSContextRef context,
        JSObjectRef object)
{
    LOGD("Entered");
}

void JSMessageAttachment::finalize(JSObjectRef object)
{
    LOGD("Entered");
    // Below holder is fetched from JSObject because holder
    // with last shared_ptr instace should be removed
    MessageAttachmentHolder* priv =
            static_cast<MessageAttachmentHolder*>(JSObjectGetPrivate(object));
    JSObjectSetPrivate(object, NULL);
    delete priv;
}

std::shared_ptr<MessageAttachment> JSMessageAttachment::getPrivateObject(
        JSContextRef context,
        JSValueRef value)
{
    if (!JSValueIsObjectOfClass(context, value, getClassRef())) {
        LOGE("Object type do not match");
        throw TypeMismatchException("Object type is not MessageAttachment");
    }

    JSObjectRef object = JSUtil::JSValueToObject(context, value);
    MessageAttachmentHolder* priv = static_cast<MessageAttachmentHolder*>(
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

void JSMessageAttachment::setPrivateObject(JSObjectRef object,
        std::shared_ptr<MessageAttachment> data)
{
    if (!data) {
        LOGE("NULL shared pointer given to set as private data");
        throw UnknownException("NULL private data given");
    }
    MessageAttachmentHolder* priv = static_cast<MessageAttachmentHolder*>(
            JSObjectGetPrivate(object));
    if (priv) {
        priv->ptr = data;
    }
    else {
        priv = new(std::nothrow) MessageAttachmentHolder();
        if (!priv) {
            LOGE("Memory allocation failure");
            throw UnknownException("Failed to allocate memory");
        }
        priv->ptr = data;
        if(!JSObjectSetPrivate(object, static_cast<void*>(priv))) {
            delete priv;
            priv = NULL;
            LOGE("Failed to set private data in MessageAttachment");
            throw UnknownException("Failed to set MessageAttachment private data");
        }
    }
}

JSObjectRef JSMessageAttachment::makeJSObject(JSContextRef context,
        std::shared_ptr<MessageAttachment> native)
{
    if (!native) {
        LOGE("NULL pointer to attachment given");
        throw UnknownException("NULL pointer to attachment given");
    }

    MessageAttachmentHolder* priv = new(std::nothrow) MessageAttachmentHolder();
    if (!priv) {
        LOGW("Failed to allocate memory for AttachmentHolder");
        throw UnknownException("Priv is null");
    }
    priv->ptr = native;

    JSObjectRef obj = JSObjectMake(context, getClassRef(), NULL);
    if(!JSObjectSetPrivate(obj, static_cast<void*>(priv))) {
        LOGE("Failed to set private in MessageAttachment");
        throw UnknownException("Private data not set");
    }
    return obj;
}

JSObjectRef JSMessageAttachment::attachmentVectorToJSObjectArray(
        JSContextRef context, const AttachmentPtrVector& atts)
{
    size_t count = atts.size();

    JSObjectRef array[count];
    for (size_t i = 0; i < count; i++) {
        array[i] = JSMessageAttachment::makeJSObject(context, atts[i]);
    }
    JSObjectRef result = JSObjectMakeArray(context, count,
            count > 0 ? array : NULL, NULL);
    if (!result) {
        LOGW("Failed to create MessageAttachment array");
        throw UnknownException("MessageAttachment array is null");
    }
    return result;
}

JSObjectRef DLL_EXPORT JSMessageAttachment::constructor(JSContextRef context,
            JSObjectRef constructor,
            size_t argumentCount,
            const JSValueRef arguments[],
            JSValueRef* exception)
{
    LOGD("Entered");

    ArgumentValidator validator(context, argumentCount, arguments);

    JSObjectRef jsObjRef = JSObjectMake(context, JSMessageAttachment::getClassRef(), NULL);

    // constructor
    JSStringRef ctorName = JSStringCreateWithUTF8CString("constructor");
    JSObjectSetProperty(context, jsObjRef, ctorName, constructor,
        kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete | kJSPropertyAttributeDontEnum, NULL);
    JSStringRelease(ctorName);

    try {
        std::string filePath = validator.toString(0);
        LOGD(" filePath: %s", filePath.c_str());

        auto priv = std::shared_ptr<MessageAttachment>(new (std::nothrow)
                MessageAttachment());
        if(!priv) {
            LOGE("Failed to allocate memory for private data");
            throw UnknownException("Memory allocation failuer");
        }

        priv->setFilePath(filePath);
        if (!validator.isNull(1) && !validator.isOmitted(1)) {
            std::string mimeType = validator.toString(1, true);
            LOGD(" mimeType: %s", mimeType.c_str());
            priv->setMimeType(mimeType);
        }

        JSMessageAttachment::setPrivateObject(jsObjRef, priv);
    }
    catch (BasePlatformException &err) {
        LOGE("MessageAttachment creation failed: %s", err.getMessage().c_str());
    }
    catch (...) {
        LOGE("MessageAttachment creation failed: Unknown exception.");
    }

    return jsObjRef;
}

JSValueRef JSMessageAttachment::getId(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception)
{
    LOGD("Entered");

    try {
        auto priv = JSMessageAttachment::getPrivateObject(context, object);

        if (priv->isIdSet()) {
            std::string stringid = std::to_string(priv->getId());
            return JSUtil::toJSValueRef(context, stringid);
        }else {
            return JSValueMakeNull(context);
        }
    }
    catch (const BasePlatformException &err) {
        LOGE("%s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    catch (...) {
        LOGE("getMessageAttachmentId - unknown exception.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageAttachment::getMsgId(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = JSMessageAttachment::getPrivateObject(context, object);

        if (priv->isMessageIdSet()) {
            std::string stringid = std::to_string(priv->getMessageId());
            return JSUtil::toJSValueRef(context, stringid);
        } else {
            return JSValueMakeNull(context);
        }
    }
    catch (const BasePlatformException &err) {
        LOGE("%s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    catch (...) {
        LOGE("getMessageAttachmentMsgId - unknown exception.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageAttachment::getMimeType(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = JSMessageAttachment::getPrivateObject(context, object);

        if (priv->isMimeTypeSet()) {
            return JSUtil::toJSValueRef(context, priv->getMimeType());
        } else {
            return JSValueMakeNull(context);
        }
    }
    catch (const BasePlatformException &err) {
        LOGE("%s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    catch (...) {
        LOGE("getMessageAttachmentMimeType - unknown exception.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageAttachment::getFilePath(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = JSMessageAttachment::getPrivateObject(context, object);

        if (priv->isFilePathSet() && priv->isSaved()) {
            return JSUtil::toJSValueRef(context, priv->getFilePath());
        } else {
            return JSValueMakeNull(context);
        }

    }
    catch (const BasePlatformException &err) {
        LOGE("%s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    catch (...) {
        LOGE("getMessageAttachmentFilePath - unknown exception.");
    }
    return JSValueMakeUndefined(context);
}

} //Messaging
} //DeviceAPI

