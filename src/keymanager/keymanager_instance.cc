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
  RegisterSyncHandler("KeyManager_removeKey",
      std::bind(&KeyManagerInstance::RemoveKey, this, _1, _2));
  RegisterSyncHandler("KeyManager_generateKeyPair",
      std::bind(&KeyManagerInstance::GenerateKeyPair, this, _1, _2));
  RegisterSyncHandler("KeyManager_getCertificate",
      std::bind(&KeyManagerInstance::GetCertificate, this, _1, _2));
  RegisterSyncHandler("KeyManager_saveCertificate",
      std::bind(&KeyManagerInstance::SaveCertificate, this, _1, _2));
  RegisterSyncHandler("KeyManager_loadCertificateFromFile",
      std::bind(&KeyManagerInstance::LoadCertificateFromFile, this, _1, _2));
  RegisterSyncHandler("KeyManager_removeCertificate",
      std::bind(&KeyManagerInstance::RemoveCertificate, this, _1, _2));
  RegisterSyncHandler("KeyManager_saveData",
      std::bind(&KeyManagerInstance::SaveData, this, _1, _2));
  RegisterSyncHandler("KeyManager_removeData",
      std::bind(&KeyManagerInstance::RemoveData, this, _1, _2));
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
}

void KeyManagerInstance::GetCertificateAliasList(const picojson::value& args,
                                                 picojson::object& out) {
  LoggerD("Enter");
}

void KeyManagerInstance::GetDataAliasList(const picojson::value& args,
                                          picojson::object& out) {
  LoggerD("Enter");
}

void KeyManagerInstance::GetKey(const picojson::value& args,
                                picojson::object& out) {
  LoggerD("Enter");
}

void KeyManagerInstance::SaveKey(const picojson::value& args,
                                 picojson::object& out) {
  LoggerD("Enter");
}

void KeyManagerInstance::RemoveKey(const picojson::value& args,
                                   picojson::object& out) {
  LoggerD("Enter");
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

void KeyManagerInstance::RemoveCertificate(const picojson::value& args,
                                           picojson::object& out) {
  LoggerD("Enter");
}

void KeyManagerInstance::SaveData(const picojson::value& args,
                                  picojson::object& out) {
  LoggerD("Enter");
}

void KeyManagerInstance::RemoveData(const picojson::value& args,
                                    picojson::object& out) {
  LoggerD("Enter");
}

void KeyManagerInstance::GetData(const picojson::value& args,
                                 picojson::object& out) {
  LoggerD("Enter");
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
}

void KeyManagerInstance::AllowAccessControl(const picojson::value& args,
                                            picojson::object& out) {
  LoggerD("Enter");
}

void KeyManagerInstance::DenyAccessControl(const picojson::value& args,
                                           picojson::object& out) {
  LoggerD("Enter");
}

} // namespace keymanager
} // namespace extension
