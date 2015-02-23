// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ARCHIVE_ARCHIVE_INSTANCE_H_
#define ARCHIVE_ARCHIVE_INSTANCE_H_

#include "common/extension.h"
#include "common/platform_exception.h"
#include "common/platform_result.h"

namespace extension {
namespace archive {

class ArchiveInstance: public common::ParsedInstance {
public:
    static ArchiveInstance& getInstance();

private:
    ArchiveInstance();
    ArchiveInstance(ArchiveInstance const&);
    void operator=(ArchiveInstance const&);
    virtual ~ArchiveInstance();

    /* ArchiveManager methods */
    void Open(const picojson::value& args, picojson::object& out);
    void Abort(const picojson::value& args, picojson::object& out);

    /* ArchiveFile methods */
    void Add(const picojson::value& args, picojson::object& out);
    void ExtractAll(const picojson::value& args, picojson::object& out);
    void GetEntries(const picojson::value& args, picojson::object& out);
    void GetEntryByName(const picojson::value& args, picojson::object& out);
    void Close(const picojson::value& args, picojson::object& out);

    /* ArchiveFileEntry methods */
    void Extract(const picojson::value& args, picojson::object& out);

    /* Filesystem related method */
    void GetWidgetPaths(const picojson::value& args, picojson::object& out);

    void PostError(const common::PlatformException& e, double callback_id);
    void PostError(const common::PlatformResult& e, double callback_id);
};

} // namespace archive
} // namespace extension

#endif // ARCHIVE_ARCHIVE_INSTANCE_H_
