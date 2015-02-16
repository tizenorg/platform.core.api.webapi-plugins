// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_FILTER_H_
#define CONTENT_FILTER_H_

#include "common/picojson.h"

#include <string>
#include <media_content.h>


namespace extension {
namespace content {

class ContentFilter {
 public:
  std::string convert(const picojson::value &jsFilter);
  
 private:
    

};

}
}

#endif 

