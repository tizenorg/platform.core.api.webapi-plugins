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
#include "Any.h"

#include <stdexcept>
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
    if (m_value.is<bool>()) {
        return m_value.get<bool>();
    } else {
        return ("true" == this->toString());
    }
}

long Any::toLong() const
{
    if (m_value.is<double>()) {
        return static_cast<long>(m_value.get<double>());
    } else if (m_value.is<std::string>()) {
        try {
            return std::stol(m_value.get<std::string>());
        } catch (...) {
            return static_cast<long>(0);
        }
    } else {
        return static_cast<long>(0);
    }
}

unsigned long Any::toULong() const
{
    if (m_value.is<double>()) {
        return static_cast<unsigned long>(m_value.get<double>());
    } else if (m_value.is<std::string>()) {
        try {
            return std::stoul(m_value.get<std::string>());
        } catch (...) {
            return static_cast<unsigned long>(0);
        }
    } else {
        return static_cast<unsigned long>(0);
    }
}

long long Any::toLongLong() const
{
    if (m_value.is<double>()) {
        return static_cast<long long>(m_value.get<double>());
    } else if (m_value.is<std::string>()) {
        try {
            return std::stoll(m_value.get<std::string>());
        } catch (...) {
            return static_cast<long long>(0);
        }
    } else {
        return static_cast<long long>(0);
    }
}

unsigned long long Any::toULongLong() const
{
    if (m_value.is<double>()) {
        return static_cast<unsigned long long>(m_value.get<double>());
    } else if (m_value.is<std::string>()) {
        try {
            return std::stoull(m_value.get<std::string>());
        } catch (...) {
            return static_cast<unsigned long long>(0);
        }
    } else {
        return static_cast<unsigned long long>(0);
    }
}

double Any::toDouble() const
{
    if (m_value.is<double>()) {
        return m_value.get<double>();
    } else if (m_value.is<std::string>()) {
        try {
            return std::stod(m_value.get<std::string>());
        } catch (...) {
            return 0.0;
        }
    } else {
        return 0.0;
    }
}

std::string Any::toString() const
{
    if (m_value.is<std::string>()) {
        return m_value.get<std::string>();
    } else {
        return "";
    }
}

std::tm* Any::toDateTm() const
{
    static std::tm t;
    memset(&t, 0, sizeof(std::tm));
    strptime(toString().c_str(), "%Y-%m-%dT%H:%M:%S.%zZ", &t);
    return &t;
}

std::time_t Any::toTimeT() const
{
    std::time_t current_time;
    std::time(&current_time);
    struct tm timeinfo = {0};
    long int gmtoff = 0;
    tzset();
    if (nullptr != localtime_r(&current_time, &timeinfo)) {
      gmtoff = timeinfo.tm_gmtoff;

      if (timeinfo.tm_isdst) {
        // if dst is set then 1 hour should be subtracted.
        // 1 hour = 60 second * 60 minutes = 3600 seconds
        gmtoff -= 3600;
      }
    }
    return mktime(toDateTm()) + gmtoff;
}

} // Tizen
} // Device_API
