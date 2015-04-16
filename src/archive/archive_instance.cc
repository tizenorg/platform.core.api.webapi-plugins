// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "archive/archive_instance.h"

//#include "archive_manager.h"

#include <functional>
#include <memory>
#include <pkgmgr-info.h>
#include "common/current_application.h"
#include "common/picojson.h"
#include "common/logger.h"
#include "common/virtual_fs.h"
#include "archive_callback_data.h"
#include "archive_manager.h"
#include "archive_utils.h"
#include "defs.h"

namespace extension {
namespace archive {

using namespace common;

namespace {
const std::string kPrivilegeFilesystemRead  = "http://tizen.org/privilege/filesystem.read";
const std::string kPrivilegeFilesystemWrite  = "http://tizen.org/privilege/filesystem.write";

const std::string kWgtPackagePathName = "wgt-package";
const std::string kWgtPrivatePathName = "wgt-private";
const std::string kWgtPrivateTmpPathName = "wgt-private-tmp";
} // namespace

ArchiveInstance::ArchiveInstance() {
    LoggerD("Entered");

    using std::placeholders::_1;
    using std::placeholders::_2;

    #define REGISTER_SYNC(c,x) \
        RegisterSyncHandler(c, std::bind(&ArchiveInstance::x, this, _1, _2));
    #define REGISTER_ASYNC(c,x) \
        RegisterHandler(c, std::bind(&ArchiveInstance::x, this, _1, _2));

    REGISTER_ASYNC("ArchiveManager_open", Open);
    REGISTER_SYNC("ArchiveManager_abort", Abort);

    REGISTER_ASYNC("ArchiveFile_add", Add);
    REGISTER_ASYNC("ArchiveFile_extractAll", ExtractAll);
    REGISTER_ASYNC("ArchiveFile_getEntries", GetEntries);
    REGISTER_ASYNC("ArchiveFile_getEntryByName", GetEntryByName);
    REGISTER_SYNC("ArchiveFile_close", Close);

    REGISTER_ASYNC("ArchiveFileEntry_extract", Extract);

    REGISTER_SYNC("Filesystem_getWidgetPaths", GetWidgetPaths);

    #undef REGISTER_ASYNC
    #undef REGISTER_SYNC
}

ArchiveInstance::~ArchiveInstance() {
    LoggerD("Entered");
}

void ArchiveInstance::PostError(const PlatformResult& e, double callback_id) {
    picojson::value val = picojson::value(picojson::object());
    picojson::object& obj = val.get<picojson::object>();
    obj[JSON_CALLBACK_ID] = picojson::value(callback_id);
    obj[JSON_DATA] = picojson::value(picojson::object());

    picojson::object& args = obj[JSON_DATA].get<picojson::object>();
    obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_ERROR);

    args[ERROR_CALLBACK_CODE] = picojson::value(static_cast<double>(e.error_code()));
    args[ERROR_CALLBACK_MESSAGE] = picojson::value(e.message());

    PostMessage(val.serialize().c_str());
}

void ArchiveInstance::Open(const picojson::value& args, picojson::object& out) {
    LoggerD("Entered");
    LoggerD("%s", args.serialize().c_str());

    CHECK_PRIVILEGE_ACCESS(kPrivilegeFilesystemWrite, &out);

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::value v_file = data.at(PARAM_FILE);
    picojson::value v_mode = data.at(PARAM_MODE);
    picojson::value v_op_id = data.at(PARAM_OPERATION_ID);
    picojson::object options = data.at(PARAM_OPTIONS).get<picojson::object>();
    picojson::value v_overwrite = options.at(PARAM_OVERWRITE);
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();
    const long operationId = static_cast<long>(v_op_id.get<double>());
    FileMode fm;
    PlatformResult result = stringToFileMode(v_mode.get<std::string>(), &fm);
    if (result.error_code() != ErrorCode::NO_ERROR) {
        LoggerE("File mode conversions error");
        PostError(result, callbackId);
        return;
    }

    OpenCallbackData *callback = new OpenCallbackData(*this);

    FilePtr file_ptr;

    callback->setOperationId(operationId);
    callback->setCallbackId(callbackId);

    bool overwrite = false;
    if(v_overwrite.is<bool>()) {
        overwrite = v_overwrite.get<bool>();
    }

    std::string location_full_path = v_file.get<std::string>();
    PathPtr path = Path::create(location_full_path);

    struct stat info;
    if (lstat(path->getFullPath().c_str(), &info) == 0) {
        NodePtr node;
        result = Node::resolve(path, &node);
        if (result.error_code() != ErrorCode::NO_ERROR) {
            LoggerE("Filesystem exception - calling error callback");
            PostError(result, callbackId);
            delete callback;
            callback = NULL;
            return;
        }

        file_ptr = FilePtr(new File(node, File::PermissionList()));
        LoggerD("open: %s mode: 0x%x overwrite: %d", location_full_path.c_str(), fm, overwrite);
        if (FileMode::WRITE == fm || FileMode::READ_WRITE == fm) {
            if (overwrite) {
                LoggerD("Deleting existing file: %s", location_full_path.c_str());
                result = file_ptr->getNode()->remove(OPT_RECURSIVE);
                if (result.error_code() != ErrorCode::NO_ERROR) {
                    LoggerE("Couldn't remove existing file: %s", location_full_path.c_str());
                    PostError(result, callbackId);
                    delete callback;
                    callback = NULL;
                    return;
                }
                file_ptr.reset();   //We need to create new empty file
            } else if (FileMode::WRITE == fm) {
                LoggerE("open: %s with mode: \"w\" file exists and overwrite is FALSE!"
                        " throwing InvalidModificationException", location_full_path.c_str());
                PostError(PlatformResult(ErrorCode::INVALID_MODIFICATION_ERR,
                                         "Zip archive already exists"), callbackId);
                delete callback;
                callback = NULL;
                return;
            }
        }
    }

    if (!file_ptr) {
        NodePtr node_ptr;

        if (FileMode::WRITE == fm ||
                FileMode::READ_WRITE == fm ||
                FileMode::ADD == fm) {
            LoggerD("Archive file not found - trying to create new one at: "
                    "full: %s", location_full_path.c_str());

            std::string parent_path_string = path->getPath();
            PathPtr parent_path = Path::create(parent_path_string);
            LoggerD("Parent path: %s", parent_path_string.c_str());

            NodePtr parent_node;
            PlatformResult result = Node::resolve(parent_path, &parent_node);
            if (result.error_code() != ErrorCode::NO_ERROR) {
                LoggerE("Filesystem exception - calling error callback");
                PostError(result, callbackId);
                delete callback;
                callback = NULL;
                return;
            }

            parent_node->setPermissions(PERM_READ | PERM_WRITE);
            std::string filename = path->getName();
            LoggerD("File name: %s", filename.c_str());
            result = parent_node->createChild(Path::create(filename), NT_FILE, &node_ptr);
            if (result.error_code() != ErrorCode::NO_ERROR) {
                LoggerE("Filesystem exception - calling error callback");
                PostError(result, callbackId);
                delete callback;
                callback = NULL;
                return;
            }
        } else {
            LoggerE("Archive file not found");
            LoggerE("Filesystem exception - calling error callback");
            PostError(PlatformResult(ErrorCode::NOT_FOUND_ERR, "Archive file not found"), callbackId);
            delete callback;
            callback = NULL;
            return;
        }
        file_ptr = FilePtr(new File(node_ptr, File::PermissionList()));
    }

    ArchiveFilePtr afp = ArchiveFilePtr(new ArchiveFile(fm));
    afp->setFile(file_ptr);
    afp->setOverwrite(overwrite);
    callback->setArchiveFile(afp);

    ArchiveManager::getInstance().open(callback);
}

void ArchiveInstance::Abort(const picojson::value& args, picojson::object& out)
{
    LoggerD("Entered");
    LoggerD("%s", args.serialize().c_str());

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::value v_op_id = data.at(PARAM_OPERATION_ID);

    const long op_id = static_cast<long>(v_op_id.get<double>());

    ArchiveManager::getInstance().abort(op_id);

    ReportSuccess(out);
}

void ArchiveInstance::Add(const picojson::value& args, picojson::object& out)
{
    LoggerD("Entered");
    LoggerD("%s", args.serialize().c_str());

    CHECK_PRIVILEGE_ACCESS(kPrivilegeFilesystemWrite, &out);

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::value v_source = data.at(PARAM_SOURCE_FILE);
    //picojson::value v_options = data.at(PARAM_OPTIONS);
    picojson::value v_op_id = data.at(PARAM_OPERATION_ID);
    picojson::value v_handle = data.at(ARCHIVE_FILE_HANDLE);

    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();
    const long operationId = static_cast<long>(v_op_id.get<double>());
    const long handle = static_cast<long>(v_handle.get<double>());

    AddProgressCallback *callback = new AddProgressCallback(*this);

    NodePtr node;
    PlatformResult result = Node::resolve(Path::create(v_source.get<std::string>()), &node);
    if (result.error_code() != ErrorCode::NO_ERROR) {
        LoggerE("Filesystem exception - calling error callback");
        PostError(result, callbackId);
        delete callback;
        callback = NULL;
        return;
    }

    FilePtr file_ptr = FilePtr(new File(node, File::PermissionList()));
    ArchiveFileEntryPtr afep = ArchiveFileEntryPtr(new ArchiveFileEntry(file_ptr));

    callback->setOperationId(operationId);
    callback->setCallbackId(callbackId);
    callback->setFileEntry(afep);

    callback->setBasePath(file_ptr->getNode()->getPath()->getPath());
    LoggerD("base path:%s base virt:%s", callback->getBasePath().c_str(),
            callback->getBaseVirtualPath().c_str());

    ArchiveFilePtr priv;
    result = ArchiveManager::getInstance().getPrivData(handle, &priv);
    if (result.error_code() != ErrorCode::NO_ERROR) {
        LoggerE("Exception occurred");
        delete callback;
        callback = NULL;
        return;
    }

    if (!priv->isAllowedOperation(ARCHIVE_FUNCTION_API_ARCHIVE_FILE_ADD)) {
        LoggerE("Not allowed operation");
        delete callback;
        callback = NULL;
        return;
    }

    result = priv->add(callback);
    if (result.error_code() != ErrorCode::NO_ERROR) {
        LoggerE("Exception occurred");
        delete callback;
        callback = NULL;
        return;
    }
}

void ArchiveInstance::ExtractAll(const picojson::value& args, picojson::object& out)
{
    LoggerD("Entered");
    LoggerD("%s", args.serialize().c_str());

    CHECK_PRIVILEGE_ACCESS(kPrivilegeFilesystemWrite, &out);

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::value v_dest_dir = data.at(PARAM_DESTINATION_DIR);
    picojson::value v_overwrite = data.at(PARAM_OVERWRITE);
    picojson::value v_op_id = data.at(PARAM_OPERATION_ID);
    picojson::value v_handle = data.at(ARCHIVE_FILE_HANDLE);

    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();
    const long operationId = static_cast<long>(v_op_id.get<double>());
    const long handle = static_cast<long>(v_handle.get<double>());

    ExtractAllProgressCallback *callback = new ExtractAllProgressCallback(*this);

    NodePtr node;
    PlatformResult result = Node::resolve(Path::create(v_dest_dir.get<std::string>()), &node);
    if (result.error_code() != ErrorCode::NO_ERROR) {
        LoggerE("Filesystem exception - calling error callback");
        PostError(result, callbackId);
        delete callback;
        callback = NULL;
        return;
    }

    FilePtr file_ptr = FilePtr(new File(node, File::PermissionList()));

    callback->setDirectory(file_ptr);
    callback->setOperationId(operationId);
    callback->setCallbackId(callbackId);

    if (v_overwrite.is<bool>()) {
        callback->setOverwrite(v_overwrite.get<bool>());
    }

    ArchiveFilePtr priv;
    result = ArchiveManager::getInstance().getPrivData(handle, &priv);
    if (result.error_code() != ErrorCode::NO_ERROR) {
        LoggerE("Exception occurred");
        delete callback;
        callback = NULL;
        return;
    }

    if (!priv->isAllowedOperation(ARCHIVE_FUNCTION_API_ARCHIVE_FILE_EXTRACT_ALL)) {
        LoggerE("Not allowed operation");
        delete callback;
        callback = NULL;
        return;
    }

    result = priv->extractAll(callback);
    if (result.error_code() != ErrorCode::NO_ERROR) {
        LoggerE("Exception occurred");
        delete callback;
        callback = NULL;
        return;
    }
}

void ArchiveInstance::GetEntries(const picojson::value& args, picojson::object& out)
{
    LoggerD("Entered");
    LoggerD("%s", args.serialize().c_str());

    CHECK_PRIVILEGE_ACCESS(kPrivilegeFilesystemRead, &out);

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::value v_op_id = data.at(PARAM_OPERATION_ID);
    picojson::value v_handle = data.at(ARCHIVE_FILE_HANDLE);

    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();
    const long operationId = static_cast<long>(v_op_id.get<double>());
    const long handle = static_cast<long>(v_handle.get<double>());

    GetEntriesCallbackData *callback = new GetEntriesCallbackData(*this);

    callback->setOperationId(operationId);
    callback->setCallbackId(callbackId);
    callback->setHandle(handle);

    ArchiveFilePtr priv;
    PlatformResult result = ArchiveManager::getInstance().getPrivData(handle, &priv);
    if (result.error_code() != ErrorCode::NO_ERROR) {
        LoggerE("Exception occurred");
        delete callback;
        callback = NULL;
        return;
    }

    if (!priv->isAllowedOperation(ARCHIVE_FUNCTION_API_ARCHIVE_FILE_GET_ENTRIES)) {
        LoggerE("Not allowed operation");
        delete callback;
        callback = NULL;
        return;
    }

    result = priv->getEntries(callback);
    if (result.error_code() != ErrorCode::NO_ERROR) {
        LoggerE("Exception occurred");
        delete callback;
        callback = NULL;
        return;
    }
}

void ArchiveInstance::GetEntryByName(const picojson::value& args, picojson::object& out)
{
    LoggerD("Entered");
    LoggerD("%s", args.serialize().c_str());

    CHECK_PRIVILEGE_ACCESS(kPrivilegeFilesystemRead, &out);

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::value v_op_id = data.at(PARAM_OPERATION_ID);
    picojson::value v_handle = data.at(ARCHIVE_FILE_HANDLE);
    picojson::value v_name = data.at(PARAM_NAME);

    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();
    const long operationId = static_cast<long>(v_op_id.get<double>());
    const long handle = static_cast<long>(v_handle.get<double>());

    GetEntryByNameCallbackData *callback = new GetEntryByNameCallbackData(*this);

    callback->setOperationId(operationId);
    callback->setCallbackId(callbackId);
    callback->setName(v_name.get<std::string>());
    callback->setHandle(handle);

    ArchiveFilePtr priv;
    PlatformResult result = ArchiveManager::getInstance().getPrivData(handle, &priv);
    if (result.error_code() != ErrorCode::NO_ERROR) {
        LoggerE("Exception occurred");
        delete callback;
        callback = NULL;
        return;
    }

    if (!priv->isAllowedOperation(ARCHIVE_FUNCTION_API_ARCHIVE_FILE_GET_ENTRY_BY_NAME)) {
        LoggerE("Not allowed operation");
        delete callback;
        callback = NULL;
        return;
    }

    result = priv->getEntryByName(callback);
    if (result.error_code() != ErrorCode::NO_ERROR) {
        LoggerE("Exception occurred");
        delete callback;
        callback = NULL;
        return;
    }
}

void ArchiveInstance::Close(const picojson::value& args, picojson::object& out)
{
    LoggerD("Entered");
    LoggerD("%s", args.serialize().c_str());

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::value v_handle = data.at(ARCHIVE_FILE_HANDLE);

    const long handle = static_cast<long>(v_handle.get<double>());

    ArchiveFilePtr priv;
    PlatformResult result = ArchiveManager::getInstance().getPrivData(handle, &priv);
    if (result.error_code() != ErrorCode::NO_ERROR) {
        LoggerD("Close method was called on already closed archive. Just end execution");
        LoggerD("%s", result.message().c_str());
    }

    priv->close();
    ArchiveManager::getInstance().erasePrivData(handle);

    ReportSuccess(out);
}

void ArchiveInstance::Extract(const picojson::value& args, picojson::object& out)
{
    LoggerD("Entered");
    LoggerD("%s", args.serialize().c_str());

    CHECK_PRIVILEGE_ACCESS(kPrivilegeFilesystemWrite, &out);

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::value v_dest_dir = data.at(PARAM_DESTINATION_DIR);
    picojson::value v_strip_name = data.at(PARAM_STRIP_NAME);
    picojson::value v_overwrite = data.at(PARAM_OVERWRITE);
    picojson::value v_op_id = data.at(PARAM_OPERATION_ID);
    picojson::value v_handle = data.at(ARCHIVE_FILE_HANDLE);
    picojson::value v_entry_name = data.at(PARAM_NAME);

    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();
    const long operationId = static_cast<long>(v_op_id.get<double>());
    const long handle = static_cast<long>(v_handle.get<double>());

    ExtractEntryProgressCallback *callback = new ExtractEntryProgressCallback(*this);

    NodePtr node;
    PlatformResult result = Node::resolve(Path::create(v_dest_dir.get<std::string>()), &node);
    if (result.error_code() != ErrorCode::NO_ERROR) {
        LoggerE("Filesystem exception - calling error callback");
        PostError(result, callbackId);
        delete callback;
        callback = NULL;
        return;
    }

    FilePtr file_ptr = FilePtr(new File(node, File::PermissionList()));

    callback->setDirectory(file_ptr);
    callback->setOperationId(operationId);
    callback->setCallbackId(callbackId);

    if (v_overwrite.is<bool>()) {
        callback->setOverwrite(v_overwrite.get<bool>());
    }
    if (v_strip_name.is<bool>()) {
        callback->setStripName(v_strip_name.get<bool>());
    }

    ArchiveFilePtr archive_file_ptr;
    result = ArchiveManager::getInstance().getPrivData(handle, &archive_file_ptr);
    if (result.error_code() != ErrorCode::NO_ERROR) {
        LoggerE("Exception occurred");
        delete callback;
        callback = NULL;
        return;
    }

    ArchiveFileEntryPtrMapPtr entries = archive_file_ptr->getEntryMap();
    auto it = entries->find(v_entry_name.get<std::string>());

    //Not found but if our name does not contain '/'
    //try looking for directory with such name
    if (it == entries->end() && !isDirectoryPath(v_entry_name.get<std::string>())) {
        const std::string try_directory = v_entry_name.get<std::string>() + "/";
        LoggerD("GetEntryByName Trying directory: [%s]", try_directory.c_str());
        it = entries->find(try_directory);
    }

    result = it->second->extractTo(callback);
    if (result.error_code() != ErrorCode::NO_ERROR) {
        LoggerE("ArchiveFileEntry.extractTo error");
        PostError(result, callbackId);
        delete callback;
        callback = NULL;
        return;
    }
}

void ArchiveInstance::GetWidgetPaths(const picojson::value& args, picojson::object& out) {
    std::string wgt_package_path =
        *(common::VirtualFs::GetInstance().GetVirtualRootDirectory(kWgtPackagePathName));
    std::string wgt_private_path =
        *(common::VirtualFs::GetInstance().GetVirtualRootDirectory(kWgtPrivatePathName));
    std::string wgt_private_tmp_path =
        *(common::VirtualFs::GetInstance().GetVirtualRootDirectory(kWgtPrivateTmpPathName));
    LoggerD("wgt-package path: %s", wgt_package_path.c_str());
    LoggerD("wgt-private path: %s", wgt_private_path.c_str());
    LoggerD("wgt-private-tmp path: %s", wgt_private_tmp_path.c_str());

    // Construction of the response
    picojson::value result{picojson::object()};
    auto& result_obj = result.get<picojson::object>();
    result_obj.insert(std::make_pair(kWgtPackagePathName, picojson::value(wgt_package_path)));
    result_obj.insert(std::make_pair(kWgtPrivatePathName, picojson::value(wgt_private_path)));
    result_obj.insert(std::make_pair(kWgtPrivateTmpPathName, picojson::value(wgt_private_tmp_path)));

    ReportSuccess(result, out);
}

} // namespace archive
} // namespace extension
