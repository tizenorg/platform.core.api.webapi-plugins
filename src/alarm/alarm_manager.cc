// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "alarm_manager.h"

#include <app.h>
#include <app_alarm.h>

#include "common/logger.h"
#include "common/converter.h"
#include "common/platform_result.h"

#include "alarm_instance.h"
#include "alarm_utils.h"

using namespace common;
using namespace common::tools;

namespace extension {
namespace alarm {

AlarmManager::AlarmManager() {
}

AlarmManager::~AlarmManager() {
}

AlarmManager& AlarmManager::GetInstance() {
  static AlarmManager instance;
  return instance;
}

void AlarmManager::Add(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
}

void AlarmManager::Remove(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
}

void AlarmManager::RemoveAll(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
}

void AlarmManager::Get(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
}

void AlarmManager::GetAll(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
}

void AlarmManager::GetRemainingSeconds(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
}

void AlarmManager::GetNextScheduledDate(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
}

} // namespace alarm
} // namespace extension
