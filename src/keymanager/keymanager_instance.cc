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

RawBuffer ToRawBuffer(const ckmc_key_s* key) {
  return RawBuffer(key->raw_key, key->raw_key + key->key_size);
}//

RawBuffer ToRawBuffer(const ckmc_raw_buffer_s* buffer) {
  return RawBuffer(buffer->data, buffer->data + buffer->size);
}

typedef int (*AliasListFunction)(ckmc_alias_list_s**);

void GetGenericAliasList(AliasListFunction func, picojson::object* out) {
  LoggerD("Enter");

  ckmc_alias_list_s* alias_list = nullptr;
  int ret = func(&alias_list);

  if (CKMC_ERROR_NONE == ret) {
    picojson::value result{picojson::array{}};
    auto& aliases = result.get<picojson::array>();
    ckmc_alias_list_s* head = alias_list;

    while (head) {
      aliases.push_back(picojson::value(alias_list->alias ? alias_list->alias : ""));
      head = head->next;
    }

    if (alias_list) {
      ckmc_alias_list_all_free(alias_list);
    }

    common::tools::ReportSuccess(result, *out);
  } else {
    LoggerE("Failed to get alias list: %d", ret);
    common::tools::ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to get alias list"), out);
  }
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
    result["password"] = picojson::value(key->password);
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

    ckmc_policy_s policy { const_cast<char*>(pass.c_str()), extractable };
    ckmc_key_s key { const_cast<unsigned char*>(&(*raw_buffer)[0]),
      raw_buffer->size(), key_type, const_cast<char*>(pass.c_str()) };

    int ret = ckmc_save_key(alias.c_str(), key, policy);
    if (CKMC_ERROR_NONE != ret) {
      PlatformResult result = PlatformResult(ErrorCode::NO_ERROR);
      if (CKMC_ERROR_INVALID_PARAMETER == ret) {
        result = PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid parameter passed.");
      } else {
        result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to save key.");
      }
      common::tools::ReportError(result, &response->get<picojson::object>());
    } else {
      common::tools::ReportSuccess(response->get<picojson::object>());
    }

    delete raw_buffer;
  };

  auto save_response = [this, callback_id](const std::shared_ptr<picojson::value>& response) -> void {
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair("callbackId", picojson::value(callback_id)));
    this->PostMessage(response->serialize().c_str());
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
  bool extractable = priv_key.get("extractable").get<bool>();
  ckmc_policy_s priv_policy { const_cast<char*>(priv_pass.c_str()), extractable };

  std::string pub_pass;
  if (pub_key.get("password").is<std::string>()) {
    pub_pass = pub_key.get("password").get<std::string>();
  }
  extractable = pub_key.get("extractable").get<bool>();
  ckmc_policy_s pub_policy { const_cast<char*>(pub_pass.c_str()), extractable };

  std::string elliptic;
  if (args.get("ellipticCurveType").is<std::string>()) {
    elliptic = args.get("ellipticCurveType").get<std::string>();
  }

  auto generate = [size, priv_policy, pub_policy, priv_name, pub_name, type, elliptic]
                   (const std::shared_ptr<picojson::value>& response) -> void {
    int ret = CKMC_ERROR_NONE;
    if (kTypeRSA == type) {
      ret = ckmc_create_key_pair_rsa(size, priv_name.c_str(),
                                     pub_name.c_str(), priv_policy, pub_policy);
    } else if (kTypeECDSA == type) {
      ret = ckmc_create_key_pair_ecdsa(GetEllipticCurveType(elliptic), priv_name.c_str(),
                                       pub_name.c_str(), priv_policy, pub_policy);
    } else {
      ret = ckmc_create_key_pair_dsa(size, priv_name.c_str(),
                                     pub_name.c_str(), priv_policy, pub_policy);
    }

    if (CKMC_ERROR_NONE != ret) {
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
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair("callbackId", picojson::value(callback_id)));
    this->PostMessage(response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<picojson::value>(
      generate,
      generate_response,
      std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));
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
}

void KeyManagerInstance::LoadCertificateFromFile(const picojson::value& args,
                                                 picojson::object& out) {
  LoggerD("Enter");
}

void KeyManagerInstance::SaveData(const picojson::value& args,
                                  picojson::object& out) {
  LoggerD("Enter");
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
    result["rawData"] = picojson::value(RawBufferToBase64(ToRawBuffer(data)));

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
}

void KeyManagerInstance::VerifySignature(const picojson::value& args,
                                         picojson::object& out) {
  LoggerD("Enter");
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
    this->PostMessage(result->serialize().c_str());
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
    int ret = ckmc_allow_access(data_name.c_str(), id.c_str(), granted);
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
    this->PostMessage(response->serialize().c_str());
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
    this->PostMessage(response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<picojson::value>(
      deny,
      deny_response,
      std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));
}

} // namespace keymanager
} // namespace extension
