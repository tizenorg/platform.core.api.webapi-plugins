/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#ifndef ARCHIVE_ARCHIVE_INSTANCE_H_
#define ARCHIVE_ARCHIVE_INSTANCE_H_

#include "common/extension.h"
#include "common/platform_exception.h"
#include "common/platform_result.h"

namespace extension {
namespace archive {

class ArchiveInstance: public common::ParsedInstance {
public:
    ArchiveInstance();
    virtual ~ArchiveInstance();

private:
    ArchiveInstance(ArchiveInstance const&);
    void operator=(ArchiveInstance const&);

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
    void FetchVirtualRoots(const picojson::value& args, picojson::object& out);

    void PostError(const common::PlatformException& e, double callback_id);
    void PostError(const common::PlatformResult& e, double callback_id);
};

} // namespace archive
} // namespace extension

#endif // ARCHIVE_ARCHIVE_INSTANCE_H_
