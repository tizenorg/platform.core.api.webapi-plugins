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

#include "AbstractFilter.h"
//#include "JSAttributeFilter.h"
//#include "JSAttributeRangeFilter.h"
//#include "JSCompositeFilter.h"
#include "common/platform_exception.h"
#include "common/logger.h"
//#include <JSUtil.h>
#include <algorithm>

namespace extension {
namespace tizen {

using namespace common;

AbstractFilter::AbstractFilter(FilterType filter_type) :
        m_filter_type(filter_type)
{
}

AbstractFilter::~AbstractFilter()
{
}

FilterType AbstractFilter::getFilterType() const
{
    return m_filter_type;
}

bool AbstractFilter::isMatching(const FilterableObject* const tested_object) const
{
    LoggerE("Calling isMatching on AbstractFilter!");
    return false;
}

AttributeFilterPtr castToAttributeFilter(AbstractFilterPtr from)
{
    if(ATTRIBUTE_FILTER != from->getFilterType()) {
        LoggerE("Trying to get AttributeFilterPtr but filter's type is: %d",
                from->getFilterType());
        return AttributeFilterPtr();
    }

    return std::dynamic_pointer_cast<AttributeFilter>(from);
}

AttributeRangeFilterPtr castToAttributeRangeFilter(AbstractFilterPtr from)
{
    if(ATTRIBUTE_RANGE_FILTER != from->getFilterType()) {
        LoggerE("Trying to get AttributeRangeFilterPtr but filter's type is: %d",
                from->getFilterType());
        return AttributeRangeFilterPtr();
    }

    return std::dynamic_pointer_cast<AttributeRangeFilter>(from);
}

CompositeFilterPtr castToCompositeFilter(AbstractFilterPtr from)
{
    if(COMPOSITE_FILTER != from->getFilterType()) {
        LoggerE("Trying to get CompositeFilterPtr but filter's type is: %d",
                from->getFilterType());
        return CompositeFilterPtr();
    }

    return std::dynamic_pointer_cast<CompositeFilter>(from);
}

namespace {

inline std::string convertToLowerCase(const std::string& input_string)
{
    std::string output_string = input_string;
    std::transform(output_string.begin(), output_string.end(), output_string.begin(),
            ::tolower);
    return output_string;
}

} // Anonymous namespace

bool FilterUtils::isStringMatching(const std::string& key,
        const std::string& value,
        tizen::FilterMatchFlag flag)
{
    switch(flag)
    {
        case tizen::ENDSWITH: {
            if (key.empty()) {
                return false;
            }
            if (key.size() > value.size()) {
                return false;
            }
            std::string lvalue = convertToLowerCase(value);
            std::string lkey = convertToLowerCase(key);
            return lvalue.substr(lvalue.size() - lkey.size(), lkey.size()) == lkey;
        }

        case tizen::EXACTLY: {
            return key == value;
        }

        case tizen::STARTSWITH: {
            if (key.empty()) {
                return false;
            }
            if (key.size() > value.size()) {
                return false;
            }
            std::string lvalue = convertToLowerCase(value);
            std::string lkey = convertToLowerCase(key);
            return lvalue.substr(0, lkey.size()) == lkey;
        }

        case tizen::CONTAINS: {
            if (key.empty()) {
                return false;
            }
            if (key.size() > value.size()) {
                return false;
            }
            std::string lvalue = convertToLowerCase(value);
            std::string lkey = convertToLowerCase(key);
            return lvalue.find(lkey) != std::string::npos;
        }

        default: {
            LoggerE("Unknown match flag");
            return false;
        }
    }
}

bool FilterUtils::isAnyStringMatching(const std::string& key,
        const std::vector<std::string>& values,
        tizen::FilterMatchFlag flag)
{
    for(auto it = values.begin(); it != values.end(); ++it) {
        if(isStringMatching(key,*it,flag)) {
            return true;
        }
    }
    return false;
}

bool FilterUtils::isTimeStampInRange(const time_t& time_stamp,
        tizen::AnyPtr& initial_value,
        tizen::AnyPtr& end_value)
{
    time_t from_time = 0;
    time_t to_time = 0;

    bool initial_is_valid_time_value = false;
    if (initial_value && !initial_value->isNullOrUndefined()) {
        struct tm ftime = *initial_value->toDateTm();
        from_time = mktime(&ftime);
        initial_is_valid_time_value = true;
    }
    if (!initial_is_valid_time_value) {
        LoggerE("initialValue is not Time!");
        return false;
    }

    bool end_is_valid_time_value = false;
    if (end_value && !end_value->isNullOrUndefined()) {
        struct tm ttime =  *end_value->toDateTm();
        to_time = mktime(&ttime);
        end_is_valid_time_value = true;
    }
    if (end_is_valid_time_value) {
        LoggerE("endValue is not Time!");
        return false;
    }

    bool is_in_range = FilterUtils::isBetweenTimeRange(time_stamp, from_time, to_time);

    LoggerD("%d is%s in time range <%d, %d>", time_stamp, (is_in_range ? "" : " NOT"),
            from_time, to_time);

    return is_in_range;
}

} //Tizen
} //DeviceAPI
