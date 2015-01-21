// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "application/application_certificate.h"

#include <app_manager.h>

#include "common/logger.h"
#include "tizen/tizen.h"

namespace extension {
namespace application {

ApplicationCertificate::ApplicationCertificate() {

}

ApplicationCertificate::ApplicationCertificate(const ApplicationCertificatePtr app) {

}

ApplicationCertificate::~ApplicationCertificate() {
}

const picojson::value& ApplicationCertificate::Value() {
  if (value_.is<picojson::null>() && IsValid()) {
    picojson::object obj;
    obj["type"] = picojson::value(cert_type_);
    obj["value"] = picojson::value(cert_value_);
    value_ = picojson::value(obj);
  }
  return value_;
}

bool ApplicationCertificate::IsValid() const {
  return error_.empty();
}

std::string ApplicationCertificate::get_cert_type() const {
  return cert_type_;
}

void ApplicationCertificate::set_cert_type(const std::string& cert_type) {
  cert_type_ = cert_type;
}

std::string ApplicationCertificate::get_cert_value() const {
  return cert_value_;
}

void ApplicationCertificate::set_cert_value(const std::string& cert_value) {
  cert_value_ = cert_value;
}
 
} // namespace application
} // namespace extension
