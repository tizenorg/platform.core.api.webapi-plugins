// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "keymanager/keymanager_instance.h"

#include <functional>
#include <ckm/ckm-manager.h>

#include "common/logger.h"
#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace keymanager {


KeyManagerInstance::KeyManagerInstance() {
  using std::placeholders::_1;
  using std::placeholders::_2;

  RegisterSyncHandler("KeyManager_getKeyAliasList",
      std::bind(&KeyManagerInstance::GetKeyAliasList, this, _1, _2));
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

} // namespace keymanager
} // namespace extension
