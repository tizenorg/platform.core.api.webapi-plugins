// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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

    unsigned char* aid = new unsigned char[v_aid.size()];
    for ( int i = 0; i < v_aid.size(); i++) {
        aid[i] = static_cast<unsigned char>(v_aid[i].get<double>());
    }
    ByteArray aid_byte_array( aid, (unsigned int) v_aid.size());
    ClientChannel* channel_ptr = static_cast<ClientChannel*>(m_session_ptr->openBasicChannelSync( aid_byte_array));
    delete aid;

    picojson::value result = picojson::value(picojson::object());
    picojson::object& obj = result.get<picojson::object>();
    obj.insert(std::make_pair("handle", (double) (long) channel_ptr));
    obj.insert(std::make_pair("isBasicChannel", channel_ptr->isBasicChannel()));

    return result;
}


picojson::value SESession::openLogicalChannel(const picojson::array& v_aid) {
    LoggerD("Entered");

    unsigned char* aid = new unsigned char[v_aid.size()];
    for ( int i = 0; i < v_aid.size(); i++) {
        aid[i] = static_cast<unsigned char>(v_aid[i].get<double>());
    }
    ByteArray aid_byte_array( aid, (unsigned int) v_aid.size());
    ClientChannel* channel_ptr = static_cast<ClientChannel*>(m_session_ptr->openLogicalChannelSync( aid_byte_array));
    delete aid;

    picojson::value result = picojson::value(picojson::object());
    picojson::object& obj = result.get<picojson::object>();
    obj.insert(std::make_pair("handle", (double) (long) channel_ptr));
    obj.insert(std::make_pair("isBasicChannel", channel_ptr->isBasicChannel()));

    return result;
}


picojson::value SESession::isClosed() {
    LoggerD("Entered");
    bool is_closed = m_session_ptr->isClosed();
    picojson::value result = picojson::value(picojson::object());
    picojson::object& obj = result.get<picojson::object>();
    obj.insert(std::make_pair("isClosed", is_closed));
    return result;
}


void SESession::close() {
    if ( m_session_ptr) {
        m_session_ptr->closeSync();
    }
}


ByteArray SESession::getATR() {
    ByteArray response;
    if ( m_session_ptr) {
        response = m_session_ptr->getATRSync();
    }
    return response;
}


void SESession::closeChannels() {
    if ( m_session_ptr) {
        m_session_ptr->closeChannels();
    }
}


} // secureelement
} // extension
