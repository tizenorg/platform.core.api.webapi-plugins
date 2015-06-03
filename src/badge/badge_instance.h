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
 
#ifndef BADGE_BADGE_INSTANCE_H_
#define BADGE_BADGE_INSTANCE_H_

#include "badge/badge_manager.h"

#include "common/extension.h"

namespace extension {
namespace badge {

class BadgeInstance : public common::ParsedInstance {
 public:
  BadgeInstance();
  virtual ~BadgeInstance();

 private:
  /**
   * Signature: @code void setBadgeCount(appId, count);
   * @endcode
   * JSON: @code data: {method: 'Badge_setBadgeCount',
   *                    args: {ApplicationId: appId, long: count}} @endcode
   * Invocation: @code native.callSync(request) @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   */
  void BadgeManagerSetBadgeCount(const JsonValue& args, JsonObject& out);

  /**
   * Signature: @code void getBadgeCount(appId);
   * @endcode
   * JSON: @code data: {method: 'Badge_getBadgeCount',
   *                    args: {ApplicationId: appId}} @endcode
   * Invocation: @code native.callSync(request) @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success', result: {count}}
   * @endcode
   */
  void BadgeManagerGetBadgeCount(const JsonValue& args, JsonObject& out);
  void BadgeManagerAddChangeListener(const JsonValue& args, JsonObject& out);
  void BadgeManagerRemoveChangeListener(const JsonValue& args, JsonObject& out);

  BadgeManager manager_;
};
}  // namespace badge
}  // namespace extension

#endif  // BADGE_BADGE_INSTANCE_H_
