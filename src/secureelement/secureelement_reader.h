// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
