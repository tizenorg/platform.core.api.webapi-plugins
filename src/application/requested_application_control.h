// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_APPLICATION_REQUESTED_APPLICATION_CONTROL_H_
#define SRC_APPLICATION_REQUESTED_APPLICATION_CONTROL_H_

#include <string>
#include <memory>

#include "common/picojson.h"
#include "tizen/tizen.h"

#include "application/application_control.h"

namespace extension {
namespace application {

class RequestedApplicationControl;
typedef std::shared_ptr<RequestedApplicationControl>
  RequestedApplicationControlPtr;

class RequestedApplicationControl {
 public:
  RequestedApplicationControl();
  ~RequestedApplicationControl();

  const picojson::value& Value();
  bool IsValid() const;

  std::string get_caller_app_id() const;
  void set_caller_app_id(const std::string& caller_app_id);

  ApplicationControl& get_app_control();
  void set_app_control(const ApplicationControl& app_control);

 private:
  std::string caller_app_id_;
  ApplicationControl app_control_;

  picojson::object data_;
  picojson::object error_;
  picojson::value value_;
};

}  // namespace application
}  // namespace extension

#endif  // SRC_APPLICATION_REQUESTED_APPLICATION_CONTROL_H_
