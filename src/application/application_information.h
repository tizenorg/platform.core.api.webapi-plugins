// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_APPLICATION_APPLICATION_INFORMATION_H_
#define SRC_APPLICATION_APPLICATION_INFORMATION_H_

#include <string>
#include <memory>
#include <vector>

#include "common/picojson.h"

namespace extension {
namespace application {

class ApplicationInformation;
typedef std::shared_ptr<ApplicationInformation> ApplicationInformationPtr;

typedef std::vector<ApplicationInformationPtr> ApplicationInformationArray;
typedef std::shared_ptr<ApplicationInformationArray>
  ApplicationInformationArrayPtr;

class ApplicationInformation {
 public:
  ApplicationInformation();
  ~ApplicationInformation();

  const picojson::value& Value();
  bool IsValid() const;

  std::string get_app_id() const;
  void set_app_id(const std::string &app_id);
  std::string get_name() const;
  void set_name(const std::string &name);
  std::string get_icon_path() const;
  void set_icon_path(const std::string &icon_path);
  bool get_show() const;
  void set_show(const bool show);
  void set_package_id(const std::string &package_id);
  std::string get_package_id() const;
  void set_version(const std::string &version);
  std::string get_version() const;
  void set_install_date(const time_t& install_date);
  double get_install_date() const;
  std::vector<std::string> get_categories() const;
  void set_categories(const std::vector<std::string> &categories);
  void add_categories(const std::string &category);
  void set_size(const int& size);
  double get_size() const;

 private:
  std::string app_id_;
  std::string name_;
  std::string icon_path_;
  bool show_;
  std::string package_id_;
  std::string version_;
  double install_date_;
  std::vector<std::string> categories_;
  double size_;

  picojson::object data_;
  picojson::object error_;
  picojson::value value_;
};

}  // namespace application
}  // namespace extension

#endif  // SRC_APPLICATION_APPLICATION_INFORMATION_H_
