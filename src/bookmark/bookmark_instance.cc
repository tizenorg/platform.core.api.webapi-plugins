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

using common::ErrorCode;
using common::PlatformResult;

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

PlatformResult BookmarkInstance::BookmarkUrlExists(const char* url,
                                                   bool* exists) {
  LoggerD("Enter");
  int ids_count = 0;
  int* ids = nullptr;
  char* compare_url = nullptr;

  if (bp_bookmark_adaptor_get_ids_p(&ids,  // ids
                                    &ids_count,  // count
                                    -1,  //limit
                                    0,  // offset
                                    -1,  //parent
                                    -1,  //type
                                    -1,  // is_operator
                                    -1,  // is_editable
                                    BP_BOOKMARK_O_DATE_CREATED,  // order_offset
                                    0  // ordering ASC
                                    ) < 0) {
    LoggerE("Failed to obtain bookmarks");
    return PlatformResult{ErrorCode::UNKNOWN_ERR, "Failed to obtain bookmarks"};
  }

  PlatformResult result{ErrorCode::NO_ERROR};
  bool url_found = false;
  for (int i = 0; (i < ids_count) && result && !url_found; ++i) {
    if (bp_bookmark_adaptor_get_url(ids[i], &compare_url) < 0) {
      LoggerE("Failed to obtain URL");
      result = PlatformResult{ErrorCode::UNKNOWN_ERR, "Failed to obtain URL"};
    } else {
      url_found = (0 == strcmp(url, compare_url));
      free(compare_url);
      compare_url = nullptr;
    }
  }

  if (result) {
    *exists = url_found;
  }

  free(ids);

  return result;
}

PlatformResult BookmarkInstance::BookmarkTitleExistsInParent(const char* title,
                                                             int parent,
                                                             bool* exists) {

  LoggerD("Enter");
  int ids_count = 0;
  int compare_parent = -1;
  int* ids = nullptr;
  char* compare_title = nullptr;

  if (bp_bookmark_adaptor_get_ids_p(&ids,  // ids
                                    &ids_count,  // count
                                    -1,  //limit
                                    0,  // offset
                                    -1,  //parent
                                    -1,  //type
                                    -1,  // is_operator
                                    -1,  // is_editable
                                    BP_BOOKMARK_O_DATE_CREATED,  // order_offset
                                    0  // ordering ASC
                                    ) < 0) {
    LoggerE("Failed to obtain bookmarks");
    return PlatformResult{ErrorCode::UNKNOWN_ERR, "Failed to obtain bookmarks"};
  }

  PlatformResult result{ErrorCode::NO_ERROR};
  bool title_found = false;
  for (int i = 0; (i < ids_count) && result && !title_found; ++i) {
    if (bp_bookmark_adaptor_get_parent_id(ids[i], &compare_parent) < 0) {
      LoggerE("Failed to obtain parent ID");
      result = PlatformResult{ErrorCode::UNKNOWN_ERR, "Failed to obtain parent ID"};
    } else if (bp_bookmark_adaptor_get_title(ids[i], &compare_title) < 0) {
      LoggerE("Failed to obtain title");
      result = PlatformResult{ErrorCode::UNKNOWN_ERR, "Failed to obtain title"};
    } else {
      title_found = (parent == compare_parent) && (0 == strcmp(title, compare_title));
      free(compare_title);
      compare_title = nullptr;
      compare_parent = -1;
    }
  }

  if (result) {
    *exists = title_found;
  }

  free(ids);

  return result;
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

  const auto& title = arg.get(kTitle).get<std::string>();
  const int parent = static_cast<int>(arg.get(kParentId).get<double>());
  const int type = static_cast<int>(arg.get(kType).get<double>());
  const auto& url = arg.get(kUrl).get<std::string>();

  if (0 == type) {  // bookmark
    bool exists = false;
    auto result = BookmarkUrlExists(url.c_str(), &exists);
    if (!result) {
      ReportError(result, &o);
      return;
    } else if (exists) {
      ReportError(PlatformResult{ErrorCode::UNKNOWN_ERR, "Bookmark already exists"}, &o);
      return;
    }
  }

  if (1 == type) {  // folder
    bool exists = false;
    auto result = BookmarkTitleExistsInParent(title.c_str(), parent, &exists);
    if (!result) {
      ReportError(result, &o);
      return;
    } else if (exists) {
      ReportError(PlatformResult{ErrorCode::UNKNOWN_ERR, "Bookmark already exists"}, &o);
      return;
    }
  }

  if (bp_bookmark_adaptor_create(&saved_id) < 0) {
    ReportError(o);
    return;
  }
  if (bp_bookmark_adaptor_set_title(saved_id, title.c_str()) < 0) {
    bp_bookmark_adaptor_delete(saved_id);
    ReportError(o);
    return;
  }
  if (bp_bookmark_adaptor_set_parent_id(saved_id, parent) < 0) {
    bp_bookmark_adaptor_delete(saved_id);
    ReportError(o);
    return;
  }
  if (bp_bookmark_adaptor_set_type(saved_id, type) < 0) {
    bp_bookmark_adaptor_delete(saved_id);
    ReportError(o);
    return;
  }
  if (bp_bookmark_adaptor_set_url(saved_id, url.c_str()) < 0) {
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
