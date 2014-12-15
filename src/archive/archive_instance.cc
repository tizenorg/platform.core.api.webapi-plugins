// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "archive/archive_instance.h"

#include <functional>

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"

namespace extension {
namespace archive {


using namespace common;
using namespace extension::archive;

ArchiveInstance::ArchiveInstance() {
    LOGD("Entered");

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
    LOGD("Entered");
}

void ArchiveInstance::Open(const picojson::value& args, picojson::object& out) {
    LOGD("Entered");
    const std::string& file = args.get("file").get<std::string>();

    ReportSuccess(out);
}

void ArchiveInstance::Abort(const picojson::value& args, picojson::object& out)
{
    LOGD("Entered");
    ReportSuccess(out);
}

void ArchiveInstance::Add(const picojson::value& args, picojson::object& out)
{
    LOGD("Entered");
    ReportSuccess(out);
}

void ArchiveInstance::ExtractAll(const picojson::value& args, picojson::object& out)
{
    LOGD("Entered");
    ReportSuccess(out);
}

void ArchiveInstance::GetEntries(const picojson::value& args, picojson::object& out)
{
    LOGD("Entered");
    ReportSuccess(out);
}

void ArchiveInstance::GetEntryByName(const picojson::value& args, picojson::object& out)
{
    LOGD("Entered");
    ReportSuccess(out);
}

void ArchiveInstance::Close(const picojson::value& args, picojson::object& out)
{
    LOGD("Entered");
    ReportSuccess(out);
}

void ArchiveInstance::Extract(const picojson::value& args, picojson::object& out)
{
    LOGD("Entered");
    ReportSuccess(out);
}

} // namespace archive
} // namespace extension
