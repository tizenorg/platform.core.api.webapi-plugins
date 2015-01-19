// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SECUREELEMENT_SESERVICE_H_
#define SECUREELEMENT_SESERVICE_H_

#include <SEService.h>
#include "common/logger.h"

namespace extension {
namespace secureelement {

class Service {
public:
    Service();
    ~Service();
    void getSEServiceCompleted();
    void eventChanged(char *seName, int event);
private:
    smartcard_service_api::SEService *m_seService;
};

} // secureelement
} // extension

#endif // SECUREELEMENT_SESERVICE_H_
