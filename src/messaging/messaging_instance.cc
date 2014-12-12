
// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "messaging_instance.h"

#include <sstream>

#include "common/logger.h"

#include "messaging_manager.h"
#include "messaging_util.h"

namespace extension {
namespace messaging {

namespace{
const char* FUN_MESSAGING_GET_MESSAGE_SERVICES = "Messaging_getMessageServices";
const char* FUN_ARGS_MESSAGE_SERVICE_TYPE = "messageServiceType";
}

MessagingInstance& MessagingInstance::getInstance()
{
    static MessagingInstance instance;
    return instance;
}

MessagingInstance::MessagingInstance()
{
    LoggerD("Entered");
    using namespace std::placeholders;
    #define REGISTER_ASYNC(c,x) \
      RegisterHandler(c, std::bind(&MessagingInstance::x, this, _1, _2));
      REGISTER_ASYNC(FUN_MESSAGING_GET_MESSAGE_SERVICES, GetMessageServices);
    #undef REGISTER_ASYNC
}

MessagingInstance::~MessagingInstance()
{
    LoggerD("Entered");
}

void MessagingInstance::GetMessageServices(const picojson::value& args,
        picojson::object& out)
{
    LoggerD("Entered");

    picojson::object data = args.get(JSON_DATA).get<picojson::object>();
    picojson::value serviceTag = data.at(FUN_ARGS_MESSAGE_SERVICE_TYPE);
    const double callbackId = args.get(JSON_CALLBACK_ID).get<double>();

    // above values should be validated in js
    MessagingManager::getInstance().getMessageServices(serviceTag.to_str(), callbackId);
}

} // namespace messaging
} // namespace extension

