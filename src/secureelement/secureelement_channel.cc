// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/picojson.h"
#include "common/logger.h"
#include "secureelement_channel.h"

using namespace smartcard_service_api;

namespace extension {
namespace secureelement {

void SEChannel::close() {
    LoggerD("Entered");
    if (m_channel_ptr && !m_channel_ptr->isClosed()) {
        m_channel_ptr->closeSync();
    }
}


ByteArray SEChannel::transmit(const picojson::array& v_command) {
    LoggerD("Entered");
    ByteArray response;
    if ( m_channel_ptr) {
        uint8_t* command = new uint8_t[v_command.size()];
        for ( int i = 0; i < v_command.size(); i++) {
            command[i] = (uint8_t) static_cast<long>(v_command[i].get<double>());
        }
        ByteArray ba_command(command, v_command.size());
        delete [] command;
        m_channel_ptr->transmitSync( ba_command, response);
    }
    return response;
}


ByteArray SEChannel::getSelectResponse() {
    LoggerD("Entered");
    ByteArray response;
    if ( m_channel_ptr) {
        response = m_channel_ptr->getSelectResponse();
    }
    return response;
}

} // secureelement
} // extension
