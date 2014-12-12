//
// Tizen Web Device API
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Apache License, Version 2.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <Commons/FunctionDefinition.h>
#include <Commons/FunctionDeclaration.h>
#include <iostream>
#include <Commons/Exception.h>
#include <map>

#include "plugin_config.h"

using namespace WrtDeviceApis::Commons;

namespace DeviceAPI {
namespace Exif {

static FunctionMapping createExifFunctions();

static FunctionMapping ExifFunctions =
    createExifFunctions();

#pragma GCC visibility push(default)
DEFINE_FUNCTION_GETTER(Exif, ExifFunctions);
#pragma GCC visibility pop

static FunctionMapping createExifFunctions()
{
    /**
     * Functions
     */
    FunctionMapping exifMapping;

    return exifMapping;
}

}
}
