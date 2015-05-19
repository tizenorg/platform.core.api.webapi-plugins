// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "secureelement_reader.h"

#include <Session.h>
#include <Reader.h>
#include "common/picojson.h"
#include "common/logger.h"

using namespace smartcard_service_api;
using namespace std;

namespace extension {
namespace secureelement {


picojson::value SEReader::getName() {
    LoggerD("Entered");

    picojson::value result = picojson::value(picojson::object());
    picojson::object& obj = result.get<picojson::object>();

    if(m_reader) {
        obj.insert(std::make_pair("name", picojson::value(m_reader->getName())));
    }

    return result;
}

picojson::value SEReader::isPresent() {
    LoggerD("Entered");

    picojson::value result = picojson::value(picojson::object());
    picojson::object& obj = result.get<picojson::object>();

    if(m_reader) {
        obj.insert(std::make_pair("isPresent", picojson::value(m_reader->isSecureElementPresent())));
    }

    return result;
}

picojson::value SEReader::openSession() {
    LoggerD("Entered");

    picojson::value result = picojson::value(picojson::object());
    picojson::object& obj = result.get<picojson::object>();

    if(m_reader) {
        Session *session_ptr = static_cast<Session*>(m_reader->openSessionSync());

        obj.insert(std::make_pair("handle", picojson::value((double) (long) session_ptr)));
        obj.insert(std::make_pair("isClosed", picojson::value(session_ptr->isClosed())));
    }

    return result;
}

void SEReader::closeSessions() {
    LoggerD("Entered");

    if(m_reader) {
        m_reader->closeSessions();
    }
}

}// secureelement
}// extension
