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

#include "tvchannel/types.h"
#include "common/logger.h"
#include "common/platform_exception.h"

namespace extension {
namespace tvchannel {

const char* NAVIGATOR_MODE_ALL = "ALL";
const char* NAVIGATOR_MODE_DIGITAL = "DIGITAL";
const char* NAVIGATOR_MODE_ANALOG = "ANALOG";
const char* NAVIGATOR_MODE_FAVORITE = "FAVORITE";

NavigatorMode stringToNavigatorMode(const std::string &mode) {
    if (mode == NAVIGATOR_MODE_ALL) {
        return NavigatorMode::ALL;
    }
    if (mode == NAVIGATOR_MODE_DIGITAL) {
        return NavigatorMode::DIGITAL;
    }
    if (mode == NAVIGATOR_MODE_ANALOG) {
        return NavigatorMode::ANALOG;
    }
    if (mode == NAVIGATOR_MODE_FAVORITE) {
        return NavigatorMode::FAVORITE;
    }
    LoggerE("Unrecognized mode");
    throw common::UnknownException("Unrecognized mode");
}

const char* TIZEN_TV_WINDOW_TYPE_MAIN = "MAIN";
// PIP is not supported - j.h.lim
// const char* TIZEN_TV_WINDOW_TYPE_PIP = "PIP";

std::string windowTypeToString(WindowType type) {
    switch (type) {
    case MAIN:
        return TIZEN_TV_WINDOW_TYPE_MAIN;
        break;
// PIP is not supported - j.h.lim
//      case PIP:
//          return TIZEN_TV_WINDOW_TYPE_PIP;
//          break;
    default:
        LoggerE("Unrecognized window type: %d", type);
        throw common::UnknownException("Unrecognized window type");
    }
}

WindowType stringToWindowType(std::string type) {
    if (type == TIZEN_TV_WINDOW_TYPE_MAIN) {
        return MAIN;
// PIP is not supported - j.h.lim
//  } else if (type == TIZEN_TV_WINDOW_TYPE_PIP) {
//      return PIP;
    } else {
        LoggerE("Unrecognized window type: %s", type.c_str());
        throw common::UnknownException("Unrecognized window type");
    }
}

}  //  namespace tvchannel
}  //  namespace extension

