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
}

void KeyManagerInstance::GetCertificate(const picojson::value& args,
                                        picojson::object& out) {
  LoggerD("Enter");
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
