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

/**
 * @file        JSMessageFolder.cpp
 */


#include <JSUtil.h>
#include <SecurityExceptions.h>

#include <Export.h>
#include <Logger.h>

#include "JSMessageFolder.h"
#include "MessageFolder.h"
#include "MessagingUtil.h"
#include "MessageFolder.h"

#include "plugin_config.h"

using namespace DeviceAPI::Common;

namespace DeviceAPI {
namespace Messaging {

namespace {
const char* MESSAGE_FOLDER = "MessageFolder";

const char* MESSAGE_FOLDER_ID = "id";
const char* MESSAGE_FOLDER_PARENT_ID = "parentId";
const char* MESSAGE_FOLDER_SERVICE_ID = "serviceId";
const char* MESSAGE_FOLDER_CONTENT_TYPE = "contentType";
const char* MESSAGE_FOLDER_NAME = "name";
const char* MESSAGE_FOLDER_PATH = "path";
const char* MESSAGE_FOLDER_TYPE = "type";
const char* MESSAGE_FOLDER_SYNCHRONIZABLE = "synchronizable";
}

JSClassRef JSMessageFolder::m_jsClassRef = NULL;

JSClassDefinition JSMessageFolder::m_classInfo = {
    0,
    kJSClassAttributeNone,
    MESSAGE_FOLDER,
    NULL,
    JSMessageFolder::m_property,
    NULL,
    JSMessageFolder::initialize,
    JSMessageFolder::finalize,
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

JSStaticValue JSMessageFolder::m_property[] = {
    { MESSAGE_FOLDER_ID, getId, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { MESSAGE_FOLDER_PARENT_ID, getParentId, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { MESSAGE_FOLDER_SERVICE_ID, getServiceId, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { MESSAGE_FOLDER_CONTENT_TYPE, getContentType, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { MESSAGE_FOLDER_NAME, getName, setName, kJSPropertyAttributeDontDelete },
    { MESSAGE_FOLDER_PATH, getPath, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { MESSAGE_FOLDER_TYPE, getType, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { MESSAGE_FOLDER_SYNCHRONIZABLE, getSynchronizable, setSynchronizable, kJSPropertyAttributeDontDelete },
    { 0, 0, 0, 0 }
};

JSClassRef JSMessageFolder::getClassRef() {
    LOGD("Entered");
    if (!m_jsClassRef) {
        m_jsClassRef = JSClassCreate(&m_classInfo);
    }
    return m_jsClassRef;
}

const JSClassDefinition* JSMessageFolder::getClassInfo() {
    LOGD("Entered");
    return &(m_classInfo);
}

void JSMessageFolder::initialize(JSContextRef context,
        JSObjectRef object)
{
    LOGD("Entered");
}

void JSMessageFolder::finalize(JSObjectRef object)
{
    LOGD("Entered");
    MessageFolderHolder* priv = static_cast<MessageFolderHolder*>(JSObjectGetPrivate(object));
    if(priv)
    {
        JSObjectSetPrivate(object,NULL);
        delete priv;
        priv = NULL;
    }
}

std::shared_ptr<MessageFolder> JSMessageFolder::getPrivateObject(JSContextRef context,
        JSValueRef value)
{
    if(!JSValueIsObjectOfClass(context, value, getClassRef())) {
        LOGW("Type mismatch");
        throw TypeMismatchException("Type mismatch");
    }

    JSObjectRef object = JSUtil::JSValueToObject(context, value);

    MessageFolderHolder* priv = static_cast<MessageFolderHolder*>(
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

void JSMessageFolder::setPrivateObject(JSObjectRef object, std::shared_ptr<MessageFolder> data)
{
    if (!data) {
        LOGE("NULL shared pointer given to set as private data");
        throw UnknownException("NULL private data given");
    }
    MessageFolderHolder* priv = static_cast<MessageFolderHolder*>(
            JSObjectGetPrivate(object));
    if (priv) {
        priv->ptr = data;
    }
    else {
        priv = new(std::nothrow) MessageFolderHolder();
        if (!priv) {
            LOGE("Memory allocation failure");
            throw UnknownException("Failed to allocate memory");
        }
        priv->ptr = data;
        if(!JSObjectSetPrivate(object, static_cast<void*>(priv))) {
            delete priv;
            priv = NULL;
            LOGE("Failed to set private data in MessageFolder");
            throw UnknownException(
                    "Failed to set MessageFolder private data");
        }
    }
}

JSObjectRef JSMessageFolder::makeJSObject(JSContextRef context,
        std::shared_ptr<MessageFolder> ptr)
{
    if (!ptr) {
        LOGE("NULL pointer to message folder given");
        throw UnknownException("NULL pointer to message folder given");
    }

    MessageFolderHolder* priv = new(std::nothrow) MessageFolderHolder();
    if (!priv) {
        LOGW("Failed to allocate memory for MessageFolderHolder");
        throw UnknownException("Priv is null");
    }
    priv->ptr = ptr;

    JSObjectRef obj = JSObjectMake(context, getClassRef(), NULL);
    if(!JSObjectSetPrivate(obj, static_cast<void*>(priv))) {
        LOGE("Failed to set private in MessageFolder");
        throw UnknownException("Private data not set");
    }
    return obj;
}

JSValueRef JSMessageFolder::getId(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);
        return JSUtil::toJSValueRef(context, priv->getId());
    }
    catch (const BasePlatformException& err) {
        LOGE("Failed to get message folder id. %s : %s", err.getName().c_str(),
                err.getMessage().c_str());
    }
    catch (...) {
        LOGE("Unsupported error while getting message folder id.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageFolder::getParentId(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);
        if (priv->isParentIdSet()) {
            return JSUtil::toJSValueRef(context, priv->getParentId());
        } else {
            return JSValueMakeNull(context);
        }
    }
    catch (const BasePlatformException& err) {
        LOGE("Failed to get message folder parent id. %s : %s", err.getName().c_str(),
                err.getMessage().c_str());
    }
    catch (...) {
        LOGE("Unsupported error while getting message folder parent id.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageFolder::getServiceId(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);
        return JSUtil::toJSValueRef(context, priv->getServiceId());
    }
    catch (const BasePlatformException& err) {
        LOGE("Failed to get message folder service id. %s : %s", err.getName().c_str(),
                err.getMessage().c_str());
    }
    catch (...) {
        LOGE("Unsupported error while getting message folder service id.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageFolder::getContentType(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);
        return JSUtil::toJSValueRef(context, priv->getContentType());
    }
    catch (const BasePlatformException& err) {
        LOGE("Failed to get message folder content type. %s : %s", err.getName().c_str(),
                err.getMessage().c_str());
    }
    catch (...) {
        LOGE("Unsupported error while getting message folder content type.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageFolder::getName(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);
        return JSUtil::toJSValueRef(context, priv->getName());
    }
    catch (const BasePlatformException& err) {
        LOGE("Failed to get message folder name. %s : %s", err.getName().c_str(),
                err.getMessage().c_str());
    }
    catch (...) {
        LOGE("Unsupported error while getting message folder name.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageFolder::getPath(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);
        return JSUtil::toJSValueRef(context, priv->getPath());
    }
    catch (const BasePlatformException& err) {
        LOGE("Failed to get message folder path. %s : %s", err.getName().c_str(),
                err.getMessage().c_str());
    }
    catch (...) {
        LOGE("Unsupported error while getting message folder path.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageFolder::getType(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);
        return JSUtil::toJSValueRef(context,
                MessagingUtil::messageFolderTypeToString(priv->getType()));
    }
    catch (const BasePlatformException& err) {
        LOGE("Failed to get message folder type. %s : %s", err.getName().c_str(),
                err.getMessage().c_str());
    }
    catch (...) {
        LOGE("Unsupported error while getting message folder type.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessageFolder::getSynchronizable(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);
        return JSUtil::toJSValueRef(context, priv->getSynchronizable());
    }
    catch (const BasePlatformException& err) {
        LOGE("Failed to get message folder is synchronizable flag. %s : %s",
                err.getName().c_str(), err.getMessage().c_str());
    }
    return JSValueMakeUndefined(context);
}

bool JSMessageFolder::setName(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef value,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);
        std::string c_value = JSUtil::JSValueToString(context, value);
        priv->setName(c_value);
    }
    catch (const BasePlatformException& err) {
        LOGE("Failed to set message folder name. %s : %s", err.getName().c_str(),
                err.getMessage().c_str());
    }
    catch (...) {
        LOGE("Unsupported error while setting message folder name.");
    }
    return true;
}

bool JSMessageFolder::setSynchronizable(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef value,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);
        bool c_value = JSUtil::JSValueToBoolean(context, value);
        priv->setSynchronizable(c_value);
    }
    catch (const BasePlatformException& err) {
        LOGE("Failed to set message folder is synchronizable flag. %s : %s",
                err.getName().c_str(), err.getMessage().c_str());
    }
    catch(...) {
        LOGE("Unsupported error while setting message folder is synchronizable flag.");
    }
    return true;
}

} //Messaging
} //DeviceAPI

