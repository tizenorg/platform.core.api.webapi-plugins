// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
  static BadgeInstance& GetInstance();

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
  void setBadgeCount(const JsonValue& args, JsonObject& out);

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
  void getBadgeCount(const JsonValue& args, JsonObject& out);
  void addChangeListener(const JsonValue& args, JsonObject& out);
  void removeChangeListener(const JsonValue& args, JsonObject& out);

};
}  // namespace badge
}  // namespace extension

#endif  // BADGE_BADGE_INSTANCE_H_
