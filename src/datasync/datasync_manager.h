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
#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace datasync {

typedef std::map<std::string, std::map<std::string, int>> PlatformEnumMap;

class DatasyncInstance;

class DataSyncManager {
 public:
  ~DataSyncManager();

  int Add(const picojson::object& args);
  void Update(const picojson::object& args);
  void Remove(const std::string& id);

  int GetMaxProfilesNum() const;
  int GetProfilesNum() const;

  void Get(const std::string& id, picojson::object& out);
  void GetAll(picojson::array& out);
  void GetLastSyncStatistics(const std::string& id, picojson::array& out);

  void StartSync(const picojson::object& args);
  void StopSync(const std::string& id);

  static DataSyncManager& Instance();

  static int StrToPlatformEnum(const std::string& field, const std::string& value);
  static std::string PlatformEnumToStr(const std::string& field, const int value);

 private:
  DataSyncManager();

  int StateChangedCallback(sync_agent_event_data_s* request);
  int ProgressCallback(sync_agent_event_data_s* request);
  void Item(ds_profile_h* profile_h, const picojson::object& args);
  void Item(const std::string& id, ds_profile_h* profile_h, picojson::object& out);
  void GetProfileId(sync_agent_event_data_s* request, std::string& profile_id);
  void Failed(picojson::object& response_obj, picojson::object& answer_obj,
              const common::ErrorCode& code, const std::string& name,
              const std::string& message);
  inline void PrepareResponseObj(const std::string& profile_id, picojson::value& response,
                                 picojson::object& response_obj, picojson::value& answer,
                                 picojson::object& answer_obj);

  std::map<std::string, std::string> callbacks_;

  static bool sync_agent_initialized_;

  DISALLOW_COPY_AND_ASSIGN(DataSyncManager);

  static const PlatformEnumMap platform_enum_map_;
};

}  // namespace datasync
}  // namespace extension

#endif  // DATASYNC_DATASYNC_MANAGER_H_
