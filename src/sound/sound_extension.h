// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SOUND_SOUND_EXTENSION_H_
#define SOUND_SOUND_EXTENSION_H_

#include "common/extension.h"

class SoundExtension : public common::Extension {
 public:
  SoundExtension();
  virtual ~SoundExtension();

 private:
  virtual common::Instance* CreateInstance();
};

#endif // SOUND_SOUND_EXTENSION_H_
