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

#ifndef FEEDBACK_FEEDBACK_INSTANCE_H_
#define FEEDBACK_FEEDBACK_INSTANCE_H_

#include "common/extension.h"
#include <memory>

namespace extension {
namespace feedback {

class FeedbackMaps;
class FeedbackManager;

class FeedbackInstance : public common::ParsedInstance {
 public:
  FeedbackInstance();
  virtual ~FeedbackInstance();

 private:
  std::shared_ptr<FeedbackMaps> m_feedbackMapsPtr;
  std::unique_ptr<FeedbackManager> m_feedbackManagerPtr;
  void IsPatternSupported
    (const picojson::value& args, picojson::object& out);
  void Play
    (const picojson::value& args, picojson::object& out);
  void Stop
    (const picojson::value& args, picojson::object& out);
};

}  // namespace feedback
}  // namespace extension

#endif  // FEEDBACK_FEEDBACK_INSTANCE_H_
