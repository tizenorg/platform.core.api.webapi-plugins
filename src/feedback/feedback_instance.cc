/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "feedback/feedback_instance.h"
#include "feedback_manager.h"

#include <functional>
#include <string>

#include "common/picojson.h"
#include "common/logger.h"
#include "common/picojson.h"
#include "common/platform_exception.h"
#include "common/platform_result.h"
#include "common/tools.h"

namespace extension {
namespace feedback {

using namespace common;

using common::TypeMismatchException;
using common::InvalidValuesException;
using common::UnknownException;
using common::NotFoundException;
using common::QuotaExceededException;

namespace {
const std::string kPrivilegeHaptic = "http://tizen.org/privilege/haptic";
} //namespace

FeedbackInstance::FeedbackInstance()
    : m_feedbackMapsPtr(new FeedbackMaps),
      m_feedbackManagerPtr(new FeedbackManager(this->m_feedbackMapsPtr))
  {
  LoggerD("Enter");
  using std::placeholders::_1;
  using std::placeholders::_2;
  #define REGISTER_SYNC(c, x) \
    RegisterSyncHandler(c, std::bind(&FeedbackInstance::x, this, _1, _2));
  REGISTER_SYNC("FeedbackManager_isPatternSupported",
                IsPatternSupported);
  REGISTER_SYNC("FeedbackManager_play",
                Play);
  REGISTER_SYNC("FeedbackManager_stop",
                Stop);
  #undef REGISTER_SYNC
}

FeedbackInstance::~FeedbackInstance() {
  LoggerD("Enter");
}

void FeedbackInstance::IsPatternSupported
  (const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");

  const auto pattern = args.get("pattern").get<std::string>();
  const auto type = args.get("type").get<std::string>();

  bool patternStatus = false;
  PlatformResult result =
      m_feedbackManagerPtr->isPatternSupported(pattern, type, &patternStatus);
  if (result.IsSuccess()) {
    ReportSuccess(picojson::value(patternStatus), out);
  } else {
    LogAndReportError(result, &out);
  }
}

void FeedbackInstance::Play
  (const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");

  CHECK_PRIVILEGE_ACCESS(kPrivilegeHaptic, &out);

  const auto pattern = args.get("pattern").get<std::string>();
  const auto type = args.get("type").get<std::string>();

  PlatformResult result =
      m_feedbackManagerPtr->play(pattern, type);
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LogAndReportError(result, &out);
  }
}

void FeedbackInstance::Stop
  (const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");

  CHECK_PRIVILEGE_ACCESS(kPrivilegeHaptic, &out);

  PlatformResult result =
      m_feedbackManagerPtr->stop();
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LogAndReportError(result, &out);
  }
}

#undef CHECK_EXIST

}  // namespace messageport
}  // namespace extension
