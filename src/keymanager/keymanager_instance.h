// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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

  CKM::ManagerAsync m_manager;
};

} // namespace keymanager
} // namespace extension

#endif // KEYMANAGER_KEYMANAGER_INSTANCE_H_
