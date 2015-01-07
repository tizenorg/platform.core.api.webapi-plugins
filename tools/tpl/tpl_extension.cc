// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "{{module.lower}}/{{module.lower}}_extension.h"

#include "{{module.lower}}/{{module.lower}}_instance.h"

// This will be generated from {{module.lower}}_api.js
extern const char kSource_{{module.lower}}_api[];

common::Extension* CreateExtension() {
  return new {{module.title}}Extension;
}

{{module.title}}Extension::{{module.title}}Extension() {
  SetExtensionName("tizen.{{module.lower}}");
  SetJavaScriptAPI(kSource_{{module.lower}}_api);
}

{{module.title}}Extension::~{{module.title}}Extension() {}

common::Instance* {{module.title}}Extension::CreateInstance() {
  return new extension::{{module.lower}}::{{module.title}}Instance;
}
