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
  void Bookmark_get(const picojson::value& arg, picojson::object& o);
  void Bookmark_add(const picojson::value& arg, picojson::object& o);
  void Bookmark_remove(const picojson::value& arg, picojson::object& o);
  void Bookmark_removeAll(const picojson::value& msg, picojson::object& o);
  void Bookmark_getRootId(const picojson::value& msg, picojson::object& o);
};
}  // namespace bookmark
}  // namespace extension
#endif  // BOOKMARK_BOOKMARK_INSTANCE_H_
