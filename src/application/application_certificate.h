// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPLICATION_APPLICATION_CERTIFICATE_H_
#define APPLICATION_APPLICATION_CERTIFICATE_H_

#include <string>
#include <memory>

#include "common/picojson.h"
#include "tizen/tizen.h"

namespace extension {
namespace application {

class ApplicationCertificate;
typedef std::shared_ptr<ApplicationCertificate> ApplicationCertificatePtr;

typedef std::vector<ApplicationCertificatePtr> ApplicationCertificateArray;
typedef std::shared_ptr<ApplicationCertificateArray> ApplicationCertificateArrayPtr;

class ApplicationCertificate {
 public:
  ApplicationCertificate();
  ApplicationCertificate(const ApplicationCertificatePtr);
  ~ApplicationCertificate();

  const picojson::value& Value();
  bool IsValid() const;

  std::string get_cert_type() const;
  void set_cert_type(const std::string& cert_type);
 
  std::string get_cert_value() const;
  void set_cert_value(const std::string& cert_value);
 

 private:
  std::string cert_type_;
  std::string cert_value_;

  picojson::object data_;
  picojson::object error_;
  picojson::value value_;
};
} // namespace application
} // namespace extension

#endif  // APPLICATION_APPLICATION_CERTIFICATE_H_
