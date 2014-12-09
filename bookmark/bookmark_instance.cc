// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "bookmark/bookmark_instance.h"

#include <string>

namespace extension {
namespace bookmark {

namespace {
  const char* kGet = "get";
  const char* kAdd = "add";
  const char* kRemove = "remove";
  const char* kCmd = "cmd";
  const char* kArg = "arg";
  const char* kError = "error";
  const char* kValue = "value";
  const char* kNotImplemented = "Not implemented";
}

BookmarkInstance::BookmarkInstance() {}

BookmarkInstance::~BookmarkInstance() {}

void BookmarkInstance::HandleMessage(const char* msg) {}

void BookmarkInstance::HandleSyncMessage(const char* msg) {
  picojson::value v;
  std::string err;

  picojson::parse(v, msg, msg + strlen(msg), &err);
  if (!err.empty()) {
    return;
  }

  std::string cmd = v.get(kCmd).to_str();
  picojson::value arg = v.get(kArg);
  picojson::value::object o;

  if (cmd == kGet) {
    HandleGet(arg, o);
  } else if (cmd == kAdd) {
    HandleAdd(arg, o);
  } else if (cmd == kRemove) {
    HandleRemove(arg, o);
  }

  if (o.empty())
    o[kError] = picojson::value(true);

  SendSyncReply(picojson::value(o).serialize().c_str());
}

void BookmarkInstance::HandleGet(const picojson::value& arg, picojson::object& o) {
  o[kValue] = picojson::value(kNotImplemented);
}

void BookmarkInstance::HandleAdd(const picojson::value& arg, picojson::object& o) {
  o[kValue] = picojson::value(kNotImplemented);
}

void BookmarkInstance::HandleRemove(const picojson::value& arg, picojson::object& o) {
  o[kValue] = picojson::value(kNotImplemented);
}

} //namespace bookmark
} //namespace extension
