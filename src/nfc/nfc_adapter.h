// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NFC_NFC_ADAPTER_H_
#define NFC_NFC_ADAPTER_H_

namespace extension {
namespace nfc {

class NFCAdapter {
public:
    bool GetPowered();

    static NFCAdapter* GetInstance();
private:
    NFCAdapter();
    virtual ~NFCAdapter();
};

} // nfc
} // extension

#endif // NFC_NFC_ADAPTER_H_
