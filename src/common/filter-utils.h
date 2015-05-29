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

#ifndef WEBAPI_PLUGINS_COMMON_FILTER_UTILS_H_
#define WEBAPI_PLUGINS_COMMON_FILTER_UTILS_H_

#include <functional>
#include <memory>

#include "picojson.h"
#include "platform_result.h"

namespace common {

enum PrimitiveType {
    kPrimitiveTypeBoolean,
    kPrimitiveTypeString,
    kPrimitiveTypeLong,
    kPrimitiveTypeId
};

enum class AttributeMatchFlag {
    kExactly,
    kFullString,
    kContains,
    kStartsWith,
    kEndsWith,
    kExists
};

PlatformResult AttributeMatchFlagFromString(
    const std::string& str, AttributeMatchFlag* filter_match_flag);

enum class CompositeFilterType {
    kUnion,
    kIntersection
};

PlatformResult CompositeFilterTypeFromString(
    const std::string& str, CompositeFilterType* comp_filter_type);

typedef std::function<PlatformResult(const std::string&, AttributeMatchFlag,
                                     const picojson::value&)>
    AttributeFilterOnVisit;

typedef std::function<PlatformResult(const std::string&, const picojson::value&,
                                     const picojson::value&)>
    AttributeRangeFilterOnVisit;

typedef std::function<PlatformResult(CompositeFilterType)>
    CompositeFilterOnBegin;

typedef std::function<PlatformResult(CompositeFilterType)> CompositeFilterOnEnd;

/**
 * @brief The FilterVisitor class
 * A helper class to convert Tizen filters stored as JSON data to native object.
 * User should set callbacks to react on each of Tizen filters detected.
 */
class FilterVisitor {
  public:
    /**
     * @brief Sets callback to be invoked on AttributeFilter.
     *
     * @param[in] func - callback with arguments:
     *  - std::string AttributeName
     *  - AttributeMatchFlag flag
     *  - picojson::value matchValue
     */
    void SetOnAttributeFilter(const AttributeFilterOnVisit& func);

    /**
     * @brief Sets callback to be invoked on AttributeRangeFilter.
     *
     * @param[in] func - callback with arguments:
     *  - std::string AttributeName
     *  - picojson::value initialValue
     *  - picojson::value endValue
     */
    void SetOnAttributeRangeFilter(const AttributeRangeFilterOnVisit& func);

    /**
     * @brief Sets callback to be invoked on begin of CompositeFilter.
     *
     * @param[in] func - callback with arguments:
     *  - CompositeFilterType type
     */
    void SetOnCompositeFilterBegin(const CompositeFilterOnBegin& func);

    /**
     * @brief Sets callback to be invoked on end of CompositeFilter.
     *
     * @param[in] func - callback with no arguments
     */
    void SetOnCompositeFilterEnd(const CompositeFilterOnEnd& func);

    /**
     * @brief Parses a json object as Tizen filter.
     * @param filter Object to be visited
     */
    PlatformResult Visit(const picojson::object& filter);

private:
    PlatformResult VisitAttributeFilter(const picojson::object& filter);
    PlatformResult VisitAttributeRangeFilter(const picojson::object& filter);
    PlatformResult VisitCompositeFilter(const picojson::object& filter);

    AttributeFilterOnVisit m_attributeFilterOnVisit;
    AttributeRangeFilterOnVisit m_attributeRangeFilterOnVisit;
    CompositeFilterOnBegin m_compositeFilterOnBegin;
    CompositeFilterOnEnd m_compositeFilterOnEnd;
};

}  // namespace common

#endif  // WEBAPI_PLUGINS_COMMON_FILTER_UTILS_H_
