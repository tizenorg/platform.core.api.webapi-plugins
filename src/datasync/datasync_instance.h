// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// File copied from Crosswalk

#ifndef DATASYNC_DATASYNC_INSTANCE_H_
#define DATASYNC_DATASYNC_INSTANCE_H_

#include <string>

#include "common/extension.h"
#include "common/picojson.h"

#include "datasync/datasync_manager.h"

namespace extension {
namespace datasync {

class DatasyncInstance : public common::ParsedInstance {
 public:
  DatasyncInstance();
  virtual ~DatasyncInstance();
  static DatasyncInstance& GetInstance();

 private:
  // Synchronous message handlers

  // with result
  void GetMaxProfilesNum(const picojson::value& args, picojson::object& out);
  void GetProfilesNum(const picojson::value& args, picojson::object& out);
  void Get(const picojson::value& args, picojson::object& out);
  void GetAll(const picojson::value& args, picojson::object& out);
  void GetLastSyncStatistics(const picojson::value& args, picojson::object& out);

  // undefined result
  void Add(const picojson::value& args, picojson::object& out);
  void Update(const picojson::value& args, picojson::object& out);
  void Remove(const picojson::value& args, picojson::object& out);
  void StartSync(const picojson::value& args, picojson::object& out);
  void StopSync(const picojson::value& args, picojson::object& out);
};

}  // namespace datasync
}  // namespace extension

#endif  // DATASYNC_DATASYNC_INSTANCE_H_
