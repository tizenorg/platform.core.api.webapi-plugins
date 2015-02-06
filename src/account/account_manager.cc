// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "account/account_manager.h"

#include <functional>

#include "common/logger.h"
#include "common/scope_exit.h"

namespace extension {
namespace account {

using common::ScopeExit;
using common::UnknownException;

#define REPORT_ERROR(out, exception) \
  out["status"] = picojson::value("error"); \
  out["error"] = exception.ToJSON();

namespace {

static bool ProviderCapabilitiesCb(char *app_id, char *key, void *user_data) {
  LoggerD("Enter");

  picojson::array* array_data = static_cast<picojson::array*>(user_data);
  if (!array_data) {
    LoggerE("user_data is NULL");
    return false;
  }

  array_data->push_back(picojson::value(key));
  return true;
}

static bool AccountProvidersGetCb(account_type_h provider, void *user_data) {
  LoggerD("Enter");

  picojson::array* array_data = static_cast<picojson::array*>(user_data);
  if (!array_data) {
    LoggerE("user_data is NULL");
    return false;
  }

  picojson::object object_info;
  if (AccountManager::ConvertProviderToObject(provider, object_info)) {
    array_data->push_back(picojson::value(object_info));
  }

  return true;
}

static bool GetAccountsCallback(account_h handle, void *user_data) {
  LoggerD("Enter");

  picojson::array* array_data = static_cast<picojson::array*>(user_data);
  if (!array_data) {
    LoggerE("user_data is NULL");
    return false;
  }

  picojson::object object_info;
  if (AccountManager::ConvertAccountToObject(handle, object_info)) {
    array_data->push_back(picojson::value(object_info));
  }

  return true;
}

} // namespace

AccountManager::AccountManager() {
  if (account_connect() != ACCOUNT_ERROR_NONE) {
    LoggerE("Failed to connect account db");
  }
}

AccountManager::~AccountManager() {
  if (account_disconnect() != ACCOUNT_ERROR_NONE) {
    LoggerE("Failed to disconnect account db");
  }
}

std::string AccountManager::GetErrorMsg(int error) {
  switch (error) {
    case ACCOUNT_ERROR_OUT_OF_MEMORY:
      return "Out of Memory";
    case ACCOUNT_ERROR_INVALID_PARAMETER:
      return "Invalid Parameter";
    case ACCOUNT_ERROR_DUPLICATED:
      return "Same user name exists in your application";
    case ACCOUNT_ERROR_NO_DATA:
      return "Empty data";
    case ACCOUNT_ERROR_RECORD_NOT_FOUND:
      return "Related record does not exist";
    case ACCOUNT_ERROR_DB_FAILED:
      return "DB operation failed";
    case ACCOUNT_ERROR_DB_NOT_OPENED:
      return "DB is not connected";
    case ACCOUNT_ERROR_QUERY_SYNTAX_ERROR:
      return "DB query syntax error";
    case ACCOUNT_ERROR_ITERATOR_END:
      return "Iterator has reached the end";
    case ACCOUNT_ERROR_NOTI_FAILED:
      return "Notification failed";
    case ACCOUNT_ERROR_PERMISSION_DENIED:
      return "Permission denied";
    case ACCOUNT_ERROR_XML_PARSE_FAILED:
      return "XML parse failed";
    case ACCOUNT_ERROR_XML_FILE_NOT_FOUND:
      return "XML file does not exist";
    case ACCOUNT_ERROR_EVENT_SUBSCRIPTION_FAIL:
      return "Subscription failed";
    case ACCOUNT_ERROR_NOT_REGISTERED_PROVIDER:
      return "Account provider is not registered";
    case ACCOUNT_ERROR_NOT_ALLOW_MULTIPLE:
      return "Multiple accounts are not supported";
    case ACCOUNT_ERROR_DATABASE_BUSY:
      return "SQLite busy handler expired";
    default:
      return "Unknown Error";
  }
}

void AccountManager::GetAccountsInfo(const std::string& application_id,
                                     picojson::object& out) {
  LoggerD("Enter");

  picojson::array array_data;
  int ret = ACCOUNT_ERROR_NONE;

  if ("" == application_id) {
    ret = account_foreach_account_from_db(GetAccountsCallback, &array_data);
  } else {
    ret = account_query_account_by_package_name(GetAccountsCallback,
                                                application_id.c_str(),
                                                &array_data);
  }

  if (ret == ACCOUNT_ERROR_NONE || ret == ACCOUNT_ERROR_RECORD_NOT_FOUND) {
    out["status"] = picojson::value("success");
    out["result"] = picojson::value(array_data);
  } else {
    LoggerE("Failed to get accounts information");
    REPORT_ERROR(out, UnknownException(GetErrorMsg(ret)));
  }
}

bool AccountManager::GetAccountInfo(int account_id, picojson::object& out) {
  LoggerD("Enter");

  account_h account = NULL;
  SCOPE_EXIT {
    account_destroy(account);
  };

  int ret = account_create(&account);
  if (ret != ACCOUNT_ERROR_NONE) {
    LoggerE("Failed to create account info");
    REPORT_ERROR(out, UnknownException(GetErrorMsg(ret)));
    return false;
  }

  ret = account_query_account_by_account_id(account_id, &account);
  if (ret != ACCOUNT_ERROR_NONE) {
    LoggerE("Failed to get account info");
    REPORT_ERROR(out, UnknownException(GetErrorMsg(ret)));
    return false;
  }

  picojson::object info;
  if (!ConvertAccountToObject(account, info)) {
    LoggerE("Failed to convert account_h into object");
    REPORT_ERROR(out, UnknownException("Unknown error occurs"));
    return false;
  }

  out["status"] = picojson::value("success");
  out["result"] = picojson::value(info);

  return true;
}

bool AccountManager::GetProviderInfo(const std::string& provider_id,
                                     picojson::object& out) {
  LoggerD("Enter");

  account_type_h provider = NULL;
  SCOPE_EXIT {
    account_type_destroy(provider);
  };

  int ret = account_type_create(&provider);
  if (ret != ACCOUNT_ERROR_NONE) {
    LoggerE("Failed to create provider info");
    REPORT_ERROR(out, UnknownException(GetErrorMsg(ret)));
    return false;
  }

  ret = account_type_query_by_app_id(provider_id.c_str(), &provider);
  if (ret != ACCOUNT_ERROR_NONE) {
    LoggerE("Failed to get provider info");
    REPORT_ERROR(out, UnknownException(GetErrorMsg(ret)));
    return false;
  }

  picojson::object info;
  if (!ConvertProviderToObject(provider, info)) {
    LoggerE("Failed to convert account_type_h into object");
    REPORT_ERROR(out, UnknownException("Unknown error occurs"));
    return false;
  }

  out["status"] = picojson::value("success");
  out["result"] = picojson::value(info);

  return true;
}

bool AccountManager::ConvertAccountToObject(account_h account,
                                            picojson::object& out) {
  LoggerD("Enter");

  char* provider_id = NULL;
  char* icon_path = NULL;
  char* user_name = NULL;

  SCOPE_EXIT {
    free(provider_id);
    free(icon_path);
    free(user_name);
  };

  int account_id = -1;
  int ret = account_get_account_id(account, &account_id);
  if (ret != ACCOUNT_ERROR_NONE) {
    LoggerE("Failed to get account ID");
    REPORT_ERROR(out, UnknownException(GetErrorMsg(ret)));
    return false;
  }
  out["id"] = picojson::value(static_cast<double>(account_id));

  ret = account_get_package_name(account, &provider_id);
  if (ret != ACCOUNT_ERROR_NONE) {
    LoggerE("Failed to get provider name");
    REPORT_ERROR(out, UnknownException(GetErrorMsg(ret)));
    return false;
  }

  picojson::object provider_info;
  if (!GetProviderInfo(provider_id, provider_info)) {
    LoggerE("Failed to get provider info");
    return false;
  }
  out["provider"] = provider_info["result"];

  picojson::object account_init;
  ret = account_get_icon_path(account, &icon_path);
  if (ret != ACCOUNT_ERROR_NONE) {
    LoggerE("Failed to get icon path");
    REPORT_ERROR(out, UnknownException(GetErrorMsg(ret)));
    return false;
  }
  account_init["iconUri"] = picojson::value(icon_path);

  ret = account_get_user_name(account, &user_name);
  if (ret != ACCOUNT_ERROR_NONE) {
    LoggerE("Failed to get user name");
    REPORT_ERROR(out, UnknownException(GetErrorMsg(ret)));
    return false;
  }
  account_init["userName"] = picojson::value(user_name);
  out["accountInitDict"] = picojson::value(account_init);

  return true;
}

bool AccountManager::ConvertProviderToObject(account_type_h provider,
                                             picojson::object& out) {
  LoggerD("Enter");

  char* provider_id = NULL;
  char* display_name = NULL;
  char* icon_uri = NULL;
  char* small_icon_uri = NULL;
  bool is_multiple_account_supported = false;
  picojson::array capabilities;

  int ret = account_type_get_app_id(provider, &provider_id);
  if (ret != ACCOUNT_ERROR_NONE) {
    LoggerE("Failed to get application id");
    REPORT_ERROR(out, UnknownException(GetErrorMsg(ret)));
    return false;
  }
  out["applicationId"] = picojson::value(provider_id);
  free(provider_id);

  // TODO: Which label should be returned?
  ret = account_type_get_label_by_locale(provider, "default", &display_name);
  if (ret == ACCOUNT_ERROR_NONE) {
    out["displayName"] = picojson::value(display_name);
    free(display_name);
  } else if (ret == ACCOUNT_ERROR_RECORD_NOT_FOUND) {
    LoggerD("There is no label");
    out["displayName"] = picojson::value("");
  } else {
    LoggerE("Failed to get label");
    REPORT_ERROR(out, UnknownException(GetErrorMsg(ret)));
    return false;
  }

  ret = account_type_get_icon_path(provider, &icon_uri);
  if (ret != ACCOUNT_ERROR_NONE) {
    LoggerE("Failed to get icon");
    REPORT_ERROR(out, UnknownException(GetErrorMsg(ret)));
    return false;
  }
  out["iconUri"] = picojson::value(icon_uri);
  free(icon_uri);

  ret = account_type_get_small_icon_path(provider, &small_icon_uri);
  if (ret != ACCOUNT_ERROR_NONE) {
    LoggerE("Failed to get small icon");
    REPORT_ERROR(out, UnknownException(GetErrorMsg(ret)));
    return false;
  }
  out["smallIconUri"] = picojson::value(small_icon_uri);
  free(small_icon_uri);

  ret = account_type_get_provider_feature_all(provider, ProviderCapabilitiesCb,
                                              &capabilities);
  if (ret != ACCOUNT_ERROR_NONE) {
    LoggerE("Failed to get capabilities");
    REPORT_ERROR(out, UnknownException(GetErrorMsg(ret)));
    return false;
  }
  out["capabilities"] = picojson::value(capabilities);

  int supported = 0;
  ret = account_type_get_multiple_account_support(provider, &supported);
  if (ret != ACCOUNT_ERROR_NONE) {
    LoggerE("Failed to get small icon");
    REPORT_ERROR(out, UnknownException(GetErrorMsg(ret)));
    return false;
  }
  is_multiple_account_supported = (supported != 0);
  out["isMultipleAccountSupported"] = picojson::value(
      is_multiple_account_supported);

  return true;
}

void AccountManager::GetProvidersInfo(const std::string& capability,
                                      picojson::object& out) {
  LoggerD("Enter");

  picojson::array array_data;
  int ret = ACCOUNT_ERROR_NONE;

  if ("" == capability) {
    ret = account_type_foreach_account_type_from_db(AccountProvidersGetCb,
                                                    &array_data);
  } else {
    ret = account_type_query_by_provider_feature(AccountProvidersGetCb,
                                                 capability.c_str(),
                                                 &array_data);
  }

  if (ret == ACCOUNT_ERROR_NONE || ret == ACCOUNT_ERROR_RECORD_NOT_FOUND) {
    out["status"] = picojson::value("success");
    out["result"] = picojson::value(array_data);
  } else {
    LoggerE("Failed to get providers information");
    REPORT_ERROR(out, UnknownException(GetErrorMsg(ret)));
  }
}

#undef REPORT_ERROR

} // namespace account
} // namespace extension
