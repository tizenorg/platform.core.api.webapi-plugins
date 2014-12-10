//
// Tizen Web Device API
// Copyright (c) 2013 Samsung Electronics Co., Ltd.
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

/**
 * @file        BaseFilterVisitor.cpp
 */

#include "BaseFilterVisitor.h"
#include <Logger.h>
#include <cstring>

namespace DeviceAPI {
namespace Messaging {

inline std::string convertToLowerCase(const std::string& input_string)
{
    std::string outputString=input_string;
    outputString.reserve(input_string.length());
    std::transform(outputString.begin(), outputString.end(), outputString.begin(),
            ::tolower);
    return outputString;
}

BaseFilterVisitor::BaseFilterVisitor()
{
}

BaseFilterVisitor::~BaseFilterVisitor()
{
}

void BaseFilterVisitor::visitPreComposite(Tizen::FilterType& type, int depth)
{
    LOGD("Entered");
    m_operand_list.push_back(FCO_START);
}

void BaseFilterVisitor::visitInComposite(Tizen::FilterType& type, int depth)
{
    LOGD("Entered");
    if(Tizen::UNION_FILTER == type) {
        m_operand_list.push_back(FCO_OR);
        LOGD("operand OR");
    }else if(Tizen::INTERSECTION_FILTER == type) {
        LOGD("operand AND");
        m_operand_list.push_back(FCO_AND);
    } else {
        LOGE("Invalid filter type:%d", type);
    }
}

void BaseFilterVisitor::visitPostComposite(Tizen::FilterType& type, int depth)
{
    LOGD("Entered");
    LOGD("Start: numResults:%d numOperands:%d", m_result_list.size(),
            m_operand_list.size());

    while(m_operand_list.back() != FCO_START) {

        const bool left =  m_result_list.back();
        m_result_list.pop_back();
        const bool right =  m_result_list.back();
        m_result_list.pop_back();
        const FilterCompositeOperand& operand = m_operand_list.back();
        m_operand_list.pop_back();

        bool result = false;
        if(FCO_AND == operand) {
            result = left && right;
        } else if(FCO_OR == operand) {
            result = left || right;
        }
        m_result_list.push_back(result);
    }

    m_operand_list.pop_back(); //remove FCO_START
    LOGD("End: numResults:%d numOperands:%d", m_result_list.size(),
            m_operand_list.size());
}

void BaseFilterVisitor::testStringMatch(const std::string& key,
        const std::string& value,
        Tizen::MatchFlag flag)
{
    m_result_list.push_back(matchString(key, value, flag));
}

void BaseFilterVisitor::testAnyStringMatch(const std::string& key,
        const std::vector<std::string>& values,
        Tizen::MatchFlag flag)
{
    m_result_list.push_back(matchStringVector(key, values, flag));
}

void BaseFilterVisitor::testTimeStampIsInRange(const time_t& timeStamp,
            Tizen::AnyPtr& initialValue, Tizen::AnyPtr& endValue)
{
    m_result_list.push_back(matchTimeStampRange(timeStamp, initialValue, endValue));
}

bool BaseFilterVisitor::matchString(const std::string& key,
        const std::string& value,
        Tizen::MatchFlag flag)
{
    bool res = false;

    switch(flag)
    {
        case Tizen::MATCH_ENDSWITH: {
            if (key.empty()) return false;
            if (key.size() > value.size()) return false;
            std::string lvalue = convertToLowerCase(value);
            std::string lkey = convertToLowerCase(key);
            res = lvalue.substr(lvalue.size() - lkey.size(), lkey.size()) == lkey;
        } break;

        case Tizen::MATCH_EXACTLY: {
            res = key == value;
        } break;

        case Tizen::MATCH_STARTSWITH: {
            if (key.empty()) return false;
            if (key.size() > value.size()) return false;
            std::string lvalue = convertToLowerCase(value);
            std::string lkey = convertToLowerCase(key);
            res = lvalue.substr(0, lkey.size()) == lkey;
        } break;

        case Tizen::MATCH_CONTAINS: {
            if (key.empty()) return false;
            if (key.size() > value.size()) return false;
            std::string lvalue = convertToLowerCase(value);
            std::string lkey = convertToLowerCase(key);
            res = lvalue.find(lkey) != std::string::npos;
        } break;

        default: {
            LOGE("Unknown match flag");
            res =  false;
        } break;
    }

    LOGD("key:%s value:%s matchFlag:%d RESULT:%d", key.c_str(), value.c_str(), flag, res);
    return res;
}

bool BaseFilterVisitor::matchStringVector(const std::string& key,
        const std::vector<std::string>& values,
        Tizen::MatchFlag flag)
{
    for(auto it = values.begin(); it != values.end(); ++it) {
        if(matchString(key,*it,flag)) {
            return true;
        }
    }
    return false;
}

bool BaseFilterVisitor:: matchTimeStampRange(const time_t& timeStamp,
        Tizen::AnyPtr& initialValue, Tizen::AnyPtr& endValue)
{
    time_t from_time;
    time_t to_time;

    if (initialValue && !initialValue->isNullOrUndefined() &&
            initialValue->isType(DeviceAPI::Tizen::PrimitiveType_Time)) {
        struct tm ftime = *initialValue->getDateTm();
        from_time = mktime(&ftime);
    } else {
        LOGE("initialValue is not Time!");
        throw Common::InvalidValuesException("initialValue is not Time!");
    }

    if (endValue && !endValue->isNullOrUndefined() &&
            endValue->isType(DeviceAPI::Tizen::PrimitiveType_Time) ) {
        struct tm ttime = *endValue->getDateTm();
        to_time = mktime(&ttime);
    } else {
        LOGE("endValue is not Time!");
        throw Common::InvalidValuesException("endValue is not Time!");
    }

    bool isInRange = isBetweenTimeRange(timeStamp, from_time, to_time);

    LOGD("%d is%s in time range <%d, %d>", timeStamp, (isInRange ? "" : " NOT"),
            from_time, to_time);

    return isInRange;
}

bool BaseFilterVisitor::isMatched()
{
    if(m_result_list.empty()) {
        LOGW("m_result_list is empty!");
        return false;
    }
    return m_result_list.back();
}

std::string BaseFilterVisitor::formatResultListChangeLog(const size_t old_res_count)
{
    const size_t new_res_count = m_result_list.size();
    const int delta = new_res_count - old_res_count;
    std::ostringstream oss;

    if(delta > 0) {
        oss << "m_result_list added: "<< delta << " {" << std::endl;
        std::vector<bool>::reverse_iterator rit = m_result_list.rbegin();
        for(size_t i = 0; i < delta; ++i, ++rit) {
            const bool result = *rit;
            oss << "    [LAST -" << (delta - (i+1)) <<"] added: "<< result;
        }
        oss << "}" << std::endl;
    } else if(delta < 0) {
        oss <<"m_result_list removed: "<< delta;
    }

    return oss.str();
}

} //namespace Messaging
} //namespace DeviceAPI
