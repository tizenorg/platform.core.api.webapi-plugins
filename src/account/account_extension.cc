// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "account/account_extension.h"

#include "account/account_instance.h"

// This will be generated from account_api.js
extern const char kSource_account_api[];

common::Extension* CreateExtension() {
  return new AccountExtension;
}

AccountExtension::AccountExtension() {
  SetExtensionName("tizen.account");
  SetJavaScriptAPI(kSource_account_api);

  const char* entry_points[] = {
      "tizen.Account",
      NULL
    };
  SetExtraJSEntryPoints(entry_points);
}

AccountExtension::~AccountExtension() {}

common::Instance* AccountExtension::CreateInstance() {
  return new extension::account::AccountInstance;
}