// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "keymanager/keymanager_extension.h"

#include "keymanager/keymanager_instance.h"

namespace {
const char* kKey = "tizen.Key";
const char* kData = "tizen.Data";
const char* kCertificate = "tizen.Certificate";
}

// This will be generated from keymanager_api.js
extern const char kSource_keymanager_api[];

common::Extension* CreateExtension() {
  return new KeyManagerExtension;
}

KeyManagerExtension::KeyManagerExtension() {
  SetExtensionName("tizen.keymanager");
  SetJavaScriptAPI(kSource_keymanager_api);
  const char* entry_points[] = {
      kKey,
      kData,
      kCertificate,
      NULL
  };
  SetExtraJSEntryPoints(entry_points);
}

KeyManagerExtension::~KeyManagerExtension() {}

common::Instance* KeyManagerExtension::CreateInstance() {
  return new extension::keymanager::KeyManagerInstance;
}
