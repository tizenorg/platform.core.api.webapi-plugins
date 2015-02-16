// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
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
  void DataSynchronizationManagerGetMaxProfilesNum(const picojson::value& args,
                                                   picojson::object& out);
  void DataSynchronizationManagerGetProfilesNum(const picojson::value& args,
                                                picojson::object& out);
  void DataSynchronizationManagerGet(const picojson::value& args,
                                     picojson::object& out);
  void DataSynchronizationManagerGetAll(const picojson::value& args,
                                        picojson::object& out);
  void DataSynchronizationManagerGetLastSyncStatistics(
      const picojson::value& args, picojson::object& out);

  // undefined result
  void DataSynchronizationManagerAdd(const picojson::value& args,
                                     picojson::object& out);
  void DataSynchronizationManagerUpdate(const picojson::value& args,
                                        picojson::object& out);
  void DataSynchronizationManagerRemove(const picojson::value& args,
                                        picojson::object& out);
  void DataSynchronizationManagerStartSync(const picojson::value& args,
                                           picojson::object& out);
  void DataSynchronizationManagerStopSync(const picojson::value& args,
                                          picojson::object& out);
};

}  // namespace datasync
}  // namespace extension

#endif  // DATASYNC_DATASYNC_INSTANCE_H_
