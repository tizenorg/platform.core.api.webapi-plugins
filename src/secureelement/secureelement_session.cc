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

#include <Session.h>
#include <ClientChannel.h>
#include "common/picojson.h"
#include "common/logger.h"
#include "secureelement_session.h"

using namespace smartcard_service_api;

namespace extension {
namespace secureelement {

picojson::value SESession::openBasicChannel(const picojson::array& v_aid) {
    LoggerD("Entered");

    size_t v_aid_size = v_aid.size();
    unsigned char* aid = new unsigned char[v_aid_size];
    for (size_t i = 0; i < v_aid_size; i++) {
        aid[i] = static_cast<unsigned char>(v_aid[i].get<double>());
    }
    ByteArray aid_byte_array( aid, (unsigned int) v_aid.size());
    ClientChannel* channel_ptr = static_cast<ClientChannel*>(m_session_ptr->openBasicChannelSync( aid_byte_array));
    delete [] aid;

    picojson::value result = picojson::value(picojson::object());
    picojson::object& obj = result.get<picojson::object>();
    obj.insert(std::make_pair("handle", picojson::value((double) (long) channel_ptr)));
    obj.insert(std::make_pair("isBasicChannel", picojson::value(channel_ptr->isBasicChannel())));

    return result;
}


picojson::value SESession::openLogicalChannel(const picojson::array& v_aid) {
    LoggerD("Entered");

    size_t v_aid_size = v_aid.size();
    unsigned char* aid = new unsigned char[v_aid_size];
    for (size_t i = 0; i < v_aid_size; i++) {
        aid[i] = static_cast<unsigned char>(v_aid[i].get<double>());
    }
    ByteArray aid_byte_array( aid, (unsigned int) v_aid.size());
    ClientChannel* channel_ptr = static_cast<ClientChannel*>(m_session_ptr->openLogicalChannelSync( aid_byte_array));
    delete [] aid;

    picojson::value result = picojson::value(picojson::object());
    picojson::object& obj = result.get<picojson::object>();
    obj.insert(std::make_pair("handle", picojson::value((double) (long) channel_ptr)));
    obj.insert(std::make_pair("isBasicChannel", picojson::value(channel_ptr->isBasicChannel())));

    return result;
}


picojson::value SESession::isClosed() {
    LoggerD("Entered");
    bool is_closed = m_session_ptr->isClosed();
    picojson::value result = picojson::value(picojson::object());
    picojson::object& obj = result.get<picojson::object>();
    obj.insert(std::make_pair("isClosed", picojson::value(is_closed)));
    return result;
}


void SESession::close() {
    LoggerD("Entered");
    if ( m_session_ptr) {
        m_session_ptr->closeSync();
    }
}


ByteArray SESession::getATR() {
    LoggerD("Entered");
    ByteArray response;
    if ( m_session_ptr) {
        response = m_session_ptr->getATRSync();
    }
    return response;
}


void SESession::closeChannels() {
    LoggerD("Entered");
    if ( m_session_ptr) {
        m_session_ptr->closeChannels();
    }
}


} // secureelement
} // extension
