// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_FILTER_H_
#define CONTENT_FILTER_H_

#include <media_content.h>
#include <string>

#include "common/filter-utils.h"
#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace content {

class ContentFilter {
 public:
  common::PlatformResult buildQuery(const picojson::object &jsFilter,
                                    std::string *query);

 private:
  common::FilterVisitor visitor;
};
}
}

#endif
