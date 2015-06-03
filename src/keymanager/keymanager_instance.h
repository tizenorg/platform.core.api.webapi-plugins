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
 
#ifndef KEYMANAGER_KEYMANAGER_INSTANCE_H_
#define KEYMANAGER_KEYMANAGER_INSTANCE_H_

#include "common/extension.h"
#include <ckm/ckm-manager-async.h>
#include "keymanager/keymanager_observers.h"


namespace extension {
namespace keymanager {

class KeyManagerInstance :
    public common::ParsedInstance,
    public KeyManagerListener {
 public:
  KeyManagerInstance();
  virtual ~KeyManagerInstance();
  void OnSaveKey(double callbackId, const common::PlatformResult& result);
  void OnCreateKeyPair(double callbackId, const common::PlatformResult& result);
  void OnSaveCert(double callbackId, const common::PlatformResult& result);
  void OnCertFileLoaded(LoadFileCert* reader,
    const common::PlatformResult& result);
  void OnSaveData(double callbackId, const common::PlatformResult& result);
  void OnCreateSignature(double callbackId, const common::PlatformResult& result, CKM::RawBuffer buffer);
  void OnVerifySignature(double callbackId, const common::PlatformResult& result);
  void OnPKCS12FileLoaded(LoadFilePKCS12* reader, const common::PlatformResult& result);
  void OnSavePKCS12(double callbackId, const common::PlatformResult& result);
  void OnAllowAccess(double callbackId, const common::PlatformResult& result);
  void OnDenyAccess(double callbackId, const common::PlatformResult& result);

 private:
  void GetAliasList(std::function<int(CKM::AliasVector&)> coreFunc,
      picojson::object& out);
  void GetKeyAliasList(picojson::value const& args, picojson::object& out);
  void GetCertificateAliasList(picojson::value const& args,
      picojson::object& out);
  void GetDataAliasList(picojson::value const& args, picojson::object& out);
  void GetKey(const picojson::value& args, picojson::object& out);
  void SaveKey(const picojson::value& args, picojson::object& out);
  void RemoveKey(const picojson::value& args, picojson::object& out);
  void GenerateKeyPair(const picojson::value& args, picojson::object& out);
  void GetCertificate(const picojson::value& args, picojson::object& out);
  void SaveCertificate(const picojson::value& args, picojson::object& out);
  void LoadCertificateFromFile(const picojson::value& args, picojson::object& out);
  void SaveCert(std::string &base64,
    const std::string &password,
    const std::string &alias,
    bool extractable,
    double callbackId);
  void RemoveCertificate(const picojson::value& args, picojson::object& out);
  common::PlatformResult RemoveAlias(const std::string &alias);
  void SaveData(const picojson::value& args, picojson::object& out);
  void RemoveData(const picojson::value& args, picojson::object& out);
  void GetData(const picojson::value& args, picojson::object& out);
  void CreateSignature(const picojson::value& args, picojson::object& out);
  void VerifySignature(const picojson::value& args, picojson::object& out);
  void LoadFromPKCS12File(const picojson::value& args, picojson::object& out);
  void AllowAccessControl(const picojson::value& args, picojson::object& out);
  void DenyAccessControl(const picojson::value& args, picojson::object& out);

  CKM::ManagerAsync m_manager;
};

} // namespace keymanager
} // namespace extension

#endif // KEYMANAGER_KEYMANAGER_INSTANCE_H_