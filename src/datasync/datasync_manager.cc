// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


// File copied from Crosswalk

#include "datasync/datasync_manager.h"

#include "common/scope_exit.h"
#include "datasync/datasync_instance.h"
#include "common/logger.h"
#include "datasync/sync_info.h"
#include "datasync/sync_service_info.h"
#include "datasync/sync_profile_info.h"
#include "common/converter.h"

#include "tizen/tizen.h"

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

int DataSyncManager::StrToPlatformEnum(const std::string& field, const std::string& value) {
  if (platform_enum_map_.find(field) == platform_enum_map_.end()) {
      throw UnknownException(std::string("Undefined platform enum type ") + field);
  }

  auto def = platform_enum_map_.at(field);
  auto def_iter = def.find(value);
  if (def_iter != def.end()) {
    return def_iter->second;
  }

  // default value - if any
  def_iter = def.find("_DEFAULT");
  if (def_iter != def.end()) {
    return def_iter->second;
  }

  std::string message = "Platform enum value " + value + " not found for " + field;
  throw InvalidValuesException(message);
}

std::string DataSyncManager::PlatformEnumToStr(const std::string& field, const int value) {
  if (platform_enum_map_.find(field) == platform_enum_map_.end()) {
    throw UnknownException(std::string("Undefined platform enum type ") + field);
  }

  auto def = platform_enum_map_.at(field);

  for (auto& item : def) {
    if (item.second == value) return item.first;
  }

  std::string message = "Platform enum value " + std::to_string(value) + " not found for " + field;
  throw InvalidValuesException(message);
}

datasync::SyncInfo::SyncMode ConvertToSyncMode(
    sync_agent_ds_sync_mode_e sync_mode) {
  switch (sync_mode) {
    case SYNC_AGENT_SYNC_MODE_MANUAL: {
      return datasync::SyncInfo::MANUAL_MODE;
    }
    case SYNC_AGENT_SYNC_MODE_PERIODIC: {
      return datasync::SyncInfo::PERIODIC_MODE;
    }
    case SYNC_AGENT_SYNC_MODE_PUSH: {
      return datasync::SyncInfo::PUSH_MODE;
    }
    default: {
      LoggerW("Error while converting a sync mode.");
      return datasync::SyncInfo::UNDEFINED_MODE;
    }
  }
}

datasync::SyncStatistics::SyncStatus ConvertToSyncStatus(char* status) {
  if (0 == strncmp(status, "success", 7)) {
    return datasync::SyncStatistics::SUCCESS_STATUS;
  } else if (0 == strncmp(status, "stop", 4)) {
    return datasync::SyncStatistics::STOP_STATUS;
  } else if (0 == strncmp(status, "fail", 4)) {
    return datasync::SyncStatistics::FAIL_STATUS;
  } else if (0 == strncmp(status, "No", 2)) {
    return datasync::SyncStatistics::NONE_STATUS;
  } else {
    LoggerW("Error while converting a sync status.");
  }

  return datasync::SyncStatistics::NONE_STATUS;
}

datasync::SyncInfo::SyncType ConvertToSyncType(
    sync_agent_ds_sync_type_e sync_type) {
  switch (sync_type) {
    case SYNC_AGENT_SYNC_TYPE_UPDATE_BOTH: {
      return datasync::SyncInfo::TWO_WAY_TYPE;
    }
    case SYNC_AGENT_SYNC_TYPE_FULL_SYNC: {
      return datasync::SyncInfo::SLOW_TYPE;
    }
    case SYNC_AGENT_SYNC_TYPE_UPDATE_TO_SERVER: {
      return datasync::SyncInfo::ONE_WAY_FROM_CLIENT_TYPE;
    }
    case SYNC_AGENT_SYNC_TYPE_REFRESH_FROM_PHONE: {
      return datasync::SyncInfo::REFRESH_FROM_CLIENT_TYPE;
    }
    case  SYNC_AGENT_SYNC_TYPE_UPDATE_TO_PHONE: {
      return datasync::SyncInfo::ONE_WAY_FROM_SERVER_TYPE;
    }
    case SYNC_AGENT_SYNC_TYPE_REFRESH_FROM_SERVER: {
      return datasync::SyncInfo::REFRESH_FROM_SERVER_TYPE;
    }
    default: {
      LoggerW("Error while converting a sync type.");
      return datasync::SyncInfo::UNDEFINED_TYPE;
    }
  }
}

datasync::SyncInfo::SyncInterval ConvertToSyncInterval(
    sync_agent_ds_sync_interval_e sync_interval) {
  switch (sync_interval) {
    case SYNC_AGENT_SYNC_INTERVAL_5_MINUTES: {
      return datasync::SyncInfo::INTERVAL_5_MINUTES;
    }
    case SYNC_AGENT_SYNC_INTERVAL_15_MINUTES: {
      return datasync::SyncInfo::INTERVAL_15_MINUTES;
    }
    case SYNC_AGENT_SYNC_INTERVAL_1_HOUR: {
      return datasync::SyncInfo::INTERVAL_1_HOUR;
    }
    case SYNC_AGENT_SYNC_INTERVAL_4_HOURS: {
      return datasync::SyncInfo::INTERVAL_4_HOURS;
    }
    case SYNC_AGENT_SYNC_INTERVAL_12_HOURS: {
      return datasync::SyncInfo::INTERVAL_12_HOURS;
    }
    case SYNC_AGENT_SYNC_INTERVAL_1_DAY: {
      return datasync::SyncInfo::INTERVAL_1_DAY;
    }
    case SYNC_AGENT_SYNC_INTERVAL_1_WEEK: {
      return datasync::SyncInfo::INTERVAL_1_WEEK;
    }
    case SYNC_AGENT_SYNC_INTERVAL_1_MONTH: {
      return datasync::SyncInfo::INTERVAL_1_MONTH;
    }
    case SYNC_AGENT_SYNC_INTERVAL_NONE: {
      return datasync::SyncInfo::INTERVAL_UNDEFINED;
    }
    default: {
      LoggerW("Error while converting a platform sync interval.");
      return datasync::SyncInfo::INTERVAL_UNDEFINED;
    }
  }
}

datasync::SyncServiceInfo::SyncServiceType ConvertToSyncServiceType(
    sync_agent_ds_service_type_e service_type) {
  switch (service_type) {
    case SYNC_AGENT_CONTACT: {
      return datasync::SyncServiceInfo::CONTACT_SERVICE_TYPE;
    }
    case SYNC_AGENT_CALENDAR: {
      return datasync::SyncServiceInfo::EVENT_SERVICE_TYPE;
    }
    default: {
      LoggerW("Error while converting a sync service type.");
      return datasync::SyncServiceInfo::UNDEFINED_SERVICE_TYPE;
    }
  }
}

}  // namespace

namespace datasync {

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

  sync_agent_event_error_e err = sync_agent_set_noti_callback(
      1, [](sync_agent_event_data_s* d, void* ud) {
           return static_cast<DataSyncManager*>(ud)->StateChangedCallback(d);
         },
      static_cast<void*>(this));
  if (err != SYNC_AGENT_EVENT_SUCCESS) {
    LoggerE("Platform error while setting state changed cb");
    return;
  }

  err = sync_agent_set_noti_callback(
      2, [](sync_agent_event_data_s* d, void* ud) {
          return static_cast<DataSyncManager*>(ud)->ProgressCallback(d);
        },
      static_cast<void*>(this));
  if (err != SYNC_AGENT_EVENT_SUCCESS) {
    LoggerE("Platform error while setting progress cb");
  }
}

DataSyncManager::~DataSyncManager() {
  // TODO(t.iwanek): sync-agent crashes internally..
  // sync-agent should fix it's deinitialization
}

void DataSyncManager::Item(ds_profile_h* profile_h, const picojson::object& args) {
  sync_agent_ds_error_e ret;

  std::string profile_name = FromJson<std::string>(args, "profileName").c_str();
  ret = sync_agent_ds_set_profile_name(profile_h, const_cast<char*>(profile_name.c_str()));
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    throw UnknownException("Platform error while settting a profile name");
  }

  const picojson::object& sync_info = FromJson<picojson::object>(args, "syncInfo");
  std::string url = FromJson<std::string>(sync_info, "url").c_str();
  std::string id = FromJson<std::string>(sync_info, "id").c_str();
  std::string password = FromJson<std::string>(sync_info, "password").c_str();
  ret = sync_agent_ds_set_server_info(profile_h, const_cast<char*>(url.c_str()),
                                      const_cast<char*>(id.c_str()),
                                      const_cast<char*>(password.c_str()));
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    throw UnknownException("Platform error while settting a server info");
  }

  int sync_mode = StrToPlatformEnum(kSyncMode, FromJson<std::string>(sync_info, "mode"));
  int sync_type = StrToPlatformEnum(kSyncType, FromJson<std::string>(sync_info, "type"));
  int sync_interval = StrToPlatformEnum(kSyncInterval,
                                        FromJson<std::string>(sync_info, "interval"));
  LoggerD("syncMode: %d, syncType: %d, syncInterval: %d", sync_mode, sync_type, sync_interval);
  ret = sync_agent_ds_set_sync_info(profile_h, static_cast<sync_agent_ds_sync_mode_e>(sync_mode),
                                    static_cast<sync_agent_ds_sync_type_e>(sync_type),
                                    static_cast<sync_agent_ds_sync_interval_e>(sync_interval));
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    throw UnknownException("Platform error while settting a sync info");
  }

  // Set the sync categories.
  if (IsNull(args, "serviceInfo")) return;

  const picojson::array& service_info = FromJson<picojson::array>(args, "serviceInfo");

  for (auto& item : service_info) {
    const picojson::object& obj = JsonCast<picojson::object>(item);

    bool enable = FromJson<bool>(obj, "enable");
    int service_type =
        StrToPlatformEnum(kSyncServiceType, FromJson<std::string>(obj, "serviceType"));
    std::string tgt_uri = FromJson<std::string>(obj, "serverDatabaseUri");
    int src_uri = StrToPlatformEnum(kSyncServiceType, FromJson<std::string>(obj, "serviceType"));

    std::string id = "";
    if (!IsNull(obj, "id")) id = FromJson<std::string>(obj, "id");

    std::string password = "";
    if (!IsNull(obj, "password")) password = FromJson<std::string>(obj, "password");

    LoggerI("serviceType: %d, tgtURI: %s, enable: %d", service_type, tgt_uri.c_str(), enable);

    ret = sync_agent_ds_set_sync_service_info(
        profile_h, static_cast<sync_agent_ds_service_type_e>(service_type), enable,
        static_cast<sync_agent_ds_src_uri_e>(src_uri), const_cast<char*>(tgt_uri.c_str()),
        0 == id.size() ? nullptr : const_cast<char*>(id.c_str()),
        0 == password.size() ? nullptr : const_cast<char*>(password.c_str()));
    if (SYNC_AGENT_DS_SUCCESS != ret) {
      throw UnknownException("Platform error while settting a sync service info");
    }
  }
}

int DataSyncManager::Add(const picojson::object& args) {
  ds_profile_h profile_h = nullptr;

  int num_profiles = GetProfilesNum();
  LoggerD("numProfiles: %d", num_profiles);

  if (MAX_PROFILES_NUM == num_profiles) {
    throw QuotaExceededException("There are already maximum number of profiles!");
  }

  sync_agent_ds_error_e ret = sync_agent_ds_create_profile_info(&profile_h);
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    throw UnknownException("Platform error while creating a profile");
  }

  Item(&profile_h, args);

  int profile_id;
  ret = sync_agent_ds_add_profile(profile_h, &profile_id);
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    throw UnknownException("Platform error while adding a profile");
  }

  LoggerD("profileId from platform: %d", profile_id);

  return profile_id;
}

void DataSyncManager::Update(const picojson::object& args) {
  ds_profile_h profile_h = nullptr;

  int profile_id = std::stoi(FromJson<std::string>(args, "profileId"));
  LoggerD("profileId: %d", profile_id);

  sync_agent_ds_error_e ret = sync_agent_ds_get_profile(profile_id, &profile_h);
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    throw NotFoundException("Platform error while getting a profile");
  }

  Item(&profile_h, args);

  ret = sync_agent_ds_update_profile(profile_h);
  if (SYNC_AGENT_DS_SUCCESS != ret && SYNC_AGENT_DS_SYNCHRONISING != ret) {
    throw UnknownException("Platform error while updating a profile");
  }
}

void DataSyncManager::Remove(const std::string& id) {
  ds_profile_h profile_h = nullptr;

  int profile_id = std::stoi(id);
  LoggerD("profileId: %d", profile_id);

  sync_agent_ds_error_e ret = sync_agent_ds_get_profile(profile_id, &profile_h);
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    throw UnknownException("Platform error while getting a profile");
  }

  ret = sync_agent_ds_delete_profile(profile_h);
  if (SYNC_AGENT_DS_SUCCESS != ret && SYNC_AGENT_DS_SYNCHRONISING != ret) {
    throw UnknownException("Platform error while deleting a profile");
  }
}

int DataSyncManager::GetMaxProfilesNum() const {
  return MAX_PROFILES_NUM;
}

int DataSyncManager::GetProfilesNum() const {
  int profile_list_size = 0;

  sync_agent_ds_error_e error_checker = sync_agent_ds_get_profile_count(&profile_list_size);

  if (SYNC_AGENT_DS_SUCCESS  != error_checker) {
    throw common::UnknownException("Error while getting number of profiles.");
  }

  return profile_list_size;
}

ResultOrError<SyncProfileInfoPtr> DataSyncManager::Get(
    const std::string& profile_id) const {
  ds_profile_h profile_h = nullptr;
  GList* category_list = nullptr;

  auto exit = common::MakeScopeExit([&category_list, &profile_h]() {
    if (category_list) {
      g_list_free(category_list);
    }

    if (profile_h) {
      sync_agent_ds_free_profile_info(profile_h);
    }
  });

  SyncProfileInfoPtr profile(new SyncProfileInfo());

  sync_agent_ds_error_e ret = SYNC_AGENT_DS_FAIL;

  int profile_id_str = std::stoi(profile_id);
  LoggerD("profileId: %d", profile_id_str);

  ret = sync_agent_ds_get_profile(profile_id_str, &profile_h);
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    return Error("NotFoundException",
        "Platform error while getting a profile");
  }

  profile->set_profile_id(profile_id);

  char* profile_name = nullptr;
  ret = sync_agent_ds_get_profile_name(profile_h, &profile_name);
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    return Error("Exception",
        "Platform error while gettting a profile name");
  }
  profile->set_profile_name(profile_name);

  sync_agent_ds_server_info server_info = {nullptr};
  ret = sync_agent_ds_get_server_info(profile_h, &server_info);
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    return Error("Exception",
        "Platform error while gettting a server info");
  }
  profile->sync_info()->set_url(server_info.addr);
  profile->sync_info()->set_id(server_info.id);
  profile->sync_info()->set_password(server_info.password);

  sync_agent_ds_sync_info sync_info;
  ret = sync_agent_ds_get_sync_info(profile_h, &sync_info);
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    return Error("Exception",
        "Platform error while gettting a sync info");
  }
  profile->sync_info()->set_sync_mode(ConvertToSyncMode(sync_info.sync_mode));
  profile->sync_info()->set_sync_type(ConvertToSyncType(sync_info.sync_type));
  profile->sync_info()->set_sync_interval(
      ConvertToSyncInterval(sync_info.interval));

  LoggerD("Sync mode: %d, type: %d, interval: %d",
          sync_info.sync_mode, sync_info.sync_type, sync_info.interval);

  sync_agent_ds_service_info* category_info = nullptr;
  ret = sync_agent_ds_get_sync_service_info(profile_h, &category_list);
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    return Error("Exception",
        "Platform error while gettting sync categories");
  }
  int category_count = g_list_length(category_list);
  LoggerD("category_count: %d", category_count);
  while (category_count--) {
    category_info = static_cast<sync_agent_ds_service_info*>(
        g_list_nth_data(category_list, category_count));
    if (SYNC_AGENT_CALENDAR < category_info->service_type) {
      LoggerD("Skip unsupported sync service type: %d", category_info->service_type);
      continue;
    }

    SyncServiceInfoPtr service_info(new SyncServiceInfo());
    service_info->set_enable(category_info->enabled);
    if (category_info->id) {
      service_info->set_id(category_info->id);
    }
    if (category_info->password) {
      service_info->set_password(category_info->password);
    }
    service_info->set_sync_service_type(
        ConvertToSyncServiceType(category_info->service_type));
    if (category_info->tgt_uri) {
      service_info->set_server_database_uri(category_info->tgt_uri);
    }

    LoggerD("Service type: %d", service_info->sync_service_type());
    profile->service_info()->push_back(service_info);
  }

  return profile;
}

ResultOrError<SyncProfileInfoListPtr> DataSyncManager::GetAll() const {
  GList* profile_list = nullptr;
  GList* iter = nullptr;

  ds_profile_h profile_h = nullptr;

  auto exit = common::MakeScopeExit([&profile_list, &profile_h, &iter]() {
    LoggerD("Free profiles list.");
    for (iter = profile_list; iter != nullptr; iter = g_list_next(iter)) {
      sync_agent_ds_free_profile_info((ds_profile_h)iter->data);
    }

    if (profile_list) {
      g_list_free(profile_list);
    }
  });

  SyncProfileInfoListPtr profiles = std::make_shared<SyncProfileInfoList>();

  sync_agent_ds_error_e ret = SYNC_AGENT_DS_FAIL;

  ret = sync_agent_ds_get_all_profile(&profile_list);
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    return Error("Exception",
        "Platform error while getting all profiles");
  }

  int profile_id;
  LoggerD("Number of profiles: %d", g_list_length(profile_list));
  for (iter = profile_list; iter != nullptr; iter = g_list_next(iter)) {
    profile_h = (ds_profile_h)iter->data;
    SyncProfileInfoPtr profile(new SyncProfileInfo());

    ret = sync_agent_ds_get_profile_id(profile_h, &profile_id);
    if (SYNC_AGENT_DS_SUCCESS != ret) {
      return Error("Exception",
          "Platform error while gettting a profile id");
    }

    profile->set_profile_id(std::to_string(profile_id));

    LoggerD("Processing a profile with id: %s", profile->profile_id().c_str());

    char* profile_name = nullptr;
    ret = sync_agent_ds_get_profile_name(profile_h, &profile_name);
    if (SYNC_AGENT_DS_SUCCESS != ret) {
      return Error("Exception",
          "Platform error while gettting a profile name");
    }
    profile->set_profile_name(profile_name);

    sync_agent_ds_server_info server_info = {nullptr};
    ret = sync_agent_ds_get_server_info(profile_h, &server_info);
    if (SYNC_AGENT_DS_SUCCESS != ret) {
      return Error("Exception",
          "Platform error while gettting a server info");
    }
    profile->sync_info()->set_url(server_info.addr);
    profile->sync_info()->set_id(server_info.id);
    profile->sync_info()->set_password(server_info.password);

    sync_agent_ds_sync_info sync_info;
    ret = sync_agent_ds_get_sync_info(profile_h, &sync_info);
    if (SYNC_AGENT_DS_SUCCESS != ret) {
      return Error("Exception",
          "Platform error while gettting a sync info");
    }
    profile->sync_info()->set_sync_mode(
        ConvertToSyncMode(sync_info.sync_mode));
    profile->sync_info()->set_sync_type(
        ConvertToSyncType(sync_info.sync_type));
    profile->sync_info()->set_sync_interval(
        ConvertToSyncInterval(sync_info.interval));

    LoggerD("Sync mode: %d, type: %d, interval: %d",
            sync_info.sync_mode, sync_info.sync_type, sync_info.interval);

    GList* category_list = nullptr;
    sync_agent_ds_service_info* category_info = nullptr;
    ret = sync_agent_ds_get_sync_service_info(profile_h, &category_list);
    if (SYNC_AGENT_DS_SUCCESS != ret) {
      return Error(
          "Exception",
          "Platform error while gettting sync categories");
    }
    int category_count = g_list_length(category_list);
    LoggerD("category_count: %d", category_count);
    while (category_count--) {
      category_info = static_cast<sync_agent_ds_service_info*>(
          g_list_nth_data(category_list, category_count));
      if (SYNC_AGENT_CALENDAR < category_info->service_type) {
        LoggerD("Skip unsupported sync service type: %d", category_info->service_type);
        continue;
      }

      SyncServiceInfoPtr service_info(new SyncServiceInfo());
      service_info->set_enable(category_info->enabled);
      if (category_info->id) {
        service_info->set_id(category_info->id);
      }
      if (category_info->password) {
        service_info->set_password(category_info->password);
      }
      service_info->set_sync_service_type(
          ConvertToSyncServiceType(category_info->service_type));
      if (category_info->tgt_uri) {
        service_info->set_server_database_uri(category_info->tgt_uri);
      }

      LoggerD("Service type: %d", service_info->sync_service_type());
      profile->service_info()->push_back(service_info);
    }
    if (category_list) {
      g_list_free(category_list);
    }

    LoggerD("Adding a profile to the list.");
    profiles->push_back(profile);
  }

  return profiles;
}

ResultOrError<void> DataSyncManager::StartSync(
    const std::string& profile_id_str, int callback_id,
    DatasyncInstance* instance) {
  ds_profile_h profile_h = nullptr;

  auto exit = common::MakeScopeExit([&profile_h]() {
    if (profile_h) {
      sync_agent_ds_free_profile_info(profile_h);
    }
  });

  sync_agent_ds_error_e ret = SYNC_AGENT_DS_FAIL;
  sync_agent_event_error_e err = SYNC_AGENT_EVENT_FAIL;

  int profile_id = std::stoi(profile_id_str);
  LoggerD("profileId: %d", profile_id);

  ret = sync_agent_ds_get_profile(profile_id, &profile_h);
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    return Error("Exception",
        "Platform error while getting a profile");
  }

  ret = sync_agent_ds_start_sync(profile_h);
  if (SYNC_AGENT_DS_SUCCESS != ret && SYNC_AGENT_DS_SYNCHRONISING != ret) {
    return Error("Exception",
        "Platform error while starting a profile");
  }

  if (callback_id >= 0) {
    callbacks_.insert({profile_id, std::make_pair(callback_id, instance)});
  }

  return {};
}

ResultOrError<void> DataSyncManager::StopSync(
    const std::string& profile_id_str) {
  ds_profile_h profile_h = nullptr;

  auto exit = common::MakeScopeExit([&profile_h]() {
    if (profile_h) {
      sync_agent_ds_free_profile_info(profile_h);
    }
  });

  sync_agent_ds_error_e ret = SYNC_AGENT_DS_FAIL;

  int profile_id = std::stoi(profile_id_str.c_str());
  LoggerD("profileId: %d", profile_id);

  ret = sync_agent_ds_get_profile(profile_id, &profile_h);
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    return Error("Exception",
        "Platform error while getting a profile");
  }

  ret = sync_agent_ds_stop_sync(profile_h);
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    return Error("Exception",
        "Platform error while stopping a profile");
  }

  return {};
}

void DataSyncManager::UnregisterInstanceCallbacks(DatasyncInstance* instance) {
  for (auto it = callbacks_.begin(); it != callbacks_.end(); ) {
    if (it->second.second == instance)
      it = callbacks_.erase(it);
    else
      ++it;
  }
}

DataSyncManager& DataSyncManager::Instance() {
  static DataSyncManager manager;
  return manager;
}

void DataSyncManager::GetLastSyncStatistics(const std::string& id, picojson::array& out) {
  ds_profile_h profile_h = nullptr;
  GList* statistics_list = nullptr;

  int profile_id = std::stoi(id);
  LoggerD("profileId: %d", profile_id);

  sync_agent_ds_error_e ret = sync_agent_ds_get_profile(profile_id, &profile_h);
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    throw NotFoundException("Platform error while getting a profile");
  }

  ret = sync_agent_ds_get_sync_statistics(profile_h, &statistics_list);
  if (SYNC_AGENT_DS_SUCCESS != ret) {
    throw UnknownException("Platform error while gettting sync statistics");
  }

  int statistics_count = g_list_length(statistics_list);
  LoggerD("statistics_count: %d", statistics_count);
  sync_agent_ds_statistics_info* statistics = nullptr;

  for (int i = 0; i < statistics_count; i++) {
    statistics = static_cast<sync_agent_ds_statistics_info*>(g_list_nth_data(statistics_list, i));

    picojson::value item = picojson::value(picojson::object());
    picojson::object& item_obj = item.get<picojson::object>();

    if (0 == i) {
      LoggerD("Statistics for contact.");
      item_obj["serviceType"] =
          picojson::value(PlatformEnumToStr(kSyncServiceType, SYNC_AGENT_CONTACT));
    } else if (1 == i) {
      LoggerD("Statistics for event.");
      item_obj["serviceType"] =
          picojson::value(PlatformEnumToStr(kSyncServiceType, SYNC_AGENT_CALENDAR));
    } else {
      LoggerW("Unsupported category for statistics: %d", i);
      continue;
    }

    out.push_back(item);

    LoggerD("dbsynced: %d", statistics->dbsynced);
    if (!statistics->dbsynced) continue;

    std::string db_synced = statistics->dbsynced;
    std::transform(db_synced.begin(), db_synced.end(), db_synced.begin(), ::toupper);

    item_obj["syncStatus"] = picojson::value(db_synced);
    item_obj["clientToServerTotal"] =
        picojson::value(static_cast<double>(statistics->client2server_total));
    item_obj["clientToServerAdded"] =
        picojson::value(static_cast<double>(statistics->client2server_nrofadd));
    item_obj["clientToServerUpdated"] =
        picojson::value(static_cast<double>(statistics->client2server_nrofreplace));
    item_obj["clientToServerRemoved"] =
        picojson::value(static_cast<double>(statistics->client2server_nrofdelete));
    item_obj["serverToClientTotal"] =
        picojson::value(static_cast<double>(statistics->server2client_total));
    item_obj["serverToClientAdded"] =
        picojson::value(static_cast<double>(statistics->server2client_nrofadd));
    item_obj["serverToClientUpdated"] =
        picojson::value(static_cast<double>(statistics->server2client_nrofreplace));
    item_obj["serverToClientRemoved"] =
        picojson::value(static_cast<double>(statistics->server2client_nrofdelete));
    item_obj["lastSyncTime"] = picojson::value(static_cast<double>(statistics->last_session_time));

    LoggerD("ClientToServerTotal: %d, ServerToClientTotal: %d", statistics->client2server_total,
            statistics->server2client_total);
  }
}

int DataSyncManager::StateChangedCallback(sync_agent_event_data_s* request) {
  LoggerD("DataSync session state changed.");

  char* profile_dir_name = nullptr;
  int sync_type = 0;
  char* progress = nullptr;
  char* error = nullptr;

  LoggerD("Get state info.");
  sync_agent_get_event_data_param(request, &profile_dir_name);
  sync_agent_get_event_data_param(request, &sync_type);
  sync_agent_get_event_data_param(request, &progress);
  sync_agent_get_event_data_param(request, &error);

  LoggerI("profileDirName: %s, sync_type: %d, progress: %s, error: %s",
          profile_dir_name, sync_type, progress,error);

  if (profile_dir_name) {
    std::string profile_dir_name_str(profile_dir_name);

    // truncate the rest
    profile_dir_name_str.resize(4);
    int profile_id = std::stoi(profile_dir_name_str);

    auto it = callbacks_.find(profile_id);
    if (it != callbacks_.end()) {
      int callback_id = it->second.first;
      DatasyncInstance* instance = it->second.second;
      callbacks_.erase(it);

      if (!progress) {
        LoggerW("nullptr status.");
//  TODO: implementation
      } else if (0 == strncmp(progress, "DONE", 4)) {
//  TODO: implementation
      } else if (0 == strncmp(progress, "CANCEL", 6)) {
//  TODO: implementation
      } else if (0 == strncmp(progress, "ERROR", 5)) {
//  TODO: implementation
      } else {
        LoggerI("Undefined status");
//  TODO: implementation
      }
    }
  }

  g_free(profile_dir_name);
  g_free(progress);
  g_free(error);

  if (request->size != nullptr) {
    g_free(request->size);
  }
  g_free(request);

  return 0;
}

int DataSyncManager::ProgressCallback(sync_agent_event_data_s* request) {
  LoggerD("DataSync progress called.");

  char* profile_dir_name = nullptr;
  int sync_type = 0;
  int uri;
  char* progress_status = nullptr;
  char* operation_type = nullptr;

  int is_from_server, total_per_operation, synced_per_operation, total_per_db,
      synced_per_db;

  LoggerD("Get progress info.");
  sync_agent_get_event_data_param(request, &profile_dir_name);
  sync_agent_get_event_data_param(request, &sync_type);
  sync_agent_get_event_data_param(request, &uri);
  sync_agent_get_event_data_param(request, &progress_status);
  sync_agent_get_event_data_param(request, &operation_type);

  LoggerI("profileDirName: %s, syncType: %d, uri: %d, progressStatus: %s, operationType %s",
          profile_dir_name, sync_type, uri, progress_status, operation_type);

  sync_agent_get_event_data_param(request, &is_from_server);
  sync_agent_get_event_data_param(request, &total_per_operation);
  sync_agent_get_event_data_param(request, &synced_per_operation);
  sync_agent_get_event_data_param(request, &total_per_db);
  sync_agent_get_event_data_param(request, &synced_per_db);

  LoggerI("isFromServer: %d, totalPerOperation: %d, syncedPerOperation: %d, totalPerDb: %d,\
          syncedPerDb %d",
          is_from_server, total_per_operation, synced_per_operation, total_per_db, synced_per_db);

  if (profile_dir_name) {
    std::string profile_dir_name_str(profile_dir_name);
    profile_dir_name_str.resize(4);
    int profile_id = std::stoi(profile_dir_name_str);

    auto it = callbacks_.find(profile_id);
    if (it != callbacks_.end()) {
      int callback_id = it->second.first;
      DatasyncInstance* instance = it->second.second;

      if (SYNC_AGENT_SRC_URI_CONTACT == uri) {
//  TODO: implementation
      } else if (SYNC_AGENT_SRC_URI_CALENDAR == uri) {
//  TODO: implementation
      } else {
        LoggerW("Wrong service type");
//  TODO: implementation
      }
    }
  }

  g_free(profile_dir_name);
  g_free(progress_status);
  g_free(operation_type);

  if (request != nullptr) {
    if (request->size != nullptr) {
      g_free(request->size);
    }
    g_free(request);
  }

  return 0;
}

}  // namespace datasync
}  // namespace extension
