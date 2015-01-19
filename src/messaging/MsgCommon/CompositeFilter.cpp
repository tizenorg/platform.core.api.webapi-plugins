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
 * @file        CompositeFilter.cpp
 */

#include "CompositeFilter.h"
#include "common/platform_exception.h"
//#include <GlobalContextManager.h>
#include "common/logger.h"

namespace extension {
namespace tizen {

CompositeFilter::CompositeFilter(CompositeFilterType type):
        AbstractFilter(COMPOSITE_FILTER),
        m_type(type)
//        m_context(NULL)
{
}

CompositeFilter::~CompositeFilter()
{
}

CompositeFilterType CompositeFilter::getType() const
{
    return m_type;
}

void CompositeFilter::setType(CompositeFilterType type)
{
    m_type = type;
}

//JSContextRef CompositeFilter::getContext() const
//{
//    return m_context;
//}

const AbstractFilterPtrVector& CompositeFilter::getFilters() const
{
    return m_filters;
}

//void CompositeFilter::setFilters(const AbstractFilterPtrVector &filters)
//{
//    if (Common::GlobalContextManager::getInstance()->isAliveGlobalContext(
//            m_context) && m_js_filters) {
//        *m_js_filters = filters;
//    }
//    m_filters = filters;
//}
//
//JSFilterArray CompositeFilter::getJSFilters(JSContextRef global_ctx)
//{
//    if (!m_context && !global_ctx) {
//        LoggerE("Context is not set");
//        throw Common::UnknownException("Context is not set");
//    }
//    else if (!m_context && global_ctx) {
//        m_context = global_ctx;
//    }
//
//    if (!Common::GlobalContextManager::getInstance()->isAliveGlobalContext(
//            m_context)) {
//        LoggerE("Context is not alive");
//        throw Common::UnknownException("Context is not alive");
//    }
//    else if (!m_js_filters) {
//        m_js_filters = std::shared_ptr<JSFilterArray>(new JSFilterArray(
//                m_context));
//        *m_js_filters = m_filters;
//        m_filters.clear();
//    }
//    return *m_js_filters;
//}


bool CompositeFilter::isMatching(const FilterableObject* const filtered_object) const
{
    if(!filtered_object) {
        LoggerE("Invalid object: NULL!");
        throw common::InvalidValuesException("Invalid object");
    }

    bool composite_result = false;

    const AbstractFilterPtrVector src_filters = getFilters();
    if(src_filters.empty()) {
        //No filters present -> object match composite filter
        composite_result = true;
    }
    else {
        AbstractFilterPtrVector::const_iterator it = src_filters.begin();
        AbstractFilterPtrVector::const_iterator end_it = src_filters.end();
        for(;it != end_it; it++) {

            const bool last_result = (*it)->isMatching(filtered_object);
            if(INTERSECTION == m_type) {
                composite_result = last_result;
                if(!last_result) {  //First false result causes whole composite
                    break;                  //filter to be false -> object does not match
                }
            } else if(UNION == m_type && last_result) { //First true result causes
                composite_result = true;                    //composite filter to be
                break;                                      //true -> object match
            }
        }
    }

    return composite_result;
}


} // Tizen
} // DeviceAPI
