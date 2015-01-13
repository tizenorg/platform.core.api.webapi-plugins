// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


// File copied from Crosswalk

#ifndef DATASYNC_DATASYNC_MANAGER_H_
#define DATASYNC_DATASYNC_MANAGER_H_

#include <sync_agent.h>

#include <map>
#include <string>
#include <utility>

#include "common/utils.h"
#include "datasync/datasync_error.h"
#include "datasync/sync_profile_info.h"
#include "datasync/sync_statistics.h"
#include "common/picojson.h"

namespace extension {
namespace datasync {

typedef std::map<std::string, std::map<std::string, int>> PlatformEnumMap;

class DatasyncInstance;

class DataSyncManager {
 public:
  typedef std::pair<int, DatasyncInstance*> CallbackPair;
  typedef std::map<int, CallbackPair> ProfileIdToCallbackMap;

  ~DataSyncManager();

  int Add(const picojson::object &args);
  ResultOrError<void> Update(SyncProfileInfo& profile_info);
  ResultOrError<void> Remove(const std::string& id);

  int GetMaxProfilesNum() const;
  int GetProfilesNum() const;

  ResultOrError<SyncProfileInfoPtr> Get(
      const std::string& profile_id) const;
  ResultOrError<SyncProfileInfoListPtr> GetAll() const;
  ResultOrError<SyncStatisticsListPtr> GetLastSyncStatistics(
      const std::string& profile_str_id) const;

  ResultOrError<void> StartSync(const std::string& profile_id_str,
      int callback_id, DatasyncInstance* instance);
  ResultOrError<void> StopSync(const std::string& profile_id_str);

  void UnregisterInstanceCallbacks(DatasyncInstance* instance);

  static DataSyncManager& Instance();

  static int StrToPlatformEnum(const std::string& field, const std::string& value);

 private:
  DataSyncManager();

  int StateChangedCallback(sync_agent_event_data_s* request);
  int ProgressCallback(sync_agent_event_data_s* request);
  void Item(ds_profile_h *profile_h, const picojson::object &args);

  ProfileIdToCallbackMap callbacks_;

  static bool sync_agent_initialized_;

  DISALLOW_COPY_AND_ASSIGN(DataSyncManager);

  static const PlatformEnumMap platform_enum_map_;
};

}  // namespace datasync
}  // namespace extension

#endif  // DATASYNC_DATASYNC_MANAGER_H_
