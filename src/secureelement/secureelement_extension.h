// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SECUREELEMENT_SECUREELEMENT_EXTENSION_H_
#define SECUREELEMENT_SECUREELEMENT_EXTENSION_H_

#include "common/extension.h"

class SecureElementExtension : public common::Extension {
public:
    SecureElementExtension();
    virtual ~SecureElementExtension();
private:
    // common::Extension implementation.
    virtual common::Instance* CreateInstance();
};

#endif  // SECUREELEMENT_SECUREELEMENT_EXTENSION_H_
