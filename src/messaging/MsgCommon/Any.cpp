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

#include "Any.h"
//#include <JSUtil.h>

namespace extension {
namespace tizen {

Any::Any(picojson::value value) :
//        m_context(context),
        m_value(value)
{
//    JSValueProtect(m_context, m_value);
}

Any::~Any()
{
//    JSValueUnprotect(m_context, m_value);
}

//JSContextRef Any::getContext() const
//{
//    return m_context;
//}

picojson::value Any::getValue() const
{
    return m_value;
}

void Any::setValue(picojson::value value)
{
//    JSValueUnprotect(m_context, m_value);
//    m_context = context;
    m_value = value;
//    JSValueProtect(m_context, m_value);
}

bool Any::isNullOrUndefined() const
{
    //TODO is it check for undefined?
    return m_value.is<picojson::null>();
}

bool Any::toBool() const
{
    return m_value.get<bool>();
}

long Any::toLong() const
{
    return static_cast<long>(m_value.get<double>());
}

unsigned long Any::toULong() const
{
    return static_cast<unsigned long>(m_value.get<double>());
}

long long Any::toLongLong() const
{
    return static_cast<long long>(m_value.get<double>());
}

unsigned long long Any::toULongLong() const
{
    return static_cast<unsigned long long>(m_value.get<double>());
}

double Any::toDouble() const
{
    return m_value.get<double>();
}

std::string Any::toString() const
{
    return m_value.get<std::string>();
}

std::tm* Any::toDateTm() const
{
    //TODO is it ok?
    std::time_t time = static_cast<std::time_t>(m_value.get<double>());
    return gmtime(&time);
}

std::time_t Any::toTimeT() const
{
    //TODO is it ok?
    return static_cast<std::time_t>(m_value.get<double>());
}

} // Tizen
} // Device_API
