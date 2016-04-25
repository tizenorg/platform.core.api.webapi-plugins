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

#include <string>
#include <feedback/feedback_manager.h>

#include "common/assert.h"
#include "common/converter.h"
#include "common/extension.h"
#include "common/logger.h"
#include "common/platform_exception.h"
#include "common/picojson.h"
#include "common/platform_result.h"
#include "common/tools.h"

using namespace common;
using namespace std;
using common::tools::ReportError;

namespace extension {
namespace feedback {

FeedbackMaps::FeedbackMaps() :
  mTypeMap{
    {"TYPE_SOUND", FEEDBACK_TYPE_SOUND},
    {"TYPE_VIBRATION", FEEDBACK_TYPE_VIBRATION}
  },
  mPatternMap{
    {"TAP", FEEDBACK_PATTERN_TAP},
    {"SIP", FEEDBACK_PATTERN_SIP},
    {"KEY0", FEEDBACK_PATTERN_KEY0},
    {"KEY1", FEEDBACK_PATTERN_KEY1},
    {"KEY2", FEEDBACK_PATTERN_KEY2},
    {"KEY3", FEEDBACK_PATTERN_KEY3},
    {"KEY4", FEEDBACK_PATTERN_KEY4},
    {"KEY5", FEEDBACK_PATTERN_KEY5},
    {"KEY6", FEEDBACK_PATTERN_KEY6},
    {"KEY7", FEEDBACK_PATTERN_KEY7},
    {"KEY8", FEEDBACK_PATTERN_KEY8},
    {"KEY9", FEEDBACK_PATTERN_KEY9},
    {"KEY_STAR", FEEDBACK_PATTERN_KEY_STAR},
    {"KEY_SHARP", FEEDBACK_PATTERN_KEY_SHARP},
    {"KEY_BACK", FEEDBACK_PATTERN_KEY_BACK},
    {"HOLD", FEEDBACK_PATTERN_HOLD},
    {"HW_TAP", FEEDBACK_PATTERN_HW_TAP},
    {"HW_HOLD", FEEDBACK_PATTERN_HW_HOLD},
    {"MESSAGE", FEEDBACK_PATTERN_MESSAGE},
    {"EMAIL", FEEDBACK_PATTERN_EMAIL},
    {"WAKEUP", FEEDBACK_PATTERN_WAKEUP},
    {"SCHEDULE", FEEDBACK_PATTERN_SCHEDULE},
    {"TIMER", FEEDBACK_PATTERN_TIMER},
    {"GENERAL", FEEDBACK_PATTERN_GENERAL},
    {"POWERON", FEEDBACK_PATTERN_POWERON},
    {"POWEROFF", FEEDBACK_PATTERN_POWEROFF},
    {"CHARGERCONN", FEEDBACK_PATTERN_CHARGERCONN},
    {"CHARGING_ERROR", FEEDBACK_PATTERN_CHARGING_ERROR},
    {"FULLCHARGED", FEEDBACK_PATTERN_FULLCHARGED},
    {"LOWBATT", FEEDBACK_PATTERN_LOWBATT},
    {"LOCK", FEEDBACK_PATTERN_LOCK},
    {"UNLOCK", FEEDBACK_PATTERN_UNLOCK},
    {"VIBRATION_ON", FEEDBACK_PATTERN_VIBRATION_ON},
    {"SILENT_OFF", FEEDBACK_PATTERN_SILENT_OFF},
    {"BT_CONNECTED", FEEDBACK_PATTERN_BT_CONNECTED},
    {"BT_DISCONNECTED", FEEDBACK_PATTERN_BT_DISCONNECTED},
    {"LIST_REORDER", FEEDBACK_PATTERN_LIST_REORDER},
    {"LIST_SLIDER", FEEDBACK_PATTERN_LIST_SLIDER},
    {"VOLUME_KEY", FEEDBACK_PATTERN_VOLUME_KEY}
  }
{};

FeedbackMaps::~FeedbackMaps()
{}

feedback_pattern_e const & FeedbackMaps::getPatternFromMap(const std::string& pattern) {
  return mPatternMap[pattern];
}

feedback_type_e const & FeedbackMaps::getTypeFromMap(const std::string& type) {
  return mTypeMap[type];
}

FeedbackManager::FeedbackManager(std::shared_ptr<FeedbackMaps> maps)
  : m_feedbackMapsPtr(maps)
{
  LoggerD("Entered");
  // feedback API initialization
  int ret = feedback_initialize();
  if(ret != FEEDBACK_ERROR_NONE) {
    LoggerE("Could not initialize Feedback Manager, error: %d", ret);
  }

}

FeedbackManager::~FeedbackManager() {
  LoggerD("Entered");

  // feedback library deinitialization
  int ret = feedback_deinitialize();
  if(ret != FEEDBACK_ERROR_NONE) {
    LoggerE("Could not deinitialize Feedback Manager, error: %d", ret);
  }
}

common::PlatformResult
FeedbackManager::isPatternSupported(const std::string &pattern, const std::string &type, bool* patternStatus) {
  LoggerD("Entered");
  int ret = feedback_is_supported_pattern(
    m_feedbackMapsPtr->getTypeFromMap(type),
    m_feedbackMapsPtr->getPatternFromMap(pattern),
    patternStatus
    );
  if(ret != FEEDBACK_ERROR_NONE) {
    LoggerE("isPatternSupported failed: %d", ret);
    return CodeToResult(ret, getFeedbackErrorMessage(ret).c_str());
    }
  return PlatformResult(ErrorCode::NO_ERROR);
}

common::PlatformResult FeedbackManager::play(const std::string &pattern, const std::string &type) {
  LoggerD("Entered");
  int ret = 0;
  if(type == "any") {
    ret = feedback_play(m_feedbackMapsPtr->getPatternFromMap(pattern));
  } else {
    ret = feedback_play_type(m_feedbackMapsPtr->getTypeFromMap(type),
        m_feedbackMapsPtr->getPatternFromMap(pattern));
  }
  if(ret != FEEDBACK_ERROR_NONE) {
    LoggerE("play failed: %d", ret);
    return CodeToResult(ret, getFeedbackErrorMessage(ret).c_str());
    }
  return PlatformResult(ErrorCode::NO_ERROR);
}

common::PlatformResult FeedbackManager::stop() {
  LoggerD("Entered");
  int ret = feedback_stop();
  if(ret != FEEDBACK_ERROR_NONE && ret != FEEDBACK_ERROR_NOT_SUPPORTED) {
    LoggerE("stop failed: %d", ret);
    return CodeToResult(ret, getFeedbackErrorMessage(ret).c_str());
    }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult FeedbackManager::CodeToResult(const int errorCode,
                                     const std::string& message) {
  LoggerD("Entered");
  switch(errorCode) {
    case FEEDBACK_ERROR_INVALID_PARAMETER:
      return LogAndCreateResult(ErrorCode::INVALID_VALUES_ERR, message);
    case FEEDBACK_ERROR_OPERATION_FAILED:
      return LogAndCreateResult(ErrorCode::SECURITY_ERR, message);
    case FEEDBACK_ERROR_NOT_SUPPORTED:
      return LogAndCreateResult(ErrorCode::NOT_SUPPORTED_ERR, message);
    case FEEDBACK_ERROR_NOT_INITIALIZED:
    default:
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, message);
  }
}

const std::string FeedbackManager::getFeedbackErrorMessage(const int error_code) {
  LoggerD("Error code : %d", error_code);
  switch(error_code) {
    case FEEDBACK_ERROR_OPERATION_FAILED:
      return "Operation not permitted";
    case FEEDBACK_ERROR_INVALID_PARAMETER:
      return "Invalid parameter";
    case FEEDBACK_ERROR_NOT_SUPPORTED:
      return "Not supported device";
    case FEEDBACK_ERROR_NOT_INITIALIZED:
      return "Not initialized";
    default:
      return "UnknownError";
  }
}

}// feedback
}// extension
