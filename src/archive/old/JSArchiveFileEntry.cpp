//
// Tizen Web Device API
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include <ArgumentValidator.h>
#include <Export.h>
#include <GlobalContextManager.h>
#include <JSFile.h>
#include <File.h>
#include <Path.h>
#include <FilesystemExternalUtils.h>
#include <JSUtil.h>
#include <JSWebAPIErrorFactory.h>
#include <Logger.h>
#include <SecurityExceptions.h>

#include "ArchiveCallbackData.h"
#include "ArchiveFile.h"
#include "JSArchiveFileEntry.h"
#include "ArchiveUtils.h"
#include "plugin_config.h"

namespace DeviceAPI {
namespace Archive {

using namespace WrtDeviceApis::Commons;
using namespace DeviceAPI::Common;
using namespace DeviceAPI::Filesystem;

namespace {
const char* ARCHIVE_FILE_ENTRY = "ArchiveFileEntry";
const char* ARCHIVE_FILE_ENTRY_NAME = "name";
const char* ARCHIVE_FILE_ENTRY_SIZE = "size";
const char* ARCHIVE_FILE_ENTRY_COMPRESSED_SIZE = "compressedSize";
const char* ARCHIVE_FILE_ENTRY_MODIFIED = "modified";
}

const char* ARCHIVE_FILE_ENTRY_OPT_DEST = "destination";
const char* ARCHIVE_FILE_ENTRY_OPT_STRIP = "stripSourceDirectory";
const char* ARCHIVE_FILE_ENTRY_OPT_COMPRESSION_LEVEL = "compressionLevel";

struct ArchiveFileEntryHolder{
    ArchiveFileEntryPtr ptr;
};

JSClassDefinition JSArchiveFileEntry::m_classInfo = {
        0,                      // version
        kJSClassAttributeNone,  // attributes
        ARCHIVE_FILE_ENTRY,     // class name
        NULL,                   // parent class
        m_property,             // static values
        m_function,             // static functions
        initialize,             // initialize
        finalize,               // finalize
        NULL,                   // hasProperty
        NULL,                   // getProperty
        NULL,                   // setProperty
        NULL,                   // deleteProperty
        NULL,                   // getPropertyNames
        NULL,                   // callAsFunctionvalidator
        NULL,                   // constructor
        NULL,                   // hasInstance
        NULL                    // convertToType
};

const JSClassDefinition* JSArchiveFileEntry::getClassInfo()
{
    return &m_classInfo;
}

JSStaticFunction JSArchiveFileEntry::m_function[] = {
        { ARCHIVE_FUNCTION_API_ARCHIVE_FILE_ENTRY_EXTRACT, extract, kJSPropertyAttributeNone },
        { 0, 0, 0 }
};

JSStaticValue JSArchiveFileEntry::m_property[] =
{
    {ARCHIVE_FILE_ENTRY_NAME, getName, NULL,
            kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
    {ARCHIVE_FILE_ENTRY_SIZE, getSize, NULL,
            kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
    {ARCHIVE_FILE_ENTRY_COMPRESSED_SIZE, getCompressedSize, NULL,
            kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
    {ARCHIVE_FILE_ENTRY_MODIFIED, getModified, NULL,
            kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
    { 0, 0, 0, 0 }
};

JSClassRef JSArchiveFileEntry::m_jsClassRef = JSClassCreate(JSArchiveFileEntry::getClassInfo());

const JSClassRef DLL_EXPORT JSArchiveFileEntry::getClassRef()
{
    if (!m_jsClassRef) {
        m_jsClassRef = JSClassCreate(&m_classInfo);
    }
    return m_jsClassRef;
}

void JSArchiveFileEntry::initialize(JSContextRef context, JSObjectRef object)
{
    LOGD("Entered");
}

void JSArchiveFileEntry::finalize(JSObjectRef object)
{
    LOGD("Entered");
    ArchiveFileEntryHolder* priv =
            static_cast<ArchiveFileEntryHolder*>(JSObjectGetPrivate(object));
    if (priv) {
        JSObjectSetPrivate(object, NULL);
        delete priv;
        priv = NULL;
    }
}

ArchiveFileEntryPtr JSArchiveFileEntry::getPrivateObject(JSContextRef context,
        JSValueRef value_ref)
{
    if (!JSValueIsObjectOfClass(context, value_ref, getClassRef())) {
        LOGE("TypeMismatch");
        throw TypeMismatchException("TypeMismatch");
    }

    JSObjectRef object = JSUtil::JSValueToObject(context, value_ref);
    ArchiveFileEntryHolder* priv = static_cast<ArchiveFileEntryHolder*>(JSObjectGetPrivate(object));
    if (!priv) {
        LOGE("Priv is null");
        throw UnknownException("Priv is null");
    }

    if (!(priv->ptr)) {
        LOGE("Native is null");
        throw UnknownException("Native is null");
    }

    return priv->ptr;
}

JSObjectRef JSArchiveFileEntry::makeJSObject(JSContextRef context,
        ArchiveFileEntryPtr native)
{
    if (!native) {
        LOGE("Native is null");
        throw TypeMismatchException("Native is null");
    }

    ArchiveFileEntryHolder* priv = new ArchiveFileEntryHolder;
    if(!priv){
        LOGE("Priv is null");
        throw UnknownException("Priv is null");
    }
    priv->ptr = native;

    JSObjectRef obj = JSObjectMake(context,
            getClassRef(), static_cast<void*>(priv));
    return obj;
}

JSValueRef JSArchiveFileEntry::getName(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);
        return JSUtil::toJSValueRef(context, priv->getName());
    }
    catch (const BasePlatformException &error) {
        LOGE("Attribute get failed: %s", error.getMessage().c_str());
    }
    catch (...) {
        LOGE("Attribute get failed");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSArchiveFileEntry::getSize(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);
        return JSUtil::toJSValueRef(context, priv->getSize());
    }
    catch (const BasePlatformException &error) {
        LOGE("Attribute get failed: %s", error.getMessage().c_str());
    }
    catch (...) {
        LOGE("Attribute get failed");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSArchiveFileEntry::getCompressedSize(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);
        return JSUtil::toJSValueRef(context, priv->getCompressedSize());
    }
    catch (const BasePlatformException &error) {
        LOGE("Attribute get failed: %s", error.getMessage().c_str());
    }
    catch (...) {
        LOGE("Attribute get failed");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSArchiveFileEntry::getModified(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);
        return JSUtil::makeDateObject(context, priv->getModified());
    }
    catch (const BasePlatformException &error) {
        LOGE("Attribute get failed: %s", error.getMessage().c_str());
    }
    catch (...) {
        LOGE("Attribute get failed");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSArchiveFileEntry::extract(JSContextRef context,
        JSObjectRef object,
        JSObjectRef thisObject,
        size_t argumentCount,
        const JSValueRef arguments[],
        JSValueRef* exception)
{
    LOGD("Entered");

    ACE_ACCESS_CHECK(
        AceSecurityStatus status =
                ARCHIVE_CHECK_ACCESS(ARCHIVE_FUNCTION_API_ARCHIVE_FILE_ENTRY_EXTRACT);
        TIZEN_SYNC_ACCESS_HANDLER(status, context, exception);
    );

    ExtractEntryProgressCallback *callback = NULL;

    try {
        ArchiveFileEntryPtr priv = JSArchiveFileEntry::getPrivateObject(context, thisObject);
        ArgumentValidator validator(context, argumentCount, arguments);

        callback = new(std::nothrow) ExtractEntryProgressCallback(
                GlobalContextManager::getInstance()->getGlobalContext(context));
        if(!callback) {
            LOGD("Couldn't allocate ExtractEntryProgressCallback");
            throw UnknownException("Memory allocation error");
        }

        callback->setSuccessCallback(validator.toFunction(1, true));
        callback->setErrorCallback(validator.toFunction(2, true));
        callback->setProgressCallback(validator.toFunction(3, true));
        callback->setStripName(validator.toBool(4,true));
        callback->setOverwrite(validator.toBool(5,true));

        if (validator.isOmitted(0)) {
            LOGE("FileReference not given");
            throw TypeMismatchException("Missing argument");
        }

        FilePtr file_ptr = fileReferenceToFile(context, arguments[0]);
        callback->setDirectory(file_ptr);

        long op_id = priv->extractTo(callback);
        LOGD("Run extract request with op_id: %d", op_id);

        return JSUtil::toJSValueRef(context, op_id);
    }
    catch (const NotFoundException &nfe) {
        callback->setError(nfe.getName(), nfe.getMessage());
    }
    catch (const IOException &ioe) {
        callback->setError(ioe.getName(), ioe.getMessage());
    }
    catch (const BasePlatformException &err) {
        LOGE("extract caught: name: %s, msg: %s",
                err.getName().c_str(), err.getMessage().c_str());
        delete callback;
        callback = NULL;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }
    catch (...) {
        LOGE("extract fails");
        delete callback;
        callback = NULL;
        return JSWebAPIErrorFactory::postException(context, exception,
                JSWebAPIErrorFactory::UNKNOWN_ERROR, "extract fails");
    }

    if (callback && callback->isError()) {
        JSObjectRef errobj = JSWebAPIErrorFactory::makeErrorObject(
                context,
                callback->getErrorName(),
                callback->getErrorMessage());
        callback->callErrorCallback(errobj);
        delete callback;
        callback = NULL;
    }

    return JSValueMakeUndefined(context);
}



} // Archive
} // DeviceAPI
