// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "contact/contact_extension.h"
#include "contact/contact_instance.h"

extern const char kSource_contact_api[];

common::Extension* CreateExtension() {
  return new ContactExtension;
}

ContactExtension::ContactExtension() {
  SetExtensionName("tizen.contact");
  SetJavaScriptAPI(kSource_contact_api);
}

ContactExtension::~ContactExtension() {}

common::Instance* ContactExtension::CreateInstance() {
  return new extension::contact::ContactInstance;
}
