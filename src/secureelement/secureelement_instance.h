// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SECUREELEMENT_SECUREELEMENT_INSTANCE_H_
#define SECUREELEMENT_SECUREELEMENT_INSTANCE_H_

#include "common/extension.h"

namespace extension {
namespace secureelement {

class SecureElementInstance: public common::ParsedInstance {
public:
    SecureElementInstance();
    virtual ~SecureElementInstance();

private:
    /* SEService methods */
    void GetReaders(const picojson::value& args, picojson::object& out);
    void RegisterSEListener(const picojson::value& args, picojson::object& out);
    void UnregisterSEListener(const picojson::value& args, picojson::object& out);
    void Shutdown(const picojson::value& args, picojson::object& out);

    /* Reader methods */
    void GetName(const picojson::value& args, picojson::object& out);
    void OpenSession(const picojson::value& args, picojson::object& out);
    void CloseSessions(const picojson::value& args, picojson::object& out);

    /* Session methods */
    void OpenBasicChannel(const picojson::value& args, picojson::object& out);
    void OpenLogicalChannel(const picojson::value& args, picojson::object& out);
    void GetATR(const picojson::value& args, picojson::object& out);
    void CloseSession(const picojson::value& args, picojson::object& out);
    void CloseChannels(const picojson::value& args, picojson::object& out);

    /* Channel methods */
    void CloseChannel(const picojson::value& args, picojson::object& out);
    void Transmit(const picojson::value& args, picojson::object& out);
    void GetSelectResponse(const picojson::value& args, picojson::object& out);

};

} // namespace secureelement
} // namespace extension

#endif // SECUREELEMENT_SECUREELEMENT_INSTANCE_H_
