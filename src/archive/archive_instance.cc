// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "archive/archive_instance.h"


//#include "archive_manager.h"

#include <functional>
#include <memory>

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
    picojson::value v_file = data.at("file");
    picojson::value v_mode = data.at("mode");
    //picojson::object options = data.at("options").get<picojson::object>();
    //picojson::value v_overwrite = options.at("overwrite");
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();
    const long operationId = static_cast<long>(data.at("opId").get<double>());

    /*
    bool overwrite = false;
    if (v_overwrite.is<bool>()) {
        overwrite = v_overwrite.get<bool>();
    }
    */

    FileMode fm = stringToFileMode(v_mode.get<std::string>());
    const std::string& file = v_file.get<std::string>();
    auto json = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_CALLBACK_ID] = picojson::value(callbackId);

    LoggerD("opId: %d", operationId);
    LoggerD("filename: %s", file.c_str());
    LoggerD("callbackId: %d", callbackId);

    OpenCallbackData *callback = new OpenCallbackData();

    NodePtr node = Node::resolve(Path::create(file));
    FilePtr file_ptr = FilePtr(new File(node, File::PermissionList()));
    ArchiveFilePtr a_ptr = ArchiveFilePtr(new ArchiveFile(FileMode::READ));
    a_ptr->setFile(file_ptr);

    callback->setArchiveFile(a_ptr);
    //callback->setFile(file);
    callback->setOperationId(operationId);
    callback->setCallbackId(callbackId);
    //callback->setMode(FileMode::READ);
    //callback->setJson(json);
    ArchiveManager::getInstance().open(callback);
}

void ArchiveInstance::Abort(const picojson::value& args, picojson::object& out)
{
    LoggerD("Entered");
    ReportSuccess(out);
}

void ArchiveInstance::Add(const picojson::value& args, picojson::object& out)
{
    LoggerD("Entered");
    ReportSuccess(out);
}

void ArchiveInstance::ExtractAll(const picojson::value& args, picojson::object& out)
{
    LoggerD("Entered");
    ReportSuccess(out);
}

void ArchiveInstance::GetEntries(const picojson::value& args, picojson::object& out)
{
    LoggerD("Entered");
    ReportSuccess(out);
}

void ArchiveInstance::GetEntryByName(const picojson::value& args, picojson::object& out)
{
    LoggerD("Entered");
    ReportSuccess(out);
}

void ArchiveInstance::Close(const picojson::value& args, picojson::object& out)
{
    LoggerD("Entered");
    ReportSuccess(out);
}

void ArchiveInstance::Extract(const picojson::value& args, picojson::object& out)
{
    LoggerD("Entered");
    ReportSuccess(out);
}

} // namespace archive
} // namespace extension
