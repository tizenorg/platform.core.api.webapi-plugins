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

#ifndef FEEDBACK_MANAGER_H_
#define FEEDBACK_MANAGER_H_

#include "common/picojson.h"
#include "common/platform_result.h"

#include <feedback.h>
#include <memory>
#include <string>

namespace extension {
namespace feedback {

using stringPatternMap = std::map<std::string, feedback_pattern_e>;
using stringTypeMap = std::map<std::string, feedback_type_e>;

class FeedbackMaps {
 public:
  FeedbackMaps();
  virtual ~FeedbackMaps();
  feedback_pattern_e const & getPatternFromMap(const std::string& pattern);
  feedback_type_e const & getTypeFromMap(const std::string& type);
 private:
  stringTypeMap mTypeMap;
  stringPatternMap mPatternMap;
};

class FeedbackManager {
 public:
  FeedbackManager(std::shared_ptr<FeedbackMaps> maps);
  virtual ~FeedbackManager();

  common::PlatformResult isPatternSupported(
      const std::string &pattern, const std::string &type, bool* patternStatus);
  common::PlatformResult play(const std::string &pattern, const std::string &type);
  common::PlatformResult stop();
 private:
  const std::string getFeedbackErrorMessage(const int error_code);
  common::PlatformResult CodeToResult(
      const int errorCode, const std::string& message);
  std::shared_ptr<FeedbackMaps> m_feedbackMapsPtr;
};

} // feedback
} // extension

#endif // FEEDBACK_MANAGER_H_
