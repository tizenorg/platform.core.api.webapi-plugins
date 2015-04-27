// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef KEYMANAGER_KEYMANAGER_OBSERVERS_H_
#define KEYMANAGER_KEYMANAGER_OBSERVERS_H_

#include <ckm/ckm-manager-async.h>
#include "common/platform_result.h"

namespace extension {
namespace keymanager {

class KeyManagerListener {
public:
  virtual void OnSaveKey(double callbackId, const common::PlatformResult& result) = 0;
  virtual void OnCreateKeyPair(double callbackId, const common::PlatformResult& result) = 0;
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

} // namespace keymanager
} // namespace extension

#endif // KEYMANAGER_KEYMANAGER_OBSERVERS_H_

