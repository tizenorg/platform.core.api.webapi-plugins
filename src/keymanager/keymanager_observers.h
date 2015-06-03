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

#ifndef KEYMANAGER_KEYMANAGER_OBSERVERS_H_
#define KEYMANAGER_KEYMANAGER_OBSERVERS_H_

#include <ckm/ckm-manager-async.h>
#include <string>
#include "common/platform_result.h"
#include "keymanager/async_file_reader.h"

namespace extension {
namespace keymanager {

class LoadFileCert;
class LoadFilePKCS12;

class KeyManagerListener {
public:
  virtual void OnSaveKey(double callbackId, const common::PlatformResult& result) = 0;
  virtual void OnCreateKeyPair(double callbackId, const common::PlatformResult& result) = 0;
  virtual void OnSaveCert(double callbackId, const common::PlatformResult& result) = 0;
  virtual void OnCertFileLoaded(LoadFileCert* reader,
    const common::PlatformResult& result) = 0;
  virtual void OnSaveData(double callbackId, const common::PlatformResult& result) = 0;
  virtual void OnCreateSignature(double callbackId, const common::PlatformResult& result, CKM::RawBuffer buffer) = 0;
  virtual void OnVerifySignature(double callbackId, const common::PlatformResult& result) = 0;
  virtual void OnPKCS12FileLoaded(LoadFilePKCS12* reader,
    const common::PlatformResult& result) = 0;
  virtual void OnSavePKCS12(double callbackId, const common::PlatformResult& result) = 0;
  virtual void OnAllowAccess(double callbackId, const common::PlatformResult& result) = 0;
  virtual void OnDenyAccess(double callbackId, const common::PlatformResult& result) = 0;
  virtual ~KeyManagerListener() {}
};

struct CommonObserver: public CKM::ManagerAsync::Observer {
  CommonObserver(KeyManagerListener* listener, double callbackId);
  virtual ~CommonObserver() {}

 protected:
  KeyManagerListener* listener;
  double callbackId;
};

struct SaveKeyObserver: public CommonObserver {
  SaveKeyObserver(KeyManagerListener* listener, double callbackId);
  void ReceivedError(int error);
  void ReceivedSaveKey();
};

struct CreateKeyObserver: public CommonObserver {
  CreateKeyObserver(KeyManagerListener* listener, double callbackId);
  void ReceivedError(int error);
  void ReceivedCreateKeyPairDSA();
  void ReceivedCreateKeyPairECDSA();
  void ReceivedCreateKeyPairRSA();
private:
  void CallSuccess();

};

struct SaveCertObserver: public CommonObserver {
  SaveCertObserver(KeyManagerListener* listener, double callbackId);
  void ReceivedError(int error);
  void ReceivedSaveCertificate();
};

struct LoadFileCert: public AsyncFileReader {
  LoadFileCert(KeyManagerListener* listener,
    double callbackId,
    const std::string &password,
    const std::string &alias,
    bool extractable);

  double callback_id;
  std::string password;
  const std::string alias;
  bool extractable;
  std::string file_content;

protected:
  void AppendBuffer(guint8* buffer, gssize size);
  void OnError(const common::PlatformResult& result);
  void OnFileLoaded();

private:
  KeyManagerListener* listener;
};

struct LoadFilePKCS12: public AsyncFileReader {
  LoadFilePKCS12(KeyManagerListener* listener,
    double callbackId,
    const std::string &password,
    const std::string &keyAlias,
    const std::string &certAlias);

  double callback_id;
  std::string password;
  const std::string key_alias;
  const std::string cert_alias;
  CKM::RawBuffer file_content;

protected:
  void AppendBuffer(guint8* buffer, gssize size);
  void OnError(const common::PlatformResult& result);
  void OnFileLoaded();

private:
  KeyManagerListener* listener;
};

struct SaveDataObserver: public CommonObserver {
  SaveDataObserver(KeyManagerListener* listener, double callbackId);
  void ReceivedError(int error);
  void ReceivedSaveData();
};

struct CreateSignatureObserver: public CommonObserver {
  CreateSignatureObserver(KeyManagerListener* listener, double callbackId);
  void ReceivedError(int error);
  void ReceivedCreateSignature(CKM::RawBuffer&&);
};

struct VerifySignatureObserver: public CommonObserver {
  VerifySignatureObserver(KeyManagerListener* listener, double callbackId);
  void ReceivedError(int error);
  void ReceivedVerifySignature();
};

struct SavePKCS12Observer: public CommonObserver {
  SavePKCS12Observer(KeyManagerListener* listener, double callback_id);
  void ReceivedError(int error);
  void ReceivedSaveKey();
  void ReceivedSaveCertificate();

private:
    bool cert_saved;
    bool key_saved;
};

struct AllowAccessObserver: public CommonObserver {
  AllowAccessObserver(KeyManagerListener* listener, double callbackId);
  void ReceivedError(int error);
  void ReceivedSetPermission();
};

struct DenyAccessObserver: public CommonObserver {
  DenyAccessObserver(KeyManagerListener* listener, double callbackId);
  void ReceivedError(int error);
  void ReceivedSetPermission();
};

} // namespace keymanager
} // namespace extension

#endif // KEYMANAGER_KEYMANAGER_OBSERVERS_H_
