// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "nfc_adapter.h"

#include <nfc.h>

#include "common/logger.h"
#include "common/platform_exception.h"

using namespace common;
using namespace std;

namespace extension {
namespace nfc {

NFCAdapter::NFCAdapter() {

}

NFCAdapter::~NFCAdapter() {

}

NFCAdapter* NFCAdapter::GetInstance(){
    static NFCAdapter instance;
    return &instance;
}

bool NFCAdapter::GetPowered() {
    return nfc_manager_is_activated();
}

}// nfc
}// extension
