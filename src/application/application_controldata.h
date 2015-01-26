// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_APPLICATION_APPLICATION_CONTROLDATA_H_
#define SRC_APPLICATION_APPLICATION_CONTROLDATA_H_

#include <string>
#include <memory>
#include <vector>

#include "common/picojson.h"
#include "tizen/tizen.h"

namespace extension {
namespace application {

class ApplicationControlData;
typedef std::shared_ptr<ApplicationControlData> ApplicationControlDataPtr;

typedef std::vector<ApplicationControlDataPtr> ApplicationControlDataArray;
typedef std::shared_ptr<ApplicationControlDataArray>
  ApplicationControlDataArrayPtr;

class ApplicationControlData {
 public:
  ApplicationControlData();
  ~ApplicationControlData();

  const picojson::value& Value();
  bool IsValid() const;

  std::string get_ctr_key() const;
  void set_ctr_key(const std::string& ctr_key);

  std::vector<std::string> get_ctr_value() const;
  void set_ctr_value(const std::vector<std::string>& ctr_values);
  void add_ctr_value(const std::string& ctr_value);

 private:
  std::string ctr_key_;
  std::vector<std::string> ctr_value_;

  picojson::object data_;
  picojson::object error_;
  picojson::value value_;
};

}  // namespace application
}  // namespace extension

#endif  // SRC_APPLICATION_APPLICATION_CONTROLDATA_H_
