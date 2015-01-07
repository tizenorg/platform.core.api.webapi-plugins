// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
