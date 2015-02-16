// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// File copied from Crosswalk

#include "datasync/datasync_manager.h"

#include "common/scope_exit.h"
#include "datasync/datasync_instance.h"
#include "common/logger.h"
#include "common/converter.h"

namespace extension {
namespace datasync {

using namespace common;

const int MAX_PROFILES_NUM = 5;

const std::string kDefaultEnumKey = "_DEFAULT";
const std::string kSyncMode = "SyncMode";
const std::string kSyncType = "SyncType";
const std::string kSyncInterval = "SyncInterval";
const std::string kSyncServiceType = "SyncServiceType";
const std::string kSyncServiceTypeToSrcUri = "SyncServiceTypeToSrcUri";
const std::string kSyncStatus = "SyncStatus";

const PlatformEnumMap DataSyncManager::platform_enum_map_ = {
    {kSyncMode,
     {{kDefaultEnumKey, SYNC_AGENT_SYNC_MODE_MANUAL},
      {"MANUAL", SYNC_AGENT_SYNC_MODE_MANUAL},
      {"PERIODIC", SYNC_AGENT_SYNC_MODE_PERIODIC},
      {"PUSH", SYNC_AGENT_SYNC_MODE_PUSH}}},
    {kSyncType,
     {{kDefaultEnumKey, SYNC_AGENT_SYNC_TYPE_UPDATE_BOTH},
      {"TWO_WAY", SYNC_AGENT_SYNC_TYPE_UPDATE_BOTH},
      {"SLOW", SYNC_AGENT_SYNC_TYPE_FULL_SYNC},
      {"ONE_WAY_FROM_CLIENT", SYNC_AGENT_SYNC_TYPE_UPDATE_TO_SERVER},
      {"REFRESH_FROM_CLIENT", SYNC_AGENT_SYNC_TYPE_REFRESH_FROM_PHONE},
      {"ONE_WAY_FROM_SERVER", SYNC_AGENT_SYNC_TYPE_UPDATE_TO_PHONE},
      {"REFRESH_FROM_SERVER", SYNC_AGENT_SYNC_TYPE_REFRESH_FROM_SERVER}}},
    {kSyncInterval,
     {{kDefaultEnumKey, SYNC_AGENT_SYNC_INTERVAL_1_WEEK},
      {"5_MINUTES", SYNC_AGENT_SYNC_INTERVAL_5_MINUTES},
      {"15_MINUTES", SYNC_AGENT_SYNC_INTERVAL_15_MINUTES},
      {"1_HOUR", SYNC_AGENT_SYNC_INTERVAL_1_HOUR},
      {"4_HOURS", SYNC_AGENT_SYNC_INTERVAL_4_HOURS},
      {"12_HOURS", SYNC_AGENT_SYNC_INTERVAL_12_HOURS},
      {"1_DAY", SYNC_AGENT_SYNC_INTERVAL_1_DAY},
      {"1_WEEK", SYNC_AGENT_SYNC_INTERVAL_1_WEEK},
      {"1_MONTH", SYNC_AGENT_SYNC_INTERVAL_1_MONTH},
      {"NONE", SYNC_AGENT_SYNC_INTERVAL_NONE}}},
    {kSyncServiceType,
     {{kDefaultEnumKey, SYNC_AGENT_CONTACT},
      {"CONTACT", SYNC_AGENT_CONTACT},
      {"EVENT", SYNC_AGENT_CALENDAR}}},
    {kSyncServiceTypeToSrcUri,
     {{kDefaultEnumKey, SYNC_AGENT_SRC_URI_CONTACT},
      {"CONTACT", SYNC_AGENT_SRC_URI_CONTACT},
      {"EVENT", SYNC_AGENT_SRC_URI_CALENDAR}}}};

PlatformResult DataSyncManager::StrToPlatformEnum(const std::string& field,
                                                  const std::string& value,
                                                  int* platform_enum) {
  assert(platform_enum);

  if (platform_enum_map_.find(field) == platform_enum_map_.end()) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          std::string("Undefined platform enum type ") + field);
  }

  auto def = platform_enum_map_.at(field);
  auto def_iter = def.find(value);
  if (def_iter != def.end()) {
    *platform_enum = def_iter->second;
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  // default value - if any
  def_iter = def.find("_DEFAULT");
  if (def_iter != def.end()) {
    *platform_enum = def_iter->second;
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  std::string message =
      "Platform enum value " + value + " not found for " + field;
  return PlatformResult(ErrorCode::INVALID_VALUES_ERR, message);
}

PlatformResult DataSyncManager::PlatformEnumToStr(const std::string& field,
                                                  const int value,
                                                  std::string* platform_str) {
  if (platform_enum_map_.find(field) == platform_enum_map_.end()) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          std::string("Undefined platform enum type ") + field);
  }

  auto def = platform_enum_map_.at(field);

  for (auto& item : def) {
    if (item.second == value) {
      *platform_str = item.first;
      return PlatformResult(ErrorCode::NO_ERROR);
    }
  }

  std::string message = "Platform enum value " + std::to_string(value) +
                        " not found for " + field;
  return PlatformResult(ErrorCode::INVALID_VALUES_ERR, message);
}

bool DataSyncManager::sync_agent_initialized_ = false;

DataSyncManager::DataSyncManager() {
  // initialize sync agent once per process
  sync_agent_ds_error_e ds_err = SYNC_AGENT_DS_SUCCESS;
  if (!sync_agent_initialized_) {
    LoggerI("Initialize the datasync manager");
    ds_err = sync_agent_ds_init();
    if (SYNC_AGENT_DS_SUCCESS != ds_err) {
      LoggerE("Failed to init oma ds.");
      return;
    }
  }
  // API was initialized and requires deinitialization
  sync_agent_initialized_ = true;
}

DataSyncManager::~DataSyncManager() {
  // TODO(t.iwanek): sync-agent crashes internally..
  // sync-agent should fix it's deinitialization
}

PlatformResult DataSyncManager::Item(ds_profile_h* profile_h,
                                     const picojson::object& args) {
  sync_agent_ds_error_e ret;

  std::string profile_name = FromJson<std::string>(args, "profileName").c_str();
  ret = sync_agent_ds_set_profile_name(profile_h,
                                       const_cast<char*>(profile_name.c_str()));
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while settting a profile name");
  }

  const picojson::object& sync_info =
      FromJson<picojson::object>(args, "syncInfo");
  std::string url = FromJson<std::string>(sync_info, "url").c_str();
  std::string id = FromJson<std::string>(sync_info, "id").c_str();
  std::string password = FromJson<std::string>(sync_info, "password").c_str();
  ret = sync_agent_ds_set_server_info(profile_h, const_cast<char*>(url.c_str()),
                                      const_cast<char*>(id.c_str()),
                                      const_cast<char*>(password.c_str()));
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while settting a server info");
  }

  int sync_mode;
  PlatformResult status = StrToPlatformEnum(
      kSyncMode, FromJson<std::string>(sync_info, "mode"), &sync_mode);
  if (status.IsError()) return status;

  int sync_type;
  status = StrToPlatformEnum(
      kSyncType, FromJson<std::string>(sync_info, "type"), &sync_type);
  if (status.IsError()) return status;

  int sync_interval;
  status = StrToPlatformEnum(kSyncInterval,
                             FromJson<std::string>(sync_info, "interval"),
                             &sync_interval);
  if (status.IsError()) return status;

  LoggerD("syncMode: %d, syncType: %d, syncInterval: %d", sync_mode, sync_type,
          sync_interval);

  ret = sync_agent_ds_set_sync_info(
      profile_h, static_cast<sync_agent_ds_sync_mode_e>(sync_mode),
      static_cast<sync_agent_ds_sync_type_e>(sync_type),
      static_cast<sync_agent_ds_sync_interval_e>(sync_interval));
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while settting a sync info");
  }

  // Set the sync categories.
  if (IsNull(args, "serviceInfo")) return PlatformResult(ErrorCode::NO_ERROR);

  const picojson::array& service_info =
      FromJson<picojson::array>(args, "serviceInfo");

  for (auto& item : service_info) {
    const picojson::object& obj = JsonCast<picojson::object>(item);

    bool enable = FromJson<bool>(obj, "enable");

    int service_type;
    status = StrToPlatformEnum(kSyncServiceType,
                               FromJson<std::string>(obj, "serviceType"),
                               &service_type);
    if (status.IsError()) return status;

    std::string tgt_uri = FromJson<std::string>(obj, "serverDatabaseUri");

    int src_uri;
    status = StrToPlatformEnum(
        kSyncServiceType, FromJson<std::string>(obj, "serviceType"), &src_uri);
    if (status.IsError()) return status;

    std::string id = "";
    if (!IsNull(obj, "id")) id = FromJson<std::string>(obj, "id");

    std::string password = "";
    if (!IsNull(obj, "password"))
      password = FromJson<std::string>(obj, "password");

    LoggerI("serviceType: %d, tgtURI: %s, enable: %d", service_type,
            tgt_uri.c_str(), enable);

    ret = sync_agent_ds_set_sync_service_info(
        profile_h, static_cast<sync_agent_ds_service_type_e>(service_type),
        enable, static_cast<sync_agent_ds_src_uri_e>(src_uri),
        const_cast<char*>(tgt_uri.c_str()),
        0 == id.size() ? nullptr : const_cast<char*>(id.c_str()),
        0 == password.size() ? nullptr : const_cast<char*>(password.c_str()));
    if (SYNC_AGENT_DS_SUCCESS != ret) {
      return PlatformResult(
          ErrorCode::UNKNOWN_ERR,
          "Platform error while settting a sync service info");
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult DataSyncManager::Item(const std::string& id,
                                     ds_profile_h* profile_h,
                                     picojson::object& out) {
  out["profileId"] = picojson::value(id);

  char* profile_name = nullptr;
  sync_agent_ds_error_e ret =
      sync_agent_ds_get_profile_name(profile_h, &profile_name);
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while gettting a profile name");
  }
  out["profileName"] = picojson::value(profile_name);

  sync_agent_ds_server_info server_info = {nullptr};
  ret = sync_agent_ds_get_server_info(profile_h, &server_info);
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while gettting a server info");
  }
  picojson::value sync_info_val = picojson::value(picojson::object());
  picojson::object& sync_info_obj = sync_info_val.get<picojson::object>();
  sync_info_obj["url"] = picojson::value(server_info.addr);
  sync_info_obj["id"] = picojson::value(server_info.id);
  sync_info_obj["password"] = picojson::value(server_info.password);

  sync_agent_ds_sync_info sync_info;
  ret = sync_agent_ds_get_sync_info(profile_h, &sync_info);
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while gettting a sync info");
  }

  std::string mode;
  PlatformResult status =
      PlatformEnumToStr(kSyncMode, sync_info.sync_mode, &mode);
  if (status.IsError()) return status;
  sync_info_obj["mode"] = picojson::value(mode);

  std::string type;
  status = PlatformEnumToStr(kSyncType, sync_info.sync_type, &type);
  if (status.IsError()) return status;
  sync_info_obj["type"] = picojson::value(type);

  std::string interval;
  status = PlatformEnumToStr(kSyncInterval, sync_info.interval, &interval);
  if (status.IsError()) return status;
  sync_info_obj["interval"] = picojson::value(interval);

  out["syncInfo"] = sync_info_val;

  LoggerD("Sync mode: %d, type: %d, interval: %d", sync_info.sync_mode,
          sync_info.sync_type, sync_info.interval);

  GList* category_list = nullptr;
  ret = sync_agent_ds_get_sync_service_info(profile_h, &category_list);
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while gettting sync categories");
  }
  int category_count = g_list_length(category_list);
  LoggerD("category_count: %d", category_count);

  if (!category_count) return PlatformResult(ErrorCode::NO_ERROR);

  picojson::value service_info_val = picojson::value(picojson::object());
  picojson::array& service_info_array = service_info_val.get<picojson::array>();

  sync_agent_ds_service_info* category_info = nullptr;
  while (category_count--) {
    category_info = static_cast<sync_agent_ds_service_info*>(
        g_list_nth_data(category_list, category_count));
    if (SYNC_AGENT_CALENDAR < category_info->service_type) {
      LoggerD("Skip unsupported sync service type: %d",
              category_info->service_type);
      continue;
    }

    picojson::value item = picojson::value(picojson::object());
    picojson::object& item_obj = item.get<picojson::object>();

    item_obj["enable"] =
        picojson::value(static_cast<bool>(category_info->enabled));
    if (category_info->id) {
      item_obj["id"] = picojson::value(category_info->id);
    }
    if (category_info->password) {
      item_obj["password"] = picojson::value(category_info->password);
    }

    std::string service_type;
    status = PlatformEnumToStr(kSyncServiceType, category_info->service_type,
                               &service_type);
    if (status.IsError()) return status;
    sync_info_obj["serviceType"] = picojson::value(service_type);

    if (category_info->tgt_uri) {
      item_obj["serverDatabaseUri"] = picojson::value(category_info->tgt_uri);
    }

    service_info_array.push_back(item);

    LoggerD("Service type: %d", category_info->service_type);
  }

  out["serviceInfo"] = service_info_val;

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult DataSyncManager::Add(const picojson::object& args,
                                    int* profile_id) {
  assert(profile_id);
  ds_profile_h profile_h = nullptr;

  int num_profiles;
  PlatformResult status = GetProfilesNum(&num_profiles);
  if (status.IsError()) return status;
  LoggerD("numProfiles: %d", num_profiles);

  if (MAX_PROFILES_NUM == num_profiles) {
    return PlatformResult(ErrorCode::QUOTA_EXCEEDED_ERR,
                          "There are already maximum number of profiles!");
  }

  sync_agent_ds_error_e ret = sync_agent_ds_create_profile_info(&profile_h);
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while creating a profile");
  }

  status = Item(&profile_h, args);
  if (status.IsError()) return status;

  ret = sync_agent_ds_add_profile(profile_h, profile_id);
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while adding a profile");
  }

  LoggerD("profileId from platform: %d", *profile_id);

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult DataSyncManager::Update(const picojson::object& args) {
  ds_profile_h profile_h = nullptr;

  int profile_id = std::stoi(FromJson<std::string>(args, "profileId"));
  LoggerD("profileId: %d", profile_id);

  sync_agent_ds_error_e ret = sync_agent_ds_get_profile(profile_id, &profile_h);
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    return PlatformResult(ErrorCode::NOT_FOUND_ERR,
                          "Platform error while getting a profile");
  }

  PlatformResult status = Item(&profile_h, args);
  if (status.IsError()) return status;

  ret = sync_agent_ds_update_profile(profile_h);
  if (SYNC_AGENT_DS_SUCCESS != ret && SYNC_AGENT_DS_SYNCHRONISING != ret) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while updating a profile");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult DataSyncManager::Remove(const std::string& id) {
  ds_profile_h profile_h = nullptr;

  int profile_id = std::stoi(id);
  LoggerD("profileId: %d", profile_id);

  sync_agent_ds_error_e ret = sync_agent_ds_get_profile(profile_id, &profile_h);
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while getting a profile");
  }

  ret = sync_agent_ds_delete_profile(profile_h);
  if (SYNC_AGENT_DS_SUCCESS != ret && SYNC_AGENT_DS_SYNCHRONISING != ret) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while deleting a profile");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

int DataSyncManager::GetMaxProfilesNum() const { return MAX_PROFILES_NUM; }

PlatformResult DataSyncManager::GetProfilesNum(int* profiles_num) {
  sync_agent_ds_error_e error_checker =
      sync_agent_ds_get_profile_count(profiles_num);

  if (SYNC_AGENT_DS_SUCCESS != error_checker) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Error while getting number of profiles.");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult DataSyncManager::Get(const std::string& id,
                                    picojson::object& out) {
  ds_profile_h profile_h = nullptr;

  int profile_id = std::stoi(id);
  LoggerD("profileId: %d", profile_id);

  sync_agent_ds_error_e ret = sync_agent_ds_get_profile(profile_id, &profile_h);
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while getting a profile");
  }

  Item(id, &profile_h, out);

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult DataSyncManager::GetAll(picojson::array& out) {
  GList* profile_list = nullptr;

  sync_agent_ds_error_e ret = sync_agent_ds_get_all_profile(&profile_list);
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while getting all profiles");
  }

  ds_profile_h profile_h = nullptr;
  GList* iter = nullptr;
  int profile_id;
  LoggerD("Number of profiles: %d", g_list_length(profile_list));

  for (iter = profile_list; iter != nullptr; iter = g_list_next(iter)) {
    profile_h = iter->data;

    ret = sync_agent_ds_get_profile_id(profile_h, &profile_id);
    if (SYNC_AGENT_DS_SUCCESS != ret) {
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Platform error while gettting a profile id");
    }

    picojson::value profile = picojson::value(picojson::object());
    picojson::object& profile_obj = profile.get<picojson::object>();

    Item(std::to_string(profile_id), &profile_h, profile_obj);

    LoggerD("Adding a profile to the list.");
    out.push_back(profile);
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult DataSyncManager::StartSync(const picojson::object& args) {
  ds_profile_h profile_h = nullptr;

  const std::string& id = FromJson<std::string>(args, "profileId");
  const std::string& listener_id = FromJson<std::string>(args, "listenerId");

  int profile_id = std::stoi(id);
  LoggerD("profileId: %d", profile_id);

  sync_agent_event_error_e err = sync_agent_set_noti_callback(
      SYNC_AGENT_DS_SYNC_SESSION,
      [](sync_agent_event_data_s* d, void* ud) {
        return static_cast<DataSyncManager*>(ud)->StateChangedCallback(d);
      },
      static_cast<void*>(this));
  if (err != SYNC_AGENT_EVENT_SUCCESS) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while setting state changed cb");
  }

  err = sync_agent_set_noti_callback(SYNC_AGENT_DS_SYNC_PROCESS,
                                     [](sync_agent_event_data_s* d, void* ud) {
                                       return static_cast<DataSyncManager*>(ud)
                                           ->ProgressCallback(d);
                                     },
                                     static_cast<void*>(this));
  if (err != SYNC_AGENT_EVENT_SUCCESS) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while setting progress cb");
  }

  sync_agent_ds_error_e ret = sync_agent_ds_start_sync(profile_h);
  if (SYNC_AGENT_DS_SUCCESS != ret && SYNC_AGENT_DS_SYNCHRONISING != ret) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while starting a profile");
  }

  callbacks_[id] = listener_id;

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult DataSyncManager::StopSync(const std::string& id) {
  ds_profile_h profile_h = nullptr;

  int profile_id = std::stoi(id.c_str());
  LoggerD("profileId: %d", profile_id);

  sync_agent_ds_error_e ret = sync_agent_ds_get_profile(profile_id, &profile_h);
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while getting a profile");
  }

  ret = sync_agent_ds_stop_sync(profile_h);
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while stopping a profile");
  }

  callbacks_.erase(id);

  return PlatformResult(ErrorCode::NO_ERROR);
}

DataSyncManager& DataSyncManager::Instance() {
  static DataSyncManager manager;
  return manager;
}

PlatformResult DataSyncManager::GetLastSyncStatistics(const std::string& id,
                                                      picojson::array& out) {
  ds_profile_h profile_h = nullptr;
  GList* statistics_list = nullptr;

  int profile_id = std::stoi(id);
  LoggerD("profileId: %d", profile_id);

  sync_agent_ds_error_e ret = sync_agent_ds_get_profile(profile_id, &profile_h);
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while getting a profile");
  }

  ret = sync_agent_ds_get_sync_statistics(profile_h, &statistics_list);
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while gettting sync statistics");
  }

  int statistics_count = g_list_length(statistics_list);
  LoggerD("statistics_count: %d", statistics_count);
  sync_agent_ds_statistics_info* statistics = nullptr;

  for (int i = 0; i < statistics_count; i++) {
    statistics = static_cast<sync_agent_ds_statistics_info*>(
        g_list_nth_data(statistics_list, i));

    picojson::value item = picojson::value(picojson::object());
    picojson::object& item_obj = item.get<picojson::object>();

    std::string service_type;
    if (0 == i) {
      LoggerD("Statistics for contact.");

      PlatformResult status = PlatformEnumToStr(
          kSyncServiceType, SYNC_AGENT_CONTACT, &service_type);
      if (status.IsError()) return status;
      item_obj["serviceType"] = picojson::value(service_type);
    } else if (1 == i) {
      LoggerD("Statistics for event.");

      PlatformResult status = PlatformEnumToStr(
          kSyncServiceType, SYNC_AGENT_CALENDAR, &service_type);
      if (status.IsError()) return status;
      item_obj["serviceType"] = picojson::value(service_type);
    } else {
      LoggerW("Unsupported category for statistics: %d", i);
      continue;
    }

    out.push_back(item);

    LoggerD("dbsynced: %d", statistics->dbsynced);
    if (!statistics->dbsynced) continue;

    std::string db_synced = statistics->dbsynced;
    std::transform(db_synced.begin(), db_synced.end(), db_synced.begin(),
                   ::toupper);

    item_obj["syncStatus"] = picojson::value(db_synced);
    item_obj["clientToServerTotal"] =
        picojson::value(static_cast<double>(statistics->client2server_total));
    item_obj["clientToServerAdded"] =
        picojson::value(static_cast<double>(statistics->client2server_nrofadd));
    item_obj["clientToServerUpdated"] = picojson::value(
        static_cast<double>(statistics->client2server_nrofreplace));
    item_obj["clientToServerRemoved"] = picojson::value(
        static_cast<double>(statistics->client2server_nrofdelete));
    item_obj["serverToClientTotal"] =
        picojson::value(static_cast<double>(statistics->server2client_total));
    item_obj["serverToClientAdded"] =
        picojson::value(static_cast<double>(statistics->server2client_nrofadd));
    item_obj["serverToClientUpdated"] = picojson::value(
        static_cast<double>(statistics->server2client_nrofreplace));
    item_obj["serverToClientRemoved"] = picojson::value(
        static_cast<double>(statistics->server2client_nrofdelete));
    item_obj["lastSyncTime"] =
        picojson::value(static_cast<double>(statistics->last_session_time));

    LoggerD("ClientToServerTotal: %d, ServerToClientTotal: %d",
            statistics->client2server_total, statistics->server2client_total);
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult DataSyncManager::GetProfileId(sync_agent_event_data_s* request,
                                             std::string& profile_id) {
  char* profile_dir_name = nullptr;

  sync_agent_get_event_data_param(request, &profile_dir_name);

  // profile_dir_name is SyncXXX , XXX is profile id
  if (strncmp(profile_dir_name, "Sync", 4) == 0) {
    profile_id = profile_dir_name + 4;
  } else {
    g_free(profile_dir_name);
    LoggerE("Invalid Sync dir name");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Invalid Sync dir name");
  }

  g_free(profile_dir_name);

  return PlatformResult(ErrorCode::NO_ERROR);
}

void DataSyncManager::Failed(picojson::object& response_obj,
                             picojson::object& answer_obj,
                             const common::ErrorCode& code,
                             const std::string& name,
                             const std::string& message) {
  LoggerE("%s", message.c_str());
  response_obj["callback_name"] = picojson::value("onfailed");

  picojson::value error = picojson::value(picojson::object());
  picojson::object& error_obj = error.get<picojson::object>();

  error_obj["code"] = picojson::value(static_cast<double>(code));
  error_obj["name"] = picojson::value(name);
  error_obj["message"] = picojson::value(message);

  answer_obj["error"] = error;
}

void DataSyncManager::PrepareResponseObj(const std::string& profile_id,
                                         picojson::value& response,
                                         picojson::object& response_obj,
                                         picojson::value& answer,
                                         picojson::object& answer_obj) {
  response = picojson::value(picojson::object());
  response_obj = response.get<picojson::object>();

  response_obj["callback_key"] = picojson::value(callbacks_[profile_id]);

  answer = picojson::value(picojson::object());
  answer_obj = answer.get<picojson::object>();

  answer_obj["profileId"] = picojson::value(profile_id);
}

int DataSyncManager::StateChangedCallback(sync_agent_event_data_s* request) {
  LoggerD("DataSync session state changed.");

  char* progress = nullptr;
  std::string profile_id;

  LoggerD("Get state info.");
  sync_agent_get_event_data_param(request, &progress);

  PlatformResult status = GetProfileId(request, profile_id);
  if (status.IsError()) return static_cast<int>(status.error_code());

  LoggerI("profileDirName: %s, progress: %s", profile_id.c_str(), progress);

  // prepare response object
  picojson::value response;
  picojson::object response_obj;
  picojson::value answer;
  picojson::object answer_obj;
  PrepareResponseObj(profile_id, response, response_obj, answer, answer_obj);

  auto it = callbacks_.find(profile_id);
  if (it != callbacks_.end()) {
    callbacks_.erase(it);

    if (!progress) {
      Failed(response_obj, answer_obj, ErrorCode::UNKNOWN_ERR, "Exception",
             "nullptr status");
    } else if (0 == strncmp(progress, "DONE", 4)) {
      response_obj["callback_name"] = picojson::value("oncompleted");
    } else if (0 == strncmp(progress, "CANCEL", 6)) {
      response_obj["callback_name"] = picojson::value("onstopped");
    } else if (0 == strncmp(progress, "ERROR", 5)) {
      Failed(response_obj, answer_obj, ErrorCode::UNKNOWN_ERR, "Exception",
             "Datasync failed");
    } else {
      Failed(response_obj, answer_obj, ErrorCode::UNKNOWN_ERR, "Exception",
             "Undefined status");
    }
  }

  response_obj["answer"] = answer;

  g_free(progress);

  DatasyncInstance::GetInstance().PostMessage(response.serialize().c_str());

  return 0;
}

int DataSyncManager::ProgressCallback(sync_agent_event_data_s* request) {
  LoggerD("DataSync progress called.");

  int uri;
  int is_from_server, total_per_operation, synced_per_operation, total_per_db,
      synced_per_db;
  std::string profile_id;

  LoggerD("Get progress info.");
  sync_agent_get_event_data_param(request, &uri);

  PlatformResult status = GetProfileId(request, profile_id);
  if (status.IsError()) return static_cast<int>(status.error_code());

  LoggerI("profileDirName: %s, uri: %d", profile_id.c_str(), uri);

  sync_agent_get_event_data_param(request, &is_from_server);
  sync_agent_get_event_data_param(request, &total_per_operation);
  sync_agent_get_event_data_param(request, &synced_per_operation);
  sync_agent_get_event_data_param(request, &total_per_db);
  sync_agent_get_event_data_param(request, &synced_per_db);

  LoggerI(
      "isFromServer: %d, totalPerOperation: %d, syncedPerOperation: %d, "
      "totalPerDb: %d, "
      "syncedPerDb %d",
      is_from_server, total_per_operation, synced_per_operation, total_per_db,
      synced_per_db);

  // prepare response object
  picojson::value response;
  picojson::object response_obj;
  picojson::value answer;
  picojson::object answer_obj;
  PrepareResponseObj(profile_id, response, response_obj, answer, answer_obj);

  auto it = callbacks_.find(profile_id);
  if (it != callbacks_.end()) {
    if (SYNC_AGENT_SRC_URI_CONTACT == uri ||
        SYNC_AGENT_SRC_URI_CALENDAR == uri) {
      response_obj["callback_name"] = picojson::value("onprogress");

      std::string service_type;
      PlatformResult status =
          PlatformEnumToStr(kSyncServiceTypeToSrcUri, uri, &service_type);
      if (status.IsError()) return static_cast<int>(status.error_code());
      answer_obj["serviceType"] = picojson::value(service_type);

      answer_obj["isFromServer"] =
          picojson::value(static_cast<double>(is_from_server));
      answer_obj["totalPerService"] =
          picojson::value(static_cast<double>(total_per_db));
      answer_obj["syncedPerService"] =
          picojson::value(static_cast<double>(synced_per_db));
    } else {
      Failed(response_obj, answer_obj, ErrorCode::UNKNOWN_ERR, "Exception",
             "Wrong service type");
    }
  }

  response_obj["answer"] = answer;

  DatasyncInstance::GetInstance().PostMessage(response.serialize().c_str());

  return 0;
}

}  // namespace datasync
}  // namespace extension
