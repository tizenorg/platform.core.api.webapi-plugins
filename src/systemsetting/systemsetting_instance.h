// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYSTEMSETTING_SYSTEMSETTING_INSTANCE_H_
#define SYSTEMSETTING_SYSTEMSETTING_INSTANCE_H_

#include "common/extension.h"

namespace extension {
namespace systemsetting {

class SystemSettingInstance : public common::ParsedInstance
{
public:
    SystemSettingInstance();
    virtual ~SystemSettingInstance();

private:

    void getProperty(const picojson::value& args, picojson::object& out);
    picojson::value getPlatformPropertyValue(const std::string &valueType, int &platformResult);
};

} // namespace systemsetting
} // namespace extension

#endif // SYSTEMSETTING_SYSTEMSETTING_INSTANCE_H_
