// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef KEYMANAGER_KEYMANAGER_OBSERVERS_H_
#define KEYMANAGER_KEYMANAGER_OBSERVERS_H_

#include <ckm/ckm-manager-async.h>
#include <gio/gio.h>
#include <string>
#include "common/platform_result.h"

namespace extension {
namespace keymanager {

class LoadFileCert;

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

struct LoadFileCert {
  LoadFileCert(KeyManagerListener* listener,
    double callbackId,
    const std::string &password,
    const std::string &alias,
    bool extractable);
  void LoadFileAsync(const std::string &fileUri);
  virtual ~LoadFileCert();

  double callbackId;
  std::string password;
  const std::string alias;
  bool extractable;
  std::string fileContent;
private:
  guint8* buffer;
  KeyManagerListener* listener;

  static void OnFileRead(GObject *source_object,
    GAsyncResult *res,
    gpointer user_data);
  static void OnStreamRead(GObject *source_object,
    GAsyncResult *res,
    gpointer user_data);
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

} // namespace keymanager
} // namespace extension

#endif // KEYMANAGER_KEYMANAGER_OBSERVERS_H_

