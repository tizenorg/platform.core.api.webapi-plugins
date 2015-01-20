// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPLICATION_APPLICATION_METADATA_H_
#define APPLICATION_APPLICATION_METADATA_H_

#include <string>
#include <memory>

#include "common/picojson.h"
#include "tizen/tizen.h"

namespace extension {
namespace application {

class ApplicationMetaData;
typedef std::shared_ptr<ApplicationMetaData> ApplicationMetaDataPtr;

typedef std::vector<ApplicationMetaDataPtr> ApplicationMetaDataArray;
typedef std::shared_ptr<ApplicationMetaDataArray> ApplicationMetaDataArrayPtr;

class ApplicationMetaData {
 public:
  ApplicationMetaData();
  ApplicationMetaData(const ApplicationMetaDataPtr);
  ~ApplicationMetaData();

  const picojson::value& Value();
  bool IsValid() const;

  std::string get_meta_key() const;
  void set_meta_key(const std::string& meta_key);
 
  std::string get_meta_value() const;
  void set_meta_value(const std::string& meta_value);
 

 private:
  std::string meta_key_;
  std::string meta_value_;

  picojson::object data_;
  picojson::object error_;
  picojson::value value_;
};
} // namespace application
} // namespace extension

#endif  // APPLICATION_APPLICATION_METADATA_H_
