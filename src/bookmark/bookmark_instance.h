// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BOOKMARK_BOOKMARK_INSTANCE_H_
#define BOOKMARK_BOOKMARK_INSTANCE_H_

#include "common/extension.h"
#include "common/picojson.h"

namespace extension {
namespace bookmark {

class BookmarkInstance : public common::Instance {
 public:
  BookmarkInstance();
  virtual ~BookmarkInstance();

 private:
  virtual void HandleMessage(const char* msg);
  virtual void HandleSyncMessage(const char* msg);

  void HandleGet(const picojson::value& arg, picojson::value::object& o);
  void HandleAdd(const picojson::value& arg, picojson::value::object& o);
  void HandleRemove(const picojson::value& arg, picojson::value::object& o);
};

} // namespace bookmark
} // namespace extension

#endif  // BOOKMARK_BOOKMARK_INSTANCE_H_
