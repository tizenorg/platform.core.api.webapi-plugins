// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BADGE_BADGE_INSTANCE_H_
#define BADGE_BADGE_INSTANCE_H_

#include "common/extension.h"

namespace extension {
namespace badge {

class BadgeInstance : public common::ParsedInstance {
 public:
  BadgeInstance();
  virtual ~BadgeInstance();

};

}  // namespace badge
}  // namespace extension

#endif  // BADGE_BADGE_INSTANCE_H_
