// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ARCHIVE_ARCHIVE_EXTENSION_H_
#define ARCHIVE_ARCHIVE_EXTENSION_H_

#include "common/extension.h"

class ArchiveExtension: public common::Extension {
public:
    ArchiveExtension();
    virtual ~ArchiveExtension();

private:
    virtual common::Instance* CreateInstance();
};

#endif // ARCHIVE_ARCHIVE_EXTENSION_H_
