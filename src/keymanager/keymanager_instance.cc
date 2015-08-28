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

#include "keymanager/keymanager_instance.h"

#include <ckmc/ckmc-manager.h>
#include <glib.h>
#include <pkgmgr-info.h>

#include "common/logger.h"
#include "common/optional.h"
#include "common/platform_result.h"
#include "common/scope_exit.h"
#include "common/task-queue.h"
#include "common/tools.h"
#include "common/virtual_fs.h"

namespace extension {
namespace keymanager {

using common::ErrorCode;
using common::optional;
using common::PlatformResult;
using common::TaskQueue;
using common::VirtualFs;

namespace {

typedef std::vector<unsigned char> RawBuffer;

const char* kTypeRSA = "RSA";
const char* kTypeECDSA = "ECDSA";

RawBuffer Base64ToRawBuffer(const std::string& base64) {
  LoggerD("Enter");

  gsize len = 0;
  guchar* raw_data = g_base64_decode(base64.c_str(), &len);
  RawBuffer raw_buffer;

  if (raw_data) {
    raw_buffer.assign(raw_data, raw_data + len);
    g_free(raw_data);
  }

  return raw_buffer;
}

std::string RawBufferToBase64(const RawBuffer& buf) {
  LoggerD("Enter");

  std::string result;

  if (!buf.empty()) {
    gchar* base64 = g_base64_encode(&buf[0], buf.size());
    result = base64;
    g_free(base64);
  }

  return result;
}

ckmc_ec_type_e GetEllipticCurveType(const std::string& type) {
  LoggerD("Enter");

  if ("EC_PRIME256V1" == type) {
    return CKMC_EC_PRIME256V1;
  } else if ("EC_SECP384R1" == type) {
    return CKMC_EC_SECP384R1;
  } else {
    return CKMC_EC_PRIME192V1;
  }
}

ckmc_key_type_e StringToKeyType(const std::string& type) {
  LoggerD("Enter");

  if ("KEY_RSA_PUBLIC" == type) {
    return CKMC_KEY_RSA_PUBLIC;
  } else if ("KEY_RSA_PRIVATE" == type) {
    return CKMC_KEY_RSA_PRIVATE;
  } else if ("KEY_ECDSA_PUBLIC" == type) {
    return CKMC_KEY_ECDSA_PUBLIC;
  } else if ("KEY_ECDSA_PRIVATE" == type) {
    return CKMC_KEY_ECDSA_PRIVATE;
  } else if ("KEY_DSA_PUBLIC" == type) {
    return CKMC_KEY_DSA_PUBLIC;
  } else if ("KEY_DSA_PRIVATE" == type) {
    return CKMC_KEY_DSA_PRIVATE;
  } else if ("KEY_AES" == type) {
    return CKMC_KEY_AES;
  } else {
    return CKMC_KEY_NONE;
  }
}

std::string KeyTypeToString(ckmc_key_type_e type) {
  LoggerD("Enter");

  switch (type) {
    case CKMC_KEY_NONE:
      return "KEY_NONE";

    case CKMC_KEY_RSA_PUBLIC:
      return "KEY_RSA_PUBLIC";

    case CKMC_KEY_RSA_PRIVATE:
      return "KEY_RSA_PRIVATE";

    case CKMC_KEY_ECDSA_PUBLIC:
      return "KEY_ECDSA_PUBLIC";

    case CKMC_KEY_ECDSA_PRIVATE:
      return "KEY_ECDSA_PRIVATE";

    case CKMC_KEY_DSA_PUBLIC:
      return "KEY_DSA_PUBLIC";

    case CKMC_KEY_DSA_PRIVATE:
      return "KEY_DSA_PRIVATE";

    case CKMC_KEY_AES:
      return "KEY_AES";
  }

  LoggerE("Unknown key type");
  return "KEY_UNKNOWN";
}

ckmc_hash_algo_e StringToHashAlgorithm(const std::string& str) {
  if ("HASH_SHA1" == str) {
    return CKMC_HASH_SHA1;
  } else if ("HASH_SHA256" == str) {
    return CKMC_HASH_SHA256;
  } else if ("HASH_SHA384" == str) {
    return CKMC_HASH_SHA384;
  } else if ("HASH_SHA512" == str) {
    return CKMC_HASH_SHA512;
  }

  return CKMC_HASH_NONE;
}

ckmc_rsa_padding_algo_e StringToRsaPadding(const std::string& str) {
  if ("PADDING_PKCS1" == str) {
    return CKMC_PKCS1_PADDING;
  } else if ("PADDING_X931" == str) {
    return CKMC_X931_PADDING;
  }

  return CKMC_NONE_PADDING;
}

RawBuffer ToRawBuffer(const ckmc_key_s* key) {
  return RawBuffer(key->raw_key, key->raw_key + key->key_size);
}//

RawBuffer ToRawBuffer(const ckmc_raw_buffer_s* buffer) {
  return RawBuffer(buffer->data, buffer->data + buffer->size);
}

RawBuffer ToRawBuffer(const ckmc_cert_s* cert) {
  return RawBuffer(cert->raw_cert, cert->raw_cert + cert->cert_size);
}

typedef int (*AliasListFunction)(ckmc_alias_list_s**);

void GetGenericAliasList(AliasListFunction func, picojson::object* out) {
  LoggerD("Enter");

  ckmc_alias_list_s* alias_list = nullptr;
  int ret = func(&alias_list);

  picojson::value result{picojson::array{}};

  if (CKMC_ERROR_NONE == ret) {
    auto& aliases = result.get<picojson::array>();
    ckmc_alias_list_s* head = alias_list;

    while (head) {
      aliases.push_back(picojson::value(head->alias ? head->alias : ""));
      head = head->next;
    }

    if (alias_list) {
      ckmc_alias_list_all_free(alias_list);
    }
  }

  common::tools::ReportSuccess(result, *out);
}
}  // namespace

KeyManagerInstance::KeyManagerInstance() {
  LoggerD("Enter");
  using std::placeholders::_1;
  using std::placeholders::_2;

  RegisterSyncHandler("KeyManager_getKeyAliasList",
      std::bind(&KeyManagerInstance::GetKeyAliasList, this, _1, _2));
  RegisterSyncHandler("KeyManager_getCertificatesAliasList",
      std::bind(&KeyManagerInstance::GetCertificateAliasList, this, _1, _2));
  RegisterSyncHandler("KeyManager_getDataAliasList",
      std::bind(&KeyManagerInstance::GetDataAliasList, this, _1, _2));
  RegisterSyncHandler("KeyManager_getKey",
      std::bind(&KeyManagerInstance::GetKey, this, _1, _2));
  RegisterSyncHandler("KeyManager_saveKey",
      std::bind(&KeyManagerInstance::SaveKey, this, _1, _2));
  RegisterSyncHandler("KeyManager_removeAlias",
      std::bind(&KeyManagerInstance::RemoveAlias, this, _1, _2));
  RegisterSyncHandler("KeyManager_generateKeyPair",
      std::bind(&KeyManagerInstance::GenerateKeyPair, this, _1, _2));
  RegisterSyncHandler("KeyManager_getCertificate",
      std::bind(&KeyManagerInstance::GetCertificate, this, _1, _2));
  RegisterSyncHandler("KeyManager_saveCertificate",
      std::bind(&KeyManagerInstance::SaveCertificate, this, _1, _2));
  RegisterSyncHandler("KeyManager_loadCertificateFromFile",
      std::bind(&KeyManagerInstance::LoadCertificateFromFile, this, _1, _2));
  RegisterSyncHandler("KeyManager_saveData",
      std::bind(&KeyManagerInstance::SaveData, this, _1, _2));
  RegisterSyncHandler("KeyManager_getData",
      std::bind(&KeyManagerInstance::GetData, this, _1, _2));
  RegisterSyncHandler("KeyManager_createSignature",
      std::bind(&KeyManagerInstance::CreateSignature, this, _1, _2));
  RegisterSyncHandler("KeyManager_verifySignature",
      std::bind(&KeyManagerInstance::VerifySignature, this, _1, _2));
  RegisterSyncHandler("KeyManager_loadFromPKCS12File",
      std::bind(&KeyManagerInstance::LoadFromPKCS12File, this, _1, _2));
  RegisterSyncHandler("KeyManager_allowAccessControl",
      std::bind(&KeyManagerInstance::AllowAccessControl, this, _1, _2));
  RegisterSyncHandler("KeyManager_denyAccessControl",
      std::bind(&KeyManagerInstance::DenyAccessControl, this, _1, _2));
  RegisterSyncHandler("KeyManager_isDataNameFound",
      std::bind(&KeyManagerInstance::IsDataNameFound, this, _1, _2));
}

KeyManagerInstance::~KeyManagerInstance() {
  LoggerD("Enter");
}

void KeyManagerInstance::GetKeyAliasList(const picojson::value& args,
                                         picojson::object& out) {
  LoggerD("Enter");
  GetGenericAliasList(ckmc_get_key_alias_list, &out);
}

void KeyManagerInstance::GetCertificateAliasList(const picojson::value& args,
                                                 picojson::object& out) {
  LoggerD("Enter");
  GetGenericAliasList(ckmc_get_cert_alias_list, &out);
}

void KeyManagerInstance::GetDataAliasList(const picojson::value& args,
                                          picojson::object& out) {
  LoggerD("Enter");
  GetGenericAliasList(ckmc_get_data_alias_list, &out);
}

void KeyManagerInstance::GetKey(const picojson::value& args,
                                picojson::object& out) {
  LoggerD("Enter");
  const auto& key_alias = args.get("name").get<std::string>();
  const auto& password_value = args.get("password");

  std::string password;

  if (password_value.is<std::string>()) {
    password = password_value.get<std::string>();
  }

  ckmc_key_s* key = nullptr;
  int ret = ckmc_get_key(key_alias.c_str(), password.c_str(), &key);

  if (CKMC_ERROR_NONE == ret) {
    picojson::object result;

    result["name"] = picojson::value(key_alias);
    if (key->password) {
      result["password"] = picojson::value(key->password);
    }

    // if key was retrieved it is extractable from DB
    result["extractable"] = picojson::value(true);
    result["keyType"] = picojson::value(KeyTypeToString(key->key_type));
    result["rawKey"] = picojson::value(RawBufferToBase64(ToRawBuffer(key)));

    ckmc_key_free(key);
    ReportSuccess(picojson::value{result}, out);
  } else {
    LoggerE("Failed to get key: %d", ret);

    PlatformResult error(ErrorCode::UNKNOWN_ERR, "Failed to get key");

    switch (ret) {
      case CKMC_ERROR_DB_ALIAS_UNKNOWN:
        error = PlatformResult(ErrorCode::NOT_FOUND_ERR, "Failed to find key");
        break;

      case CKMC_ERROR_INVALID_PARAMETER:
        error = PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Input parameter is invalid");
        break;
    }

    ReportError(error, &out);
  }
}

void KeyManagerInstance::SaveKey(const picojson::value& args,
                                 picojson::object& out) {
  LoggerD("Enter");

  const picojson::value& key_obj = args.get("key");
  const std::string& alias = key_obj.get("name").get<std::string>();
  const std::string& type = key_obj.get("keyType").get<std::string>();
  bool extractable = key_obj.get("extractable").get<bool>();
  const double callback_id = args.get("callbackId").get<double>();

  std::string base64;
  if (args.get("rawKey").is<std::string>()) {
    base64 = args.get("rawKey").get<std::string>();
  }

  std::string pass;
  if (key_obj.get("password").is<std::string>()) {
    pass = key_obj.get("password").get<std::string>();
  }

  RawBuffer* raw_buffer = new RawBuffer(std::move(Base64ToRawBuffer(base64)));
  ckmc_key_type_e key_type = StringToKeyType(type);

  auto save = [alias, pass, key_type, extractable, raw_buffer]
               (const std::shared_ptr<picojson::value>& response) -> void {

    LoggerD("Enter save_key");
    ckmc_policy_s policy { const_cast<char*>(pass.c_str()), extractable };
    ckmc_key_s key { const_cast<unsigned char*>(&(*raw_buffer)[0]),
      raw_buffer->size(), key_type, const_cast<char*>(pass.c_str()) };

    int ret = ckmc_save_key(alias.c_str(), key, policy);
    if (CKMC_ERROR_NONE != ret) {
      LoggerE("Failed to save key alias [%d]", ret);
      PlatformResult result = PlatformResult(ErrorCode::NO_ERROR);
      if (CKMC_ERROR_INVALID_PARAMETER == ret) {
        result = PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid parameter passed.");
      } else {
        result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to save key.");
      }
      common::tools::ReportError(result, &response->get<picojson::object>());
    } else {
      //as key_type is determined inside key manager during storing keys
      //we have to get saved key and check key_type again.
      ckmc_key_s * saved_key = nullptr;
      ret = ckmc_get_key(alias.c_str(), pass.c_str(), &saved_key);
      if (CKMC_ERROR_NONE == ret) {
        picojson::object& obj = response->get<picojson::object>();
        obj["keyType"] = picojson::value(KeyTypeToString(saved_key->key_type));
        ckmc_key_free(saved_key);
      }

      common::tools::ReportSuccess(response->get<picojson::object>());
    }

    delete raw_buffer;
  };

  auto save_response = [this, callback_id](const std::shared_ptr<picojson::value>& response) -> void {
    LoggerD("Enter save_key_result");
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair("callbackId", picojson::value(callback_id)));
    Instance::PostMessage(this, response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<picojson::value>(
      save,
      save_response,
      std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));
}

void KeyManagerInstance::RemoveAlias(const picojson::value& args,
                                   picojson::object& out) {
  LoggerD("Enter");

  const std::string& alias = args.get("alias").get<std::string>();
  int ret = ckmc_remove_alias(alias.c_str());

  if (CKMC_ERROR_NONE != ret) {
    LoggerE("Failed to remove alias [%d]", ret);
    PlatformResult result = PlatformResult(ErrorCode::NO_ERROR);
    switch(ret) {
      case CKMC_ERROR_INVALID_PARAMETER:
        result = PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid parameter passed");
        break;
      case CKMC_ERROR_DB_ALIAS_UNKNOWN:
        result = PlatformResult(ErrorCode::NOT_FOUND_ERR, "Alias not found");
        break;
      default:
        result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to remove alias");
    }
    ReportError(result, &out);
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
  const int size = std::stoi(args.get("size").get<std::string>());
  const double callback_id = args.get("callbackId").get<double>();

  std::string priv_pass;
  if (priv_key.get("password").is<std::string>()) {
    priv_pass = priv_key.get("password").get<std::string>();
  }
  bool priv_extractable = priv_key.get("extractable").get<bool>();

  std::string pub_pass;
  if (pub_key.get("password").is<std::string>()) {
    pub_pass = pub_key.get("password").get<std::string>();
  }
  bool pub_extractable = pub_key.get("extractable").get<bool>();

  std::string elliptic;
  if (args.get("ellipticCurveType").is<std::string>()) {
    elliptic = args.get("ellipticCurveType").get<std::string>();
  }

  auto generate =
      [size, priv_pass, pub_pass, priv_extractable, pub_extractable, priv_name, pub_name, type, elliptic]
      (const std::shared_ptr<picojson::value>& response) -> void {
    LoggerD("Enter generate");
    int ret = CKMC_ERROR_NONE;

    ckmc_policy_s priv_policy { const_cast<char*>(priv_pass.c_str()), priv_extractable };
    ckmc_policy_s pub_policy { const_cast<char*>(pub_pass.c_str()), pub_extractable };

    if (kTypeRSA == type) {
      LoggerD("Generating RSA, size: %d", size);
      ret = ckmc_create_key_pair_rsa(size, priv_name.c_str(),
                                     pub_name.c_str(), priv_policy, pub_policy);
      LoggerD("Generating RSA - done");
    } else if (kTypeECDSA == type) {
      LoggerD("Generating ECDSA, curve: %s", elliptic.c_str());
      ret = ckmc_create_key_pair_ecdsa(GetEllipticCurveType(elliptic), priv_name.c_str(),
                                       pub_name.c_str(), priv_policy, pub_policy);
      LoggerD("Generating ECDSA - done");
    } else {
      LoggerD("Generating DSA, size: %d", size);
      ret = ckmc_create_key_pair_dsa(size, priv_name.c_str(),
                                     pub_name.c_str(), priv_policy, pub_policy);
      LoggerD("Generating DSA - done");
    }

    if (CKMC_ERROR_NONE != ret) {
      LoggerD("Failed to generate key pair: %d", ret);
      PlatformResult result = PlatformResult(ErrorCode::NO_ERROR);
      if (CKMC_ERROR_INVALID_PARAMETER == ret) {
        result = PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid value passed.");
      } else {
        result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to create key pair.");
      }
      common::tools::ReportError(result, &response->get<picojson::object>());
    } else {
      common::tools::ReportSuccess(response->get<picojson::object>());
    }
  };

  auto generate_response = [this, callback_id](const std::shared_ptr<picojson::value>& response) -> void {
    LoggerD("Enter generate_response");
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair("callbackId", picojson::value(callback_id)));
    Instance::PostMessage(this, response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<picojson::value>(
      generate,
      generate_response,
      std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));

  ReportSuccess(out);
}

void KeyManagerInstance::GetCertificate(const picojson::value& args,
                                        picojson::object& out) {
  LoggerD("Enter");

  const std::string& alias = args.get("name").get<std::string>();

  std::string pass;
  if (args.get("password").is<std::string>()) {
    pass = args.get("password").get<std::string>();
  }

  ckmc_cert_s* cert = nullptr;
  int ret = ckmc_get_cert(alias.c_str(), pass.c_str(), &cert);

  if (CKMC_ERROR_NONE != ret) {
    LoggerE("Failed to get certificate: %d", ret);
    PlatformResult result = PlatformResult(ErrorCode::NO_ERROR);
    switch (ret) {
      case CKMC_ERROR_DB_ALIAS_UNKNOWN:
        result = PlatformResult(ErrorCode::NOT_FOUND_ERR, "Certificate alias not found");
        break;
      case CKMC_ERROR_INVALID_PARAMETER:
        result = PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid parameter passed");
        break;
      default:
        result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to get certificate");
    }

    ReportError(result, &out);
  } else {
    picojson::value result = picojson::value(picojson::object());
    picojson::object& obj = result.get<picojson::object>();

    //if cert was retrieved it is extractable from db
    obj["extractable"] = picojson::value(true);
    obj["name"] = picojson::value(alias);
    if (!pass.empty()) {
      obj["password"] = picojson::value(pass);
    }

    RawBuffer raw_cert (cert->raw_cert, cert->raw_cert + cert->cert_size);
    obj["rawCert"] = picojson::value(RawBufferToBase64(raw_cert));

    ReportSuccess(result, out);
  }
}

void KeyManagerInstance::SaveCertificate(const picojson::value& args,
                                         picojson::object& out) {
  LoggerD("Enter");

  RawBuffer* raw_buffer = new RawBuffer(std::move(Base64ToRawBuffer(args.get("rawCert").get<std::string>())));
  const auto& certificate = args.get("certificate");
  const auto& alias = certificate.get("name").get<std::string>();
  const auto& password_value = certificate.get("password");
  const auto extractable = certificate.get("extractable").get<bool>();
  double callback_id = args.get("callbackId").get<double>();

  std::string password;

  if (password_value.is<std::string>()) {
    password = password_value.get<std::string>();
  }

  auto save_certificate = [raw_buffer, password, extractable, alias](const std::shared_ptr<picojson::value>& result) {
    LoggerD("Enter save_certificate");

    ckmc_cert_s certificate { const_cast<unsigned char*>(&(*raw_buffer)[0]), raw_buffer->size(), CKMC_FORM_DER };
    ckmc_policy_s policy { const_cast<char*>(password.c_str()), extractable };

    int ret = ckmc_save_cert(alias.c_str(), certificate, policy);

    PlatformResult success(ErrorCode::NO_ERROR);

    switch (ret) {
      case CKMC_ERROR_NONE:
        break;

      case CKMC_ERROR_DB_ALIAS_UNKNOWN:
        success = PlatformResult(ErrorCode::NOT_FOUND_ERR, "Alias not found");
        break;

      case CKMC_ERROR_INVALID_PARAMETER:
        success = PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Failed to save certificate");
        break;

      default:
        success = PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to save certificate");
        break;
    }

    if (success) {
      common::tools::ReportSuccess(result->get<picojson::object>());
    } else {
      LoggerE("Failed to save certificate: %d", ret);
      common::tools::ReportError(success, &result->get<picojson::object>());
    }

    delete raw_buffer;
  };

  auto save_certificate_result = [this, callback_id](const std::shared_ptr<picojson::value>& result) {
    LoggerD("Enter save_certificate_result");
    result->get<picojson::object>()["callbackId"] = picojson::value{callback_id};
    Instance::PostMessage(this, result->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<picojson::value>(
      save_certificate,
      save_certificate_result,
      std::shared_ptr<picojson::value>{new picojson::value{picojson::object()}});

  ReportSuccess(out);
}

void KeyManagerInstance::LoadCertificateFromFile(const picojson::value& args,
                                                 picojson::object& out) {
  LoggerD("Enter");

  const auto& file_uri = args.get("fileURI").get<std::string>();
  const auto& certificate = args.get("certificate");
  const auto& alias = certificate.get("name").get<std::string>();
  const auto& password_value = args.get("password");
  const auto extractable = certificate.get("extractable").get<bool>();
  double callback_id = args.get("callbackId").get<double>();

  std::string password;

  if (password_value.is<std::string>()) {
    password = password_value.get<std::string>();
  }

  auto load_certificate = [file_uri, password, extractable, alias](const std::shared_ptr<picojson::value>& result) {
    LoggerD("Enter load_certificate");

    std::string file = VirtualFs::GetInstance().GetRealPath(file_uri);
    ckmc_cert_s* certificate = nullptr;
    int ret = ckmc_load_cert_from_file(file.c_str(), &certificate);
    std::string certificate_data;

    if (CKMC_ERROR_NONE == ret) {
      ckmc_policy_s policy { const_cast<char*>(password.c_str()), extractable };
      ret = ckmc_save_cert(alias.c_str(), *certificate, policy);
      if (CKMC_ERROR_NONE != ret) {
        LoggerE("Failed to save certificate: %d", ret);
      }
      certificate_data = RawBufferToBase64(ToRawBuffer(certificate));
      ckmc_cert_free(certificate);
    } else {
      LoggerE("Failed to load certificate: %d", ret);
    }

    PlatformResult success(ErrorCode::NO_ERROR);

    switch (ret) {
      case CKMC_ERROR_NONE:
        break;

      case CKMC_ERROR_FILE_ACCESS_DENIED:
        success = PlatformResult(ErrorCode::NOT_FOUND_ERR, "File not found");
        break;

      case CKMC_ERROR_INVALID_PARAMETER:
      case CKMC_ERROR_INVALID_FORMAT:
        success = PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Failed to load certificate");
        break;

      default:
        success = PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to load certificate");
        break;
    }

    if (success) {
      common::tools::ReportSuccess(picojson::value(certificate_data), result->get<picojson::object>());
    } else {
      common::tools::ReportError(success, &result->get<picojson::object>());
    }
  };

  auto load_certificate_result = [this, callback_id](const std::shared_ptr<picojson::value>& result) {
    LoggerD("Enter load_certificate_result");
    result->get<picojson::object>()["callbackId"] = picojson::value{callback_id};
    Instance::PostMessage(this, result->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<picojson::value>(
      load_certificate,
      load_certificate_result,
      std::shared_ptr<picojson::value>{new picojson::value{picojson::object()}});

  ReportSuccess(out);
}

void KeyManagerInstance::SaveData(const picojson::value& args,
                                  picojson::object& out) {
  LoggerD("Enter");

  std::string data_raw = args.get("rawData").get<std::string>();
  const auto& data = args.get("data");
  const auto& alias = data.get("name").get<std::string>();
  const auto& password_value = data.get("password");
  const auto extractable = data.get("extractable").get<bool>();
  double callback_id = args.get("callbackId").get<double>();

  std::string password;

  if (password_value.is<std::string>()) {
    password = password_value.get<std::string>();
  }

  auto save_data = [data_raw, password, extractable, alias](const std::shared_ptr<picojson::value>& result) {
    LoggerD("Enter save_data");

    unsigned char* data = new unsigned char[data_raw.size()];
    std::copy(data_raw.begin(), data_raw.end(), data);

    ckmc_raw_buffer_s raw_data { data, data_raw.size() };
    ckmc_policy_s policy { const_cast<char*>(password.c_str()), extractable };

    int ret = ckmc_save_data(alias.c_str(), raw_data, policy);

    PlatformResult success(ErrorCode::NO_ERROR);

    switch (ret) {
      case CKMC_ERROR_NONE:
        break;

      case CKMC_ERROR_INVALID_PARAMETER:
        success = PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Failed to save data");
        break;

      default:
        success = PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to save data");
        break;
    }

    if (success) {
      common::tools::ReportSuccess(result->get<picojson::object>());
    } else {
      LoggerE("Failed to save data: %d", ret);
      common::tools::ReportError(success, &result->get<picojson::object>());
    }

    delete[] data;
  };

  auto save_data_result = [this, callback_id](const std::shared_ptr<picojson::value>& result) {
    LoggerD("Enter save_data_result");
    result->get<picojson::object>()["callbackId"] = picojson::value{callback_id};
    Instance::PostMessage(this, result->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<picojson::value>(
      save_data,
      save_data_result,
      std::shared_ptr<picojson::value>{new picojson::value{picojson::object()}});

  ReportSuccess(out);
}

void KeyManagerInstance::GetData(const picojson::value& args,
                                 picojson::object& out) {
  LoggerD("Enter");

  const auto& data_alias = args.get("name").get<std::string>();
  const auto& password_value = args.get("password");

  std::string password;

  if (password_value.is<std::string>()) {
    password = password_value.get<std::string>();
  }

  ckmc_raw_buffer_s* data = nullptr;
  int ret = ckmc_get_data(data_alias.c_str(), password.c_str(), &data);

  if (CKMC_ERROR_NONE == ret) {
    picojson::object result;

    result["name"] = picojson::value(data_alias);
    result["password"] = picojson::value(password);
    // if key was retrieved it is extractable from DB
    result["extractable"] = picojson::value(true);
    result["rawData"] = picojson::value(std::string (data->data, data->data + data->size));

    ckmc_buffer_free(data);
    ReportSuccess(picojson::value{result}, out);
  } else {
    LoggerE("Failed to get data: %d", ret);

    PlatformResult error(ErrorCode::UNKNOWN_ERR, "Failed to get key");

    switch (ret) {
      case CKMC_ERROR_DB_ALIAS_UNKNOWN:
        error = PlatformResult(ErrorCode::NOT_FOUND_ERR, "Failed to find key");
        break;

      case CKMC_ERROR_INVALID_PARAMETER:
        error = PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Input parameter is invalid");
        break;
    }

    ReportError(error, &out);
  }
}

void KeyManagerInstance::CreateSignature(const picojson::value& args,
                                         picojson::object& out) {
  LoggerD("Enter");

  const auto& alias = args.get("privKeyAlias").get<std::string>();
  RawBuffer* raw_buffer = new RawBuffer(std::move(Base64ToRawBuffer(args.get("message").get<std::string>())));
  const auto& password_value = args.get("password");
  double callback_id = args.get("callbackId").get<double>();
  ckmc_hash_algo_e hash = StringToHashAlgorithm(args.get("hashAlgorithmType").get<std::string>());
  ckmc_rsa_padding_algo_e padding = StringToRsaPadding(args.get("padding").get<std::string>());

  std::string password;

  if (password_value.is<std::string>()) {
    password = password_value.get<std::string>();
  }

  auto create_certificate = [alias, raw_buffer, password, hash, padding](const std::shared_ptr<picojson::value>& result) {
    LoggerD("Enter create_certificate");

    ckmc_raw_buffer_s* signature = nullptr;
    ckmc_raw_buffer_s message = { const_cast<unsigned char*>(&(*raw_buffer)[0]), raw_buffer->size() };
    int ret = ckmc_create_signature(alias.c_str(), password.c_str(), message, hash, padding, &signature);

    PlatformResult success(ErrorCode::NO_ERROR);

    switch (ret) {
      case CKMC_ERROR_NONE:
        break;

      case CKMC_ERROR_DB_ALIAS_UNKNOWN:
        success = PlatformResult(ErrorCode::NOT_FOUND_ERR, "Alias not found");
        break;

      case CKMC_ERROR_INVALID_PARAMETER:
        success = PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Failed to create signature");
        break;

      default:
        success = PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to create signature");
        break;
    }

    if (success) {
      common::tools::ReportSuccess(picojson::value(RawBufferToBase64(ToRawBuffer(signature))), result->get<picojson::object>());
      ckmc_buffer_free(signature);
    } else {
      LoggerE("Failed to create signature: %d", ret);
      common::tools::ReportError(success, &result->get<picojson::object>());
    }

    delete raw_buffer;
  };

  auto create_certificate_result = [this, callback_id](const std::shared_ptr<picojson::value>& result) {
    LoggerD("Enter create_certificate_result");
    result->get<picojson::object>()["callbackId"] = picojson::value{callback_id};
    Instance::PostMessage(this, result->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<picojson::value>(
      create_certificate,
      create_certificate_result,
      std::shared_ptr<picojson::value>{new picojson::value{picojson::object()}});

  ReportSuccess(out);
}

void KeyManagerInstance::VerifySignature(const picojson::value& args,
                                         picojson::object& out) {
  LoggerD("Enter");

  const auto& alias = args.get("pubKeyAlias").get<std::string>();
  RawBuffer* message = new RawBuffer(std::move(Base64ToRawBuffer(args.get("message").get<std::string>())));
  RawBuffer* signature = new RawBuffer(std::move(Base64ToRawBuffer(args.get("signature").get<std::string>())));
  const auto& password_value = args.get("password");
  double callback_id = args.get("callbackId").get<double>();
  ckmc_hash_algo_e hash = StringToHashAlgorithm(args.get("hashAlgorithmType").get<std::string>());
  ckmc_rsa_padding_algo_e padding = StringToRsaPadding(args.get("padding").get<std::string>());

  std::string password;

  if (password_value.is<std::string>()) {
    password = password_value.get<std::string>();
  }

  auto verify_certificate = [alias, message, signature, password, hash, padding](const std::shared_ptr<picojson::value>& result) {
    LoggerD("Enter verify_certificate");

    ckmc_raw_buffer_s message_buf = { const_cast<unsigned char*>(&(*message)[0]), message->size() };
    ckmc_raw_buffer_s signature_buf = { const_cast<unsigned char*>(&(*signature)[0]), signature->size() };

    int ret = ckmc_verify_signature(alias.c_str(), password.c_str(), message_buf, signature_buf, hash , padding);

    PlatformResult success(ErrorCode::NO_ERROR);

    switch (ret) {
      case CKMC_ERROR_NONE:
        break;

      case CKMC_ERROR_DB_ALIAS_UNKNOWN:
        success = PlatformResult(ErrorCode::NOT_FOUND_ERR, "Alias not found");
        break;

      case CKMC_ERROR_INVALID_PARAMETER:
        success = PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Failed to verify signature");
        break;

      default:
        success = PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to verify signature");
        break;
    }

    if (success) {
      common::tools::ReportSuccess(result->get<picojson::object>());
    } else {
      LoggerE("Failed to verify signature: %d", ret);
      common::tools::ReportError(success, &result->get<picojson::object>());
    }

    delete message;
    delete signature;
  };

  auto verify_certificate_result = [this, callback_id](const std::shared_ptr<picojson::value>& result) {
    LoggerD("Enter verify_certificate_result");
    result->get<picojson::object>()["callbackId"] = picojson::value{callback_id};
    Instance::PostMessage(this, result->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<picojson::value>(
      verify_certificate,
      verify_certificate_result,
      std::shared_ptr<picojson::value>{new picojson::value{picojson::object()}});

  ReportSuccess(out);
}

void KeyManagerInstance::LoadFromPKCS12File(const picojson::value& args,
                                            picojson::object& out) {
  LoggerD("Enter");

  const auto& file_uri = args.get("fileURI").get<std::string>();
  const auto& key_alias = args.get("privKeyName").get<std::string>();
  const auto& cert_alias = args.get("certificateName").get<std::string>();
  const auto& password_value = args.get("password");
  double callback_id = args.get("callbackId").get<double>();

  std::string password;

  if (password_value.is<std::string>()) {
    password = password_value.get<std::string>();
  }

  auto load_file = [file_uri, password, cert_alias, key_alias](const std::shared_ptr<picojson::value>& result) {
    LoggerD("Enter load_file");
    std::string file = VirtualFs::GetInstance().GetRealPath(file_uri);
    ckmc_pkcs12_s* pkcs12 = nullptr;

    int ret = ckmc_pkcs12_load(file.c_str(), password.c_str(), &pkcs12);

    if (CKMC_ERROR_NONE == ret) {
      SCOPE_EXIT {
        ckmc_pkcs12_free(pkcs12);
      };
      ckmc_policy_s policy { const_cast<char*>(password.c_str()), true };

      // it's safer to use ckmc_save_pkcs12() here, however JS API specifies
      // two different aliases for private key and certificate
      if (pkcs12->cert) {
        ret = ckmc_save_cert(cert_alias.c_str(), *pkcs12->cert, policy);
        if (CKMC_ERROR_NONE != ret) {
          LoggerE("Failed to save certificate: %d", ret);
        }
      }

      if (CKMC_ERROR_NONE == ret && pkcs12->priv_key) {
        ret = ckmc_save_key(key_alias.c_str(), *pkcs12->priv_key, policy);
        if (CKMC_ERROR_NONE != ret) {
          LoggerE("Failed to save private key: %d", ret);
          // rollback
          if (pkcs12->cert) {
            ckmc_remove_cert(cert_alias.c_str());
          }
        }
      }
    } else {
      LoggerE("Failed to load PKCS12 file: %d", ret);
    }

    PlatformResult success(ErrorCode::NO_ERROR);

    switch (ret) {
      case CKMC_ERROR_NONE:
        break;

      case CKMC_ERROR_FILE_ACCESS_DENIED:
        success = PlatformResult(ErrorCode::NOT_FOUND_ERR, "Certificate file not found");
        break;

      case CKMC_ERROR_INVALID_FORMAT:
      case CKMC_ERROR_INVALID_PARAMETER:
        success = PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Certificate file has wrong format");
        break;

      case CKMC_ERROR_PERMISSION_DENIED:
        success = PlatformResult(ErrorCode::IO_ERR, "Permission has been denied");
        break;

      default:
        success = PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to open certificate file");
        break;
    }

    if (success) {
      common::tools::ReportSuccess(result->get<picojson::object>());
    } else {
      common::tools::ReportError(success, &result->get<picojson::object>());
    }
  };

  auto load_file_result = [this, callback_id](const std::shared_ptr<picojson::value>& result) {
    LoggerD("Enter load_file_result");
    result->get<picojson::object>()["callbackId"] = picojson::value{callback_id};
    Instance::PostMessage(this, result->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<picojson::value>(
      load_file,
      load_file_result,
      std::shared_ptr<picojson::value>{new picojson::value{picojson::object()}});

  ReportSuccess(out);
}

void KeyManagerInstance::AllowAccessControl(const picojson::value& args,
                                            picojson::object& out) {
  LoggerD("Enter");

  const std::string& data_name = args.get("dataName").get<std::string>();
  const std::string& id = args.get("id").get<std::string>();
  const double callback_id = args.get("callbackId").get<double>();
  const std::string& access = args.get("accessControlType").get<std::string>();
  ckmc_access_right_e granted = CKMC_AR_READ;
  if ("READ_REMOVE" == access) {
    granted = CKMC_AR_READ_REMOVE;
  }

  auto allow = [data_name, id, granted](const std::shared_ptr<picojson::value>& response) -> void {
    //as ckmc_allow_access does not check if package id exists
    //it has to be done before allowing access
    pkgmgrinfo_pkginfo_h handle = nullptr;
    int ret = pkgmgrinfo_pkginfo_get_pkginfo(id.c_str(), &handle);
    if (PMINFO_R_OK != ret) {
      LoggerE("Package id not found.");
      common::tools::ReportError(PlatformResult(
          ErrorCode::NOT_FOUND_ERR, "Package id not found."), &response->get<picojson::object>());
      return;
    }
    pkgmgrinfo_pkginfo_destroy_pkginfo(handle);

    ret = ckmc_allow_access(data_name.c_str(), id.c_str(), granted);
    if (CKMC_ERROR_NONE != ret) {
      PlatformResult result = PlatformResult(ErrorCode::NO_ERROR);
      if (CKMC_ERROR_DB_ALIAS_UNKNOWN == ret) {
        result = PlatformResult(ErrorCode::NOT_FOUND_ERR, "Alias not found.");
      } else {
        result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to allow access.");
      }
      common::tools::ReportError(result, &response->get<picojson::object>());
    } else {
      common::tools::ReportSuccess(response->get<picojson::object>());
    }
  };

  auto allow_response = [this, callback_id](const std::shared_ptr<picojson::value>& response) -> void {
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair("callbackId", picojson::value(callback_id)));
    Instance::PostMessage(this, response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<picojson::value>(
      allow,
      allow_response,
      std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));
}

void KeyManagerInstance::DenyAccessControl(const picojson::value& args,
                                           picojson::object& out) {
  LoggerD("Enter");

  const std::string& data_name = args.get("dataName").get<std::string>();
  const std::string& id = args.get("id").get<std::string>();
  const double callback_id = args.get("callbackId").get<double>();

  auto deny = [data_name, id](const std::shared_ptr<picojson::value>& response) -> void {
    int ret = ckmc_deny_access(data_name.c_str(), id.c_str());
    if (CKMC_ERROR_NONE != ret) {
      PlatformResult result = PlatformResult(ErrorCode::NO_ERROR);
      if (CKMC_ERROR_DB_ALIAS_UNKNOWN == ret) {
        result = PlatformResult(ErrorCode::NOT_FOUND_ERR, "Alias not found.");
      } else {
        result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to deny access.");
      }
      common::tools::ReportError(result, &response->get<picojson::object>());
    } else {
      common::tools::ReportSuccess(response->get<picojson::object>());
    }
  };

  auto deny_response = [this, callback_id](const std::shared_ptr<picojson::value>& response) -> void {
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair("callbackId", picojson::value(callback_id)));
    Instance::PostMessage(this, response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<picojson::value>(
      deny,
      deny_response,
      std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));
}

void KeyManagerInstance::IsDataNameFound(const picojson::value& args,
                                           picojson::object& out){
  LoggerD("Entered");

  const std::string& data_name = args.get("dataName").get<std::string>();
  bool data_found = false;
  ckmc_alias_list_s* alias_list = nullptr;

  int ret = ckmc_get_data_alias_list(&alias_list);
  if (CKMC_ERROR_NONE != ret) {
    LoggerE("Failed to get data list [%d]", ret);
    PlatformResult result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to get data list.");
    if (CKMC_ERROR_DB_ALIAS_UNKNOWN == ret) {
      result = PlatformResult(ErrorCode::NOT_FOUND_ERR, "Data name not found.");
    }

    common::tools::ReportError(result, &out);
    return;
  }

  ckmc_alias_list_s* head = alias_list;
  while (head) {
    if (!strcmp(head->alias, data_name.c_str())) {
      data_found = true;
      break;
    }
    head = head->next;
  }

  if (alias_list) {
    ckmc_alias_list_all_free(alias_list);
  }

  LoggerD("Data name found: %d", data_found);
  if (data_found) {
    common::tools::ReportSuccess(out);
  } else {
    common::tools::ReportError(
        PlatformResult(ErrorCode::NOT_FOUND_ERR, "Data name not found."), &out);
  }
}

} // namespace keymanager
} // namespace extension
