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

#ifndef __TIZEN_TIZEN_ANY_H__
#define __TIZEN_TIZEN_ANY_H__

#include <memory>
//#include <JavaScriptCore/JavaScript.h>
#include <string>
#include <ctime>
#include "common/picojson.h"

namespace extension {
namespace tizen {

enum PrimitiveType {
    PrimitiveType_NoType,
    PrimitiveType_Null,
    PrimitiveType_Boolean,
    PrimitiveType_Long,
    PrimitiveType_ULong,
    PrimitiveType_LongLong,
    PrimitiveType_ULongLong,
    PrimitiveType_Double,
    PrimitiveType_String,
    PrimitiveType_Time,
    PrimitiveType_Object,
    PrimitiveType_PlatformObject
};

class Any;
typedef std::shared_ptr<Any> AnyPtr;

class Any {
public:
    Any(picojson::value value);
    virtual ~Any();

//    JSContextRef getContext() const;
    picojson::value getValue() const;
    void setValue(picojson::value value);

    bool isNullOrUndefined() const;

    bool toBool() const;
    long toLong() const;
    unsigned long toULong() const;
    long long toLongLong() const;
    unsigned long long toULongLong() const;
    double toDouble() const;
    std::string toString() const;
    std::tm* toDateTm() const;
    std::time_t toTimeT() const;

private:
//    JSContextRef m_context;
    picojson::value m_value;
};

} // Tizen
} // DeviceAPI

#endif // __TIZEN_TIZEN_ANY_H__
