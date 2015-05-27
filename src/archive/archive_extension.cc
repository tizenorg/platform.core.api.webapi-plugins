// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "archive_extension.h"
#include "archive_instance.h"
#include "common/logger.h"

// This will be generated from archive_api.js
extern const char kSource_archive_api[];

common::Extension* CreateExtension()
{
    LoggerD("Enter");
    return new ArchiveExtension;
}

ArchiveExtension::ArchiveExtension()
{
    LoggerD("Enter");
    SetExtensionName("tizen.archive");
    SetJavaScriptAPI(kSource_archive_api);
}

ArchiveExtension::~ArchiveExtension()
{
    LoggerD("Enter");
}

common::Instance* ArchiveExtension::CreateInstance()
{
    LoggerD("Enter");
    return new extension::archive::ArchiveInstance();
}
