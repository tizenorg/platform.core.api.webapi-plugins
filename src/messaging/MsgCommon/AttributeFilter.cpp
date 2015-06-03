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

#include "AttributeFilter.h"
#include "common/platform_exception.h"
#include "common/logger.h"

namespace extension {
namespace tizen {

AttributeFilter::AttributeFilter(const std::string &attribute_name) :
        AbstractFilter(ATTRIBUTE_FILTER),
        m_attribute_name(attribute_name),
        m_match_flag(EXACTLY)
{
}

AttributeFilter::~AttributeFilter()
{
}

std::string AttributeFilter::getAttributeName() const
{
    return m_attribute_name;
}

void AttributeFilter::setAttributeName(const std::string &attribute_name)
{
    m_attribute_name = attribute_name;
}

FilterMatchFlag AttributeFilter::getMatchFlag() const
{
    return m_match_flag;
}

void AttributeFilter::setMatchFlag(FilterMatchFlag match_flag)
{
    m_match_flag = match_flag;
}

AnyPtr AttributeFilter::getMatchValue() const
{
    return m_match_value;
}

void AttributeFilter::setMatchValue(AnyPtr match_value)
{
    m_match_value = match_value;
}

bool AttributeFilter::isMatching(const FilterableObject* const filtered_object) const
{
    if (!filtered_object) {
        LoggerE("Invalid object: NULL!");
        return false;
    }

    return filtered_object->isMatchingAttribute(m_attribute_name, m_match_flag,
            m_match_value);
}

} // Tizen
} // DeviceAPI
