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

#ifndef SRC_TVCHANNEL_TYPES_H_
#define SRC_TVCHANNEL_TYPES_H_

#include <string>

namespace extension {
namespace tvchannel {

enum NavigatorMode {
    NOT_DEFINED,
    ALL,
    DIGITAL,
    ANALOG,
    FAVORITE
};

enum WindowType {
    MAIN = 0,
//  PIP is not supported - j.h.lim
//  PIP
};

extern const char* TIZEN_TV_WINDOW_TYPE_MAIN;

enum class SourceType {
    TV,
    AV,
    SVIDEO,
    COMP,
    PC,
    HDMI,
    SCART,
    DVI,
    MEDIA,
    IPTV,
    UNKNOWN
};

extern const char* NAVIGATOR_MODE_ALL;
extern const char* NAVIGATOR_MODE_DIGITAL;
extern const char* NAVIGATOR_MODE_ANALOG;
extern const char* NAVIGATOR_MODE_FAVORITE;

NavigatorMode stringToNavigatorMode(std::string const& mode);
std::string windowTypeToString(WindowType type);
WindowType stringToWindowType(std::string type);

}  //  namespace tvchannel
}  //  namespace extension

#endif  // SRC_TVCHANNEL_TYPES_H_
