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

#include "bookmark/bookmark_instance.h"

#include <web/bookmark-adaptor.h>
#include <string>

#include "common/platform_exception.h"
#include "common/converter.h"
#include "common/logger.h"

namespace extension {
namespace bookmark {

namespace {
  const char kId[] = "id";
  const char kTitle[] = "title";
  const char kType[] = "type";
  const char kParentId[] = "parentId";
  const char kUrl[] = "url";
}  // namespace

BookmarkInstance::BookmarkInstance() {
  LoggerD("Enter");
  using std::placeholders::_1;
  using std::placeholders::_2;

#define REGISTER_SYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&BookmarkInstance::x, this, _1, _2));
  REGISTER_SYNC("Bookmark_get", BookmarkGet);
  REGISTER_SYNC("Bookmark_add", BookmarkAdd);
  REGISTER_SYNC("Bookmark_remove", BookmarkRemove);
  REGISTER_SYNC("Bookmark_removeAll", BookmarkRemoveAll);
  REGISTER_SYNC("Bookmark_getRootId", BookmarkGetRootId);
#undef REGISTER_SYNC
}

BookmarkInstance::~BookmarkInstance() {
  LoggerD("Enter");
}

bool BookmarkInstance::bookmark_foreach(
    Context& ctx, bp_bookmark_info_fmt& info) {

  LoggerD("Enter");
  int ids_count = 0;
  int* ids = NULL;
  BookmarkObject item;
  if (bp_bookmark_adaptor_get_ids_p(&ids, &ids_count, -1, 0, -1, -1, -1, -1,
                                    BP_BOOKMARK_O_DATE_CREATED, 0) < 0)
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

bool BookmarkInstance::bookmark_url_exists(const char* url) {
  LoggerD("Enter");
  int ids_count = 0;
  int* ids = NULL;
  char* compare_url = NULL;

  if (bp_bookmark_adaptor_get_ids_p(&ids, &ids_count, -1, 0, -1, -1, -1, -1,
                                    BP_BOOKMARK_O_DATE_CREATED, 0) < 0)
    return true;
  if (ids_count > 0) {
    for (int i = 0; i < ids_count; i++) {
      if (bp_bookmark_adaptor_get_url(ids[i], &compare_url) < 0) {
        free(ids);
        return true;
      }
      if (strcmp(url, compare_url) == 0) {
        free(compare_url);
        free(ids);
        return true;
      }
    }
  }
  free(compare_url);
  free(ids);
  return false;
}

bool BookmarkInstance::bookmark_title_exists_in_parent(
    const char* title, int parent) {

  LoggerD("Enter");
  int ids_count = 0;
  int compare_parent = -1;
  int* ids = NULL;
  char* compare_title = NULL;

  if (bp_bookmark_adaptor_get_ids_p(&ids, &ids_count, -1, 0, -1, -1, -1, -1,
                                    BP_BOOKMARK_O_DATE_CREATED, 0) < 0)
    return true;
  if (ids_count > 0) {
    for (int i = 0; i < ids_count; i++) {
      if (bp_bookmark_adaptor_get_title(ids[i], &compare_title) < 0) {
        free(ids);
        return true;
      }
      if (bp_bookmark_adaptor_get_parent_id(ids[i], &compare_parent) < 0) {
        free(compare_title);
        free(ids);
        return true;
      }
      if (strcmp(title, compare_title) == 0
        && (parent == compare_parent)) {
        free(compare_title);
        free(ids);
        return true;
      }
    }
  }
  free(compare_title);
  free(ids);
  return false;
}

void BookmarkInstance::BookmarkGet(
    const picojson::value& arg, picojson::object& o) {

  LoggerD("Enter");
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

void BookmarkInstance::BookmarkAdd(
    const picojson::value& arg, picojson::object& o) {

  LoggerD("Enter");
  int saved_id =-1;
  bp_bookmark_info_fmt data = {0};

  data.title  = const_cast<char*>(arg.get(kTitle).to_str().c_str());
  data.parent = arg.get(kParentId).get<double>();
  data.type   = arg.get(kType).get<double>();
  data.url    = const_cast<char*>(arg.get(kUrl).to_str().c_str());

  if (!data.type && bookmark_url_exists(data.url)) {
    ReportError(o);
    return;
  }
  if (data.type && bookmark_title_exists_in_parent(data.title, data.parent)) {
    ReportError(o);
    return;
  }

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

void BookmarkInstance::BookmarkRemove(
    const picojson::value& arg, picojson::object& o) {

  LoggerD("Enter");
  int id = common::stol(
      common::FromJson<std::string>(arg.get<picojson::object>(), kId));
  if (bp_bookmark_adaptor_delete(id) < 0) {
    ReportError(o);
    return;
  }
  ReportSuccess(o);
}

void BookmarkInstance::BookmarkRemoveAll(
    const picojson::value& msg, picojson::object& o) {

  LoggerD("Enter");
  if (bp_bookmark_adaptor_reset() < 0) {
    ReportError(o);
    return;
  }
  ReportSuccess(o);
}

void BookmarkInstance::BookmarkGetRootId(
    const picojson::value& msg, picojson::object& o) {

  LoggerD("Enter");
  int rootId(0);
  if (bp_bookmark_adaptor_get_root(&rootId) < 0) {
    ReportError(o);
    return;
  }
  ReportSuccess(picojson::value(std::to_string(rootId)), o);
}
}  // namespace bookmark
}  // namespace extension
