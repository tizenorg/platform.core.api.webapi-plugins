//
// Tizen Web Device API
// Copyright (c) 2013 Samsung Electronics Co., Ltd.
//
// Licensed under the Apache License, Version 2.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

/**
 * @file        MessageSMS.cpp
 */

#include <iterator>
#include <PlatformException.h>
#include <Logger.h>
#include "MessageSMS.h"

namespace DeviceAPI {
namespace Messaging {

MessageSMS::MessageSMS():
    Message()
{
    LOGD("MessageSMS constructor.");
    this->m_type = MessageType(MessageType(SMS));
}

MessageSMS::~MessageSMS()
{
    LOGD("MessageSMS destructor.");
}

} // Messaging
} // DeviceAPI
