// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


// File copied from Crosswalk

#include <memory>

#include "datasync/datasync_instance.h"
#include "datasync/datasync_serialization.h"
#include "tizen/tizen.h"


namespace extension {
namespace datasync {

using namespace common;

DatasyncInstance::DatasyncInstance() {
    using namespace std::placeholders;
    #define REGISTER_SYNC(c, x) \
        RegisterSyncHandler(c, std::bind(&DatasyncInstance::x, this, _1, _2));
        REGISTER_SYNC("Datasync_add", Add);
        REGISTER_SYNC("Datasync_update", Update);
        REGISTER_SYNC("Datasync_remove", Remove);
        REGISTER_SYNC("Datasync_getMaxProfilesNum", GetMaxProfilesNum);
        REGISTER_SYNC("Datasync_getProfilesNum", GetProfilesNum);
        REGISTER_SYNC("Datasync_get", Get);
        REGISTER_SYNC("Datasync_getAll", GetAll);
        REGISTER_SYNC("Datasync_startSync", StartSync);
        REGISTER_SYNC("Datasync_stopSync", StopSync);
        REGISTER_SYNC("Datasync_getLastSyncStatistics", GetLastSyncStatistics);
    #undef REGISTER_SYNC
}

DatasyncInstance::~DatasyncInstance() {
}

void DatasyncInstance::GetMaxProfilesNum(const picojson::value &args, picojson::object &out) {
  ReportSuccess(
      picojson::value(static_cast<double>(DataSyncManager::Instance().GetMaxProfilesNum())), out);
}

void DatasyncInstance::GetProfilesNum(const picojson::value &args, picojson::object &out) {
  ReportSuccess(
      picojson::value(static_cast<double>(DataSyncManager::Instance().GetProfilesNum())), out);
}

void DatasyncInstance::Get(const picojson::value &args, picojson::object &out) {
//  TODO: implementation

    picojson::value val{picojson::object{}};

    ReportSuccess(val, out);
}

void DatasyncInstance::GetAll(const picojson::value &args, picojson::object &out) {
//  TODO: implementation

    picojson::value val{picojson::object{}};

    ReportSuccess(val, out);
}

void DatasyncInstance::GetLastSyncStatistics(const picojson::value &args, picojson::object &out) {
//  TODO: implementation

    picojson::value val{picojson::object{}};

    ReportSuccess(val, out);
}

void DatasyncInstance::Add(const picojson::value &args, picojson::object &out) {
  ReportSuccess(
      picojson::value(static_cast<double>(DataSyncManager::Instance().Add(
                                              args.get<picojson::object>()))), out);
}

void DatasyncInstance::Update(const picojson::value &args, picojson::object &out) {
//  TODO: implementation

    picojson::value val{picojson::object{}};

    ReportSuccess(val, out);
}

void DatasyncInstance::Remove(const picojson::value &args, picojson::object &out) {
//  TODO: implementation

    picojson::value val{picojson::object{}};

    ReportSuccess(val, out);
}

void DatasyncInstance::StartSync(const picojson::value &args, picojson::object &out) {
//  TODO: implementation

    picojson::value val{picojson::object{}};

    ReportSuccess(val, out);
}

void DatasyncInstance::StopSync(const picojson::value &args, picojson::object &out) {
//  TODO: implementation

    picojson::value val{picojson::object{}};

    ReportSuccess(val, out);
}

}  // namespace datasync
}  // namespace extension
