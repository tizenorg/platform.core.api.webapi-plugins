// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef KEYMANAGER_KEYMANAGER_INSTANCE_H_
#define KEYMANAGER_KEYMANAGER_INSTANCE_H_

#include "common/extension.h"

namespace extension {
namespace keymanager {

class KeyManagerInstance : public common::ParsedInstance {
 public:
  KeyManagerInstance();
  virtual ~KeyManagerInstance();
 private:
  void GetKeyAliasList(picojson::value const& args, picojson::object& out);
};

} // namespace keymanager
} // namespace extension

#endif // KEYMANAGER_KEYMANAGER_INSTANCE_H_
