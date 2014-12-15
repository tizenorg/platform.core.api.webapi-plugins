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
#include <Logger.h>

#include <Export.h>
#include <JSWebAPIErrorFactory.h>
#include <ArgumentValidator.h>
#include <SecurityExceptions.h>
#include <FilesystemExternalUtils.h>
#include <JSFile.h>
#include <Path.h>
#include <GlobalContextManager.h>
#include <JSUtil.h>

#include "JSArchiveManager.h"
#include "ArchiveManager.h"
#include "ArchiveUtils.h"
#include "plugin_config.h"

namespace DeviceAPI {
namespace Archive {

using namespace WrtDeviceApis::Commons;
using namespace DeviceAPI::Common;
using namespace DeviceAPI::Filesystem;

namespace {
const char* ARCHIVE_MANAGER = "ArchiveManager";
const char* ARCHIVE_FILE_OPTIONS_OVERWRITE = "overwrite";
}

JSClassDefinition JSArchiveManager::m_classInfo = {
        0,                      // version
        kJSClassAttributeNone,  // attributes
        ARCHIVE_MANAGER,        // class name
        NULL,                   // parent class
        NULL,                   // static values
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

const JSClassDefinition* JSArchiveManager::getClassInfo()
{
    return &m_classInfo;
}

JSStaticFunction JSArchiveManager::m_function[] = {
        { ARCHIVE_FUNCTION_API_ARCHIVE_MANAGER_ABORT, abort, kJSPropertyAttributeNone },
        { ARCHIVE_FUNCTION_API_ARCHIVE_MANAGER_OPEN, open, kJSPropertyAttributeNone },
        { 0, 0, 0 }
};

JSClassRef JSArchiveManager::m_jsClassRef = JSClassCreate(JSArchiveManager::getClassInfo());

const JSClassRef DLL_EXPORT JSArchiveManager::getClassRef()
{
    if (!m_jsClassRef) {
        m_jsClassRef = JSClassCreate(&m_classInfo);
    }
    return m_jsClassRef;
}

void JSArchiveManager::initialize(JSContextRef context, JSObjectRef object)
{
    LOGD("Entered");
}

void JSArchiveManager::finalize(JSObjectRef object)
{
    LOGD("Entered");
}

JSValueRef JSArchiveManager::open(JSContextRef context,
        JSObjectRef object,
        JSObjectRef thisObject,
        size_t argumentCount,
        const JSValueRef arguments[],
        JSValueRef* exception)
{
    LOGD("Entered");

    ACE_ACCESS_CHECK(
        AceSecurityStatus status = ARCHIVE_CHECK_ACCESS(ARCHIVE_FUNCTION_API_ARCHIVE_MANAGER_OPEN);
        TIZEN_SYNC_ACCESS_HANDLER(status, context, exception);
    );

    OpenCallbackData *open_cb = NULL;
    try{
        long op_id = -1;
        ArgumentValidator validator(context, argumentCount, arguments);
        FilePtr file_ptr;
        // process file mode argument (is checked when opening archive)
        std::string fm_string = validator.toString(1);
        // in case of invalid file mode exception is thrown
        FileMode fm = stringToFileMode(fm_string);

        auto g_ctx = GlobalContextManager::getInstance()->getGlobalContext(context);

        // set callbacks
        open_cb = new (std::nothrow) OpenCallbackData(g_ctx);
        if(!open_cb) {
            LOGE("Couldn't allocate OpenCallbackData");
            throw UnknownException("Memory allocation error");
        }
        open_cb->setSuccessCallback(validator.toFunction(2));
        open_cb->setErrorCallback(validator.toFunction(3, true));

        bool overwrite = false;
        // check and set options (if given)
        if(!validator.isOmitted(4)) {
            if(validator.isUndefined(4)) {
                LOGE("Type mismath error");
                throw TypeMismatchException("ArchiveFileOptions is undefined");
            }

            LOGD("Processing options dictionary.");
            JSObjectRef options = validator.toObject(4, true);
            JSValueRef opt_overwrite = JSUtil::getProperty(context, options,
                    ARCHIVE_FILE_OPTIONS_OVERWRITE);
            if (!JSValueIsUndefined(context, opt_overwrite)) {
                overwrite = JSUtil::JSValueToBoolean(context, opt_overwrite);
                LOGD("Overwrite: %d", overwrite);

                if(FileMode::ADD == fm) {
                    LOGW("File mode is: \"a\" -> overwrite will be ignored");
                }
            }
        }

        std::string location_string = validator.toString(0);
        std::string location_full_path;

        LOGD("location_string: %s", location_string.c_str());

        // process file reference
        try {
            file_ptr = fileReferenceToFile(context, arguments[0]);
            const std::string full_path = file_ptr->getNode()->getPath()->getFullPath();
            LOGD("open: %s mode: 0x%x overwrite: %d", full_path.c_str(), fm, overwrite);

            if(FileMode::WRITE == fm || FileMode::READ_WRITE == fm) {
                if(overwrite) {
                    try {
                        //Store requested full path
                        location_string = std::string();    // remove "[object File]"
                        location_full_path = full_path;

                        LOGD("Deleting existing file: %s", full_path.c_str());
                        file_ptr->getNode()->remove(OPT_RECURSIVE);
                        file_ptr.reset();   //We need to create new empty file
                    } catch(...) {
                        LOGE("Couldn't remove existing file: %s", full_path.c_str());
                        throw Common::IOException("Could not remove existing file");
                    }
                }
                else if(FileMode::WRITE == fm) {
                    LOGE("open: %s with mode: \"w\" file exists and overwrite is FALSE!"
                            " throwing InvalidModificationException", full_path.c_str());
                    throw Common::InvalidModificationException("Zip archive already exists");
                }
            }
        } catch (const NotFoundException& nfe) {
            LOGD("location_string: %s is not file reference", location_string.c_str());
            file_ptr.reset();
        }

        if(!file_ptr) {
            NodePtr node_ptr;

            if(FileMode::WRITE == fm ||
                    FileMode::READ_WRITE == fm ||
                    FileMode::ADD == fm) {

                if(location_full_path.empty()) {
                    location_full_path = External::fromVirtualPath(location_string, g_ctx);
                }
                LOGD("Archive file not found - trying to create new one at: "
                        "original: %s full: %s", location_string.c_str(),
                        location_full_path.c_str());

                PathPtr path = Path::create(location_full_path);

                std::string parent_path_string = path->getPath();
                PathPtr parent_path = Path::create(parent_path_string);
                LOGD("Parent path: %s", parent_path_string.c_str());

                NodePtr parent_node = Node::resolve(parent_path);
                parent_node->setPermissions(PERM_READ | PERM_WRITE);
                std::string filename = path->getName();
                LOGD("File name: %s", filename.c_str());
                node_ptr = parent_node->createChild(Path::create(filename), NT_FILE);
            }
            else {
                LOGE("Archive file not found");
                throw Common::NotFoundException("Archive file not found");
            }
            file_ptr = FilePtr(new File(node_ptr, File::PermissionList()));
        }

        // prepare empty archive file and assign File pointer
        ArchiveFilePtr afp = ArchiveFilePtr(new ArchiveFile(fm));
        afp->setFile(file_ptr);
        afp->setOverwrite(overwrite);
        open_cb->setArchiveFile(afp);

        op_id = ArchiveManager::getInstance().open(open_cb);
        return JSUtil::toJSValueRef(context, op_id);
    }
    catch (const NotFoundException &nfe) {
       open_cb->setError(nfe.getName(), nfe.getMessage());
    }
    catch (const IOException &ioe) {
       open_cb->setError(ioe.getName(), ioe.getMessage());
    }
    catch (const BasePlatformException &error) {
        LOGE("Open failed: %s", error.getMessage().c_str());
        delete open_cb;
        open_cb = NULL;
        return JSWebAPIErrorFactory::postException(context, exception, error);
    }
    catch (...) {
        LOGE("Open failed");
        delete open_cb;
        open_cb = NULL;
        return JSWebAPIErrorFactory::postException(context, exception,
                JSWebAPIErrorFactory::UNKNOWN_ERROR, "Unknown error");
    }

    if (open_cb && open_cb->isError()) {
        JSObjectRef errobj = JSWebAPIErrorFactory::makeErrorObject(context,
                open_cb->getErrorName(),
                open_cb->getErrorMessage());
        open_cb->callErrorCallback(errobj);
        delete open_cb;
        open_cb = NULL;
    }

    return JSValueMakeUndefined(context);
}


JSValueRef JSArchiveManager::abort(JSContextRef context,
        JSObjectRef object,
        JSObjectRef thisObject,
        size_t argumentCount,
        const JSValueRef arguments[],
        JSValueRef* exception)
{
    LOGD("Entered");

    try{
        ArgumentValidator validator(context, argumentCount, arguments);
        long operation_id = validator.toLong(0);

        ArchiveManager::getInstance().abort(operation_id);
    }
    catch (const BasePlatformException &error) {
        LOGE("Abort failed: %s", error.getMessage().c_str());
        return JSWebAPIErrorFactory::postException(context, exception, error);
    }
    catch (...) {
        LOGE("Abort failed");
        return JSWebAPIErrorFactory::postException(context, exception,
                JSWebAPIErrorFactory::UNKNOWN_ERROR, "Unknown error");
    }

    return JSValueMakeUndefined(context);
}

} // Archive
} // DeviceAPI
