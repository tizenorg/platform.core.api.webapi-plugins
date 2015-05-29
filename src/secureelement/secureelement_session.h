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

#ifndef SECUREELEMENT_SESSION_H_
#define SECUREELEMENT_SESSION_H_

#include <Session.h>
#include "common/picojson.h"
#include "common/logger.h"

namespace extension {
namespace secureelement {

class SESession {
public:
    SESession(smartcard_service_api::Session* session_ptr) : m_session_ptr(session_ptr) {};
    ~SESession() {};
    picojson::value openBasicChannel( const picojson::array& v_aid);
    picojson::value openLogicalChannel( const picojson::array& v_aid);
    picojson::value isClosed();
    void close();
    smartcard_service_api::ByteArray getATR();
    void closeChannels();
private:
    smartcard_service_api::Session* m_session_ptr;
    void* m_user_data;
};

} // secureelement
} // extension

#endif // SECUREELEMENT_SESSION_H_
