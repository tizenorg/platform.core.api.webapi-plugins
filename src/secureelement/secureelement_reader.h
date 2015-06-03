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

#ifndef SECUREELEMENT_READER_H_
#define SECUREELEMENT_READER_H_

#include "common/picojson.h"
#include <Reader.h>

namespace extension {
namespace secureelement {

class SEReader {
public:
    SEReader(smartcard_service_api::Reader* reader_ptr) : m_reader(reader_ptr) {};
    ~SEReader() {};

    picojson::value getName();
    picojson::value isPresent();
    picojson::value openSession();
    void closeSessions();
private:
    smartcard_service_api::Reader* m_reader;
};

} // secureelement
} // extension

#endif // SECUREELEMENT_READER_H_
