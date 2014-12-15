// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "archive/archive_extension.h"

#include "archive/archive_instance.h"

// This will be generated from archive_api.js
extern const char kSource_archive_api[];

common::Extension* CreateExtension()
{
    return new ArchiveExtension;
}

ArchiveExtension::ArchiveExtension()
{
    SetExtensionName("tizen.archive");
    SetJavaScriptAPI(kSource_archive_api);
}

ArchiveExtension::~ArchiveExtension()
{
}

common::Instance* ArchiveExtension::CreateInstance()
{
    return new extension::archive::ArchiveInstance;
}
