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

#include "bookmark/bookmark_extension.h"
#include "bookmark/bookmark_instance.h"
#include "common/logger.h"

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

  if (bp_bookmark_adaptor_initialize()) {
     LOGGER(ERROR) << "Fail: Bookmark not supported";
  }
}

BookmarkExtension::~BookmarkExtension() {
  if (bp_bookmark_adaptor_deinitialize()) {
    LOGGER(ERROR) << "Fail: Deinitialize Bookmark";
  }
}

common::Instance* BookmarkExtension::CreateInstance() {
  return new extension::bookmark::BookmarkInstance;
}
