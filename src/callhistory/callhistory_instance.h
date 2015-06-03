// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CALLHISTORY_CALLHISTORY_INSTANCE_H_
#define CALLHISTORY_CALLHISTORY_INSTANCE_H_

#include "common/extension.h"
#include "callhistory.h"

namespace extension {
namespace callhistory {

class CallHistoryInstance : public common::ParsedInstance {
 public:
  CallHistoryInstance();
  virtual ~CallHistoryInstance();

  void CallHistoryChange(picojson::object& data);
 private:
  void Find(const picojson::value& args, picojson::object& out);
  void Remove(const picojson::value& args, picojson::object& out);
  void RemoveBatch(const picojson::value& args, picojson::object& out);
  void RemoveAll(const picojson::value& args, picojson::object& out);
  void AddChangeListener (const picojson::value& args, picojson::object& out);
  void RemoveChangeListener(const picojson::value& args, picojson::object& out);
  void SetMissedDirection(const picojson::value& args, picojson::object& out);

  CallHistory history_;
};

} // namespace callhistory
} // namespace extension

#endif // CALLHISTORY_CALLHISTORY_INSTANCE_H_
