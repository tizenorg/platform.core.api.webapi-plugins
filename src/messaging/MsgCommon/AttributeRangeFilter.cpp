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
 
#include "AttributeRangeFilter.h"
#include "common/platform_exception.h"
#include "common/logger.h"

namespace extension {
namespace tizen {

AttributeRangeFilter::AttributeRangeFilter(const std::string &attribute_name) :
        m_attribute_name(attribute_name)
{
    m_filter_type = ATTRIBUTE_RANGE_FILTER;
}

AttributeRangeFilter::~AttributeRangeFilter()
{

}

std::string AttributeRangeFilter::getAttributeName() const
{
    return m_attribute_name;
}

void AttributeRangeFilter::setAttributeName(const std::string &attribute_name)
{
    m_attribute_name = attribute_name;
}


AnyPtr AttributeRangeFilter::getInitialValue() const
{
    return m_initial_value;
}

void AttributeRangeFilter::setInitialValue(AnyPtr initial_value)
{
    m_initial_value = initial_value;
}

AnyPtr AttributeRangeFilter::getEndValue() const
{
    return m_end_value;
}

void AttributeRangeFilter::setEndValue(AnyPtr end_value)
{
    m_end_value = end_value;
}

bool AttributeRangeFilter::isMatching(const FilterableObject* const filtered_object) const
{
    if (!filtered_object) {
        LoggerE("Invalid object: NULL!");
        return false;
    }

    return filtered_object->isMatchingAttributeRange(m_attribute_name, m_initial_value,
            m_end_value);
}

} // Tizen
} // DeviceAPI
