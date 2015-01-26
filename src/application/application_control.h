// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_APPLICATION_APPLICATION_CONTROL_H_
#define SRC_APPLICATION_APPLICATION_CONTROL_H_

#include <string>
#include <memory>

#include "common/picojson.h"
#include "tizen/tizen.h"

#include "application/application_controldata.h"

namespace extension {
namespace application {

class ApplicationControl;
typedef std::shared_ptr<ApplicationControl> ApplicationControlPtr;

class ApplicationControl {
 public:
  ApplicationControl();
  ~ApplicationControl();

  const picojson::value& Value();
  bool IsValid() const;

  std::string get_operation() const;
  void set_operation(const std::string& operation);
  std::string get_uri() const;
  void set_uri(const std::string& uri);
  std::string get_mime() const;
  void set_mime(const std::string& mime);
  std::string get_category() const;
  void set_category(const std::string& category);
  ApplicationControlDataArray get_data_array() const;
  void set_data_array(const ApplicationControlDataArray& data_array);
  void add_data_array(const ApplicationControlDataPtr& data_ptr);

 private:
  std::string operation_;
  std::string uri_;
  std::string mime_;
  std::string category_;
  ApplicationControlDataArray data_array_;

  picojson::object data_;
  picojson::object error_;
  picojson::value value_;
};

}  // namespace application
}  // namespace extension

#endif  // SRC_APPLICATION_APPLICATION_CONTROL_H_
