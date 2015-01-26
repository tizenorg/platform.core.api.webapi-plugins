// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_APPLICATION_APPLICATION_H_
#define SRC_APPLICATION_APPLICATION_H_

#include <string>
#include <memory>

#include "application/application_information.h"

namespace extension {
namespace application {

class Application;
typedef std::shared_ptr<Application> ApplicationPtr;

class Application {
 public:
  Application();
  ~Application();

  void Hide();
  void Exit();

  const picojson::value& Value();
  bool IsValid() const;

  std::string get_context_id();
  void set_context_id(const std::string& context_id);

  ApplicationInformationPtr get_app_info() const;
  void set_app_info(const ApplicationInformationPtr &appInfo);

 private:
  std::string context_id_;
  ApplicationInformationPtr app_info_;

  picojson::object data_;
  picojson::object error_;
  picojson::value value_;
};

}  // namespace application
}  // namespace extension

#endif  // SRC_APPLICATION_APPLICATION_H_
