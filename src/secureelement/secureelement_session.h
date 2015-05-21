// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
