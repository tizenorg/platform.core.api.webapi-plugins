//
// Tizen Web Device API
// Copyright (c) 2012-2013 Samsung Electronics Co., Ltd.
//
// Licensed under the Apache License, Version 2.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#ifndef _MESSAGING_PLUGIN_CONFIG_H_
#define _MESSAGING_PLUGIN_CONFIG_H_

#include <Commons/FunctionDeclaration.h>
#include <Security.h>
#include "plugin_config_impl.h"

namespace DeviceAPI {
namespace Messaging {

DECLARE_FUNCTION_GETTER(Messaging);

#define MESSAGING_CHECK_ACCESS(functionName)   \
    aceCheckAccess<AceFunctionGetter, DefaultArgsVerifier<> >(        \
    getMessagingFunctionData,                                              \
    functionName)

}
}

#endif //MESSAGING_PLUGIN_CONFIG_H_

