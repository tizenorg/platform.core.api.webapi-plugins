/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "download/download_instance.h"

#include <functional>

#include <net_connection.h>

#include "common/tools.h"
#include "common/picojson.h"
#include "common/logger.h"
#include "common/typeutil.h"
#include "common/filesystem/filesystem_provider_storage.h"

namespace extension {
namespace download {

std::vector<DownloadInstance*> DownloadInstance::instances_;
std::mutex DownloadInstance::instances_mutex_;

namespace {
// The privileges that required in Download API
const std::string kPrivilegeDownload = "http://tizen.org/privilege/download";

}  // namespace

DownloadInstance::DownloadInstance() {
  LoggerD("Entered");
  using std::placeholders::_1;
  using std::placeholders::_2;
  #define REGISTER_SYNC(c, x) \
    RegisterSyncHandler(c, std::bind(&DownloadInstance::x, this, _1, _2));
  REGISTER_SYNC("DownloadManager_pause", DownloadManagerPause);
  REGISTER_SYNC
    ("DownloadManager_getDownloadRequest", DownloadManagerGetdownloadrequest);
  REGISTER_SYNC("DownloadManager_setListener", DownloadManagerSetlistener);
  REGISTER_SYNC("DownloadManager_getMIMEType", DownloadManagerGetmimetype);
  REGISTER_SYNC("DownloadManager_start", DownloadManagerStart);
  REGISTER_SYNC("DownloadManager_cancel", DownloadManagerCancel);
  REGISTER_SYNC("DownloadManager_resume", DownloadManagerResume);
  REGISTER_SYNC("DownloadManager_getState", DownloadManagerGetstate);
  #undef REGISTER_SYNC

  std::lock_guard<std::mutex> lock(instances_mutex_);
  instances_.push_back(this);
}

DownloadInstance::~DownloadInstance() {
  LoggerD("Entered");
  int ret;
  for (DownloadCallbackMap::iterator it = download_callbacks.begin();
    it != download_callbacks.end(); ++it) {
    DownloadInfoPtr diPtr = it->second->instance->diMap[it->second->callbackId];
    SLoggerD("~DownloadInstance() for callbackID %d Called", it->second->callbackId);

    ret = download_unset_state_changed_cb(diPtr->download_id);
    if (ret != DOWNLOAD_ERROR_NONE)
      LoggerE("download_unset_state_changed_cb() is failed. (%s)", get_error_message (ret));

    ret = download_unset_progress_cb(diPtr->download_id);
    if (ret != DOWNLOAD_ERROR_NONE)
      LoggerE("download_unset_progress_cb() is failed. (%s)", get_error_message (ret));

    ret = download_cancel(diPtr->download_id);
    if (ret != DOWNLOAD_ERROR_NONE)
      LoggerE("download_cancel() is failed. (%s)", get_error_message (ret));

    ret = download_destroy(diPtr->download_id);
    if (ret != DOWNLOAD_ERROR_NONE)
      LoggerE("download_destroy() is failed. (%s)", get_error_message (ret));

    delete (it->second);
  }

  std::lock_guard<std::mutex> lock(instances_mutex_);
  for (auto it = instances_.begin(); it != instances_.end(); it++) {
    if (*it == this) {
      instances_.erase(it);
      break;
    }
  }
}

bool DownloadInstance::CheckInstance(DownloadInstance* instance) {
  LoggerD("Entered");
  for (auto vec_instance : instances_) {
    if (vec_instance == instance) {
      return true;
    }
  }

  return false;
}

common::PlatformResult DownloadInstance::convertError(int err) {
  char* error = get_error_message(err);
  switch (err) {
    case DOWNLOAD_ERROR_INVALID_PARAMETER:
      return LogAndCreateResult(common::ErrorCode::INVALID_VALUES_ERR, error,
                                ("Error %d (%s)", err, error));
    case DOWNLOAD_ERROR_OUT_OF_MEMORY:
      return LogAndCreateResult(common::ErrorCode::UNKNOWN_ERR, error,
                                ("Error %d (%s)", err, error));
    case DOWNLOAD_ERROR_NETWORK_UNREACHABLE:
      return LogAndCreateResult(common::ErrorCode::UNKNOWN_ERR, error,
                                ("Error %d (%s)", err, error));
    case DOWNLOAD_ERROR_CONNECTION_TIMED_OUT:
      return LogAndCreateResult(common::ErrorCode::UNKNOWN_ERR, error,
                                ("Error %d (%s)", err, error));
    case DOWNLOAD_ERROR_NO_SPACE:
      return LogAndCreateResult(common::ErrorCode::UNKNOWN_ERR, error,
                                ("Error %d (%s)", err, error));
    case DOWNLOAD_ERROR_PERMISSION_DENIED:
      return LogAndCreateResult(common::ErrorCode::UNKNOWN_ERR, error,
                                ("Error %d (%s)", err, error));
    case DOWNLOAD_ERROR_NOT_SUPPORTED:
      return LogAndCreateResult(common::ErrorCode::UNKNOWN_ERR, error,
                                ("Error %d (%s)", err, error));
    case DOWNLOAD_ERROR_INVALID_STATE:
      return LogAndCreateResult(common::ErrorCode::INVALID_VALUES_ERR, error,
                                ("Error %d (%s)", err, error));
    case DOWNLOAD_ERROR_CONNECTION_FAILED:
      return LogAndCreateResult(common::ErrorCode::UNKNOWN_ERR, error,
                                ("Error %d (%s)", err, error));
    case DOWNLOAD_ERROR_INVALID_URL:
      return LogAndCreateResult(common::ErrorCode::UNKNOWN_ERR, error,
                                ("Error %d (%s)", err, error));
    case DOWNLOAD_ERROR_INVALID_DESTINATION:
      return LogAndCreateResult(common::ErrorCode::UNKNOWN_ERR, error,
                                ("Error %d (%s)", err, error));
    case DOWNLOAD_ERROR_TOO_MANY_DOWNLOADS:
      return LogAndCreateResult(common::ErrorCode::UNKNOWN_ERR, error,
                                ("Error %d (%s)", err, error));
    case DOWNLOAD_ERROR_QUEUE_FULL:
      return LogAndCreateResult(common::ErrorCode::UNKNOWN_ERR, error,
                                ("Error %d (%s)", err, error));
    case DOWNLOAD_ERROR_ALREADY_COMPLETED:
      return LogAndCreateResult(common::ErrorCode::UNKNOWN_ERR, error,
                                ("Error %d (%s)", err, error));
    case DOWNLOAD_ERROR_FILE_ALREADY_EXISTS:
      return LogAndCreateResult(common::ErrorCode::UNKNOWN_ERR, error,
                                ("Error %d (%s)", err, error));
    case DOWNLOAD_ERROR_CANNOT_RESUME:
      return LogAndCreateResult(common::ErrorCode::UNKNOWN_ERR, error,
                                ("Error %d (%s)", err, error));
    case DOWNLOAD_ERROR_FIELD_NOT_FOUND:
      return LogAndCreateResult(common::ErrorCode::UNKNOWN_ERR, error,
                                ("Error %d (%s)", err, error));
    case DOWNLOAD_ERROR_TOO_MANY_REDIRECTS:
      return LogAndCreateResult(common::ErrorCode::UNKNOWN_ERR, error,
                                ("Error %d (%s)", err, error));
    case DOWNLOAD_ERROR_UNHANDLED_HTTP_CODE:
      return LogAndCreateResult(common::ErrorCode::UNKNOWN_ERR, error,
                                ("Error %d (%s)", err, error));
    case DOWNLOAD_ERROR_REQUEST_TIMEOUT:
      return LogAndCreateResult(common::ErrorCode::UNKNOWN_ERR, error,
                                ("Error %d (%s)", err, error));
    case DOWNLOAD_ERROR_RESPONSE_TIMEOUT:
      return LogAndCreateResult(common::ErrorCode::UNKNOWN_ERR, error,
                                ("Error %d (%s)", err, error));
    case DOWNLOAD_ERROR_SYSTEM_DOWN:
      return LogAndCreateResult(common::ErrorCode::UNKNOWN_ERR, error,
                                ("Error %d (%s)", err, error));
    case DOWNLOAD_ERROR_ID_NOT_FOUND:
      return LogAndCreateResult(common::ErrorCode::UNKNOWN_ERR, error,
                                ("Error %d (%s)", err, error));
    case DOWNLOAD_ERROR_INVALID_NETWORK_TYPE:
      return LogAndCreateResult(common::ErrorCode::UNKNOWN_ERR, error,
                                ("Error %d (%s)", err, error));
    case DOWNLOAD_ERROR_NO_DATA:
      return LogAndCreateResult(common::ErrorCode::UNKNOWN_ERR, error,
                                ("Error %d (%s)", err, error));
    case DOWNLOAD_ERROR_IO_ERROR:
      return LogAndCreateResult(common::ErrorCode::UNKNOWN_ERR, error,
                                ("Error %d (%s)", err, error));
    default:
      return LogAndCreateResult(common::ErrorCode::UNKNOWN_ERR, "Unknown error.",
                                ("Unknown error: %d", err));
  }
}

#define CHECK_EXIST(args, name, out) \
    if (!args.contains(name)) {\
      LogAndReportError(common::PlatformResult(common::ErrorCode::TYPE_MISMATCH_ERR, name" is required argument"), &out);\
      return;\
    }

void DownloadInstance::OnStateChanged(int download_id,
  download_state_e state, void* user_data) {
  LoggerD("Entered");
  CallbackPtr downCbPtr = static_cast<CallbackPtr>(user_data);

  downCbPtr->state = state;
  downCbPtr->downloadId = download_id;

  SLoggerD("State for callbackId %d changed to %d",
    downCbPtr->callbackId, static_cast<int>(state));

  switch (state) {
    case DOWNLOAD_STATE_NONE:
      break;
    case DOWNLOAD_STATE_DOWNLOADING:
      OnStart(download_id, user_data);
      break;
    case DOWNLOAD_STATE_PAUSED:
      g_idle_add(OnPaused, downCbPtr);
      break;
    case DOWNLOAD_STATE_COMPLETED:
      g_idle_add(OnFinished, downCbPtr);
      break;
    case DOWNLOAD_STATE_CANCELED:
      g_idle_add(OnCanceled, downCbPtr);
      break;
    case DOWNLOAD_STATE_FAILED:
      g_idle_add(OnFailed, downCbPtr);
      break;
    default:
      LoggerD("Unexpected download state: %d", state);
      break;
  }
}

gboolean DownloadInstance::OnProgressChanged(void* user_data) {
  LoggerD("Entered");
  CallbackPtr downCbPtr = static_cast<CallbackPtr>(user_data);
  std::lock_guard<std::mutex> lock(instances_mutex_);
  if (!CheckInstance(downCbPtr->instance)) {
    return FALSE;
  }

  DownloadInfoPtr diPtr = downCbPtr->instance->diMap[downCbPtr->callbackId];

  picojson::value::object out;
  out["status"] = picojson::value("progress");
  out["callbackId"] =
    picojson::value(static_cast<double>(downCbPtr->callbackId));
  out["receivedSize"] =
    picojson::value(static_cast<double>(downCbPtr->received));
  out["totalSize"] = picojson::value(static_cast<double>(diPtr->file_size));

  LoggerD("OnProgressChanged for callbackId %d Called: Received: %ld",
    downCbPtr->callbackId, downCbPtr->received);

  picojson::value v = picojson::value(out);
  Instance::PostMessage(downCbPtr->instance, v.serialize().c_str());

  return FALSE;
}

void DownloadInstance::OnStart(int download_id, void* user_data) {
  LoggerD("Entered");
  unsigned long long totalSize;

  CallbackPtr downCbPtr = static_cast<CallbackPtr>(user_data);
  std::lock_guard<std::mutex> lock(instances_mutex_);
  if (!CheckInstance(downCbPtr->instance)) {
    return;
  }

  SLoggerD("OnStart for callbackId %d Called", downCbPtr->callbackId);

  DownloadInfoPtr diPtr = downCbPtr->instance->diMap[downCbPtr->callbackId];

  download_get_content_size(download_id, &totalSize);

  diPtr->file_size = totalSize;
}

gboolean DownloadInstance::OnFinished(void* user_data) {
  LoggerD("Entered");
  char* fullPath = NULL;

  CallbackPtr downCbPtr = static_cast<CallbackPtr>(user_data);
  std::lock_guard<std::mutex> lock(instances_mutex_);
  if (!CheckInstance(downCbPtr->instance)) {
    return FALSE;
  }

  int callback_id = downCbPtr->callbackId;
  DownloadInfoPtr diPtr = downCbPtr->instance->diMap[callback_id];

  LoggerD("OnFinished for callbackID %d Called", callback_id);

  picojson::value::object out;

  int ret = download_get_downloaded_file_path(downCbPtr->downloadId, &fullPath);
  if (ret != DOWNLOAD_ERROR_NONE) {
    LogAndReportError(convertError(ret), &out,
                      ("download_get_downloaded_file_path error: %d (%s)", ret, get_error_message(ret)));
  } else {
    ret = download_unset_state_changed_cb(diPtr->download_id);
    if (ret != DOWNLOAD_ERROR_NONE) {
      LoggerW("%s", get_error_message(ret));
    }
    ret = download_unset_progress_cb(diPtr->download_id);
    if (ret != DOWNLOAD_ERROR_NONE) {
      LoggerW("%s", get_error_message(ret));
    }
    ret = download_destroy(diPtr->download_id);
    if (ret != DOWNLOAD_ERROR_NONE) {
      LoggerW("%s", get_error_message(ret));
    }
    out["status"] = picojson::value("completed");
  }

  out["callbackId"] = picojson::value(static_cast<double>(callback_id));

  out["fullPath"] = picojson::value(common::FilesystemProviderStorage::Create().GetVirtualPath(fullPath));

  Instance::PostMessage(downCbPtr->instance, picojson::value(out).serialize().c_str());
  downCbPtr->instance->download_callbacks.erase(callback_id);
  delete (downCbPtr);

  free(fullPath);

  return FALSE;
}

gboolean DownloadInstance::OnPaused(void* user_data) {
  LoggerD("Entered");
  CallbackPtr downCbPtr = static_cast<CallbackPtr>(user_data);
  std::lock_guard<std::mutex> lock(instances_mutex_);
  if (!CheckInstance(downCbPtr->instance)) {
    return FALSE;
  }

  int callback_id = downCbPtr->callbackId;
  DownloadInfoPtr diPtr = downCbPtr->instance->diMap[callback_id];

  LoggerD("OnPaused for callbackID %d Called", callback_id);

  picojson::value::object out;
  out["status"] = picojson::value("paused");
  out["callbackId"] =
    picojson::value(static_cast<double>(callback_id));

  Instance::PostMessage(downCbPtr->instance, picojson::value(out).serialize().c_str());
  return FALSE;
}

gboolean DownloadInstance::OnCanceled(void* user_data) {
  LoggerD("Entered");
  CallbackPtr downCbPtr = static_cast<CallbackPtr>(user_data);
  std::lock_guard<std::mutex> lock(instances_mutex_);
  if (!CheckInstance(downCbPtr->instance)) {
    return FALSE;
  }

  int callback_id = downCbPtr->callbackId;
  DownloadInfoPtr diPtr = downCbPtr->instance->diMap[callback_id];

  LoggerD("OnCanceled for callbackID %d Called", callback_id);

  int ret = download_unset_state_changed_cb(diPtr->download_id);
  if (ret != DOWNLOAD_ERROR_NONE) {
    LoggerE("%s", get_error_message(ret));
  }

  ret = download_unset_progress_cb(diPtr->download_id);
  if (ret != DOWNLOAD_ERROR_NONE) {
    LoggerE("%s", get_error_message(ret));
  }

  ret = download_destroy(diPtr->download_id);
  if (ret != DOWNLOAD_ERROR_NONE) {
    LoggerE("%s", get_error_message(ret));
  }

  picojson::value::object out;
  out["status"] = picojson::value("canceled");
  out["callbackId"] =
    picojson::value(static_cast<double>(callback_id));

  Instance::PostMessage(downCbPtr->instance, picojson::value(out).serialize().c_str());
  downCbPtr->instance->download_callbacks.erase(callback_id);
  delete (downCbPtr);

  return FALSE;
}

gboolean DownloadInstance::OnFailed(void* user_data) {
  LoggerD("Entered");
  download_error_e error;
  picojson::object out;

  CallbackPtr downCbPtr = static_cast<CallbackPtr>(user_data);
  std::lock_guard<std::mutex> lock(instances_mutex_);
  if (!CheckInstance(downCbPtr->instance)) {
    return FALSE;
  }

  LoggerD("OnFailed for callbackID %d Called", downCbPtr->callbackId);

  download_get_error(downCbPtr->downloadId, &error);

  if (DOWNLOAD_ERROR_NONE != error) {
    LogAndReportError(convertError(error), &out,
                      ("download_get_error error: %d (%s)", error, get_error_message(error)));
  }

  out["callbackId"] =
    picojson::value(static_cast<double>(downCbPtr->callbackId));

  Instance::PostMessage(downCbPtr->instance, picojson::value(out).serialize().c_str());
  return FALSE;
}

void DownloadInstance::progress_changed_cb
  (int download_id, long long unsigned received, void* user_data) {
  LoggerD("Entered");
  CallbackPtr downCbPtr = static_cast<CallbackPtr>(user_data);
  downCbPtr->received = received;
  downCbPtr->downloadId = download_id;

  g_idle_add(OnProgressChanged, downCbPtr);
}

void DownloadInstance::DownloadManagerStart
  (const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  CHECK_PRIVILEGE_ACCESS(kPrivilegeDownload, &out);
  CHECK_EXIST(args, "callbackId", out)

  int ret;
  std::string networkType;

  DownloadInfoPtr diPtr(new DownloadInfo);

  diPtr->callbackId = static_cast<int>(args.get("callbackId").get<double>());
  diPtr->url = args.get("url").get<std::string>();

  if (!args.get("destination").is<picojson::null>()) {
    if (args.get("destination").get<std::string>() != "") {
      diPtr->destination = args.get("destination").get<std::string>();
      diPtr->destination = common::FilesystemProviderStorage::Create().GetRealPath(diPtr->destination);
    }
  }

  if (!args.get("fileName").is<picojson::null>()) {
    if (args.get("fileName").get<std::string>() != "") {
      diPtr->file_name = args.get("fileName").get<std::string>();
    }
  }

  if (!args.get("networkType").is<picojson::null>()) {
    networkType = args.get("networkType").get<std::string>();
  }

  bool network_support = false;
  bool cell_support = false;
  bool wifi_support = false;
  bool ethernet_support = false;

  system_info_get_platform_bool("http://tizen.org/feature/network.telephony",
                                &cell_support);
  system_info_get_platform_bool("http://tizen.org/feature/network.wifi",
                                &wifi_support);
  system_info_get_platform_bool("http://tizen.org/feature/network.ethernet",
                                &ethernet_support);

  connection_h connection = nullptr;
  connection_create(&connection);

  connection_cellular_state_e cell_state = CONNECTION_CELLULAR_STATE_OUT_OF_SERVICE;
  connection_wifi_state_e wifi_state = CONNECTION_WIFI_STATE_DEACTIVATED;
  connection_ethernet_state_e ethernet_state = CONNECTION_ETHERNET_STATE_DEACTIVATED;

  connection_get_cellular_state(connection, &cell_state);
  connection_get_wifi_state(connection, &wifi_state);
  connection_get_ethernet_state(connection, &ethernet_state);
  connection_destroy(connection);

  bool network_available = false;
  bool cell_available = (CONNECTION_CELLULAR_STATE_CONNECTED == cell_state);
  bool wifi_available = (CONNECTION_WIFI_STATE_CONNECTED == wifi_state);
  bool ethernet_available = CONNECTION_ETHERNET_STATE_CONNECTED == ethernet_state;

  if (networkType == "CELLULAR") {
    network_support = cell_support;
    network_available = cell_available;
    diPtr->network_type = DOWNLOAD_NETWORK_DATA_NETWORK;
  } else if (networkType == "WIFI") {
    network_support = wifi_support;
    network_available = wifi_available;
    diPtr->network_type = DOWNLOAD_NETWORK_WIFI;
  } else if (networkType == "ALL") {
    network_support = cell_support || wifi_support || ethernet_support;
    network_available = cell_available || wifi_available || ethernet_available;
    diPtr->network_type = DOWNLOAD_NETWORK_ALL;
  } else {
    LogAndReportError(
        common::PlatformResult(common::ErrorCode::INVALID_VALUES_ERR,
                               "The input parameter contains an invalid network type."),
        &out,
        ("The input parameter contains an invalid network type: %s", networkType.c_str()));
    return;
  }

  if (!network_support) {
    LogAndReportError(
        common::PlatformResult(common::ErrorCode::NOT_SUPPORTED_ERR,
                               "The networkType of the given DownloadRequest "
                               "is not supported on this device."),
        &out,
        ("Requested network type (%s) is not supported.", networkType.c_str()));
    return;
  }

  if (!network_available) {
    LogAndReportError(
        common::PlatformResult(common::ErrorCode::NETWORK_ERR,
                               "The networkType of the given DownloadRequest "
                               "is currently not available on this device."),
        &out,
        ("Requested network type (%s) is not available.", networkType.c_str()));
    return;
  }

  CallbackPtr downCbPtr(new DownloadCallback);

  downCbPtr->callbackId = diPtr->callbackId;
  downCbPtr->instance = this;

  download_callbacks[downCbPtr->callbackId] = downCbPtr;

  ret = download_create(&diPtr->download_id);
  if (ret != DOWNLOAD_ERROR_NONE) {
    LogAndReportError(convertError(ret), &out,
                      ("download_create error: %d (%s)", ret, get_error_message(ret)));
    return;
  }

  ret = download_set_state_changed_cb (diPtr->download_id, OnStateChanged,
                                       static_cast<void*>(downCbPtr));
  if (ret != DOWNLOAD_ERROR_NONE) {
    LogAndReportError(convertError(ret), &out,
                      ("download_set_state_changed_cb error: %d (%s)", ret, get_error_message(ret)));
    return;
  }

  ret = download_set_progress_cb (diPtr->download_id, progress_changed_cb,
                                  static_cast<void*>(downCbPtr));
  if (ret != DOWNLOAD_ERROR_NONE) {
    LogAndReportError(convertError(ret), &out,
                      ("download_set_progress_cb error: %d (%s)", ret, get_error_message(ret)));
    return;
  }

  ret = download_set_url(diPtr->download_id, diPtr->url.c_str());
  if (ret != DOWNLOAD_ERROR_NONE) {
    LogAndReportError(convertError(ret), &out,
                      ("download_set_url error: %d (%s)", ret, get_error_message(ret)));
    return;
  }

  if (diPtr->destination.size() != 0) {
    ret = download_set_destination(diPtr->download_id, diPtr->destination.c_str());
    if (ret != DOWNLOAD_ERROR_NONE) {
      LogAndReportError(convertError(ret), &out,
                        ("download_set_destination error: %d (%s)", ret, get_error_message(ret)));
      return;
    }
  }

  if (!diPtr->file_name.empty()) {
    ret = download_set_file_name(diPtr->download_id, diPtr->file_name.c_str());
    if (ret != DOWNLOAD_ERROR_NONE) {
      LogAndReportError(convertError(ret), &out,
                        ("download_set_file_name error: %d (%s)", ret, get_error_message(ret)));
      return;
    }
  }

  ret = download_set_network_type(diPtr->download_id, diPtr->network_type);

  if (args.get("httpHeader").is<picojson::object>()) {
    picojson::object obj = args.get("httpHeader").get<picojson::object>();
    for (picojson::object::const_iterator it = obj.begin();
    it != obj.end(); ++it) {
      download_add_http_header_field
        (diPtr->download_id, it->first.c_str(), it->second.to_str().c_str());
    }
  }

  diMap[downCbPtr->callbackId] = diPtr;

  ret = download_start(diPtr->download_id);

  if (ret == DOWNLOAD_ERROR_NONE) {
    ReportSuccess(out);
  } else {
    LogAndReportError(convertError(ret), &out,
                      ("download_start error: %d (%s)", ret, get_error_message(ret)));
  }
}

void DownloadInstance::DownloadManagerCancel
  (const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  CHECK_EXIST(args, "downloadId", out)
  int downloadId, ret;

  int callbackId = static_cast<int>(args.get("downloadId").get<double>());

  if (!GetDownloadID(callbackId, downloadId)) {
    LogAndReportError(common::PlatformResult(common::ErrorCode::NOT_FOUND_ERR,
                          "The identifier does not match any download operation in progress"),
                      &out,
                      ("The identifier %d does not match any download operation in progress", downloadId));
    return;
  }

  ret = download_cancel(downloadId);

  if (ret == DOWNLOAD_ERROR_NONE) {
    ReportSuccess(out);
  } else {
    LogAndReportError(convertError(ret), &out,
                      ("download_cancel error: %d (%s)", ret, get_error_message(ret)));
  }
}

void DownloadInstance::DownloadManagerPause
  (const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  CHECK_EXIST(args, "downloadId", out)
  int downloadId, ret;

  int callbackId = static_cast<int>(args.get("downloadId").get<double>());

  if (!GetDownloadID(callbackId, downloadId)) {
    LogAndReportError(common::PlatformResult(common::ErrorCode::NOT_FOUND_ERR,
                        "The identifier does not match any download operation in progress"),
                      &out,
                      ("The identifier %d does not match any download operation in progress", downloadId));
    return;
  }

  ret = download_pause(downloadId);

  if (ret == DOWNLOAD_ERROR_NONE) {
    ReportSuccess(out);
  } else {
    LogAndReportError(convertError(ret), &out,
                      ("download_pause error: %d (%s)", ret, get_error_message(ret)));
  }
}

void DownloadInstance::DownloadManagerResume
  (const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  CHECK_EXIST(args, "downloadId", out)
  int downloadId, ret;

  int callbackId = static_cast<int>(args.get("downloadId").get<double>());

  if (!GetDownloadID(callbackId, downloadId)) {
    LogAndReportError(common::PlatformResult(common::ErrorCode::NOT_FOUND_ERR,
                        "The identifier does not match any download operation in progress"),
                      &out,
                      ("The identifier %d does not match any download operation in progress", downloadId));
    return;
  }

  ret = download_start(downloadId);

  if (ret == DOWNLOAD_ERROR_NONE) {
    ReportSuccess(out);
  } else {
    LogAndReportError(convertError(ret), &out,
                      ("download_start error: %d (%s)", ret, get_error_message(ret)));
  }
}

void DownloadInstance::DownloadManagerGetstate
  (const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  CHECK_EXIST(args, "downloadId", out)
  int downloadId, ret;
  std::string stateValue;
  download_state_e state;

  int callbackId = static_cast<int>(args.get("downloadId").get<double>());

  if (!GetDownloadID(callbackId, downloadId)) {
    LogAndReportError(common::PlatformResult(common::ErrorCode::NOT_FOUND_ERR,
                        "The identifier does not match any download operation in progress"),
                      &out,
                      ("The identifier %d does not match any download operation in progress", downloadId));
    return;
  }

  ret = download_get_state(downloadId, &state);

  if (ret == DOWNLOAD_ERROR_NONE) {
    switch (state) {
     case DOWNLOAD_STATE_NONE:
      break;
     case DOWNLOAD_STATE_QUEUED:
       stateValue = "QUEUED";
       break;
     case DOWNLOAD_STATE_DOWNLOADING:
       stateValue = "DOWNLOADING";
       break;
     case DOWNLOAD_STATE_PAUSED:
       stateValue = "PAUSED";
       break;
     case DOWNLOAD_STATE_COMPLETED:
       stateValue = "COMPLETED";
       break;
     case DOWNLOAD_STATE_FAILED:
       stateValue = "FAILED";
       break;
     case DOWNLOAD_STATE_CANCELED:
       stateValue = "CANCELED";
       break;
     default:
      LoggerD("Unexpected download state: %d", state);
      break;
    }

    ReportSuccess(picojson::value(stateValue), out);
  } else {
    LogAndReportError(convertError(ret), &out,
                      ("download_get_state error: %d (%s)", ret, get_error_message(ret)));
  }
}

void DownloadInstance::DownloadManagerGetmimetype
  (const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  CHECK_EXIST(args, "downloadId", out)

  int downloadId, ret;
  char* mimetype = NULL;

  int callbackId = static_cast<int>(args.get("downloadId").get<double>());

  if (!GetDownloadID(callbackId, downloadId)) {
    LogAndReportError(common::PlatformResult(common::ErrorCode::NOT_FOUND_ERR,
                        "The identifier does not match any download operation in progress"),
                      &out,
                      ("The identifier %d does not match any download operation in progress", downloadId));
    return;
  }

  ret = download_get_mime_type(downloadId, &mimetype);

  if (ret == DOWNLOAD_ERROR_NONE) {
    ReportSuccess(picojson::value(mimetype), out);
  } else {
    LogAndReportError(convertError(ret), &out,
                      ("download_get_mime_type error: %d (%s)", ret, get_error_message(ret)));
  }
  free(mimetype);
}

bool DownloadInstance::GetDownloadID
  (const int callback_id, int& download_id) {
  LoggerD("Entered");
  if (diMap.find(callback_id) != diMap.end()) {
    download_id = diMap.find(callback_id)->second->download_id;
  } else {
    return false;
  }
  return true;
}

void DownloadInstance::DownloadManagerGetdownloadrequest
  (const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  // Nothing to do
}

void DownloadInstance::DownloadManagerSetlistener
  (const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  // Nothing to do
}


#undef CHECK_EXIST

}  // namespace download
}  // namespace extension
