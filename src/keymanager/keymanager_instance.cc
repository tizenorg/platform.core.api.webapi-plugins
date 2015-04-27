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

namespace {
const char* kTypeRSA = "RSA";
const char* kTypeECDSA = "ECDSA";
}

KeyManagerInstance::KeyManagerInstance() {
  using std::placeholders::_1;
  using std::placeholders::_2;

  RegisterSyncHandler("KeyManager_getKeyAliasList",
      std::bind(&KeyManagerInstance::GetKeyAliasList, this, _1, _2));
  RegisterSyncHandler("KeyManager_getKey",
      std::bind(&KeyManagerInstance::GetKey, this, _1, _2));
  RegisterSyncHandler("KeyManager_saveKey",
      std::bind(&KeyManagerInstance::SaveKey, this, _1, _2));
  RegisterSyncHandler("KeyManager_removeKey",
      std::bind(&KeyManagerInstance::RemoveKey, this, _1, _2));
  RegisterSyncHandler("KeyManager_generateKeyPair",
      std::bind(&KeyManagerInstance::GenerateKeyPair, this, _1, _2));
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

void KeyManagerInstance::GenerateKeyPair(const picojson::value& args,
    picojson::object& out) {
  LoggerD("Enter");

  const picojson::value& priv_key = args.get("privKeyName");
  const picojson::value& pub_key = args.get("pubKeyName");
  const std::string& priv_name = priv_key.get("name").get<std::string>();
  const std::string& pub_name = pub_key.get("name").get<std::string>();
  const std::string& type = args.get("type").get<std::string>();
  int size = std::stoi(args.get("size").get<std::string>());

  CKM::ManagerAsync::ObserverPtr observer(new CreateKeyObserver(this,
    args.get("callbackId").get<double>()));

  CKM::Password pass;
  if (priv_key.get("password").is<std::string>()) {
    pass = priv_key.get("password").get<std::string>().c_str();
  }
  CKM::Policy priv_policy(pass, priv_key.get("extractable").get<bool>());

  if (pub_key.get("password").is<std::string>()) {
    pass = pub_key.get("password").get<std::string>().c_str();
  } else {
    pass = "";
  }
  CKM::Policy pub_policy(pass, pub_key.get("extractable").get<bool>());

  if (type == kTypeRSA) {
    m_manager.createKeyPairRSA(observer, size, priv_name, pub_name, priv_policy, pub_policy);
  } else if (type == kTypeECDSA) {
    CKM::ElipticCurve eliptic = CKM::ElipticCurve::prime192v1;
    if (args.get("ellipticCurveType").is<std::string>()) {
      const std::string& eType = args.get("ellipticCurveType").get<std::string>();
      if (eType == "PRIME256V1") {
        eliptic = CKM::ElipticCurve::prime256v1;
      } else if (eType == "EC_SECP384R1") {
        eliptic = CKM::ElipticCurve::secp384r1;
      }
    }
    m_manager.createKeyPairECDSA(observer, eliptic, priv_name, pub_name, priv_policy, pub_policy);
  } else {
    m_manager.createKeyPairDSA(observer, size, priv_name, pub_name, priv_policy, pub_policy);
  }

  ReportSuccess(out);
}

void KeyManagerInstance::OnCreateKeyPair(double callbackId,
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

void KeyManagerInstance::GetKey(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  using CKM::KeyType;

  const std::string& alias = args.get("name").get<std::string>();
  CKM::Password pass;
  if (args.get("password").is<std::string>()) {
    pass = args.get("password").get<std::string>().c_str();
  }

  CKM::KeyShPtr key;
  int ret = CKM::Manager::create()->getKey(alias, pass, key);
  if (ret != CKM_API_SUCCESS) {
    LoggerE("Failed to get key: %d", ret);
    if (ret == CKM_API_ERROR_DB_ALIAS_UNKNOWN) {
      ReportError(common::PlatformResult(common::ErrorCode::NOT_FOUND_ERR,
        "Key alias not found"), &out);
    } else {
      ReportError(common::PlatformResult(common::ErrorCode::UNKNOWN_ERR,
        "Failed to get key"), &out);
    }
  } else {
    picojson::object dict;
    dict["name"] = args.get("name");
    if (args.get("password").is<std::string>()) {
      dict["password"] = args.get("password");
    }
    switch (key->getType()) {
      case KeyType::KEY_NONE:
        dict["keyType"] = picojson::value("KEY_NONE");
        break;
      case KeyType::KEY_RSA_PUBLIC:
        dict["keyType"] = picojson::value("KEY_RSA_PUBLIC");
        break;
      case KeyType::KEY_RSA_PRIVATE:
        dict["keyType"] = picojson::value("KEY_RSA_PRIVATE");
        break;
      case KeyType::KEY_ECDSA_PUBLIC:
        dict["keyType"] = picojson::value("KEY_ECDSA_PUBLIC");
        break;
      case KeyType::KEY_ECDSA_PRIVATE:
        dict["keyType"] = picojson::value("KEY_ECDSA_PRIVATE");
        break;
      case KeyType::KEY_DSA_PUBLIC:
        dict["keyType"] = picojson::value("KEY_DSA_PUBLIC");
        break;
      case KeyType::KEY_DSA_PRIVATE:
        dict["keyType"] = picojson::value("KEY_DSA_PRIVATE");
        break;
      case KeyType::KEY_AES:
        dict["keyType"] = picojson::value("KEY_AES");
        break;
    }
    CKM::RawBuffer buf = key->getDER();
    if (!buf.empty()) {
      gchar* base64 = g_base64_encode(&buf[0], buf.size());
      dict["rawKey"] = picojson::value(std::string(base64));
      g_free(base64);
    } else {
      dict["rawKey"] = picojson::value(std::string());
    }
    //if key was retrieved it is extractable from db
    dict["extractable"] = picojson::value(true);

    picojson::value res(dict);
    ReportSuccess(res, out);
  }
}

} // namespace keymanager
} // namespace extension
