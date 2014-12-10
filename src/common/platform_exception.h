// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMMON_PLATFORM_EXCEPTION_H_
#define COMMON_PLATFORM_EXCEPTION_H_

#include <string>

#include "common/picojson.h"

namespace common {

class PlatformException {
 public:
  PlatformException(const std::string& name, const std::string& message);
  virtual ~PlatformException();

  std::string name() const;
  std::string message() const;
  picojson::value ToJSON() const;

 protected:
  std::string name_;
  std::string message_;
};

#define DEFINE_EXCEPTION(NAME) \
class NAME##Exception: public PlatformException { \
 public: \
  NAME##Exception(const std::string& msg) \
      : PlatformException(#NAME "Error", msg) { \
  } \
};
DEFINE_EXCEPTION(Unknown)
DEFINE_EXCEPTION(TypeMismatch)
DEFINE_EXCEPTION(InvalidValues)
DEFINE_EXCEPTION(IO)
DEFINE_EXCEPTION(ServiceNotAvailable)
DEFINE_EXCEPTION(Security)
DEFINE_EXCEPTION(NotSupported)
DEFINE_EXCEPTION(NotFound)
DEFINE_EXCEPTION(InvalidAccess)
DEFINE_EXCEPTION(QuotaExceeded)
#undef DEFINE_EXCEPTION

} // namespace common

#endif // COMMON_PLATFORM_EXCEPTION_H_