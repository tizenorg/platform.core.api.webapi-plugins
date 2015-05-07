// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
