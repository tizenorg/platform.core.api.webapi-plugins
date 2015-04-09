// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "secureelement/secureelement_extension.h"

#include "secureelement/secureelement_instance.h"

common::Extension* CreateExtension() {
  return new SecureElementExtension;
}

// This will be generated from secureelement_api.js.
extern const char kSource_secureelement_api[];

SecureElementExtension::SecureElementExtension() {
  SetExtensionName("tizen.seService");
  SetJavaScriptAPI(kSource_secureelement_api);
}

SecureElementExtension::~SecureElementExtension() {}

common::Instance* SecureElementExtension::CreateInstance() {
  return new extension::secureelement::SecureElementInstance();
}
