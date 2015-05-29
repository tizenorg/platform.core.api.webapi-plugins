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
