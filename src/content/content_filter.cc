// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/content_filter.h"

#include <vector>

#include "common/converter.h"
#include "common/logger.h"

using common::AttributeMatchFlag;
using common::CompositeFilterType;
using common::ErrorCode;
using common::JsonCast;
using common::PlatformResult;

namespace extension {
namespace content {

namespace {

std::map<std::string, std::string> const attributeNameMap = {
    {"id", "MEDIA_ID"},
    {"type", "MEDIA_TYPE"},
    {"mimeType", "MEDIA_MIME_TYPE"},
    {"name", "MEDIA_DISPLAY_NAME"},
    {"title", "MEDIA_TITLE"},
    {"contentURI", "MEDIA_PATH"},
    {"thumbnailURIs", "MEDIA_THUMBNAIL_PATH"},
    {"description", "MEDIA_DESCRIPTION"},
    {"rating", "MEDIA_RATING"},
    {"createdDate", "MEDIA_ADDED_TIME"},
    {"releaseDate", "MEDIA_DATETAKEN"},
    {"modifiedDate", "MEDIA_MODIFIED_TIME"},
    {"geolocation.latitude", "MEDIA_LATITUDE"},
    {"geolocation.longitude", "MEDIA_LONGITUDE"},
    {"duration", "MEDIA_DURATION"},
    {"album", "MEDIA_ALBUM"},
    {"artists", "MEDIA_ARTIST"},
    {"width", "MEDIA_WIDTH"},
    {"height", "MEDIA_HEIGHT"},
    {"genres", "MEDIA_GENRE"},
    {"size", "MEDIA_SIZE"},
};

std::string escapeValueString(const std::string& data) {
  std::string out;
  // If string won't be resized, then it will be faster
  out.reserve(data.size());
  for (auto c : data) {
    if (c == '\\')
      out += "\\\\";
    else if (c == '\"')
      out += "\\\"";
    else if (c == '\'')
      out += "\\\'";
    else if (c == '\n')
      out += "\\\n";
    else if (c == '\r')
      out += "\\\r";
    else
      out += c;
  }
  return out;
}

}  // namespace

PlatformResult ContentFilter::buildQuery(const picojson::object& jsFilter,
                                         std::string* queryToCall) {
  std::vector<std::vector<std::string> > partialqueries;
  partialqueries.push_back(std::vector<std::string>());

  visitor.SetOnAttributeFilter([&](const std::string& name,
                                   AttributeMatchFlag match_flag,
                                   const picojson::value& match_value) {
    std::string query;
    LoggerD("entered OnAttributeFilter");
    std::string matchValue;
    auto it = attributeNameMap.find(name);
    if (it != attributeNameMap.end())
      query += it->second;
    else
      return PlatformResult(ErrorCode::INVALID_VALUES_ERR);

    if (AttributeMatchFlag::kExactly == match_flag ||
        AttributeMatchFlag::kFullString == match_flag) {
      query += " = ";
    } else if (AttributeMatchFlag::kContains == match_flag ||
               AttributeMatchFlag::kStartsWith == match_flag ||
               AttributeMatchFlag::kEndsWith == match_flag) {
      query += " LIKE ";
    } else if (AttributeMatchFlag::kExists == match_flag) {
      query += " IS NOT NULL ";
    } else {
      return PlatformResult(ErrorCode::INVALID_VALUES_ERR);
    }
    query.append("\"");

    if (AttributeMatchFlag::kExists != match_flag) {
      matchValue = escapeValueString(JsonCast<std::string>(match_value));
      if (name == "type") {
        if (matchValue == "IMAGE") {
          matchValue = "0";
        } else if (matchValue == "VIDEO") {
          matchValue = "1";
        } else if (matchValue == "AUDIO") {
          matchValue = "3";
        } else {  // OTHER
          matchValue = "4";
        }
      }
      query += matchValue;
    }
    query.append("\"");
    partialqueries.back().push_back(query);

    LoggerD("about to call with condition %s", query.c_str());
    return PlatformResult(ErrorCode::NO_ERROR);
  });

  visitor.SetOnCompositeFilterBegin([&](CompositeFilterType type) {
    LoggerD("entered OnCompositeFilterBegin");
    partialqueries.push_back(std::vector<std::string>());
    return PlatformResult(ErrorCode::NO_ERROR);
  });

  visitor.SetOnCompositeFilterEnd([&](CompositeFilterType calType) {
    LoggerD("entered OnCompositeFilterEnd");
    std::string finalQuery;
    std::string separator;

    if (CompositeFilterType::kUnion == calType)
      separator = " OR ";
    else
      separator = " AND ";

    LoggerD("Composite filter: %i", partialqueries.back().size());
    if (partialqueries.back().empty()) {
      partialqueries.pop_back();
      return PlatformResult(ErrorCode::NO_ERROR);
    }
    if (partialqueries.back().size() != 1)
      finalQuery.append("(");

    for (unsigned long i = 0; i < partialqueries.back().size(); i++) {
      finalQuery += partialqueries.back().at(i);
      if (i != partialqueries.back().size() - 1) {
        finalQuery += separator;
      }
    }

    if (partialqueries.back().size() != 1)
      finalQuery.append(")");
    partialqueries.pop_back();
    partialqueries.back().push_back(finalQuery);
    return PlatformResult(ErrorCode::NO_ERROR);
  });

  visitor.SetOnAttributeRangeFilter([&](const std::string& name,
                                        const picojson::value& initial_value,
                                        const picojson::value& end_value) {
    LoggerD("entered OnAttributeFilter");
    std::string query = "";
    std::string paramName;
    auto it = attributeNameMap.find(name);
    if (it != attributeNameMap.end())
      paramName = it->second;
    else
      return PlatformResult(ErrorCode::INVALID_VALUES_ERR);
    std::string initialValue = escapeValueString(JsonCast<std::string>(initial_value));
    std::string endValue = escapeValueString(JsonCast<std::string>(end_value));
    query += paramName;
    query += " >= \"";
    query += initialValue;
    query += "\" AND ";
    query += paramName;
    query += " <= \"";
    query += endValue;
    query += "\"";
    partialqueries.back().push_back(query);

    LoggerD("about to call with condition %s", query.c_str());
    return PlatformResult(ErrorCode::NO_ERROR);
  });

  if (!visitor.Visit(jsFilter)) {
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR);
  }

  if (partialqueries.empty()) {
    LoggerE("Filter parsing error!");
    return PlatformResult(ErrorCode::SYNTAX_ERR);
  }
  if (partialqueries.back().empty()) {
    LoggerD("Resolved to empty string!");
    *queryToCall = "";
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  *queryToCall = partialqueries.back().front();
  return PlatformResult(ErrorCode::NO_ERROR);
}

}  // namespace content
}  // namespace extension
