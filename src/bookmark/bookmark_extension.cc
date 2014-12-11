// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "bookmark/bookmark_extension.h"
#include "bookmark/bookmark_instance.h"

namespace {
  const char kBookmark[] = "tizen.bookmark";
  const char kBookmarkItem[] = "tizen.BookmarkItem";
  const char kBookmarkFolder[] = "tizen.BookmarkFolder";
}

// This will be generated from bookmark_api.js.
extern const char kSource_bookmark_api[];

common::Extension* CreateExtension() {
  return new BookmarkExtension;
}

BookmarkExtension::BookmarkExtension() {
  SetExtensionName(kBookmark);
  SetJavaScriptAPI(kSource_bookmark_api);

  const char* entry_points[] = {
    kBookmarkItem,
    kBookmarkFolder,
    NULL
  };
  SetExtraJSEntryPoints(entry_points);
}

BookmarkExtension::~BookmarkExtension() {}

common::Instance* BookmarkExtension::CreateInstance() {
  return new extension::bookmark::BookmarkInstance;
}
