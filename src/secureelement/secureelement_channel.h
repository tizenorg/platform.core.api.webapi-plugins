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

#ifndef SECUREELEMENT_CHANNEL_H_
#define SECUREELEMENT_CHANNEL_H_

#include <ClientChannel.h>
#include "common/picojson.h"

namespace extension {
namespace secureelement {

class SEChannel {
public:
    SEChannel( smartcard_service_api::ClientChannel* channel_ptr) : m_channel_ptr(channel_ptr) {};
    virtual ~SEChannel() {};
    void close();
    smartcard_service_api::ByteArray transmit(const picojson::array& v_command);
    smartcard_service_api::ByteArray getSelectResponse();
private:
    smartcard_service_api::ClientChannel* m_channel_ptr;
};

} // secureelement
} // extension

#endif // SECUREELEMENT_CHANNEL_H_
