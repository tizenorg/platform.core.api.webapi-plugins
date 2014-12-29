// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/logger.h"
#include "secureelement_seservice.h"

using namespace smartcard_service_api;

namespace extension {
namespace secureelement {

class SEServiceEventHandler : public SEServiceListener {
    void serviceConnected(SEServiceHelper *service, void *context) {
        LoggerD("Entered");
        if (context) {
            (static_cast<Service*>(context))->getSEServiceCompleted();
        }
    }

    void eventHandler(SEServiceHelper *service, char *seName, int event, void *context) {
        LoggerD("Entered");
        (static_cast<Service*>(context))->eventChanged(seName, event);
    }

    void errorHandler(SEServiceHelper *service, int error, void *context) {
        LoggerD("Entered");
    }
};


static SEServiceEventHandler SEEventHandler;


Service::Service() {
    LoggerD("Entered");
    m_seService = new SEService((void *)this, &SEEventHandler);
}


Service::~Service() {
    LoggerD("Entered");
    if ( m_seService != NULL) {
        m_seService->shutdownSync();
        delete m_seService;
        m_seService = NULL;
    }
}


void Service::getSEServiceCompleted() {
    LoggerD("Entered");
}


void Service::eventChanged(char *seName, int event) {
    LoggerD("Entered");
}

} // secureelement
} // extension
