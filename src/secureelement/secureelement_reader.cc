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
