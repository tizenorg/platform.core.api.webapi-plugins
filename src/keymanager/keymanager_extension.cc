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
