// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "keymanager/keymanager_instance.h"

#include <functional>
#include <ckm/ckm-manager.h>
#include <glib.h>
#include <pcrecpp.h>

#include "common/logger.h"
#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace keymanager {


KeyManagerInstance::KeyManagerInstance() {
  using std::placeholders::_1;
  using std::placeholders::_2;

  RegisterSyncHandler("KeyManager_getKeyAliasList",
      std::bind(&KeyManagerInstance::GetKeyAliasList, this, _1, _2));
  RegisterSyncHandler("KeyManager_saveKey",
      std::bind(&KeyManagerInstance::SaveKey, this, _1, _2));
  RegisterSyncHandler("KeyManager_removeKey",
      std::bind(&KeyManagerInstance::RemoveKey, this, _1, _2));
}

KeyManagerInstance::~KeyManagerInstance() {
}

void KeyManagerInstance::GetKeyAliasList(const picojson::value& args,
    picojson::object& out) {
  LoggerD("Enter");
  CKM::AliasVector result;
  int ret = CKM::Manager::create()->getKeyAliasVector(result);
  if (ret != CKM_API_SUCCESS) {
      LoggerE("Failed to fetch list of key alias: %d", ret);
      ReportError(common::PlatformResult(common::ErrorCode::UNKNOWN_ERR,
        "Failed to fetch list of key alias"), &out);
  } else {
      picojson::array aliases;
      for (auto& item: result) {
          aliases.push_back(picojson::value(item));
      }
      picojson::value res(aliases);
      ReportSuccess(res, out);
  }
}

void KeyManagerInstance::SaveKey(const picojson::value& args,
    picojson::object& out) {
  LoggerD("Enter");

  const picojson::value& key_object = args.get("key");
  const std::string& alias = key_object.get("name").get<std::string>();
  std::string password;
  if (key_object.get("password").is<std::string>()) {
    password = key_object.get("password").get<std::string>();
  }
  std::string base64 = args.get("rawKey").get<std::string>();
  pcrecpp::RE_Options opt;
  opt.set_multiline(true);
  //remove first line and last line
  pcrecpp::RE("-----[^-]*-----", opt).GlobalReplace("", &base64);
  gsize len = 0;
  guchar* raw_data = g_base64_decode(base64.c_str(), &len);
  CKM::RawBuffer raw_buffer;
  raw_buffer.assign(raw_data, raw_data + len);
  g_free(raw_data);
  CKM::Password pass(password.c_str());
  CKM::KeyShPtr key = CKM::Key::create(raw_buffer, pass);
  CKM::Policy policy(pass, key_object.get("extractable").get<bool>());
  CKM::ManagerAsync::ObserverPtr observer(new SaveKeyObserver(this,
    args.get("callbackId").get<double>()));
  m_manager.saveKey(observer, alias, key, policy);

  ReportSuccess(out);
}

void KeyManagerInstance::OnSaveKey(double callbackId,
    const common::PlatformResult& result) {
  LoggerD("Enter");
  picojson::value::object dict;
  dict["callbackId"] = picojson::value(callbackId);
  if (result.IsError()) {
    LoggerE("There was an error");
    ReportError(result, &dict);
  }
  picojson::value res(dict);
  PostMessage(res.serialize().c_str());
}

void KeyManagerInstance::RemoveKey(const picojson::value& args,
    picojson::object& out) {
  LoggerD("Enter");

  const std::string& alias = args.get("key").get("name").get<std::string>();
  int ret = CKM::Manager::create()->removeAlias(alias);
  if (ret != CKM_API_SUCCESS) {
    LoggerE("Failed to remove key alias: %d", ret);
    if (ret == CKM_API_ERROR_DB_ALIAS_UNKNOWN) {
      ReportError(common::PlatformResult(common::ErrorCode::NOT_FOUND_ERR,
        "Key alias not found"), &out);
    } else {
      ReportError(common::PlatformResult(common::ErrorCode::UNKNOWN_ERR,
        "Failed to remove key alias"), &out);
    }
  } else {
    ReportSuccess(out);
  }
}

} // namespace keymanager
} // namespace extension
