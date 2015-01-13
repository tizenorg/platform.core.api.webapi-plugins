// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "bookmark/bookmark_instance.h"

#include <web/bookmark-adaptor.h>
#include <string>
#include "common/logger.h"

#include "common/platform_exception.h"
#include "common/converter.h"

namespace extension {
namespace bookmark {

namespace {
  const char kGet[] = "Bookmark_get";
  const char kAdd[] = "Bookmark_add";
  const char kRemove[] = "Bookmark_remove";
  const char kRemoveAll[] = "Bookmark_removeAll";
  const char kGetRootId[] = "Bookmark_getRootId";
  const char kId[] = "id";
  const char kTitle[] = "title";
  const char kType[] = "type";
  const char kParentId[] = "parentId";
  const char kUrl[] = "url";
}  // namespace

BookmarkInstance::BookmarkInstance() {
  using namespace std::placeholders;
#define REGISTER_SYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&BookmarkInstance::x, this, _1, _2));
  REGISTER_SYNC(kGet, Bookmark_get);
  REGISTER_SYNC(kAdd, Bookmark_add);
  REGISTER_SYNC(kRemove, Bookmark_remove);
  REGISTER_SYNC(kRemoveAll, Bookmark_removeAll);
  REGISTER_SYNC(kGetRootId, Bookmark_getRootId);
#undef REGISTER_SYNC
  if (bp_bookmark_adaptor_initialize()) {
    throw common::NotSupportedException("Fail: Bookmark not supported");
  }
}

BookmarkInstance::~BookmarkInstance() {
  if (bp_bookmark_adaptor_deinitialize()) {
    throw common::NotSupportedException("Fail: Deinitialize Bookmark");
  }
}

bool BookmarkInstance::bookmark_foreach(
    Context& ctx, bp_bookmark_info_fmt& info) {
  int ids_count = 0;
  int *ids = NULL;
  BookmarkObject item;
  if (bp_bookmark_adaptor_get_full_ids_p(&ids, &ids_count) < 0)
    return false;
  if (ids_count > 0) {
    for (int i = 0; i < ids_count; i++) {
      bp_bookmark_adaptor_get_easy_all(ids[i], &info);
      item.id = ids[i];
      item.bookmark_info = info;
      if ((ctx.shouldGetItems && item.bookmark_info.parent != ctx.id) ||
        (!ctx.shouldGetItems && item.id != ctx.id))
        continue;
      ctx.folders.push_back(item);
    }
  }
  free(ids);
  return true;
}

void BookmarkInstance::Bookmark_get(
    const picojson::value& arg, picojson::object& o) {
  Context ctx = {0};
  bp_bookmark_info_fmt info = {0};
  picojson::value::array arr;

  ctx.shouldGetItems = arg.get("shouldGetItems").get<double>();
  ctx.id             = arg.get(kId).get<double>();

  if (!bookmark_foreach(ctx, info)) {
    ReportError(o);
    return;
  }

  std::vector<BookmarkObject>::iterator it;
  for (it = ctx.folders.begin(); it!= ctx.folders.end(); ++it) {
    picojson::object obj;
    BookmarkObject entry = *it;

    obj[kTitle] = picojson::value(entry.bookmark_info.title);
    obj[kId] = picojson::value(std::to_string(entry.id));
    obj[kType] = picojson::value(std::to_string(entry.bookmark_info.type));
    obj[kParentId] = picojson::value(std::to_string(
        entry.bookmark_info.parent));
    if (!entry.bookmark_info.type)
      obj[kUrl] = picojson::value(entry.bookmark_info.url);

    arr.push_back(picojson::value(obj));
  }
  ReportSuccess(picojson::value(arr), o);
}

void BookmarkInstance::Bookmark_add(
    const picojson::value& arg, picojson::object& o) {
  int saved_id =-1;
  bp_bookmark_info_fmt data = {0};

  data.title  = const_cast<char*>(arg.get(kTitle).to_str().c_str());
  data.parent = arg.get(kParentId).get<double>();
  data.type   = arg.get(kType).get<double>();
  data.url    = const_cast<char*>(arg.get(kUrl).to_str().c_str());
  if (bp_bookmark_adaptor_create(&saved_id) < 0) {
    ReportError(o);
    return;
  }
  if (bp_bookmark_adaptor_set_title(saved_id, data.title) < 0) {
    bp_bookmark_adaptor_delete(saved_id);
    ReportError(o);
    return;
  }
  if (bp_bookmark_adaptor_set_parent_id(saved_id, data.parent) < 0) {
    bp_bookmark_adaptor_delete(saved_id);
    ReportError(o);
    return;
  }
  if (bp_bookmark_adaptor_set_type(saved_id, data.type) < 0) {
    bp_bookmark_adaptor_delete(saved_id);
    ReportError(o);
    return;
  }
  if (bp_bookmark_adaptor_set_url(saved_id, data.url) < 0) {
    bp_bookmark_adaptor_delete(saved_id);
    ReportError(o);
    return;
  }
  ReportSuccess(picojson::value(std::to_string(saved_id)), o);
}

void BookmarkInstance::Bookmark_remove(
    const picojson::value& arg, picojson::object& o) {
  int id = common::stol(
      common::FromJson<std::string>(arg.get<picojson::object>(), kId));
  if (bp_bookmark_adaptor_delete(id) < 0) {
    ReportError(o);
    return;
  }
  if (bp_bookmark_adaptor_clear_deleted_ids() < 0) {
    ReportError(o);
    return;
  }
  ReportSuccess(o);
}

void BookmarkInstance::Bookmark_removeAll(
    const picojson::value& msg, picojson::object& o) {
  if (bp_bookmark_adaptor_reset() < 0) {
    ReportError(o);
    return;
  }
  ReportSuccess(o);
}

void BookmarkInstance::Bookmark_getRootId(
    const picojson::value& msg, picojson::object& o) {
  int rootId(0);
  if (bp_bookmark_adaptor_get_root(&rootId) < 0) {
    ReportError(o);
    return;
  }
  ReportSuccess(picojson::value(std::to_string(rootId)), o);
}
}  // namespace bookmark
}  // namespace extension
