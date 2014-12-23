/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
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

#include "common/filter-utils.h"

#include "common/logger.h"
#include "common/converter.h"

namespace common {

AttributeMatchFlag AttributeMatchFlagFromString(const std::string &str) {
    if (str == "EXACTLY") {
        return AttributeMatchFlag::kExactly;
    }
    if (str == "FULLSTRING") {
        return AttributeMatchFlag::kFullString;
    }
    if (str == "CONTAINS") {
        return AttributeMatchFlag::kContains;
    }
    if (str == "STARTSWITH") {
        return AttributeMatchFlag::kStartsWith;
    }
    if (str == "ENDSWITH") {
        return AttributeMatchFlag::kEndsWith;
    }
    if (str == "EXISTS") {
        return AttributeMatchFlag::kExists;
    }

    LoggerE("Invalid attribute match string: %i", str.c_str());

    throw InvalidValuesException("Invalid attribute match string!");
}

CompositeFilterType CompositeFilterTypeFromString(const std::string &str) {
    if (str == "UNION") {
        return CompositeFilterType::kUnion;
    }
    if (str == "INTERSECTION") {
        return CompositeFilterType::kIntersection;
    }

    LoggerE("Invalid composite type string: %i", str.c_str());

    throw InvalidValuesException("Invalid composite type string!");
}

void FilterVisitor::SetOnAttributeFilter(const AttributeFilterOnVisit &func) {
    m_attributeFilterOnVisit = func;
}

void FilterVisitor::SetOnAttributeRangeFilter(const AttributeRangeFilterOnVisit &func) {
    m_attributeRangeFilterOnVisit = func;
}

void FilterVisitor::SetOnCompositeFilterBegin(const CompositeFilterOnBegin &func) {
    m_compositeFilterOnBegin = func;
}

void FilterVisitor::SetOnCompositeFilterEnd(const CompositeFilterOnEnd &func) {
    m_compositeFilterOnEnd = func;
}

void FilterVisitor::Visit(const picojson::object &filter) {
    const std::string& filterType = FromJson<std::string>(filter, "filterType");
    if (filterType == "AttributeFilter") {
        VisitAttributeFilter(filter);
    } else if (filterType == "AttributeRangeFilter") {
        VisitAttributeRangeFilter(filter);
    } else if (filterType == "CompositeFilter") {
        VisitCompositeFilter(filter);
    } else {
        throw InvalidValuesException("Invalid filter type!");
    }
}

void FilterVisitor::VisitAttributeFilter(const picojson::object &filter) {
    const std::string& attributeName = FromJson<std::string>(filter, "attributeName");
    AttributeMatchFlag matchFlag =
            AttributeMatchFlagFromString(FromJson<std::string>(filter, "matchFlag"));
    const picojson::value& matchValue = FindValue(filter, "matchValue");

    if (m_attributeFilterOnVisit) {
        m_attributeFilterOnVisit(attributeName, matchFlag, matchValue);
    }
}

void FilterVisitor::VisitAttributeRangeFilter(const picojson::object &filter) {
    const std::string& attributeName = FromJson<std::string>(filter, "attributeName");
    const picojson::value& initialValue = FindValue(filter, "initialValue");
    const picojson::value& endValue = FindValue(filter, "endValue");

    if (m_attributeRangeFilterOnVisit) {
        m_attributeRangeFilterOnVisit(attributeName, initialValue, endValue);
    }
}

void FilterVisitor::VisitCompositeFilter(const picojson::object &filter) {
    CompositeFilterType filterType =
            CompositeFilterTypeFromString(FromJson<std::string>(filter, "type"));
    const picojson::array& filters = FromJson<picojson::array>(filter, "filters");

    if (m_compositeFilterOnBegin) {
        m_compositeFilterOnBegin(filterType);
    }

    for (std::size_t i = 0; i < filters.size(); ++i) {
        Visit(JsonCast<picojson::object>(filters[i]));
    }

    if (m_compositeFilterOnEnd) {
        m_compositeFilterOnEnd(filterType);
    }
}

}
