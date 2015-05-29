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

#ifndef SECUREELEMENT_SECUREELEMENT_INSTANCE_H_
#define SECUREELEMENT_SECUREELEMENT_INSTANCE_H_

#include "common/extension.h"
#include "secureelement_seservice.h"

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
    void IsPresent(const picojson::value& args, picojson::object& out);
    void OpenSession(const picojson::value& args, picojson::object& out);
    void CloseSessions(const picojson::value& args, picojson::object& out);

    /* Session methods */
    void OpenBasicChannel(const picojson::value& args, picojson::object& out);
    void OpenLogicalChannel(const picojson::value& args, picojson::object& out);
    void GetATR(const picojson::value& args, picojson::object& out);
    void IsSessionClosed(const picojson::value& args, picojson::object& out);
    void CloseSession(const picojson::value& args, picojson::object& out);
    void CloseChannels(const picojson::value& args, picojson::object& out);

    /* Channel methods */
    void CloseChannel(const picojson::value& args, picojson::object& out);
    void Transmit(const picojson::value& args, picojson::object& out);
    void GetSelectResponse(const picojson::value& args, picojson::object& out);

    SEService service_;
};

} // namespace secureelement
} // namespace extension

#endif // SECUREELEMENT_SECUREELEMENT_INSTANCE_H_
