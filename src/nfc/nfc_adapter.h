// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NFC_NFC_ADAPTER_H_
#define NFC_NFC_ADAPTER_H_

#include "nfc/nfc_instance.h"

#include "common/picojson.h"
#include <memory>

#ifdef APP_CONTROL_SETTING_SUPPORT
#include <app_control.h>
#endif

namespace extension {
namespace nfc {

class NFCInstance;

class NFCAdapter {
public:
    bool GetPowered();
    void SetPowered(const picojson::value& args);

// cardEmulationModer getter and setter
    std::string GetCardEmulationMode();
    void SetCardEmulationMode(std::string mode);


    static NFCAdapter* GetInstance();
    NFCInstance *xwalk_instance;
private:
    NFCAdapter();
    virtual ~NFCAdapter();
};

} // nfc
} // extension

#endif // NFC_NFC_ADAPTER_H_
