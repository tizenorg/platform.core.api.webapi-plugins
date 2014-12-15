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
#include <JSWebAPIErrorFactory.h>
#include <ArgumentValidator.h>
#include <SecurityExceptions.h>
#include <GlobalContextManager.h>
#include <File.h>
#include <Path.h>
#include <JSUtil.h>
#include <FilesystemExternalUtils.h>
#include <JSFile.h>
#include <Export.h>

#include "JSArchiveFile.h"
#include "JSArchiveFileEntry.h"
#include "JSFile.h"
#include "ArchiveCallbackData.h"
#include "ArchiveUtils.h"
#include "plugin_config.h"
#include <Logger.h>

namespace DeviceAPI {
namespace Archive {

using namespace WrtDeviceApis::Commons;
using namespace DeviceAPI::Common;
using namespace DeviceAPI::Filesystem;

namespace {
const char* ARCHIVE_FILE = "ArchiveFile";
const char* ARCHIVE_FILE_MODE = "mode";
const char* ARCHIVE_FILE_DECOMPRESSED_SIZE = "decompressedSize";
}

JSClassDefinition JSArchiveFile::m_classInfo = {
        0,                      // version
        kJSClassAttributeNone,  // attributes
        ARCHIVE_FILE,           // class name
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
        NULL,
        NULL                    // convertToType
};

const JSClassDefinition* JSArchiveFile::getClassInfo()
{
    return &m_classInfo;
}

JSStaticFunction JSArchiveFile::m_function[] = {
        { ARCHIVE_FUNCTION_API_ARCHIVE_FILE_ADD, add, kJSPropertyAttributeNone },
        { ARCHIVE_FUNCTION_API_ARCHIVE_FILE_EXTRACT_ALL, extractAll, kJSPropertyAttributeNone },
        { ARCHIVE_FUNCTION_API_ARCHIVE_FILE_GET_ENTRIES, getEntries, kJSPropertyAttributeNone },
        { ARCHIVE_FUNCTION_API_ARCHIVE_FILE_GET_ENTRY_BY_NAME, getEntryByName, kJSPropertyAttributeNone },
        { ARCHIVE_FUNCTION_API_ARCHIVE_FILE_CLOSE, close, kJSPropertyAttributeNone },
        { 0, 0, 0 }
};

JSStaticValue JSArchiveFile::m_property[] =
{
    {ARCHIVE_FILE_MODE, getMode, NULL,
            kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
    {ARCHIVE_FILE_DECOMPRESSED_SIZE, getDecompressedSize, NULL,
            kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
    { 0, 0, 0, 0 }
};

JSClassRef JSArchiveFile::m_jsClassRef = JSClassCreate(JSArchiveFile::getClassInfo());

const DLL_EXPORT JSClassRef JSArchiveFile::getClassRef()
{
    if (!m_jsClassRef) {
        m_jsClassRef = JSClassCreate(&m_classInfo);
    }
    return m_jsClassRef;
}

void JSArchiveFile::initialize(JSContextRef context, JSObjectRef object)
{
    LOGD("Entered");
}

void JSArchiveFile::finalize(JSObjectRef object)
{
    LOGD("Entered");
    ArchiveFileHolder* priv =
            static_cast<ArchiveFileHolder*>(JSObjectGetPrivate(object));
    if (priv) {
        JSObjectSetPrivate(object, NULL);
        delete priv;
        priv = NULL;
    }
}

ArchiveFilePtr JSArchiveFile::getPrivateObject(JSContextRef context,
        JSValueRef value_ref)
{
    if (!JSValueIsObjectOfClass(context, value_ref, getClassRef())) {
        LOGE("TypeMismatch");
        throw TypeMismatchException("TypeMismatch");
    }

    JSObjectRef object = JSUtil::JSValueToObject(context, value_ref);
    ArchiveFileHolder* priv = static_cast<ArchiveFileHolder*>(JSObjectGetPrivate(object));
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

JSObjectRef JSArchiveFile::makeJSObject(JSContextRef context,
        ArchiveFilePtr native)
{
    if (!native) {
        LOGE("Native is null");
        throw TypeMismatchException("Native is null");
    }

    ArchiveFileHolder* priv = new ArchiveFileHolder;
    if(!priv){
        LOGE("Priv is null");
        throw UnknownException("Priv is null");
    }
    priv->ptr = native;

    JSObjectRef obj = JSObjectMake(context,
            getClassRef(), static_cast<void*>(priv));
    return obj;
}

JSValueRef JSArchiveFile::getMode(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);
        std::string fm_string = fileModeToString(priv->getFileMode());
        return JSUtil::toJSValueRef(context, fm_string);
    }
    catch (const BasePlatformException &error) {
        LOGE("Attribute get failed: %s", error.getMessage().c_str());
    }
    catch (...) {
        LOGE("Attribute get failed");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSArchiveFile::getDecompressedSize(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);
        return JSUtil::toJSValueRef(context, priv->getDecompressedSize());
    }
    catch (const BasePlatformException &error) {
        LOGE("Attribute get failed: %s", error.getMessage().c_str());
    }
    catch (...) {
        LOGE("Attribute get failed");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSArchiveFile::add(JSContextRef context,
        JSObjectRef object,
        JSObjectRef thisObject,
        size_t argumentCount,
        const JSValueRef arguments[],
        JSValueRef* exception)
{
    LOGD("Entered");

    ACE_ACCESS_CHECK(
        AceSecurityStatus status = ARCHIVE_CHECK_ACCESS(ARCHIVE_FUNCTION_API_ARCHIVE_FILE_ADD);
        TIZEN_SYNC_ACCESS_HANDLER(status, context, exception);
    );

    AddProgressCallback *callback = NULL;

    try {
        ArchiveFilePtr priv = JSArchiveFile::getPrivateObject(context, thisObject);
        ArgumentValidator validator(context, argumentCount, arguments);

        auto g_ctx = GlobalContextManager::getInstance()->getGlobalContext(context);
        callback = new(std::nothrow) AddProgressCallback(g_ctx);
        if(!callback) {
            LOGE("Couldn't allocate AddProgressCallback");
            throw UnknownException("Memory allocation error");
        }

        // set all callbacks
        callback->setSuccessCallback(validator.toFunction(1, true));
        callback->setErrorCallback(validator.toFunction(2, true));
        callback->setProgressCallback(validator.toFunction(3, true));

        // process sourceFile argument (File or path)
        if (validator.isOmitted(0)) {
            LOGE("FileReference not given");
            throw TypeMismatchException("Missing argument");
        }
        FilePtr file_ptr = fileReferenceToFile(context, arguments[0]);

        // prepare empty archive file and assign File pointer
        ArchiveFileEntryPtr afep = ArchiveFileEntryPtr(
            new ArchiveFileEntry(file_ptr));
        callback->setFileEntry(afep);

        callback->setBasePath(file_ptr->getNode()->getPath()->getPath());
        LOGD("base path:%s base virt:%s", callback->getBasePath().c_str(),
                callback->getBaseVirtualPath().c_str());

        // check and set options
        if(!validator.isOmitted(4)){
            if(validator.isUndefined(4)){
                LOGE("Type mismath error");
                throw TypeMismatchException("ArchiveFileEntryOptions is undefined");
            }
            LOGD("Processing dictionary");
            JSObjectRef dictionary = validator.toObject(4,true);
            JSValueRef dic_destination = JSUtil::getProperty(
                            context, dictionary, ARCHIVE_FILE_ENTRY_OPT_DEST);
            if (!JSValueIsUndefined(context, dic_destination)) {
                LOGD("Setting destination path");
                afep->setDestination(
                        JSUtil::JSValueToString(context, dic_destination));
            }
            JSValueRef dic_strip = JSUtil::getProperty(
                            context, dictionary, ARCHIVE_FILE_ENTRY_OPT_STRIP);
            if (!JSValueIsUndefined(context, dic_strip)) {
                LOGD("Setting strip option");
                afep->setStriped(
                        JSUtil::JSValueToBoolean(context, dic_strip));
            }
            JSValueRef dic_compression_level = JSUtil::getProperty(
                            context, dictionary, ARCHIVE_FILE_ENTRY_OPT_COMPRESSION_LEVEL);
            if (!JSValueIsUndefined(context, dic_compression_level)) {
                LOGD("Setting compression level");
                afep->setCompressionLevel(
                        JSUtil::JSValueToULong(context, dic_compression_level));
            }
        }

        if (!priv->isAllowedOperation(ARCHIVE_FUNCTION_API_ARCHIVE_FILE_ADD)) {
            LOGE("Not allowed operation");
            throw InvalidAccessException("Not allowed operation");
        }

        long op_id = priv->add(callback);
        LOGD("Run add request with op_id: %d", op_id);

        return JSUtil::toJSValueRef(context, op_id);
    }
    catch (const NotFoundException &nfe) {
        callback->setError(nfe.getName(), nfe.getMessage());
    }
    catch (const IOException &ioe) {
        callback->setError(ioe.getName(), ioe.getMessage());
    }
    catch (const BasePlatformException &err) {
        LOGE("add BasePlarformException caught: name: %s, msg: %s",
                err.getName().c_str(), err.getMessage().c_str());
        delete callback;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }
    catch (...) {
        LOGE("add fails");
        delete callback;
        return JSWebAPIErrorFactory::postException(context, exception,
                JSWebAPIErrorFactory::UNKNOWN_ERROR, "add fails");
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

JSValueRef JSArchiveFile::extractAll(JSContextRef context,
        JSObjectRef object,
        JSObjectRef thisObject,
        size_t argumentCount,
        const JSValueRef arguments[],
        JSValueRef* exception)
{
    LOGD("Entered");

    ACE_ACCESS_CHECK(
        AceSecurityStatus status = ARCHIVE_CHECK_ACCESS(ARCHIVE_FUNCTION_API_ARCHIVE_FILE_EXTRACT_ALL);
        TIZEN_SYNC_ACCESS_HANDLER(status, context, exception);
    );

    ExtractAllProgressCallback *callback = NULL;

    try {
        ArchiveFilePtr priv = JSArchiveFile::getPrivateObject(context, thisObject);
        ArgumentValidator validator(context, argumentCount, arguments);

        auto g_ctx = GlobalContextManager::getInstance()->getGlobalContext(context);
        callback = new ExtractAllProgressCallback(g_ctx);
        callback->setSuccessCallback(validator.toFunction(1, true));
        callback->setErrorCallback(validator.toFunction(2, true));
        callback->setProgressCallback(validator.toFunction(3, true));

        // process destinationDirectory (File or path)
        if (validator.isOmitted(0)) {
            LOGE("FileReference not given");
            throw TypeMismatchException("Missing argument");
        }
        FilePtr file_ptr = fileReferenceToFile(context, arguments[0]);

        // prepare empty archive file and assign File pointer
        callback->setDirectory(file_ptr);

        // overwrite is optional - if not given use default (false)
        bool opt_overwrite = validator.toBool(4, true, false);
        callback->setOverwrite(opt_overwrite);

        if (!priv->isAllowedOperation(ARCHIVE_FUNCTION_API_ARCHIVE_FILE_EXTRACT_ALL)) {
            LOGE("Not allowed operation");
            throw InvalidAccessException("Not allowed operation");
        }

        long op_id = priv->extractAll(callback);
        LOGD("Run extract all request with op_id: %d", op_id);

        return JSUtil::toJSValueRef(context, op_id);
    }
    catch (const NotFoundException &nfe) {
        callback->setError(nfe.getName(), nfe.getMessage());
    }
    catch (const IOException &ioe) {
        callback->setError(ioe.getName(), ioe.getMessage());
    }
    catch (const BasePlatformException &err) {
        LOGE("extractAll BasePlarformException caught: name: %s, msg: %s",
                err.getName().c_str(), err.getMessage().c_str());
        delete callback;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }
    catch (...) {
        LOGE("extractAll fails");
        delete callback;
        return JSWebAPIErrorFactory::postException(context, exception,
                JSWebAPIErrorFactory::UNKNOWN_ERROR, "extractAll fails");
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

JSValueRef JSArchiveFile::getEntries(JSContextRef context,
        JSObjectRef object,
        JSObjectRef thisObject,
        size_t argumentCount,
        const JSValueRef arguments[],
        JSValueRef* exception)
{
    LOGD("Entered");

    ACE_ACCESS_CHECK(
        AceSecurityStatus status =
                ARCHIVE_CHECK_ACCESS(ARCHIVE_FUNCTION_API_ARCHIVE_FILE_GET_ENTRIES);
        TIZEN_SYNC_ACCESS_HANDLER(status, context, exception);
    );

    GetEntriesCallbackData *callback = NULL;

    try {
        ArchiveFilePtr priv = JSArchiveFile::getPrivateObject(context, thisObject);
        ArgumentValidator validator(context, argumentCount, arguments);

        callback = new GetEntriesCallbackData(
                GlobalContextManager::getInstance()->getGlobalContext(context));

        callback->setSuccessCallback(validator.toFunction(0));
        callback->setErrorCallback(validator.toFunction(1, true));

        if (!priv->isAllowedOperation(ARCHIVE_FUNCTION_API_ARCHIVE_FILE_GET_ENTRIES)) {
            LOGE("Not allowed operation");
            throw InvalidAccessException("Not allowed operation");
        }

        long op_id = priv->getEntries(callback);
        LOGD("Run get entries request with op_id: %d", op_id);

        return JSUtil::toJSValueRef(context, op_id);
    }
    catch (const BasePlatformException &err) {
        LOGE("getEntries BasePlarformException caught: name: %s, msg: %s",
                err.getName().c_str(), err.getMessage().c_str());
        delete callback;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }
    catch (...) {
        LOGE("getEntries fails");
        delete callback;
        return JSWebAPIErrorFactory::postException(context, exception,
                JSWebAPIErrorFactory::UNKNOWN_ERROR, "getEntries fails");
    }
}

JSValueRef JSArchiveFile::getEntryByName(JSContextRef context,
        JSObjectRef object,
        JSObjectRef thisObject,
        size_t argumentCount,
        const JSValueRef arguments[],
        JSValueRef* exception)
{
    LOGD("Entered");

    ACE_ACCESS_CHECK(
        AceSecurityStatus status =
                ARCHIVE_CHECK_ACCESS(ARCHIVE_FUNCTION_API_ARCHIVE_FILE_GET_ENTRIES);
        TIZEN_SYNC_ACCESS_HANDLER(status, context, exception);
    );

    GetEntryByNameCallbackData *callback = NULL;

    try {
        ArchiveFilePtr priv = JSArchiveFile::getPrivateObject(context, thisObject);
        ArgumentValidator validator(context, argumentCount, arguments);

        callback = new GetEntryByNameCallbackData(
                GlobalContextManager::getInstance()->getGlobalContext(context));

        callback->setName(validator.toString(0));
        callback->setSuccessCallback(validator.toFunction(1));
        callback->setErrorCallback(validator.toFunction(2, true));

        if (!priv->isAllowedOperation(ARCHIVE_FUNCTION_API_ARCHIVE_FILE_GET_ENTRY_BY_NAME)) {
            LOGE("Not allowed operation");
            throw InvalidAccessException("Not allowed operation");
        }

        long op_id = priv->getEntryByName(callback);
        LOGD("Run get entry by name request with op_id: %d", op_id);

        return JSUtil::toJSValueRef(context, op_id);
    }
    catch (const BasePlatformException &err) {
        LOGE("getEntryByName BasePlarformException caught: name: %s, msg: %s",
                err.getName().c_str(), err.getMessage().c_str());
        delete callback;
        return JSWebAPIErrorFactory::postException(context, exception, err);
    }
    catch (...) {
        LOGE("getEntryByName fails");
        delete callback;
        return JSWebAPIErrorFactory::postException(context, exception,
                JSWebAPIErrorFactory::UNKNOWN_ERROR, "getEntryByName fails");
    }
}

JSValueRef JSArchiveFile::close(JSContextRef context,
        JSObjectRef object,
        JSObjectRef thisObject,
        size_t argumentCount,
        const JSValueRef arguments[],
        JSValueRef* exception)
{
    LOGD("Entered");

    try{
        ArchiveFilePtr priv = JSArchiveFile::getPrivateObject(context, thisObject);

        priv->close();
    }
    catch (const BasePlatformException &error) {
        LOGE("close failed: %s", error.getMessage().c_str());
        return JSWebAPIErrorFactory::postException(context, exception, error);
    }
    catch (...) {
        LOGE("close failed");
        return JSWebAPIErrorFactory::postException(context, exception,
                JSWebAPIErrorFactory::UNKNOWN_ERROR, "Unknown error");
    }

    return JSValueMakeUndefined(context);
}

} // Archive
} // DeviceAPI
