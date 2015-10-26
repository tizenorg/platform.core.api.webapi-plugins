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
  LoggerD("Enter");
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

PlatformResult ContentFilter::MapField(const std::string& name,
                                       std::string* result) {
  LoggerD("Enter");
  auto it = attributeNameMap.find(name);
  if (it != attributeNameMap.end())
    *result = it->second;
  else
  {
    LoggerE("INVALID_VALUES_ERR");
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR);
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ContentFilter::BuildQuery(const picojson::object& jsFilter,
                                         std::string* queryToCall) {
  LoggerD("Enter");
  std::vector<std::vector<std::string> > partialqueries;
  partialqueries.push_back(std::vector<std::string>());

  visitor.SetOnAttributeFilter([&](const std::string& name,
                                   AttributeMatchFlag match_flag,
                                   const picojson::value& match_value) {
    LoggerD("entered OnAttributeFilter");


    std::string query;
    std::string matchValue;

    PlatformResult result = MapField(name, &query);
    if (!result)
      return result;

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
      LoggerE("INVALID_VALUES_ERR");
      return PlatformResult(ErrorCode::INVALID_VALUES_ERR);
    }
    if (AttributeMatchFlag::kExists != match_flag) {
      query.append("\"");
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
      } else if (name == "contentURI") {
        const char* uri_prefix = "file://";
        size_t found = matchValue.find(uri_prefix);
        if (found != std::string::npos) {
          //simple convertion of URI to globalpath
          matchValue = matchValue.substr(found + strlen(uri_prefix));
        }
      }
      switch (match_flag) {
        case AttributeMatchFlag::kStartsWith :
          query += matchValue + "%";
          break;
        case AttributeMatchFlag::kEndsWith :
          query += "%" + matchValue;
          break;
        case AttributeMatchFlag::kContains :
          query += "%" + matchValue + "%";
          break;
        default :
          query += matchValue;
      }
      query.append("\"");
    }

    partialqueries.back().push_back(query);

    return result;
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
    PlatformResult result = MapField(name, &paramName);
    if (!result)
      return result;

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

    return result;
  });

  if (!visitor.Visit(jsFilter)) {
    LoggerE("INVALID_VALUES_ERR");
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
