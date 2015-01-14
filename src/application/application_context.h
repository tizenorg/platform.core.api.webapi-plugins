// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPLICATION_APPLICATION_CONTEXT_H_
#define APPLICATION_APPLICATION_CONTEXT_H_

#include <string>
#include <memory>

#include "common/picojson.h"
#include "tizen/tizen.h"

namespace extension {
namespace application {

class ApplicationContext;
typedef std::shared_ptr<ApplicationContext> ApplicationContextPtr;

class ApplicationContext {
 public:
  ApplicationContext();
  ApplicationContext(const std::string& context_id);
  ApplicationContext(const ApplicationContextPtr);
  ~ApplicationContext();

  const picojson::value& Value();
  bool IsValid() const;

  std::string get_context_id();
  void set_context_id(const std::string& context_id);
 
  std::string get_app_id();
  void set_app_id(const std::string& app_id);
 

 private:
  std::string context_id_;
  std::string app_id_;

  picojson::object data_;
  picojson::object error_;
  picojson::value value_;
};
} // namespace application
} // namespace extension

#endif  // APPLICATION_APPLICATION_CONTEXT_H_
