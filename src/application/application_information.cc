// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "application/application_information.h"

#include <sstream>
#include <vector>

#include "common/logger.h"
#include "tizen/tizen.h"

namespace extension {
namespace application {

ApplicationInformation::ApplicationInformation() {
}

ApplicationInformation::~ApplicationInformation() {
}

std::string ApplicationInformation::get_app_id() const {
  return app_id_;
}

void ApplicationInformation::set_app_id(const std::string &app_id) {
  app_id_ = app_id;
}

std::string ApplicationInformation::get_name() const {
  return name_;
}

void ApplicationInformation::set_name(const std::string &name) {
  name_ = name;
}

std::string ApplicationInformation::get_icon_path() const {
  return icon_path_;
}

void ApplicationInformation::set_icon_path(const std::string &icon_path) {
  icon_path_ = icon_path;
}

bool ApplicationInformation::get_show() const {
  return show_;
}

void ApplicationInformation::set_show(const bool show) {
  show_ = show;
}

std::string ApplicationInformation::get_package_id() const {
  return package_id_;
}

void ApplicationInformation::set_package_id(const std::string &package_id) {
  package_id_ = package_id;
}

std::string ApplicationInformation::get_version() const {
  return version_;
}

void ApplicationInformation::set_version(const std::string &version) {
  version_ = version;
}

double ApplicationInformation::get_install_date() const {
  return install_date_;
}

void ApplicationInformation::set_install_date(const time_t &install_date) {
  install_date_ = static_cast<double>(install_date);

  // pkgmgrinfo_pkginfo_get_installed_time() returns installed time
  // by using int type. but, it can't have millisecond value fully
  install_date_ = install_date_ * 1000;
}

std::vector<std::string> ApplicationInformation::get_categories() const {
  return categories_;
}

void ApplicationInformation::set_categories
  (const std::vector<std::string> &categories) {
  categories_ = categories;
}

void ApplicationInformation::add_categories(const std::string &category) {
  categories_.push_back(category);
}

void ApplicationInformation::set_size(const int &size) {
  size_ = static_cast<double>(size);
}

double ApplicationInformation::get_size() const {
  return size_;
}

const picojson::value& ApplicationInformation::Value() {
  if (value_.is<picojson::null>() && IsValid()) {
    data_["id"] = picojson::value(app_id_);
    data_["name"] = picojson::value(name_);
    data_["iconPath"] = picojson::value(icon_path_);
    data_["show"] = picojson::value(show_);
    data_["packageId"] = picojson::value(package_id_);
    data_["version"] = picojson::value(version_);
    data_["installDate"] = picojson::value(install_date_);
    picojson::value categories = picojson::value(picojson::array());
    picojson::array& categories_array = categories.get<picojson::array>();
    for (auto it = categories_.begin(); it != categories_.end(); it++) {
      categories_array.push_back(picojson::value(*it));
    }
    data_["categories"] = categories;
    LoggerD("size: %f", size_);
    data_["size"] = picojson::value(size_);

    value_ = picojson::value(data_);
  }
  return value_;
}

bool ApplicationInformation::IsValid() const {
  return error_.empty();
}

}  // namespace application
}  // namespace extension
