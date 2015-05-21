// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_MEDIAKEY_MEDIAKEY_EXTENSION_H_
#define SRC_MEDIAKEY_MEDIAKEY_EXTENSION_H_

#include "common/extension.h"

namespace extension {
namespace mediakey {

class MediaKeyManager;

class MediaKeyExtension : public common::Extension {
 public:
  MediaKeyExtension();
  virtual ~MediaKeyExtension();

 private:
  virtual common::Instance* CreateInstance();
};

}  // namespace mediakey
}  // namespace extension

#endif  // SRC_MEDIAKEY_MEDIAKEY_EXTENSION_H_
