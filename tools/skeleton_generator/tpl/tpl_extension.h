// Copyright {{year}} Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef {{module.upper}}_{{module.upper}}_EXTENSION_H_
#define {{module.upper}}_{{module.upper}}_EXTENSION_H_

#include "common/extension.h"

class {{module.title}}Extension : public common::Extension {
 public:
  {{module.title}}Extension();
  virtual ~{{module.title}}Extension();

 private:
  virtual common::Instance* CreateInstance();
};

#endif // {{module.upper}}_{{module.upper}}_EXTENSION_H_

