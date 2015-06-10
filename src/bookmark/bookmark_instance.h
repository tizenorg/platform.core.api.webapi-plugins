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

#ifndef BOOKMARK_BOOKMARK_INSTANCE_H_
#define BOOKMARK_BOOKMARK_INSTANCE_H_

#include <web/web_bookmark.h>
#include <vector>

#include "common/extension.h"
#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace bookmark {

struct BookmarkObject {
  int id;
  bp_bookmark_info_fmt bookmark_info;
};

struct Context {
  int id;
  int shouldGetItems;
  std::vector<BookmarkObject> folders;
};

class BookmarkInstance : public common::ParsedInstance {
 public:
  BookmarkInstance();
  virtual ~BookmarkInstance();

 private:
  bool bookmark_foreach(Context& ctx, bp_bookmark_info_fmt& info);
  common::PlatformResult BookmarkUrlExists(const char* url, bool* exists);
  common::PlatformResult BookmarkTitleExistsInParent(const char* title,
                                                     int parent, bool* exists);
  void BookmarkGet(const picojson::value& arg, picojson::object& o);
  void BookmarkAdd(const picojson::value& arg, picojson::object& o);
  void BookmarkRemove(const picojson::value& arg, picojson::object& o);
  void BookmarkRemoveAll(const picojson::value& msg, picojson::object& o);
  void BookmarkGetRootId(const picojson::value& msg, picojson::object& o);
};
}  // namespace bookmark
}  // namespace extension
#endif  // BOOKMARK_BOOKMARK_INSTANCE_H_
