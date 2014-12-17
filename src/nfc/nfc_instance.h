// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NFC_NFC_INSTANCE_H_
#define NFC_NFC_INSTANCE_H_

#include "common/extension.h"

#include "nfc_adapter.h"

namespace extension {
namespace nfc {

class NFCInstance: public common::ParsedInstance {
public:
    static NFCInstance& getInstance();

    void InstanceReportSuccess(picojson::object& out);
    void InstanceReportSuccess(const picojson::value& result, picojson::object& out);
    void InstanceReportError(picojson::object& out);
    void InstanceReportError(const common::PlatformException& ex, picojson::object& out);
private:
    NFCInstance();
    virtual ~NFCInstance();

    void GetDefaultAdapter(const picojson::value& args, picojson::object& out);
    void SetExclusiveMode(const picojson::value& args, picojson::object& out);
    void SetPowered(const picojson::value& args, picojson::object& out);
    void GetPowered(const picojson::value& args, picojson::object& out);
    void SetTagListener(const picojson::value& args, picojson::object& out);
    void SetPeerListener(const picojson::value& args, picojson::object& out);
    void UnsetTagListener(const picojson::value& args, picojson::object& out);
    void UnsetPeerListener(const picojson::value& args, picojson::object& out);
    void AddCardEmulationModeChangeListener(const picojson::value& args, picojson::object& out);
    void RemoveCardEmulationModeChangeListener(const picojson::value& args, picojson::object& out);
    void AddTransactionEventListener(const picojson::value& args, picojson::object& out);
    void RemoveTransactionEventListener(const picojson::value& args, picojson::object& out);
    void AddActiveSecureElementChangeListener(const picojson::value& args, picojson::object& out);
    void RemoveActiveSecureElementChangeListener(const picojson::value& args, picojson::object& out);
    void GetCachedMessage(const picojson::value& args, picojson::object& out);
    void SetExclusiveModeForTransaction(const picojson::value& args, picojson::object& out);
    void ReadNDEF(const picojson::value& args, picojson::object& out);
    void WriteNDEF(const picojson::value& args, picojson::object& out);
    void Transceive(const picojson::value& args, picojson::object& out);
    void SetReceiveNDEFListener(const picojson::value& args, picojson::object& out);
    void UnsetReceiveNDEFListener(const picojson::value& args, picojson::object& out);
    void SendNDEF(const picojson::value& args, picojson::object& out);
    void ToByte(const picojson::value& args, picojson::object& out);

};

} // namespace nfc
} // namespace extension

#endif // NFC_NFC_INSTANCE_H_
