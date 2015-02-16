// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// File copied from Crosswalk

#include "datasync/datasync_instance.h"

#include <memory>

#include "tizen/tizen.h"
#include "common/task-queue.h"

namespace extension {
namespace datasync {

namespace {
// The privileges that required in Datasynchronization API
const std::string kPrivilegeDatasynchronization =
    "http://tizen.org/privilege/datasync";

}  // namespace

using namespace common;

DatasyncInstance &DatasyncInstance::GetInstance() {
  static DatasyncInstance instance;
  return instance;
}

DatasyncInstance::DatasyncInstance() {
  using namespace std::placeholders;
#define REGISTER_SYNC(c, x) \
  RegisterSyncHandler(c, std::bind(&DatasyncInstance::x, this, _1, _2));
  REGISTER_SYNC("DataSynchronizationManager_add",
                DataSynchronizationManagerAdd);
  REGISTER_SYNC("DataSynchronizationManager_update",
                DataSynchronizationManagerUpdate);
  REGISTER_SYNC("DataSynchronizationManager_remove",
                DataSynchronizationManagerRemove);
  REGISTER_SYNC("DataSynchronizationManager_getMaxProfilesNum",
                DataSynchronizationManagerGetMaxProfilesNum);
  REGISTER_SYNC("DataSynchronizationManager_getProfilesNum",
                DataSynchronizationManagerGetProfilesNum);
  REGISTER_SYNC("DataSynchronizationManager_get",
                DataSynchronizationManagerGet);
  REGISTER_SYNC("DataSynchronizationManager_getAll",
                DataSynchronizationManagerGetAll);
  REGISTER_SYNC("DataSynchronizationManager_startSync",
                DataSynchronizationManagerStartSync);
  REGISTER_SYNC("DataSynchronizationManager_stopSync",
                DataSynchronizationManagerStopSync);
  REGISTER_SYNC("DataSynchronizationManager_getLastSyncStatistics",
                DataSynchronizationManagerGetLastSyncStatistics);
#undef REGISTER_SYNC
}

DatasyncInstance::~DatasyncInstance() {}

void DatasyncInstance::DataSynchronizationManagerGetMaxProfilesNum(
    const picojson::value &args, picojson::object &out) {
  ReportSuccess(picojson::value(static_cast<double>(
                    DataSyncManager::Instance().GetMaxProfilesNum())),
                out);
}

void DatasyncInstance::DataSynchronizationManagerGetProfilesNum(
    const picojson::value &args, picojson::object &out) {
  int profiles_num;

  PlatformResult status =
      DataSyncManager::Instance().GetProfilesNum(&profiles_num);

  if (status.IsSuccess())
    ReportSuccess(picojson::value(static_cast<double>(profiles_num)), out);
  else
    ReportError(status, &out);
}

void DatasyncInstance::DataSynchronizationManagerGet(
    const picojson::value &args, picojson::object &out) {
  picojson::value result = picojson::value(picojson::object());

  PlatformResult status = DataSyncManager::Instance().Get(
      args.get("profileId").get<std::string>(), result.get<picojson::object>());
  if (status.IsSuccess())
    ReportSuccess(result, out);
  else
    ReportError(status, &out);
}

void DatasyncInstance::DataSynchronizationManagerGetAll(
    const picojson::value &args, picojson::object &out) {
  picojson::value result = picojson::value(picojson::array());

  PlatformResult status =
      DataSyncManager::Instance().GetAll(result.get<picojson::array>());

  if (status.IsSuccess())
    ReportSuccess(result, out);
  else
    ReportError(status, &out);
}

void DatasyncInstance::DataSynchronizationManagerGetLastSyncStatistics(
    const picojson::value &args, picojson::object &out) {
  picojson::value result = picojson::value(picojson::array());

  DataSyncManager::Instance().GetLastSyncStatistics(
      args.get("profileId").get<std::string>(), result.get<picojson::array>());

  ReportSuccess(result, out);
}

void DatasyncInstance::DataSynchronizationManagerAdd(
    const picojson::value &args, picojson::object &out) {
  int profile_id;

  PlatformResult status = DataSyncManager::Instance().Add(
      args.get<picojson::object>(), &profile_id);

  if (status.IsSuccess())
    ReportSuccess(picojson::value(static_cast<double>(profile_id)), out);
  else
    ReportError(status, &out);
}

void DatasyncInstance::DataSynchronizationManagerUpdate(
    const picojson::value &args, picojson::object &out) {
  PlatformResult status =
      DataSyncManager::Instance().Update(args.get<picojson::object>());

  if (status.IsSuccess())
    ReportSuccess(out);
  else
    ReportError(status, &out);
}

void DatasyncInstance::DataSynchronizationManagerRemove(
    const picojson::value &args, picojson::object &out) {
  PlatformResult status = DataSyncManager::Instance().Remove(
      args.get("profileId").get<std::string>());

  if (status.IsSuccess())
    ReportSuccess(out);
  else
    ReportError(status, &out);
}

void DatasyncInstance::DataSynchronizationManagerStartSync(
    const picojson::value &args, picojson::object &out) {
  PlatformResult status =
      DataSyncManager::Instance().StartSync(args.get<picojson::object>());

  if (status.IsSuccess())
    ReportSuccess(out);
  else
    ReportError(status, &out);
}

void DatasyncInstance::DataSynchronizationManagerStopSync(
    const picojson::value &args, picojson::object &out) {
  PlatformResult status = DataSyncManager::Instance().StopSync(
      args.get("profileId").get<std::string>());

  if (status.IsSuccess())
    ReportSuccess(out);
  else
    ReportError(status, &out);
}

}  // namespace datasync
}  // namespace extension
