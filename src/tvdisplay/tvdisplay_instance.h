// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_TVDISPLAY_TVDISPLAY_INSTANCE_H_
#define SRC_TVDISPLAY_TVDISPLAY_INSTANCE_H_

#include <memory>
#include <string>
#include <vector>

#include "common/extension.h"
#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace tvdisplay {

class TVDisplayInstance : public common::ParsedInstance {
 public:
    TVDisplayInstance();
    virtual ~TVDisplayInstance();

 private:
    void  Is3DModeEnabled(
        const picojson::value& value,
        picojson::object& out);

    void Get3DEffectMode(
        const picojson::value& value,
        picojson::object& out);

    void GetSupported3DEffectModeList(
        const picojson::value& value,
        picojson::object& out);

    void GetSupported3DEffectModeListTask(
        std::shared_ptr<picojson::object> const& data);

    void GetSupported3DEffectModeListTaskAfter(
        std::shared_ptr<picojson::object> const& data);

 public:
    static TVDisplayInstance* getInstance();
};

}  // namespace tvdisplay
}  // namespace extension

#endif  // SRC_TVDISPLAY_TVDISPLAY_INSTANCE_H_
