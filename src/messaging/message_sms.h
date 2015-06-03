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
 
#ifndef __MESSAGING_MESSAGE_SMS_H__
#define __MESSAGING_MESSAGE_SMS_H__

// Header with core msg-service declarations
#include <msg.h>

#include "message.h"

namespace extension {
namespace messaging {

class MessageSMS: public Message {
public:
// constructor
    MessageSMS();
    ~MessageSMS();

};

} // messaging
} // extension

#endif // __MESSAGING_MESSAGE_SMS_H__