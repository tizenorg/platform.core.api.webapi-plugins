// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BOOKMARK_BOOKMARK_INSTANCE_H_
#define BOOKMARK_BOOKMARK_INSTANCE_H_

#include <web/web_bookmark.h>
#include <vector>

#include "common/extension.h"
#include "common/picojson.h"

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
  bool bookmark_url_exists(const char* url);
  bool bookmark_title_exists_in_parent(const char* title, int parent);
  void BookmarkGet(const picojson::value& arg, picojson::object& o);
  void BookmarkAdd(const picojson::value& arg, picojson::object& o);
  void BookmarkRemove(const picojson::value& arg, picojson::object& o);
  void BookmarkRemoveAll(const picojson::value& msg, picojson::object& o);
  void BookmarkGetRootId(const picojson::value& msg, picojson::object& o);
};
}  // namespace bookmark
}  // namespace extension
#endif  // BOOKMARK_BOOKMARK_INSTANCE_H_
