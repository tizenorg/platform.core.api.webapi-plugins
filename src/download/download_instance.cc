// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "download/download_instance.h"

#include <functional>

#include <net_connection.h>

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"
#include "common/typeutil.h"
#include "common/virtual_fs.h"

namespace extension {
namespace download {

namespace {
// The privileges that required in Download API
const std::string kPrivilegeDownload = "http://tizen.org/privilege/download";

}  // namespace

using common::NotFoundException;
using common::UnknownException;
using common::NetworkException;
using common::SecurityException;
using common::QuotaExceededException;
using common::NotSupportedException;
using common::InvalidStateException;
using common::IOException;
using common::InvalidValuesException;
using common::ServiceNotAvailableException;
using common::TypeMismatchException;

DownloadInstance::DownloadInstance() {
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
}

DownloadInstance::~DownloadInstance() {
  for (DownloadCallbackVector::iterator it = downCbVector.begin();
    it != downCbVector.end(); it++) {
    delete (*it);
  }
}

#define CHECK_EXIST(args, name, out) \
    if (!args.contains(name)) {\
      ReportError(TypeMismatchException(name" is required argument"), out);\
      return;\
    }

void DownloadInstance::OnStateChanged(int download_id,
  download_state_e state, void* user_data) {
  DownloadCallback* downCbPtr = static_cast<DownloadCallback*>(user_data);

  downCbPtr->state = state;
  downCbPtr->downloadId = download_id;

  SLoggerD("State for callbackId %d changed to %d",
    downCbPtr->callbackId, static_cast<int>(state));

  switch (state) {
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
  }
}

gboolean DownloadInstance::OnProgressChanged(void* user_data) {
  DownloadCallback* downCbPtr = static_cast<DownloadCallback*>(user_data);
  DownloadInfoPtr diPtr = downCbPtr->instance->diMap[downCbPtr->callbackId];

  picojson::value::object out;
  out["status"] = picojson::value("progress");
  out["callbackId"] =
    picojson::value(static_cast<double>(downCbPtr->callbackId));
  out["receivedSize"] =
    picojson::value(static_cast<double>(downCbPtr->received));
  out["totalSize"] = picojson::value(static_cast<double>(diPtr->file_size));

  SLoggerD("OnProgressChanged for callbackId %d Called: Received: %ld",
    downCbPtr->callbackId, downCbPtr->received);

  picojson::value v = picojson::value(out);
  downCbPtr->instance->PostMessage(v.serialize().c_str());

  return FALSE;
}

void DownloadInstance::OnStart(int download_id, void* user_data) {
  unsigned long long totalSize;
  int ret;

  DownloadCallback* downCbPtr = static_cast<DownloadCallback*>(user_data);

  SLoggerD("OnStart for callbackId %d Called", downCbPtr->callbackId);

  DownloadInfoPtr diPtr = downCbPtr->instance->diMap[downCbPtr->callbackId];

  download_get_content_size(download_id, &totalSize);

  diPtr->file_size = totalSize;
}

gboolean DownloadInstance::OnFinished(void* user_data) {
  char* fullPath = NULL;

  DownloadCallback* downCbPtr = static_cast<DownloadCallback*>(user_data);
  DownloadInfoPtr diPtr = downCbPtr->instance->diMap[downCbPtr->callbackId];

  SLoggerD("OnFinished for callbackID %d Called", downCbPtr->callbackId);

  download_get_downloaded_file_path(downCbPtr->downloadId, &fullPath);

  download_unset_state_changed_cb(diPtr->download_id);
  download_unset_progress_cb(diPtr->download_id);
  download_destroy(diPtr->download_id);

  picojson::value::object out;
  out["status"] = picojson::value("completed");
  out["callbackId"] =
    picojson::value(static_cast<double>(downCbPtr->callbackId));
  out["fullPath"] = picojson::value(fullPath);

  downCbPtr->instance->PostMessage(picojson::value(out).serialize().c_str());
  return FALSE;
}

gboolean DownloadInstance::OnPaused(void* user_data) {
  DownloadCallback* downCbPtr = static_cast<DownloadCallback*>(user_data);
  DownloadInfoPtr diPtr = downCbPtr->instance->diMap[downCbPtr->callbackId];

  SLoggerD("OnPaused for callbackID %d Called", downCbPtr->callbackId);

  picojson::value::object out;
  out["status"] = picojson::value("paused");
  out["callbackId"] =
    picojson::value(static_cast<double>(downCbPtr->callbackId));

  downCbPtr->instance->PostMessage(picojson::value(out).serialize().c_str());
  return FALSE;
}

gboolean DownloadInstance::OnCanceled(void* user_data) {
  DownloadCallback* downCbPtr = static_cast<DownloadCallback*>(user_data);
  DownloadInfoPtr diPtr = downCbPtr->instance->diMap[downCbPtr->callbackId];

  SLoggerD("OnCanceled for callbackID %d Called", downCbPtr->callbackId);

  download_unset_state_changed_cb(diPtr->download_id);
  download_unset_progress_cb(diPtr->download_id);
  download_destroy(diPtr->download_id);

  picojson::value::object out;
  out["status"] = picojson::value("canceled");
  out["callbackId"] =
    picojson::value(static_cast<double>(downCbPtr->callbackId));

  downCbPtr->instance->PostMessage(picojson::value(out).serialize().c_str());
  return FALSE;
}

gboolean DownloadInstance::OnFailed(void* user_data) {
  download_error_e error;
  picojson::object out;

  DownloadCallback* downCbPtr = static_cast<DownloadCallback*>(user_data);
  DownloadInstance* instance = downCbPtr->instance;

  SLoggerD("OnFailed for callbackID %d Called", downCbPtr->callbackId);

  download_get_error(downCbPtr->downloadId, &error);

  switch (error) {
    case DOWNLOAD_ERROR_INVALID_PARAMETER:
      instance->ReportError(NotFoundException("not found"), out);
      break;
    case DOWNLOAD_ERROR_OUT_OF_MEMORY:
      instance->ReportError(UnknownException("Out of memory"), out);
      break;
    case DOWNLOAD_ERROR_NETWORK_UNREACHABLE:
      instance->ReportError(NetworkException("Network is unreachable"), out);
      break;
    case DOWNLOAD_ERROR_CONNECTION_TIMED_OUT:
      instance->ReportError(NetworkException("HTTP session timeout"), out);
      break;
    case DOWNLOAD_ERROR_NO_SPACE:
      instance->ReportError(QuotaExceededException(
        "No space left on device"), out);
      break;
    case DOWNLOAD_ERROR_PERMISSION_DENIED:
      instance->ReportError(SecurityException(
        "The application does not have the privilege to call this method."),
        out);
      break;
    case DOWNLOAD_ERROR_NOT_SUPPORTED:
      instance->ReportError(NotSupportedException("Not supported"), out);
      break;
    case DOWNLOAD_ERROR_INVALID_STATE:
      instance->ReportError(InvalidStateException("Invalid state"), out);
      break;
    case DOWNLOAD_ERROR_CONNECTION_FAILED:
      instance->ReportError(NetworkException("Connection failed"), out);
      break;
    case DOWNLOAD_ERROR_INVALID_URL:
      instance->ReportError(InvalidValuesException("Invalid URL"), out);
      break;
    case DOWNLOAD_ERROR_INVALID_DESTINATION:
      instance->ReportError(InvalidValuesException(
        "Invalid destination"), out);
      break;
    case DOWNLOAD_ERROR_TOO_MANY_DOWNLOADS:
      instance->ReportError(QuotaExceededException(
        "Too many simultaneous downloads"), out);
      break;
    case DOWNLOAD_ERROR_QUEUE_FULL:
      instance->ReportError(QuotaExceededException(
        "Download server queue is full"), out);
      break;
    case DOWNLOAD_ERROR_ALREADY_COMPLETED:
      instance->ReportError(InvalidStateException(
        "The download is already completed"), out);
      break;
    case DOWNLOAD_ERROR_FILE_ALREADY_EXISTS:
      instance->ReportError(IOException(
        "Failed to rename the downloaded file"), out);
      break;
    case DOWNLOAD_ERROR_CANNOT_RESUME:
      instance->ReportError(NotSupportedException("Cannot resume"), out);
      break;
    case DOWNLOAD_ERROR_FIELD_NOT_FOUND:
      instance->ReportError(NotFoundException(
        "Specified field not found"), out);
      break;
    case DOWNLOAD_ERROR_TOO_MANY_REDIRECTS:
      instance->ReportError(NetworkException(
        "Too many redirects from HTTP response header"), out);
      break;
    case DOWNLOAD_ERROR_UNHANDLED_HTTP_CODE:
      instance->ReportError(NetworkException(
        "The download cannot handle the HTTP status value"), out);
      break;
    case DOWNLOAD_ERROR_REQUEST_TIMEOUT:
      instance->ReportError(NetworkException(
        "No action after client creates a download ID"), out);
      break;
    case DOWNLOAD_ERROR_RESPONSE_TIMEOUT:
      instance->ReportError(NetworkException(
        "No call to start API for some time although the download is created"),
        out);
      break;
    case DOWNLOAD_ERROR_SYSTEM_DOWN:
      instance->ReportError(ServiceNotAvailableException(
        "No response from client after rebooting download daemon"), out);
      break;
    case DOWNLOAD_ERROR_ID_NOT_FOUND:
      instance->ReportError(NotFoundException(
        "Download ID does not exist in download service module"), out);
      break;
    case DOWNLOAD_ERROR_INVALID_NETWORK_TYPE:
      instance->ReportError(InvalidValuesException(
        "Network bonding is set but network type is not set as ALL"), out);
      break;
    case DOWNLOAD_ERROR_NO_DATA:
      instance->ReportError(NotFoundException(
        "No data because the set API is not called"), out);
      break;
    case DOWNLOAD_ERROR_IO_ERROR:
      instance->ReportError(IOException("Internal I/O error"), out);
      break;
  }

  out["callbackId"] =
    picojson::value(static_cast<double>(downCbPtr->callbackId));

  downCbPtr->instance->PostMessage(picojson::value(out).serialize().c_str());
  return FALSE;
}

void DownloadInstance::progress_changed_cb
  (int download_id, long long unsigned received, void* user_data) {
  DownloadCallback* downCbPtr = static_cast<DownloadCallback*>(user_data);
  downCbPtr->received = received;
  downCbPtr->downloadId = download_id;

  g_idle_add(OnProgressChanged, downCbPtr);
}

void DownloadInstance::DownloadManagerStart
  (const picojson::value& args, picojson::object& out) {
  CHECK_PRIVILEGE_ACCESS(kPrivilegeDownload, &out);
  CHECK_EXIST(args, "callbackId", out)

  int ret, downlodId;
  std::string networkType;

  DownloadInfoPtr diPtr(new DownloadInfo);

  diPtr->callbackId = static_cast<int>(args.get("callbackId").get<double>());
  diPtr->url = args.get("url").get<std::string>();

  if (!args.get("destination").is<picojson::null>()) {
    if (args.get("destination").get<std::string>() != "") {
      diPtr->destination = args.get("destination").get<std::string>();
      // TODO: move conversion to JS
      diPtr->destination = common::VirtualFs::GetInstance().GetRealPath(diPtr->destination);
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

  system_info_get_platform_bool("http://tizen.org/feature/network.telephony",
                                &cell_support);
  system_info_get_platform_bool("http://tizen.org/feature/network.wifi",
                                &wifi_support);

  connection_h connection = nullptr;
  connection_create(&connection);

  connection_cellular_state_e cell_state = CONNECTION_CELLULAR_STATE_OUT_OF_SERVICE;
  connection_wifi_state_e wifi_state = CONNECTION_WIFI_STATE_DEACTIVATED;

  connection_get_cellular_state(connection, &cell_state);
  connection_get_wifi_state(connection, &wifi_state);
  connection_destroy(connection);

  bool network_available = false;
  bool cell_available = (CONNECTION_CELLULAR_STATE_CONNECTED == cell_state);
  bool wifi_available = (CONNECTION_WIFI_STATE_CONNECTED == wifi_state);

  if (networkType == "CELLULAR") {
    network_support = cell_support;
    network_available = cell_available;
    diPtr->network_type = DOWNLOAD_NETWORK_DATA_NETWORK;
  } else if (networkType == "WIFI") {
    network_support = wifi_support;
    network_available = wifi_available;
    diPtr->network_type = DOWNLOAD_NETWORK_WIFI;
  } else if (networkType == "ALL") {
    network_support = cell_support || wifi_support;
    network_available = cell_available || wifi_available;
    diPtr->network_type = DOWNLOAD_NETWORK_ALL;
  } else {
    ReportError(
        InvalidValuesException(
            "The input parameter contains an invalid network type."),
        out);
    return;
  }

  if (!network_support) {
    SLoggerE("Requested network type (%s) is not supported.", networkType.c_str());
    ReportError(
        common::PlatformResult(common::ErrorCode::NOT_SUPPORTED_ERR,
                               "The networkType of the given DownloadRequest "
                               "is not supported on this device."),
        &out);
    return;
  }

  if (!network_available) {
    SLoggerE("Requested network type (%s) is not available.", networkType.c_str());
    ReportError(
        common::PlatformResult(common::ErrorCode::NETWORK_ERR,
                               "The networkType of the given DownloadRequest "
                               "is currently not available on this device."),
        &out);
    return;
  }

  DownloadCallback* downCbPtr(new DownloadCallback);

  downCbPtr->callbackId = diPtr->callbackId;
  downCbPtr->instance = this;

  downCbVector.push_back(downCbPtr);

  ret = download_create(&diPtr->download_id);
  ret =
    download_set_state_changed_cb
    (diPtr->download_id, OnStateChanged, static_cast<void*>(downCbPtr));
  ret =
    download_set_progress_cb
    (diPtr->download_id, progress_changed_cb, static_cast<void*>(downCbPtr));
  ret =
    download_set_url(diPtr->download_id, diPtr->url.c_str());

  const char* dest;

  if (diPtr->destination.size() != 0) {
    ret = download_set_destination(diPtr->download_id, diPtr->destination.c_str());
  }

  if (!diPtr->file_name.empty()) {
    ret = download_set_file_name(diPtr->download_id, diPtr->file_name.c_str());
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

  if (ret == DOWNLOAD_ERROR_NONE)
    ReportSuccess(out);
  else if (ret == DOWNLOAD_ERROR_INVALID_PARAMETER)
    ReportError(InvalidValuesException
    ("The input parameter contains an invalid value."), out);
  else if (ret == DOWNLOAD_ERROR_OUT_OF_MEMORY)
    ReportError(UnknownException("Out of memory"), out);
  else if (ret == DOWNLOAD_ERROR_INVALID_STATE)
    ReportError(InvalidValuesException("Invalid state"), out);
  else if (ret == DOWNLOAD_ERROR_IO_ERROR)
    ReportError(UnknownException("Internal I/O error"), out);
  else if (ret == DOWNLOAD_ERROR_INVALID_URL)
    ReportError(InvalidValuesException(
    "The input parameter contains an invalid url."), out);
  else if (ret == DOWNLOAD_ERROR_INVALID_DESTINATION)
    ReportError(InvalidValuesException(
    "The input parameter contains an invalid destination."), out);
  else if (ret == DOWNLOAD_ERROR_ID_NOT_FOUND)
    ReportError(InvalidValuesException("No such a download ID found"), out);
  else if (ret == DOWNLOAD_ERROR_QUEUE_FULL)
    ReportError(UnknownException("Download server queue is full"), out);
  else if (ret == DOWNLOAD_ERROR_PERMISSION_DENIED)
    ReportError(SecurityException(
    "The application does not have the privilege to call this method."), out);
  else
    ReportError(UnknownException("Unknown Error"), out);
}

void DownloadInstance::DownloadManagerCancel
  (const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "downloadId", out)
  int downloadId, ret;

  int callbackId = static_cast<int>(args.get("downloadId").get<double>());

  if (!GetDownloadID(callbackId, downloadId)) {
    ReportError(NotFoundException
      ("The identifier does not match any download operation in progress"),
      out);
    return;
  }

  ret = download_cancel(downloadId);

  if (ret == DOWNLOAD_ERROR_NONE)
    ReportSuccess(out);
  else if (ret == DOWNLOAD_ERROR_INVALID_PARAMETER)
    ReportError(InvalidValuesException
    ("The input parameter contains an invalid value."), out);
  else if (ret == DOWNLOAD_ERROR_OUT_OF_MEMORY)
    ReportError(UnknownException("Out of memory"), out);
  else if (ret == DOWNLOAD_ERROR_INVALID_STATE)
    ReportError(InvalidValuesException("Invalid state"), out);
  else if (ret == DOWNLOAD_ERROR_IO_ERROR)
    ReportError(UnknownException("Internal I/O error"), out);
  else if (ret == DOWNLOAD_ERROR_PERMISSION_DENIED)
    ReportError(UnknownException("Permission denied"), out);
  else
    ReportError(UnknownException("Unknown Error"), out);
}

void DownloadInstance::DownloadManagerPause
  (const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "downloadId", out)
  int downloadId, ret;

  int callbackId = static_cast<int>(args.get("downloadId").get<double>());

  if (!GetDownloadID(callbackId, downloadId)) {
    ReportError(NotFoundException(
      "The identifier does not match any download operation in progress"),
      out);
    return;
  }

  ret = download_pause(downloadId);

  if (ret == DOWNLOAD_ERROR_NONE)
    ReportSuccess(out);
  else if (ret == DOWNLOAD_ERROR_INVALID_PARAMETER)
    ReportError(InvalidValuesException(
    "The input parameter contains an invalid value."), out);
  else if (ret == DOWNLOAD_ERROR_OUT_OF_MEMORY)
    ReportError(UnknownException("Out of memory"), out);
  else if (ret == DOWNLOAD_ERROR_INVALID_STATE)
    ReportError(InvalidValuesException("Invalid state"), out);
  else if (ret == DOWNLOAD_ERROR_IO_ERROR)
    ReportError(UnknownException("Internal I/O error"), out);
  else if (ret == DOWNLOAD_ERROR_PERMISSION_DENIED)
    ReportError(UnknownException("Permission denied"), out);
  else
    ReportError(UnknownException("Unknown Error"), out);
}

void DownloadInstance::DownloadManagerResume
  (const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "downloadId", out)
  int downloadId, ret;

  int callbackId = static_cast<int>(args.get("downloadId").get<double>());

  if (!GetDownloadID(callbackId, downloadId)) {
    ReportError(NotFoundException
      ("The identifier does not match any download operation in progress"),
      out);
    return;
  }

  ret = download_start(downloadId);

  if (ret == DOWNLOAD_ERROR_NONE)
    ReportSuccess(out);
  else if (ret == DOWNLOAD_ERROR_INVALID_PARAMETER)
    ReportError(InvalidValuesException(
    "The input parameter contains an invalid value."), out);
  else if (ret == DOWNLOAD_ERROR_OUT_OF_MEMORY)
    ReportError(UnknownException("Out of memory"), out);
  else if (ret == DOWNLOAD_ERROR_INVALID_STATE)
    ReportError(InvalidValuesException("Invalid state"), out);
  else if (ret == DOWNLOAD_ERROR_IO_ERROR)
    ReportError(UnknownException("Internal I/O error"), out);
  else if (ret == DOWNLOAD_ERROR_INVALID_URL)
    ReportError(InvalidValuesException(
    "The input parameter contains an invalid url."), out);
  else if (ret == DOWNLOAD_ERROR_INVALID_DESTINATION)
    ReportError(InvalidValuesException(
    "The input parameter contains an invalid destination."), out);
  else if (ret == DOWNLOAD_ERROR_ID_NOT_FOUND)
    ReportError(InvalidValuesException("No such a download ID found"), out);
  else if (ret == DOWNLOAD_ERROR_QUEUE_FULL)
    ReportError(UnknownException("Download server queue is full"), out);
  else if (ret == DOWNLOAD_ERROR_PERMISSION_DENIED)
    ReportError(SecurityException(
    "Application does not have the privilege to call this method."), out);
  else
    ReportError(UnknownException("Unknown Error"), out);
}
void DownloadInstance::DownloadManagerGetstate
  (const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "downloadId", out)
  int downloadId, ret;
  std::string stateValue;
  download_state_e state;

  int callbackId = static_cast<int>(args.get("downloadId").get<double>());

  if (!GetDownloadID(callbackId, downloadId)) {
    ReportError(NotFoundException
      ("The identifier does not match any download operation in progress"),
      out);
    return;
  }

  ret = download_get_state(downloadId, &state);

  if (ret == DOWNLOAD_ERROR_NONE) {
    switch (state) {
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
     }

    ReportSuccess(picojson::value(stateValue), out);
  } else if (ret == DOWNLOAD_ERROR_INVALID_PARAMETER) {
      ReportError(InvalidValuesException(
      "The input parameter contains an invalid value."), out);
    } else if (ret == DOWNLOAD_ERROR_OUT_OF_MEMORY) {
      ReportError(UnknownException("Out of memory"), out);
    } else if (ret == DOWNLOAD_ERROR_INVALID_STATE) {
      ReportError(InvalidValuesException("Invalid state"), out);
    } else if (ret == DOWNLOAD_ERROR_IO_ERROR) {
      ReportError(UnknownException("Internal I/O error"), out);
    } else if (ret == DOWNLOAD_ERROR_PERMISSION_DENIED) {
      ReportError(UnknownException("Permission denied"), out);
    } else {
      ReportError(UnknownException("Unknown Error"), out);
    }
}

void DownloadInstance::DownloadManagerGetmimetype
  (const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "downloadId", out)

  int downloadId, ret;
  char* mimetype = NULL;

  int callbackId = static_cast<int>(args.get("downloadId").get<double>());

  if (!GetDownloadID(callbackId, downloadId)) {
    ReportError(NotFoundException
      ("The identifier does not match any download operation in progress"),
      out);
    return;
  }

  ret = download_get_mime_type(downloadId, &mimetype);

  if (ret == DOWNLOAD_ERROR_NONE) {
    ReportSuccess(picojson::value(mimetype), out);
  } else if (ret == DOWNLOAD_ERROR_INVALID_PARAMETER) {
    ReportError(InvalidValuesException(
    "The input parameter contains an invalid value."), out);
    } else if (ret == DOWNLOAD_ERROR_OUT_OF_MEMORY) {
      ReportError(UnknownException("Out of memory"), out);
    } else if (ret == DOWNLOAD_ERROR_INVALID_STATE) {
      ReportError(InvalidValuesException("Invalid state"), out);
    } else if (ret == DOWNLOAD_ERROR_IO_ERROR) {
      ReportError(UnknownException("Internal I/O error"), out);
    } else if (ret == DOWNLOAD_ERROR_PERMISSION_DENIED) {
      ReportError(UnknownException("Permission denied"), out);
    } else {
      ReportError(UnknownException("Unknown Error"), out);
    }
}

bool DownloadInstance::GetDownloadID
  (const int callback_id, int& download_id) {

  if (diMap.find(callback_id) != diMap.end()) {
    download_id = diMap.find(callback_id)->second->download_id;
  } else {
    return false;
  }
  return true;
}

void DownloadInstance::DownloadManagerGetdownloadrequest
  (const picojson::value& args, picojson::object& out) {
  // Nothing to do
}

void DownloadInstance::DownloadManagerSetlistener
  (const picojson::value& args, picojson::object& out) {
  // Nothing to do
}


#undef CHECK_EXIST

}  // namespace download
}  // namespace extension
