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
#include "common/platform_exception.h"
#include "archive_callback_data.h"
#include "archive_manager.h"
#include "archive_utils.h"

#include "defs.h"

namespace extension {
namespace archive {

using namespace common;


ArchiveInstance& ArchiveInstance::getInstance()
{
    static ArchiveInstance instance;
    return instance;
}

ArchiveInstance::ArchiveInstance() {
    LoggerD("Entered");

    using namespace std::placeholders;
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

void ArchiveInstance::Open(const picojson::value& args, picojson::object& out) {
    LoggerD("Entered");
    LoggerD("%s", args.serialize().c_str());

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::value v_file = data.at(PARAM_FILE);
    picojson::value v_mode = data.at(PARAM_MODE);
    picojson::value v_op_id = data.at(PARAM_OPERATION_ID);
    picojson::object options = data.at(PARAM_OPTIONS).get<picojson::object>();
    picojson::value v_overwrite = options.at(PARAM_OVERWRITE);
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();
    const long operationId = static_cast<long>(v_op_id.get<double>());
    FileMode fm = stringToFileMode(v_mode.get<std::string>());

    OpenCallbackData *callback = new OpenCallbackData();

    try {
        FilePtr file_ptr;

        callback->setOperationId(operationId);
        callback->setCallbackId(callbackId);

        bool overwrite = false;
        if(v_overwrite.is<bool>()) {
            overwrite = v_overwrite.get<bool>();
        }

        std::string location_full_path = v_file.get<std::string>();

        try {
            NodePtr node = Node::resolve(Path::create(location_full_path));
            file_ptr = FilePtr(new File(node, File::PermissionList()));
            LoggerD("open: %s mode: 0x%x overwrite: %d", location_full_path.c_str(), fm, overwrite);

            if(FileMode::WRITE == fm || FileMode::READ_WRITE == fm) {
                if(overwrite) {
                    try {
                        LoggerD("Deleting existing file: %s", location_full_path.c_str());
                        file_ptr->getNode()->remove(OPT_RECURSIVE);
                        file_ptr.reset();   //We need to create new empty file
                    } catch(...) {
                        LoggerE("Couldn't remove existing file: %s", location_full_path.c_str());
                        throw IOException("Could not remove existing file");
                    }
                }
                else if(FileMode::WRITE == fm) {
                    LoggerE("open: %s with mode: \"w\" file exists and overwrite is FALSE!"
                            " throwing InvalidModificationException", location_full_path.c_str());
                    throw InvalidModificationException("Zip archive already exists");
                }
            }
        } catch (const NotFoundException& nfe) {
            LoggerD("location_string: %s is not file reference", location_full_path.c_str());
            file_ptr.reset();
        }

        if (!file_ptr) {
            NodePtr node_ptr;

            if(FileMode::WRITE == fm ||
                    FileMode::READ_WRITE == fm ||
                    FileMode::ADD == fm) {
                LoggerD("Archive file not found - trying to create new one at: "
                        "full: %s", location_full_path.c_str());

                PathPtr path = Path::create(location_full_path);

                std::string parent_path_string = path->getPath();
                PathPtr parent_path = Path::create(parent_path_string);
                LoggerD("Parent path: %s", parent_path_string.c_str());

                NodePtr parent_node = Node::resolve(parent_path);
                parent_node->setPermissions(PERM_READ | PERM_WRITE);
                std::string filename = path->getName();
                LoggerD("File name: %s", filename.c_str());
                node_ptr = parent_node->createChild(Path::create(filename), NT_FILE);
            }
            else {
                LoggerE("Archive file not found");
                throw NotFoundException("Archive file not found");
            }
            file_ptr = FilePtr(new File(node_ptr, File::PermissionList()));
        }

        ArchiveFilePtr afp = ArchiveFilePtr(new ArchiveFile(fm));
        afp->setFile(file_ptr);
        afp->setOverwrite(overwrite);
        callback->setArchiveFile(afp);

        ArchiveManager::getInstance().open(callback);
    }
    catch (...) {
        LoggerE("Exception occurred");
        delete callback;
        callback = NULL;
        throw;
    }
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

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::value v_source = data.at(PARAM_SOURCE_FILE);
    //picojson::value v_options = data.at(PARAM_OPTIONS);
    picojson::value v_op_id = data.at(PARAM_OPERATION_ID);
    picojson::value v_handle = data.at(ARCHIVE_FILE_HANDLE);

    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();
    const long operationId = static_cast<long>(v_op_id.get<double>());
    const long handle = static_cast<long>(v_handle.get<double>());

    AddProgressCallback *callback = new AddProgressCallback();

    try {
        NodePtr node = Node::resolve(Path::create(v_source.get<std::string>()));
        FilePtr file_ptr = FilePtr(new File(node, File::PermissionList()));

        ArchiveFileEntryPtr afep = ArchiveFileEntryPtr(
                new ArchiveFileEntry(file_ptr));

        callback->setOperationId(operationId);
        callback->setCallbackId(callbackId);
        callback->setFileEntry(afep);

        callback->setBasePath(file_ptr->getNode()->getPath()->getPath());
        LoggerD("base path:%s base virt:%s", callback->getBasePath().c_str(),
                callback->getBaseVirtualPath().c_str());

        ArchiveFilePtr priv = ArchiveManager::getInstance().getPrivData(handle);
        if (!priv->isAllowedOperation(ARCHIVE_FUNCTION_API_ARCHIVE_FILE_ADD)) {
            LoggerE("Not allowed operation");
            throw InvalidAccessException("Not allowed operation");
        }

        priv->add(callback);
    }
    catch (...) {
        LoggerE("Exception occurred");
        delete callback;
        callback = NULL;
        throw;
    }
}

void ArchiveInstance::ExtractAll(const picojson::value& args, picojson::object& out)
{
    LoggerD("Entered");
    LoggerD("%s", args.serialize().c_str());

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::value v_dest_dir = data.at(PARAM_DESTINATION_DIR);
    picojson::value v_overwrite = data.at(PARAM_OVERWRITE);
    picojson::value v_op_id = data.at(PARAM_OPERATION_ID);
    picojson::value v_handle = data.at(ARCHIVE_FILE_HANDLE);

    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();
    const long operationId = static_cast<long>(v_op_id.get<double>());
    const long handle = static_cast<long>(v_handle.get<double>());

    ExtractAllProgressCallback *callback = new ExtractAllProgressCallback();

    try {
        NodePtr node = Node::resolve(Path::create(v_dest_dir.get<std::string>()));
        FilePtr file_ptr = FilePtr(new File(node, File::PermissionList()));

        callback->setDirectory(file_ptr);
        callback->setOperationId(operationId);
        callback->setCallbackId(callbackId);

        if (v_overwrite.is<bool>()) {
            callback->setOverwrite(v_overwrite.get<bool>());
        }

        ArchiveFilePtr priv = ArchiveManager::getInstance().getPrivData(handle);
        if (!priv->isAllowedOperation(ARCHIVE_FUNCTION_API_ARCHIVE_FILE_EXTRACT_ALL)) {
            LoggerE("Not allowed operation");
            throw InvalidAccessException("Not allowed operation");
        }
        priv->extractAll(callback);
    }
    catch (...) {
        LoggerE("Exception occurred");
        delete callback;
        callback = NULL;
        throw;
    }
}

void ArchiveInstance::GetEntries(const picojson::value& args, picojson::object& out)
{
    LoggerD("Entered");
    LoggerD("%s", args.serialize().c_str());

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::value v_op_id = data.at(PARAM_OPERATION_ID);
    picojson::value v_handle = data.at(ARCHIVE_FILE_HANDLE);

    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();
    const long operationId = static_cast<long>(v_op_id.get<double>());
    const long handle = static_cast<long>(v_handle.get<double>());

    GetEntriesCallbackData *callback = new GetEntriesCallbackData();

    try {
        callback->setOperationId(operationId);
        callback->setCallbackId(callbackId);
        callback->setHandle(handle);

        ArchiveFilePtr priv = ArchiveManager::getInstance().getPrivData(handle);
        if (!priv->isAllowedOperation(ARCHIVE_FUNCTION_API_ARCHIVE_FILE_GET_ENTRIES)) {
            LoggerE("Not allowed operation");
            throw InvalidAccessException("Not allowed operation");
        }

        priv->getEntries(callback);
    }
    catch (...) {
        LoggerE("Exception occurred");
        delete callback;
        callback = NULL;
        throw;
    }
}

void ArchiveInstance::GetEntryByName(const picojson::value& args, picojson::object& out)
{
    LoggerD("Entered");
    LoggerD("%s", args.serialize().c_str());

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::value v_op_id = data.at(PARAM_OPERATION_ID);
    picojson::value v_handle = data.at(ARCHIVE_FILE_HANDLE);
    picojson::value v_name = data.at(PARAM_NAME);

    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();
    const long operationId = static_cast<long>(v_op_id.get<double>());
    const long handle = static_cast<long>(v_handle.get<double>());

    GetEntryByNameCallbackData *callback = new GetEntryByNameCallbackData();

    try {
        callback->setOperationId(operationId);
        callback->setCallbackId(callbackId);
        callback->setName(v_name.get<std::string>());
        callback->setHandle(handle);

        ArchiveFilePtr priv = ArchiveManager::getInstance().getPrivData(handle);
        if (!priv->isAllowedOperation(ARCHIVE_FUNCTION_API_ARCHIVE_FILE_GET_ENTRY_BY_NAME)) {
            LoggerE("Not allowed operation");
            throw InvalidAccessException("Not allowed operation");
        }

        priv->getEntryByName(callback);
    }
    catch (...) {
        LoggerE("Exception occurred");
        delete callback;
        callback = NULL;
        throw;
    }
}

void ArchiveInstance::Close(const picojson::value& args, picojson::object& out)
{
    LoggerD("Entered");
    LoggerD("%s", args.serialize().c_str());

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::value v_handle = data.at(ARCHIVE_FILE_HANDLE);

    const long handle = static_cast<long>(v_handle.get<double>());

    ArchiveFilePtr priv = ArchiveManager::getInstance().getPrivData(handle);
    priv->close();
    ArchiveManager::getInstance().erasePrivData(handle);

    ReportSuccess(out);
}

void ArchiveInstance::Extract(const picojson::value& args, picojson::object& out)
{
    LoggerD("Entered");
    LoggerD("%s", args.serialize().c_str());

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

    ExtractEntryProgressCallback *callback = new ExtractEntryProgressCallback();

    try {
        NodePtr node = Node::resolve(Path::create(v_dest_dir.get<std::string>()));
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
        ArchiveFilePtr archive_file_ptr = ArchiveManager::getInstance().getPrivData(handle);
        ArchiveFileEntryPtrMapPtr entries = archive_file_ptr->getEntryMap();
        auto it = entries->find(v_entry_name.get<std::string>());

        //Not found but if our name does not contain '/'
        //try looking for directory with such name
        //
        if (it == entries->end() && !isDirectoryPath(v_entry_name.get<std::string>())) {
            const std::string try_directory = v_entry_name.get<std::string>() + "/";
            LoggerD("GetEntryByName Trying directory: [%s]", try_directory.c_str());
            it = entries->find(try_directory);
        }

        it->second->extractTo(callback);
    }
    catch (...) {
        LoggerE("Exception occurred");
        delete callback;
        callback = NULL;
        throw;
    }
}

void ArchiveInstance::GetWidgetPaths(const picojson::value& args, picojson::object& out) {
    char *root_path = NULL;
    std::string pkg_id = CurrentApplication::GetInstance().GetPackageId();

    pkgmgrinfo_pkginfo_h handle = NULL;
    if (PMINFO_R_OK != pkgmgrinfo_pkginfo_get_pkginfo(pkg_id.c_str(), &handle)) {
        throw UnknownException("Error while getting package info");
    }

    if (PMINFO_R_OK != pkgmgrinfo_pkginfo_get_root_path(handle, &root_path)) {
        throw UnknownException("Error while getting package info");
    }

    // Construction of the response
    std::string root(root_path);
    LoggerD("root path: %s", root_path);

    pkgmgrinfo_pkginfo_destroy_pkginfo(handle);

    picojson::value result{picojson::object()};
    auto& result_obj = result.get<picojson::object>();
    result_obj.insert(std::make_pair("wgt-package", picojson::value(root + "/res/wgt")));
    result_obj.insert(std::make_pair("wgt-private", picojson::value(root + "/data")));
    result_obj.insert(std::make_pair("wgt-private-tmp", picojson::value(root + "/tmp")));

    ReportSuccess(result, out);
}

} // namespace archive
} // namespace extension
