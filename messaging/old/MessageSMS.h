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
 * @file        MessageSMS.h
 */

#ifndef __TIZEN_MESSAGE_SMS_H__
#define __TIZEN_MESSAGE_SMS_H__

// Header with core msg-service declarations
#include <msg.h>

#include "Message.h"

namespace DeviceAPI {
namespace Messaging {

class MessageSMS: public Message {
public:
// constructor
    MessageSMS();
    ~MessageSMS();

};

} // Messaging
} // DeviceAPI

#endif // __TIZEN_MESSAGE_SMS_H__
