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
 
#include "common/filter-utils.h"

#include "common/logger.h"
#include "common/converter.h"

namespace common {

PlatformResult AttributeMatchFlagFromString(
    const std::string &str, AttributeMatchFlag *filter_match_flag) {
  LoggerD("Enter");
  if (str == "EXACTLY") {
    *filter_match_flag = AttributeMatchFlag::kExactly;
  } else if (str == "FULLSTRING") {
    *filter_match_flag = AttributeMatchFlag::kFullString;
  } else if (str == "CONTAINS") {
    *filter_match_flag = AttributeMatchFlag::kContains;
  } else if (str == "STARTSWITH") {
    *filter_match_flag = AttributeMatchFlag::kStartsWith;
  } else if (str == "ENDSWITH") {
    *filter_match_flag = AttributeMatchFlag::kEndsWith;
  } else if (str == "EXISTS") {
    *filter_match_flag = AttributeMatchFlag::kExists;
  } else {
    LoggerE("Invalid attribute match string: %i", str.c_str());
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                          "Invalid attribute match string!");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CompositeFilterTypeFromString(
    const std::string &str, CompositeFilterType *comp_filter_type) {
  LoggerD("Enter");
  if (str == "UNION") {
    *comp_filter_type = CompositeFilterType::kUnion;
  } else if (str == "INTERSECTION") {
    *comp_filter_type = CompositeFilterType::kIntersection;
  } else {
    LoggerE("Invalid composite type string: %i", str.c_str());

    return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                          "Invalid composite type string!");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

void FilterVisitor::SetOnAttributeFilter(const AttributeFilterOnVisit &func) {
  LoggerD("Enter");
  m_attributeFilterOnVisit = func;
}

void FilterVisitor::SetOnAttributeRangeFilter(const AttributeRangeFilterOnVisit &func) {
  LoggerD("Enter");
  m_attributeRangeFilterOnVisit = func;
}

void FilterVisitor::SetOnCompositeFilterBegin(const CompositeFilterOnBegin &func) {
  LoggerD("Enter");
  m_compositeFilterOnBegin = func;
}

void FilterVisitor::SetOnCompositeFilterEnd(const CompositeFilterOnEnd &func) {
  LoggerD("Enter");
  m_compositeFilterOnEnd = func;
}

PlatformResult FilterVisitor::Visit(const picojson::object &filter) {
  LoggerD("Enter");
  const std::string &filterType = FromJson<std::string>(filter, "filterType");
  if (filterType == "AttributeFilter") {
    PlatformResult status = VisitAttributeFilter(filter);
    if (status.IsError()) return status;
  } else if (filterType == "AttributeRangeFilter") {
    PlatformResult status = VisitAttributeRangeFilter(filter);
    if (status.IsError()) return status;
  } else if (filterType == "CompositeFilter") {
    PlatformResult status = VisitCompositeFilter(filter);
    if (status.IsError()) return status;
  } else {
    LoggerE("Invalid filter type!");
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                          "Invalid filter type!");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult FilterVisitor::VisitAttributeFilter(
    const picojson::object &filter) {
  LoggerD("Enter");
  const std::string &attribute_name =
      FromJson<std::string>(filter, "attributeName");

  AttributeMatchFlag match_flag;
  PlatformResult status = AttributeMatchFlagFromString(
      FromJson<std::string>(filter, "matchFlag"), &match_flag);
  if (status.IsError()) return status;

  const picojson::value &match_value = FindValue(filter, "matchValue");

  if (m_attributeFilterOnVisit) {
    status = m_attributeFilterOnVisit(attribute_name, match_flag, match_value);
    if (status.IsError()) return status;
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult FilterVisitor::VisitAttributeRangeFilter(
    const picojson::object &filter) {
  LoggerD("Enter");
  const std::string &attributeName =
      FromJson<std::string>(filter, "attributeName");
  const picojson::value &initialValue = FindValue(filter, "initialValue");
  const picojson::value &endValue = FindValue(filter, "endValue");

  if (m_attributeRangeFilterOnVisit) {
    PlatformResult status =
        m_attributeRangeFilterOnVisit(attributeName, initialValue, endValue);
    if (status.IsError()) return status;
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult FilterVisitor::VisitCompositeFilter(
    const picojson::object &filter) {
  LoggerD("Enter");
  CompositeFilterType filter_type;
  PlatformResult status = CompositeFilterTypeFromString(
      FromJson<std::string>(filter, "type"), &filter_type);
  if (status.IsError()) return status;

  const picojson::array &filters = FromJson<picojson::array>(filter, "filters");

  if (m_compositeFilterOnBegin) {
    status = m_compositeFilterOnBegin(filter_type);
    if (status.IsError()) return status;
  }

  for (std::size_t i = 0; i < filters.size(); ++i) {
    PlatformResult status = Visit(JsonCast<picojson::object>(filters[i]));
    if (status.IsError()) return status;
  }

  if (m_compositeFilterOnEnd) {
    status = m_compositeFilterOnEnd(filter_type);
    if (status.IsError()) return status;
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

}
