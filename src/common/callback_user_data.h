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

// Class added for backward compatibility with WRT plugins.
// TODO To be cleaned/replaced in the future.

#ifndef COMMON_CALLBACK_USER_DATA_H_
#define COMMON_CALLBACK_USER_DATA_H_

#include "common/picojson.h"

#include <memory>

namespace common {

class CallbackUserData {
public:
  CallbackUserData();

  virtual ~CallbackUserData();

  void setActive(bool act);
  bool isActive() const;

  void setJson(std::shared_ptr<picojson::value> json);
  std::shared_ptr<picojson::value> getJson() const;

  virtual void setError(const std::string& err_name,
                        const std::string& err_message) = 0;

protected:
  std::shared_ptr<picojson::value> m_json;

private:
  bool m_is_act;
};

} // common

#endif  // COMMON_CALLBACK_USER_DATA_H_
