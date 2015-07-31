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
        size_t v_command_size = v_command.size();
        uint8_t* command = new uint8_t[v_command_size];
        for (size_t i = 0; i < v_command_size; i++) {
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
