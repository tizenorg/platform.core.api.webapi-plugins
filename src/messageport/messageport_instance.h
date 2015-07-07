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

#ifndef MESSAGEPORT_MESSAGEPORT_INSTANCE_H_
#define MESSAGEPORT_MESSAGEPORT_INSTANCE_H_

#include <bundle.h>
#include <bundle_internal.h>
#include <message_port.h>

#include "common/extension.h"

namespace extension {
namespace messageport {

class MessageportInstance : public common::ParsedInstance {
 public:
  MessageportInstance();
  virtual ~MessageportInstance();

 private:
  void MessagePortManagerRequestlocalmessageport
    (const picojson::value& args, picojson::object& out);
  void MessagePortManagerRequesttrustedlocalmessageport
    (const picojson::value& args, picojson::object& out);
  void MessagePortManagerRequestremotemessageport
    (const picojson::value& args, picojson::object& out);
  void MessagePortManagerRequesttrustedremotemessageport
    (const picojson::value& args, picojson::object& out);
  void RemoteMessagePortSendmessage
    (const picojson::value& args, picojson::object& out);
};

}  // namespace messageport
}  // namespace extension

#endif  // MESSAGEPORT_MESSAGEPORT_INSTANCE_H_
