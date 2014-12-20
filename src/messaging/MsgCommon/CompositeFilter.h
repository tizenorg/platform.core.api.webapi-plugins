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
 * @file        CompositeFilter.h
 */

#ifndef __TIZEN_TIZEN_COMPOSITE_FILTER_H__
#define __TIZEN_TIZEN_COMPOSITE_FILTER_H__

#include "AbstractFilter.h"
//#include <JSArray.h>

namespace extension {
namespace tizen {

enum CompositeFilterType {
    UNION,
    INTERSECTION
};

class CompositeFilter;
typedef std::shared_ptr<CompositeFilter> CompositeFilterPtr;

class CompositeFilter: public AbstractFilter {
public:
    CompositeFilter(CompositeFilterType type);
    virtual ~CompositeFilter();

    CompositeFilterType getType() const;
    void setType(CompositeFilterType type);
    const AbstractFilterPtrVector& getFilters() const;
    void setFilters(const AbstractFilterPtrVector &filter);
//    JSFilterArray getJSFilters(JSContextRef ctx);
//    JSContextRef getContext() const;

    virtual bool isMatching(const FilterableObject* const filtered_object) const;
private:
    CompositeFilterType m_type;
    AbstractFilterPtrVector m_filters;
//    JSContextRef m_context;
//    std::shared_ptr<JSFilterArray> m_js_filters;

};

} //Tizen
} //DeviceAPI

#endif // __TIZEN_TIZEN_COMPOSITE_FILTER_H__
